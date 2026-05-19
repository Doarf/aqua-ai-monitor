const express  = require('express');
const http     = require('http');
const multer   = require('multer');
const FormData = require('form-data');
const fetch    = require('node-fetch');
const fs       = require('fs');
const path     = require('path');
const app      = express();
const server   = http.createServer(app);

// ── CSV logging ──────────────────────────────────────────────────────────────
const CSV_PATH = path.join(__dirname, 'data_aqua.csv');
const CSV_HEADER = 'timestamp,ph,ntu,temperature,humidity,waterTemp\n';

// Crée le fichier avec l'en-tête s'il n'existe pas encore
if (!fs.existsSync(CSV_PATH)) {
  fs.writeFileSync(CSV_PATH, CSV_HEADER);
  console.log(`[CSV] Fichier créé : ${CSV_PATH}`);
}

function appendCSV(data) {
  if (data.ph == null || data.ntu == null) return;
  const line = `${data.updatedAt},${data.ph},${data.ntu},${data.temperature ?? ''},${data.humidity ?? ''},${data.waterTemp ?? ''}\n`;
  fs.appendFile(CSV_PATH, line, err => {
    if (err) console.error('[CSV] Erreur écriture :', err.message);
  });
}

const PORT    = 3000;
const AI_URL  = 'http://localhost:5001';

let clients = [];
let lastSensorData = {
  temperature: null,
  humidity:    null,
  ph:          null,
  ntu:         null,
  waterTemp:   null,
  updatedAt:   null
};
let lastAiResult = {
  anomalie:  false,
  score:     null,
  raison:    'Modèle non entraîné',
  trained:   false,
  updatedAt: null
};

app.use(express.json());

// ── Fichiers statiques ───────────────────────────────────────────────────────
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/public/index.html');
});

// ── Réception données capteurs (depuis ESP32) ────────────────────────────────
app.post('/data', async (req, res) => {
  lastSensorData = { ...req.body, updatedAt: new Date().toISOString() };
  console.log(`[DATA] T:${lastSensorData.temperature} H:${lastSensorData.humidity} pH:${lastSensorData.ph} NTU:${lastSensorData.ntu} Teau:${lastSensorData.waterTemp}`);
  appendCSV(lastSensorData);
  res.sendStatus(200);

  // Appel IA si pH et NTU disponibles
  if (lastSensorData.ph !== null && lastSensorData.ntu !== null) {
    try {
      const aiRes = await fetch(`${AI_URL}/predict`, {
        method:  'POST',
        headers: { 'Content-Type': 'application/json' },
        body:    JSON.stringify({ ph: lastSensorData.ph, ntu: lastSensorData.ntu }),
        timeout: 3000
      });
      const aiData = await aiRes.json();
      lastAiResult = { ...aiData, updatedAt: new Date().toISOString() };

      if (lastAiResult.anomalie) {
        console.log(`[AI] ⚠ ANOMALIE — score:${lastAiResult.score} — ${lastAiResult.raison}`);
      }
    } catch (e) {
      console.warn(`[AI] Service injoignable : ${e.message}`);
    }
  }
});

// ── Téléchargement du CSV enregistré ────────────────────────────────────────
app.get('/csv', (req, res) => {
  if (!fs.existsSync(CSV_PATH))
    return res.status(404).json({ error: 'Aucun CSV disponible' });
  res.download(CSV_PATH, 'data_aqua.csv');
});

// ── Lecture données capteurs (depuis dashboard) ──────────────────────────────
app.get('/data', (req, res) => {
  res.json(lastSensorData);
});

// ── Lecture résultat IA ──────────────────────────────────────────────────────
app.get('/ai', (req, res) => {
  res.json(lastAiResult);
});

// ── Upload CSV → entraînement IA ─────────────────────────────────────────────
const upload = multer({ storage: multer.memoryStorage(), limits: { fileSize: 10 * 1024 * 1024 } });

app.post('/train', upload.single('file'), async (req, res) => {
  if (!req.file) {
    return res.status(400).json({ error: 'Aucun fichier reçu' });
  }

  console.log(`[TRAIN] CSV reçu : ${req.file.originalname} (${req.file.size} bytes)`);

  try {
    const form = new FormData();
    form.append('file', req.file.buffer, {
      filename:    req.file.originalname,
      contentType: 'text/csv'
    });

    const aiRes = await fetch(`${AI_URL}/train`, {
      method:  'POST',
      body:    form,
      headers: form.getHeaders(),
      timeout: 30000
    });

    const text = await aiRes.text();
    console.log(`[TRAIN] Réponse Flask brute : ${text}`);
    let result;
    try {
      result = JSON.parse(text);
    } catch(parseErr) {
      console.error(`[TRAIN] Réponse non-JSON : ${text}`);
      return res.status(500).json({ error: `Réponse invalide du service IA : ${text.slice(0, 200)}` });
    }
    if (aiRes.ok) {
      console.log(`[TRAIN] Modèle entraîné sur ${result.n_samples} points ✓`);
    }
    res.status(aiRes.status).json(result);
  } catch (e) {
    console.error(`[TRAIN] Erreur : ${e.message}`);
    res.status(500).json({ error: `Service IA injoignable : ${e.message}` });
  }
});

// ── Statut du modèle IA ──────────────────────────────────────────────────────
app.get('/ai/status', async (req, res) => {
  try {
    const r = await fetch(`${AI_URL}/status`, { timeout: 2000 });
    res.json(await r.json());
  } catch (e) {
    res.status(503).json({ error: 'Service IA injoignable' });
  }
});

// ── Proxy flux vidéo MJPEG ───────────────────────────────────────────────────
app.post('/stream', (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setTimeout(0);
  req.setTimeout(0);

  let buffer = Buffer.alloc(0);

  req.on('data', chunk => {
    buffer = Buffer.concat([buffer, chunk]);
    let start = -1;
    for (let i = 0; i < buffer.length - 1; i++) {
      if (buffer[i] === 0xFF && buffer[i + 1] === 0xD8) start = i;
      if (start !== -1 && buffer[i] === 0xFF && buffer[i + 1] === 0xD9) {
        const frame = buffer.slice(start, i + 2);
        buffer = buffer.slice(i + 2);
        clients.forEach(client => {
          try {
            client.write('--frame\r\nContent-Type: image/jpeg\r\n\r\n');
            client.write(frame);
            client.write('\r\n');
          } catch (e) {}
        });
        start = -1;
        i = 0;
      }
    }
  });

  req.on('end',   () => res.sendStatus(200));
  req.on('error', () => res.sendStatus(500));
});

app.get('/video', (req, res) => {
  res.setHeader('Content-Type', 'multipart/x-mixed-replace; boundary=frame');
  res.setHeader('Cache-Control', 'no-cache');
  res.setHeader('Connection', 'keep-alive');
  res.setTimeout(0);
  clients.push(res);
  console.log(`[+] Client vidéo connecté (${clients.length})`);
  req.on('close', () => {
    clients = clients.filter(c => c !== res);
    console.log(`[-] Client vidéo déconnecté (${clients.length})`);
  });
});

// ── Démarrage ────────────────────────────────────────────────────────────────
server.listen(PORT, '0.0.0.0', () => {
  console.log(`\n=== SPI Aquaculture Server ===`);
  console.log(`Site web   : http://localhost:${PORT}`);
  console.log(`Service IA : ${AI_URL}`);
});
"""
ai_service.py — Microservice Flask de détection d'anomalies aquaculture
Utilise Isolation Forest sur pH et turbidité (NTU)

Endpoints :
  POST /train   — reçoit un CSV multipart (colonnes ph, ntu), entraîne le modèle
  POST /predict — reçoit {"ph": float, "ntu": float}, retourne le résultat IA
  GET  /status  — état du modèle (entraîné ou non, nb points, date)
"""

from flask import Flask, request, jsonify
from flask_cors import CORS
import pandas as pd
import numpy as np
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
import joblib
import io
import os
from datetime import datetime

app = Flask(__name__)
CORS(app)

# ── État global du modèle ────────────────────────────────────────────────────
MODEL_PATH   = "model_aqua.pkl"
SCALER_PATH  = "scaler_aqua.pkl"

model_state = {
    "trained":    False,
    "n_samples":  0,
    "trained_at": None,
    "ph_mean":    None,
    "ph_std":     None,
    "ntu_mean":   None,
    "ntu_std":    None,
}

model  = None
scaler = None

# Seuils métier complémentaires (garde-fous explicables)
PH_MIN,  PH_MAX  = 6.0, 9.0      # plage normale aquaculture
NTU_MIN, NTU_MAX = 0.0, 200.0    # eau claire → légèrement trouble


def _load_model_if_exists():
    """Charge le modèle depuis le disque si disponible au démarrage."""
    global model, scaler
    if os.path.exists(MODEL_PATH) and os.path.exists(SCALER_PATH):
        model  = joblib.load(MODEL_PATH)
        scaler = joblib.load(SCALER_PATH)
        model_state["trained"] = True
        print("[AI] Modèle chargé depuis le disque ✓")


# ── /train ───────────────────────────────────────────────────────────────────
@app.route("/train", methods=["POST"])
def train():
    global model, scaler

    # Récupération du fichier CSV
    if "file" not in request.files:
        return jsonify({"error": "Aucun fichier reçu (champ 'file' manquant)"}), 400

    file = request.files["file"]
    try:
        df = pd.read_csv(io.StringIO(file.read().decode("utf-8")))
    except Exception as e:
        return jsonify({"error": f"Lecture CSV impossible : {str(e)}"}), 400

    # Recherche flexible des colonnes pH et NTU
    col_map = {}
    for col in df.columns:
        cl = col.strip().lower()
        if cl in ("ph", "ph_value", "ph_val"):
            col_map["ph"] = col
        if cl in ("ntu", "turbidity", "turbidite", "turb"):
            col_map["ntu"] = col

    if "ph" not in col_map or "ntu" not in col_map:
        return jsonify({
            "error": "Colonnes introuvables. Attendu : ph (ou ph_value) + ntu (ou turbidity)",
            "colonnes_reçues": list(df.columns)
        }), 400

    df = df[[col_map["ph"], col_map["ntu"]]].rename(columns={
        col_map["ph"]:  "ph",
        col_map["ntu"]: "ntu"
    })
    df = df.dropna()
    df["ph"]  = pd.to_numeric(df["ph"],  errors="coerce")
    df["ntu"] = pd.to_numeric(df["ntu"], errors="coerce")
    df = df.dropna()

    if len(df) < 20:
        return jsonify({"error": f"Trop peu de données valides ({len(df)} lignes, minimum 20)"}), 400

    X = df[["ph", "ntu"]].values

    # Normalisation
    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(X)

    # Entraînement Isolation Forest
    # contamination = proportion estimée d'anomalies dans les données d'entraînement
    model = IsolationForest(
        n_estimators=200,
        contamination=0.05,   # 5 % d'anomalies supposées
        random_state=42,
        max_samples="auto"
    )
    model.fit(X_scaled)

    # Sauvegarde
    joblib.dump(model,  MODEL_PATH)
    joblib.dump(scaler, SCALER_PATH)

    # Mise à jour état
    model_state.update({
        "trained":    True,
        "n_samples":  len(df),
        "trained_at": datetime.now().isoformat(),
        "ph_mean":    round(float(df["ph"].mean()),  3),
        "ph_std":     round(float(df["ph"].std()),   3),
        "ntu_mean":   round(float(df["ntu"].mean()), 3),
        "ntu_std":    round(float(df["ntu"].std()),  3),
    })

    print(f"[AI] Modèle entraîné sur {len(df)} points ✓")
    return jsonify({
        "message": "Modèle entraîné avec succès",
        "n_samples": len(df),
        "ph_mean":   model_state["ph_mean"],
        "ph_std":    model_state["ph_std"],
        "ntu_mean":  model_state["ntu_mean"],
        "ntu_std":   model_state["ntu_std"],
    })


# ── /predict ─────────────────────────────────────────────────────────────────
@app.route("/predict", methods=["POST"])
def predict():
    if not model_state["trained"]:
        return jsonify({
            "anomalie": False,
            "score":    None,
            "raison":   "Modèle non entraîné — uploadez un CSV d'abord",
            "trained":  False
        })

    data = request.get_json(force=True)
    try:
        ph  = float(data["ph"])
        ntu = float(data["ntu"])
    except (KeyError, TypeError, ValueError):
        return jsonify({"error": "Corps JSON attendu : {ph: float, ntu: float}"}), 400

    # ── Vérification seuils métier ────────────────────────────────────────────
    raisons = []
    if not (PH_MIN <= ph <= PH_MAX):
        raisons.append(f"pH hors plage ({PH_MIN}–{PH_MAX}) : {ph:.2f}")
    if not (NTU_MIN <= ntu <= NTU_MAX):
        raisons.append(f"Turbidité hors plage ({NTU_MIN}–{NTU_MAX} NTU) : {ntu:.1f}")

    # ── Inférence Isolation Forest ────────────────────────────────────────────
    X = scaler.transform([[ph, ntu]])
    pred  = model.predict(X)[0]          # -1 = anomalie, 1 = normal
    score = model.score_samples(X)[0]    # plus négatif = plus anormal

    # Normalisation du score vers [0, 1] (0 = normal, 1 = très anormal)
    # Les scores IF sont typiquement dans [-0.7, 0.1]
    score_normalise = round(float(np.clip((-score - 0.0) / 0.7, 0.0, 1.0)), 3)

    is_anomalie = bool((pred == -1) or (len(raisons) > 0))

    if pred == -1 and not raisons:
        raisons.append("Pattern inhabituel détecté par le modèle IA")

    return jsonify({
        "anomalie":  bool(is_anomalie),
        "score":     float(score_normalise),
        "raison":    " | ".join(raisons) if raisons else "Paramètres normaux",
        "detail": {
            "ph":            float(ph),
            "ntu":           float(ntu),
            "if_prediction": int(pred),
            "if_raw_score":  float(round(float(score), 4)),
        },
        "trained": True
    })


# ── /status ──────────────────────────────────────────────────────────────────
@app.route("/status", methods=["GET"])
def status():
    return jsonify(model_state)


# ── Démarrage ─────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    _load_model_if_exists()
    print("[AI] Microservice démarré sur http://localhost:5001")
    app.run(host="0.0.0.0", port=5001, debug=False)
# Elements/Replays/

Rejeux publiés, sélectionnables depuis l'entrée de menu « Regarder l'IA jouer »
(`LOT-ANNEXE-18`, `EX-IA-019`).

- Un rejeu est un fichier JSON au format de rejeu v2 (`aisolver::ReplayFile`,
  `LOT-ANNEXE-07`/`LOT-ANNEXE-17`) : séquence de `core::PlayerInput` par pas fixe, empreinte du
  niveau d'origine, métadonnées d'entraînement (algorithme, graine, récompense finale, durée).
- **Distinct de `/TrainingRuns/`** (non versionné, réservé aux runs d'entraînement bruts produits
  par les générations 2/3 du programme Lot-Annexe) : un rejeu jugé démonstratif doit être copié
  manuellement de l'un vers l'autre pour devenir sélectionnable dans le menu. Aucun mécanisme
  automatique de « publication » — décision de cadrage délibérée (`LOT-ANNEXE-18`, epic.md).
- Chargé et validé (`aisolver::validateReplay`, `LOT-ANNEXE-17`) avant tout usage : un rejeu dont le
  niveau référencé (sous `Elements/Levels/`) a changé depuis l'export est refusé avec un message
  explicite, jamais joué partiellement.

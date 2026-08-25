# AiSolver/Stats/

Journalisation d'un entraînement : ce qui permet de dire *après coup* si un run a convergé, stagné
ou divergé.

- `TrainingStatsRecorder` — écriture CSV **incrémentale**, partagée par tous les algorithmes
  (évolutionniste ou par gradient) : une ligne par génération ou par épisode, écrite au fil de l'eau
  plutôt qu'à la fin, pour qu'un run interrompu laisse quand même ses mesures.
- `CsvFormat` — le format exact des colonnes, réutilisé **sans changement** par le harnais de
  benchmark : un seul schéma, donc les mêmes outils de lecture partout.
- `TrainingRunPath` — construction du chemin du CSV d'un run, sous `/TrainingRuns/` (dossier généré,
  non versionné).
- `MovingAverage` — moyenne mobile sur fenêtre glissante, pour détecter un plateau.

Réf. specs : `EX-IA-010`, lot [`LOT-ANNEXE-09`](Documentation/Lot-Annexe/LOT-ANNEXE-09-journalisation-statistiques/epic.md).

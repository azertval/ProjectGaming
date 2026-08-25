# AiSolver/Replay/

Le **format de rejeu** : ce qu'un entraînement rapporte au jeu. Pas un réseau de neurones, mais une
séquence d'actions déterministe, rejouable à l'identique par le moteur — c'est ce qui permet au jeu
de n'embarquer aucune inférence.

- `ReplayFile` — format v2 **versionné** : la séquence d'entrées, l'empreinte du niveau d'origine et
  les métadonnées d'entraînement.
- `LevelFingerprint` — empreinte déterministe d'un fichier de niveau. Réimplémentée à l'identique en
  Python par `scripts/check_ai_replays.py`, pour que la CI puisse valider les rejeux publiés **sans
  construire le C++**.
- `ReplayValidation` — validation à la lecture, **avant** toute utilisation. Un rejeu dont le niveau
  a changé depuis l'export est **refusé au chargement**, avec un message explicite : jamais joué à
  moitié.

Réf. specs : `EX-IA-008`, `EX-IA-018` ; lots [`LOT-ANNEXE-07`](Documentation/Lot-Annexe/LOT-ANNEXE-07-espace-action-format-rejeu/epic.md) et [`LOT-ANNEXE-17`](Documentation/Lot-Annexe/LOT-ANNEXE-17-validation-format-rejeu/epic.md).

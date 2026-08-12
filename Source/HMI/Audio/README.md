# HMI/Audio/

Lecture audio (`LOT-60`). Enveloppe fine autour de Qt Multimedia (`QSoundEffect`) — aucune
dépendance dans `Core` (`EX-ARCH-012`).

- `AudioEngine` : ouverture du périphérique de sortie, préchargement, volume global, repli
  « muet » si aucun périphérique n'est disponible (`EX-REN-047`, `EX-NFR-040`).
- `SoundCatalog` : nom logique d'événement → fichier (`Source/Elements/Audio/sounds.json`),
  logique pure sans Qt ni périphérique, sur le patron de `Graphics/SkinCatalog`.
- `AudioLog.h` : macros de journalisation de la catégorie « Audio ».

Minimal au MVP : bruitages seulement (saut, interrupteur, victoire, échec…), pas de musique.
Réf. specs : `EX-REN-040`, `EX-REN-047`, `EX-REN-048`.

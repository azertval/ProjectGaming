# Elements/Themes/

Feuilles de style **Qt** (`.qss`) de l'IHM, éditables hors code et embarquées via
`../UI/resources.qrc`.

- `theme.qss` — thème sombre du menu et des options (fond sombre, accents ambre). Porté par
  `objectName` (`#MainMenu`, `#OptionsPage`) pour ne pas restyler les docks de l'éditeur, qui gardent
  le thème Qt par défaut. Chargé au démarrage par `HMI/main.cpp` (`:/resources/theme.qss`).

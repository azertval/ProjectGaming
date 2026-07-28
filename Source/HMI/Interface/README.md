# HMI/Interface/

Widgets **Qt** de l'IHM hors-jeu : la fenêtre principale et les écrans qui ne relèvent pas de
l'éditeur de niveau. Les mises en page sont décrites hors code dans `Elements/UI/*.ui` (Qt Designer)
et le thème dans `Elements/Themes/theme.qss`.

- `MainWindow` — fenêtre principale : `QStackedWidget` (menu / options / éditeur) et panneaux
  dockables (`QDockWidget` — Palette, Outils, Niveaux, Liens) autour du viewport central. Persiste
  la disposition via `QSettings` (`EX-IHM-011`) et lance la séquence de jeu (`startGame`).
- `MainMenu` — menu principal (Jouer / Éditeur / Options / Quitter), depuis `MainMenu.ui`.
- `OptionsPage` — options en onglets, depuis `OptionsPage.ui` : **V-Sync** (`EX-REN-022`), un onglet
  **Général** (sélecteur de **langue** `EX-REN-033` + bouton **« Enregistrer les journaux »**) et le
  remappage clavier/manette (onglets ajoutés en code). Résolution/FPS sont présents mais désactivés,
  l'audio est un espace réservé. Émet `languageChanged`/`saveLogsRequested` vers `MainWindow`.
- `KeybindingsWidget` / `GamepadBindingsWidget` — capture et affichage du remappage des touches et
  des boutons de manette (délèguent à `hmi::GameKeyBindings` / `EditorKeyBindings` /
  `GamepadBindings`, logique pure testée). Le remappage manette affiche aussi l'**état de connexion**
  (sondage XInput périodique tant que l'onglet est visible).

Réf. specs : [`interface-ihm.md`](../../../Documentation/Specification/interface-ihm.md) (`EX-IHM-*`),
guide [`guide-ihm-qt`](../../../Documentation/Guide/guide-ihm-qt.md).

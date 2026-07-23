# HMI/Interface/

Écrans de l'application et navigation entre eux.

- `IScreen` — interface d'un écran : `update(input, fixedDelta)` (renvoie une intention de
  transition : rester / basculer / quitter) et `render(RenderContext&)`.
- `RenderContext` — ressources de rendu partagées passées aux écrans (lot de sprites, atlas,
  police, catalogue de traduction, dimensions de la surface).
- `ScreenManager` — détient l'écran courant, applique les transitions et fabrique l'écran
  suivant via une **fabrique** injectée (découplé des écrans concrets, testable hors GPU).
- `MenuModel` / `MenuScreen` — logique (testable) et dessin du menu principal.
- `OptionsModel` / `OptionsScreen` — menu d'options (bascule V-Sync, langue, état manette).
- `GameScreen` — joue une **séquence de niveaux** (`LevelSequence`) : personnage jouable, physique,
  mécanismes, enchaînement à la sortie, redémarrage sur danger/chute.
- `LevelSequence` — liste ordonnée des niveaux joués par `GameScreen` et progression dans cette
  liste (`EX-LVL-010`, `EX-LVL-011`).
- `EditorScreen` — éditeur de niveaux intégré : peinture, mécanismes, undo/redo, essai immédiat.
- `LanguageSelector` — logique (testable) du bouton de langue (bas-droit, bascule fr/en).
- `SaveLogButton` — logique (testable) du bouton d'enregistrement des logs (à gauche du bouton de langue).
- `SessionLog` — sérialisation et écriture fichier des logs de la session (`serializeSessionLog` pur + `saveSessionLog`).

À venir : écrans de pause et de fin de niveau dédiés (`EX-REN-031`, ⚠️ non implémenté — Échap
quitte directement vers le menu, l'enchaînement de niveaux ne passe par aucun écran intermédiaire).

Réf. specs : `EX-REN-030`, `EX-REN-032`, `EX-REN-033`, `EX-NFR-010`.

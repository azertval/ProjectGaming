# HMI/Interface/

Écrans de l'application et navigation entre eux.

- `IScreen` — interface d'un écran : `update(input, fixedDelta)` (renvoie une intention de
  transition : rester / basculer / quitter) et `render(RenderContext&)`.
- `RenderContext` — ressources de rendu partagées passées aux écrans (lot de sprites, atlas,
  police, catalogue de traduction, dimensions de la surface).
- `ScreenManager` — détient l'écran courant, applique les transitions et fabrique l'écran
  suivant via une **fabrique** injectée (découplé des écrans concrets, testable hors GPU).

À venir : écrans de pause et de fin de niveau (`EX-REN-031`).

Réf. specs : `EX-REN-030`, `EX-REN-032`, `EX-NFR-010`.

# HMI/Input/

Acquisition des entrées et traduction en **actions logiques**.

- `InputState` : état clavier/souris **et manette** par frame, avec fronts **pressée / maintenue /
  relâchée** (`EX-CTRL-011`), échantillonné une fois par frame (`EX-CTRL-021`). Indépendant
  de toute fenêtre (aucun `<Windows.h>`), donc testable en isolation ; la capture Win32/XInput est
  faite par `HMI/Platform/Window` (`pollGamepad`), qui **fusionne** clavier et manette en lecture
  seule sur les mêmes `Key` (`EX-CTRL-002`).
- `PlayerInputMapper` (`toPlayerInput`) : traduit l'`InputState` en `core::PlayerInput`, la seule
  correspondance touches → intentions de jeu (`EX-CTRL-010`).
- À venir : mapping d'actions **reconfigurable** par l'utilisateur (`EX-CTRL-012`, ⚠️ souhaité —
  les actions sont déjà dissociées des touches physiques, mais fixées en dur dans le code).

Réf. specs : `EX-CTRL-001`…`EX-CTRL-021`.

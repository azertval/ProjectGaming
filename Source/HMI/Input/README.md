# HMI/Input/

Acquisition des entrées et, à terme, traduction en **actions logiques**.

- `InputState` : état clavier/souris par frame, avec fronts **pressée / maintenue /
  relâchée** (`EX-CTRL-011`), échantillonné une fois par frame (`EX-CTRL-021`). Indépendant
  de toute fenêtre (aucun `<Windows.h>`), donc testable en isolation ; la capture Win32 est
  faite par `HMI/Platform/Window`.
- À venir : manette (XInput, `EX-CTRL-002`), mapping d'actions reconfigurable dissocié des
  touches physiques (`EX-CTRL-010`, `EX-CTRL-012`).

Réf. specs : `EX-CTRL-001`…`EX-CTRL-021`.

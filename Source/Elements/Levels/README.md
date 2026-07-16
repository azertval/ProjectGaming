# Elements/Levels/

Niveaux du jeu, un fichier **JSON** par niveau (`EX-LVL-001`, `EX-LVL-003`).

- Un niveau est un objet JSON : `name`, `width`, `height`, et une liste **`tiles`** d'objets
  `{ "x", "y", "type", … }`. Les cases **vides** ne sont pas listées (absence = vide).
- Types de tuiles : `entry`, `exit`, `solid`, `danger`, `switch`, `door`.
- Coordonnées `x` = colonne, `y` = ligne, origine **haut-gauche** ; toute tuile doit rester dans
  les bornes `width × height`.
- **Mécanismes** : un `switch` porte un `id`, une `door` le référence via `opensWith` (liaison
  interrupteur↔porte). Schéma extensible à d'autres champs par tuile.
- Contraintes de validité : exactement une `entry` et une `exit`, pas de deux tuiles à la même
  position, toute `door.opensWith` doit référencer un `switch` existant.

Chargés à l'exécution par `core::LevelLoader` (copiés à côté de l'exécutable par CMake).

Réf. specs : `EX-LVL-001` (fichier externe), `EX-LVL-003` (format), `EX-LVL-004` (validation).

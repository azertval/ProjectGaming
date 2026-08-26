# AiSolver/Env/

Rendre le jeu **jouable par une machine** : simuler un niveau sans fenêtre, traduire son état en
tenseur, et noter ce que l'agent y fait.

- `HeadlessLevelEnvironment` — fait tourner un niveau **sans fenêtre ni GPU**, en répliquant pas à
  pas l'ordre de résolution de la partie réelle (plateformes mobiles, blocs poussés, mécanismes,
  physique du personnage, dangers). Un test **système** permanent compare les deux trajectoires :
  l'agent ne peut pas apprendre une physique que le joueur n'aurait pas.

**Observation** — trois encodeurs complémentaires, assemblés à plat :
- `TileWindowEncoder` — fenêtre de tuiles centrée sur le personnage, encodage catégoriel.
- `PlayerStateEncoder` — vecteur cinématique du personnage (vitesse, appuis, budgets restants).
- `MechanismStateEncoder` — état dynamique des mécanismes visibles.
- `ObservationEncoder` — assemble les trois en un unique vecteur d'entrée `[inputSize(), 1]`.

**Action** — `ActionSpace` (produit fini des directions, du saut, du maintien de saut, du dash et de
l'interaction) et `ActionDecoding` (décodage au maximum, ou par tirage à température).

**Signal d'apprentissage** :
- `GridDistanceField` — champ de distances de **plus court chemin sur la grille**, respectant les
  murs. C'est la seule mesure qui récompense un détour imposé par un mur, là où une distance à vol
  d'oiseau punirait le seul chemin praticable.
- `Reward` — récompense unique et partagée par **tous** les algorithmes : progression vers
  l'**objectif immédiat** (tant qu'une porte est verrouillée, sa clé est un but à part entière), sans
  que rien n'ait à connaître l'ordre de résolution attendu.
- `Episode` — classification explicite de fin d'épisode : gagné, mort, bloqué, budget épuisé.

Réf. specs : `EX-IA-007` à `EX-IA-009`, `EX-IA-023` ; lots [`LOT-ANNEXE-05`](Documentation/Lot-Annexe/LOT-ANNEXE-05-environnement-simulation-headless/epic.md) à [`LOT-ANNEXE-08`](Documentation/Lot-Annexe/LOT-ANNEXE-08-fonction-recompense-episodes/epic.md).

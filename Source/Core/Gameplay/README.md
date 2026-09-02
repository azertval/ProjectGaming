# Core/Gameplay/

Règles du jeu et logique de haut niveau.

Implémenté :
- `MechanismController` — fait vivre les mécanismes **interrupteur/plaque de pression ↔ porte**
  d'un niveau (`EX-GP-020`, `EX-GP-025`) : état des portes (ouverte/fermée), grille de collision
  dérivée (une porte fermée est solide, ouverte elle ne l'est plus), résolu chaque pas fixe.
- `SinkingBlockController` — **blocs descendants** (`EX-GP-027`, `LOT-74`) : armés par un contact
  quelconque du personnage, ils descendent ensuite à vitesse constante, s'arrêtent définitivement
  contre la matière pleine et sont retirés s'ils franchissent le bord bas. Position **continue**,
  sur le modèle de `PlatformController` — et surtout, ce contrôleur n'expose que des
  `core::PlatformSample` : c'est l'appelant qui les concatène à ceux des plateformes, ce qui donne
  au bloc portage, collision continue et interpolation **sans aucun code de collision dédié**.
- `VolatileBlockController` — blocs **fragile** (`EX-GP-028`) et **éphémère** (`EX-GP-029`),
  `LOT-74`. Aucun des deux ne bouge : tous deux se contentent de **quitter** la grille de collision,
  sur le patron de la copie mutable du `TileMap` déjà tenue par `MechanismController` pour ses
  portes — la carte du `Level` reste immuable. Le premier est brisé par un ground pound
  (`EX-GP-058`) venu du dessus, et par ce geste seul ; le second disparaît un délai fixe après que
  le personnage a cessé d'y **reposer** (un front de départ, jamais un simple contact). Un seul
  contrôleur pour deux règles de retrait, afin de n'avoir qu'un recouvrement et qu'un ordre de
  composition.

Conditions de fin de niveau (succès à la sortie, échec sur danger/chute, redémarrage) sont
implémentées mais vivent dans `hmi::GameSession` (orchestration), pas dans ce dossier. Pas de
machine à états `Pause`/`LevelComplete` dédiée (cf. `Documentation/Specification/gameplay.md`,
`EX-GP-040`, ⚠️ partiellement implémenté).

Réf. specs : `EX-GP-020`…`EX-GP-041`, `EX-ARCH-090`.

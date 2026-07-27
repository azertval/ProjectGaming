# Core/Gameplay/

Règles du jeu et logique de haut niveau.

Implémenté :
- `MechanismController` — fait vivre les mécanismes **interrupteur/plaque de pression ↔ porte**
  d'un niveau (`EX-GP-020`, `EX-GP-025`) : état des portes (ouverte/fermée), grille de collision
  dérivée (une porte fermée est solide, ouverte elle ne l'est plus), résolu chaque pas fixe.

Conditions de fin de niveau (succès à la sortie, échec sur danger/chute, redémarrage) sont
implémentées mais vivent dans `hmi::GameSession` (orchestration), pas dans ce dossier. Pas de
machine à états `Pause`/`NiveauTermine` dédiée (cf. `Documentation/Specification/gameplay.md`,
`EX-GP-040`, ⚠️ partiellement implémenté).

Réf. specs : `EX-GP-020`…`EX-GP-041`, `EX-ARCH-090`.

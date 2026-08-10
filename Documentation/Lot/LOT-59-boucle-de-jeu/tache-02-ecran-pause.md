# TACHE-02 — Écran de pause et suspension du pas fixe {#lot-59-tache-02-ecran-pause}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/HMI/Interface`, `Source/HMI/Game` ·
**Statut :** non commencé

## Contexte
Échap, en cours de partie, quitte **directement** vers le menu principal. Une pression malheureuse
sur un tableau difficile perd la partie en cours, sans confirmation ni retour possible. C'est le
comportement décrit — et déploré — par `EX-REN-031`.

Le point délicat n'est pas l'écran : c'est ce que devient la simulation pendant qu'il est affiché.

## Travail à réaliser
- **Écran de pause** avec quatre entrées : *Reprendre*, *Recommencer le niveau*, *Options*,
  *Quitter vers le menu*. Le dernier demande confirmation, puisqu'il perd la progression du tableau
  en cours.
- **Échap ouvre la pause** ; Échap depuis la pause reprend. La manette suit (bouton Start / B),
  comme le reste de l'IHM.
- **Suspension réelle du pas fixe** : tant que l'écran de pause est affiché, `hmi::GameViewport`
  cesse d'**alimenter** l'accumulateur de `core::FixedTimestep` — il ne lui passe pas un `dt` nul,
  il ne l'appelle pas. À la reprise, l'horloge de référence (`_previousFrame`) est **réarmée** sur
  l'instant courant, faute de quoi le temps passé en pause serait rattrapé d'un coup au retour.
- **Le rendu continue** : la scène reste dessinée derrière l'écran de pause (l'interpolation
  d'affichage se fige naturellement, la position simulée n'avançant plus).
- *Recommencer le niveau* passe par le **même** chemin que le redémarrage après échec
  (`EX-GP-032`), déjà implémenté dans `hmi::GameSession` — pas un second chemin de rechargement.

## Fichiers impactés
- `Source/HMI/Interface/PauseScreen.{h,cpp}` (nouveau) et sa mise en page dans
  `Source/Elements/UI/PauseScreen.ui`.
- `Source/HMI/Game/GameViewport.{h,cpp}` — suspension de l'alimentation du pas fixe, réarmement de
  l'horloge à la reprise.
- `Source/HMI/Interface/MainWindow.{h,cpp}` — câblage sur la table de la `TACHE-01`.
- `Source/Elements/Localization/{fr,en}.lang` — libellés.
- `Source/Test/Unit/Core/Time/test_fixed_timestep.cpp` — suspension et réarmement.

## Tests (obligatoires)
- **Aucun pas consommé en pause** : sur une séquence où l'alimentation est suspendue puis reprise,
  le nombre total de pas est celui du temps **hors** pause, à la tolérance d'un pas près.
- **Pas de rattrapage à la reprise** : une pause longue ne produit pas de rafale de pas au retour
  (c'est le piège exact de l'accumulateur non réarmé).
- Reprendre restitue l'état exact : position, vitesse, budgets de sauts/dashs, phase des dangers
  temporisés (`EX-GP-053`) inchangés entre l'entrée et la sortie de pause.
- *Recommencer* remet le niveau dans son état initial, identiquement au redémarrage après échec.
- Les libellés de l'écran existent dans les **deux** catalogues de traduction.

## Points d'attention
- **Ne pas multiplier `dt` par zéro.** La boucle consommerait toujours des pas, et tout ce qui se
  compte en pas — animations, dangers temporisés, budgets — dériverait pendant la pause.
- **Réarmer `_previousFrame`** est la moitié oubliée du travail : sans cela, `elapsedSeconds` au
  premier retour vaut la durée de la pause entière, et `FixedTimestep::advance` rend des dizaines de
  pas d'un coup — le personnage traverse le niveau.
- La pause ne doit pas être atteignable depuis l'essai de l'éditeur sans que le retour soit défini :
  trancher explicitement dans la table de la `TACHE-01`.
- La manette est *pollée* : vérifier qu'un appui maintenu au moment de la pause ne fait pas
  osciller entrée/sortie.

## Définition de fait (DoD)
- Échap ouvre une pause qui suspend réellement la simulation et la restitue à l'identique ; aucun
  rattrapage au retour ; *Recommencer* réutilise le chemin de redémarrage existant ; navigable
  clavier / souris / manette, traduit ; `/W4 /WX` propre.

## Exigences
`EX-IHM-004` (écran de pause) ; lève `EX-REN-031` pour sa partie pause ; réutilise `EX-GP-032`
(redémarrage), `EX-GP-041` (transitions), `EX-REN-021` (pas fixe), `EX-NFR-002` (déterminisme),
`EX-REN-033` (traduction), `EX-CTRL-012` (remappage).

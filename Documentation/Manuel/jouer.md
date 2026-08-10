# Jouer {#manuel-jouer}

Ce guide s'adresse aux **joueurs** : comment naviguer le menu, contrôler le personnage et
comprendre les mécaniques rencontrées dans les niveaux.

## Le menu principal

Quatre entrées, navigables aux flèches **↑**/**↓** (ou à la souris) et validées par **Entrée**
(ou clic) : **Jouer**, **Mode Édition** (voir [créer un niveau](@ref manuel-partager-niveau)),
**Options**, **Quitter**. Un bouton **drapeau** en bas à droite bascule la langue de
l'interface (français/anglais) à tout moment.

## Contrôles en jeu

| Action | Clavier | Manette |
|--------|---------|---------|
| Se déplacer à gauche / droite | **←**/**Q** · **→**/**D** | Stick gauche / D-pad |
| Sauter (hauteur variable selon la durée d'appui) | **Espace** ou **W** | **A** |
| Dash (8 directions, selon les touches directionnelles maintenues) | **Maj** | Épaule droite (**RB**) |
| Viser le dash vers le haut / le bas | **↑** / **↓** | Stick gauche / D-pad |
| Voir le niveau tel qu'il est construit | **F8** | — |
| Quitter vers le menu | **Échap** | **B** ou **Start** |

Une manette **XInput** peut être branchée ou débranchée à tout moment ; elle **complète** le
clavier plutôt que de le remplacer (les deux fonctionnent simultanément).

**F8** bascule entre deux façons de voir le niveau. L'affichage normal montre le niveau **habillé** —
les cases physiques (sol, pentes, blocs) y projettent une ombre légère sur ce qu'il y a derrière
elles, pour aider à distinguer d'un coup d'œil ce qui porte de ce qui n'est que décor ; l'autre le
montre **tel qu'il est réellement construit**, chaque case dans une couleur unie selon son rôle —
pratique pour comprendre exactement où commence un sol, une pente ou un piège quand un passage
résiste. La bascule ne change rien au jeu lui-même : ni la difficulté, ni la position du personnage,
ni la progression. Le choix est conservé pour les fois suivantes.

## Objectif d'un niveau

Rejoindre la **sortie** termine le niveau et enchaîne automatiquement sur le suivant ; après le
dernier niveau de la séquence, retour au menu. Toucher un **danger** ou tomber hors du niveau
**redémarre** le niveau courant à son entrée — aucune pénalité au-delà de recommencer.

Certains niveaux ajoutent des **mécanismes** à résoudre :
- un **interrupteur** ouvre durablement la **porte** à laquelle il est relié (bascule : reste
  ouverte une fois activée) ;
- une **plaque de pression** ouvre la porte lui étant reliée **tant qu'un poids** (le personnage)
  y repose, et la referme dès qu'il en part ;
- un **bloc** peut être poussé horizontalement en marchant contre lui (s'il y a de la place pour
  qu'il avance), et tombe s'il n'est plus soutenu par le dessous — utile pour franchir un
  obstacle ou atteindre un endroit hors de portée du saut seul.

## Le menu d'options

Accessible depuis le menu principal : bascule le **V-Sync**, la **langue**, affiche l'état de
connexion de la **manette**, et propose un retour au menu. Navigable au clavier, à la souris ou
à la manette, comme le reste de l'interface.

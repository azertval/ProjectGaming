# Jouer {#manuel-jouer}

Ce guide s'adresse aux **joueurs** : comment naviguer le menu, contrôler le personnage et
comprendre les mécaniques rencontrées dans les niveaux.

## Le menu principal

Sept entrées, navigables aux flèches **↑**/**↓** (ou à la souris) et validées par **Entrée**
(ou clic) : **Continuer** (reprend au tableau atteint ; grisée tant qu'aucune partie n'a été
commencée), **Nouvelle partie** (recommence au premier tableau — demande confirmation si une
progression existe, puisqu'elle sera effacée), **Choisir un niveau** (voir plus bas), **Mode
Édition** (voir [créer un niveau](@ref manuel-partager-niveau)), **Options**, **Crédits**
(développement et bruitages), **Quitter**. Un bouton **drapeau** en bas à droite bascule la langue
de l'interface (français/anglais) à tout moment.

## Contrôles en jeu

| Action | Clavier | Manette |
|--------|---------|---------|
| Se déplacer à gauche / droite | **←**/**Q** · **→**/**D** | Stick gauche / D-pad |
| Sauter (hauteur variable selon la durée d'appui) | **Espace** ou **W** | **A** |
| Dash (8 directions, selon les touches directionnelles maintenues) | **Maj** | Épaule droite (**RB**) |
| Viser le dash vers le haut / le bas | **↑** / **↓** | Stick gauche / D-pad |
| Voir le niveau tel qu'il est construit | **F8** | — |
| Afficher le compteur de diagnostic (cadence, primitives) | **F9** | — |
| Mettre en pause | **Échap** | **B** ou **Start** |

Une manette **XInput** peut être branchée ou débranchée à tout moment ; elle **complète** le
clavier plutôt que de le remplacer (les deux fonctionnent simultanément).

**F8** bascule entre deux façons de voir le niveau. L'affichage normal montre le niveau **habillé** —
les cases physiques (sol, pentes, blocs) y projettent une ombre légère sur ce qu'il y a derrière
elles, pour aider à distinguer d'un coup d'œil ce qui porte de ce qui n'est que décor ; l'autre le
montre **tel qu'il est réellement construit**, chaque case dans une couleur unie selon son rôle —
pratique pour comprendre exactement où commence un sol, une pente ou un piège quand un passage
résiste. La bascule ne change rien au jeu lui-même : ni la difficulté, ni la position du personnage,
ni la progression. Le choix est conservé pour les fois suivantes.

**F9** affiche, en haut à droite de l'écran, un petit compteur technique : la cadence de rendu (en
images par seconde), le nombre de primitives dessinées et le nombre de passes de dessin. Il ne sert
qu'à vérifier que le jeu tourne bien à la vitesse attendue ; désactivé par défaut, il n'a aucun
effet sur la partie et n'est pas conservé d'une session à l'autre.

## Objectif d'un niveau

Rejoindre la **sortie** termine le tableau et affiche un **écran de fin de niveau** :
**Continuer** charge le tableau suivant, **Rejouer** relance le même. Après le dernier tableau de
la séquence, un écran de **fin de séquence** propose de retourner au menu. Toucher un **danger**
ou tomber hors du niveau **redémarre** le niveau courant à son entrée — aucune pénalité au-delà
de recommencer.

## Pause

**Échap** (ou **B**/**Start** à la manette) en cours de partie ouvre un écran de **pause** : la
partie est **suspendue exactement telle quelle** (position, vitesse, budgets de sauts/dashs,
dangers temporisés) — rien n'avance derrière l'écran. Quatre choix : **Reprendre**, **Recommencer
le niveau** (repart de l'entrée, comme un échec), **Options**, **Quitter vers le menu** (demande
confirmation : la progression du tableau en cours, non terminé, sera perdue).

## Progression et sélection de niveau

Le tableau atteint et les tableaux déjà **terminés** sont conservés d'un lancement à l'autre :
fermer l'application ne fait jamais reculer la partie. **Choisir un niveau**, depuis le menu
principal, liste les tableaux de la séquence avec leur état — terminé, atteint (le prochain à
jouer) ou verrouillé — ainsi que les niveaux **personnels** créés dans l'éditeur, jouables
directement sans passer par l'essai de l'éditeur ni affecter la progression de la séquence. Seuls
les tableaux déjà terminés et le suivant sont jouables ; les tableaux plus loin restent
verrouillés jusqu'à ce que la progression les atteigne.

Certains niveaux ajoutent des **mécanismes** à résoudre :
- un **interrupteur** ouvre durablement la **porte** à laquelle il est relié (bascule : reste
  ouverte une fois activée) ;
- une **plaque de pression** ouvre la porte lui étant reliée **tant qu'un poids** (le personnage)
  y repose, et la referme dès qu'il en part ;
- un **bloc** peut être poussé horizontalement en marchant contre lui (s'il y a de la place pour
  qu'il avance), et tombe s'il n'est plus soutenu par le dessous — utile pour franchir un
  obstacle ou atteindre un endroit hors de portée du saut seul.

## Le menu d'options

Accessible depuis le menu principal ou depuis la pause : bascule le **V-Sync**, règle le
**volume** (un son d'essai se joue au relâchement du curseur, pour régler sans le faire à
l'aveugle), change la **langue**, affiche l'état de connexion de la **manette**, et propose un
retour au menu. Navigable au clavier, à la souris ou à la manette, comme le reste de l'interface.

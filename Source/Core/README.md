# Source/Core/

Fonctions **back** : le moteur et la logique du jeu.

## Périmètre
- Boucle de jeu (update / render tick, gestion du temps).
- Physique de plateforme (gravité, collisions, déplacement).
- Logique de puzzle (règles, états, résolution).
- Gestion des entités, des niveaux et des états du jeu.
- Chargement des ressources depuis `../Elements/`.

Ce dossier est indépendant de la présentation (`../HMI/`) : il expose un état, il ne l'affiche pas.

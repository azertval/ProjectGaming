# Source/Test/Systeme/

Tests **système** (bout en bout) : le jeu joué comme un tout, sur les **assets livrés** (niveaux),
avec des scénarios d'**entrées déterministes**.

Ils rejouent la boucle de jeu **sans la couche GPU** (fenêtre, rendu Direct3D — vérifiée
visuellement, cf. conventions) : chargement des niveaux réels, physique du personnage, règles de
fin et **enchaînement** de la séquence. Objectif : prouver qu'un **parcours complet**
(titre → niveaux → titre) est jouable de bout en bout, et le garder non régressif.

Distinction : `Unit/` teste une brique isolée ; `Integration/` teste quelques briques assemblées ;
`Systeme/` rejoue un **scénario de jeu complet** sur le contenu livré.

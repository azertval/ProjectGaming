# Elements/Fonts/

Polices bitmap pour l'affichage de texte **dans la scène rendue** (viewport Direct3D 11) : atlas de
glyphes PNG accompagné de ses métriques.

Le texte de l'**interface hors-jeu** (menus, options, éditeur) ne passe pas par ici : il est rendu
par Qt depuis `LOT-38`, avec les polices du système.

Dossier vide à ce jour. La police bitmap historique (`hmi::BitmapFont`) alimentait l'ancienne UI
« maison » et a été retirée avec elle au `LOT-38` ; le texte en scène est réintroduit au `LOT-52`.

Consommées par `HMI/Graphics`. Réf. specs : `EX-REN-032`, `EX-IHM-003`.

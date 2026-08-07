# Elements/Assets/Objects/

Textures assignées **par instance** à une case précise (`LOT-45`, `EX-EDIT-043`), prioritaires sur
le skin du type de la case (`Skins/`, `LOT-42`).

Déposer un fichier ici suffit à le rendre sélectionnable dans la section « Objets » du panneau
« Textures » de l'éditeur : la liste est peuplée par **balayage de ce dossier**, jamais par saisie
d'un chemin (même patron que `Skins/`/`Backgrounds/`).

Dimensions en **grille de cases** de 16×16 pixels (`hmi::AssetFamily::Object`, `EX-REN-007`), le
nombre d'images restant libre : seule la **première case** (coin haut-gauche) est affichée par ce
lot — un asset animé (plusieurs cases) s'affiche donc en image fixe jusqu'à ce que `LOT-46` ajoute
le choix du clip, sans que le format d'override change ensuite. Un fichier absent ou illisible
retombe sur le damier magenta, avec un avertissement journalisé nommant l'asset (`EX-NFR-040`).

# Atelier pixel art intégré {#guide-atelier-pixel-art}

> Statut : **livré** (`LOT-54`, `EX-EDIT-045`). Deux points restent hors périmètre, actés dans
> l'épic : le point d'entrée depuis le panneau Textures n'est pas câblé, et l'aperçu d'animation
> n'est pas livré (l'aperçu de **raccords**, lui, l'est).

## Le problème : quitter l'application pour changer un pixel

Depuis le `LOT-42`, l'apparence d'un niveau tient dans des fichiers image : planches de skins,
fonds, objets, plans picturaux, images d'animation. Le `LOT-43` a donné de quoi les **importer**, les
renommer, les recharger à chaud — mais pas de quoi les **modifier**. Corriger un pixel mal placé
demandait d'ouvrir un éditeur externe, enregistrer, revenir, attendre le rechargement, constater,
recommencer. Sur un raccord automatique à seize cases, où la justesse d'un bord ne se voit qu'une
fois la planche assemblée dans un niveau, cette boucle est le principal coût du travail.

L'atelier ferme la boucle : dessiner, voir le résultat dans le niveau, corriger — sans quitter
l'application.

Ce lot s'exécute **derrière** [LOT-56](@ref guide-design-ihm) et `LOT-57` et bâtit sur ce qu'ils
livrent. Le canevas n'invente ni son habillage, ni ses commandes, ni son affichage d'état : il
réutilise les jetons de design, le catalogue d'actions et le modèle de barre d'état déjà en place.
C'est ce qui explique qu'un lot d'apparence tienne en huit tâches tout en livrant des fonctions
d'édition — la plomberie d'interface était déjà payée.

## Écrire un PNG sans jamais laisser de fichier tronqué

\ref hmi::encodeImageFile "hmi::encodeImageFile" est le pendant exact de `decodeImageFile`
(`LOT-40`) : même type d'image en mémoire (`hmi::DecodedImage`, `RGBA8888`, alpha **non**
prémultiplié), si bien que décoder puis réencoder restitue les mêmes pixels, canal alpha compris.

L'écriture est **atomique** — fichier temporaire, puis remplacement — et ce n'est pas de la
prudence gratuite : le rechargement à chaud (`LOT-43`) surveille ce même dossier et peut lire
pendant qu'on écrit. Une écriture directe exposerait un PNG à demi écrit, que le chargeur
rejetterait en affichant la texture manquante — au pire moment, celui où l'auteur vient d'appuyer
sur « Enregistrer ». Le fichier temporaire ne subsiste ni après un succès, ni après un échec.

L'export d'atlas en ligne de commande (`--export-atlas`) passe désormais par ce même chemin
d'écriture, plutôt que par une seconde implémentation.

## Les outils : des fonctions pures sur un tampon de pixels

`HMI/Editor/PixelOperations.h` contient **tout** ce que font les outils, sous forme de fonctions
libres sans état, sans Qt ni GPU (`EX-NFR-010`) —
donc entièrement testables. Le widget, lui, ne fait que traduire des gestes en appels.

- \ref hmi::setPixel "setPixel" / \ref hmi::erasePixel "erasePixel" — un pixel, posé ou effacé.
- \ref hmi::drawLine "drawLine" / \ref hmi::eraseLine "eraseLine" — le tracé de **Bresenham**
  ⧉ entre deux positions successives du curseur. Sans lui, un glisser rapide laisserait des
  trous : les événements de souris arrivent à la cadence du système, pas à celle du déplacement
  de la main. La gomme a son pendant exact pour la même raison — un glisser d'effacement troué
  serait tout aussi visible qu'un tracé troué.
- \ref hmi::floodFill "floodFill" — le remplissage par zone contiguë, **itératif** (file
  explicite), jamais récursif : une région de 4096 × 4096 pixels d'un seul tenant ferait déborder
  la pile d'appels d'une version récursive.
- \ref hmi::pickColor "pickColor" — la pipette, qui rend la couleur d'un pixel.

Chaque opération renvoie une \ref hmi::PixelRegion "PixelRegion" : le rectangle de pixels
réellement touché. Ce n'est pas une commodité d'affichage, c'est ce qui borne le coût mémoire de
l'historique (ci-dessous).

Les **outils de région** (`TACHE-06`) suivent le même principe : \ref hmi::flipHorizontal
"flipHorizontal" et `flipVertical`, \ref hmi::rotateClockwise "rotateClockwise" et son inverse,
\ref hmi::moveRegion "moveRegion", et le couple \ref hmi::copyRegion "copyRegion" /
\ref hmi::pasteClipboard "pasteClipboard" par un `hmi::PixelClipboard` — un tampon autonome, qui
survit donc à la fermeture de l'image dont il provient.

## L'historique : local au canevas, et nommé

\ref hmi::PixelHistory "PixelHistory" est une pile d'annulation/rétablissement **propre au
canevas**, totalement indépendante de celle de `core::LevelDraft` (@ref guide-editeur). C'est un
critère d'acceptation explicite du lot : annuler un coup de pinceau ne doit jamais annuler la pose
d'une tuile, et réciproquement. Les deux historiques ne se rencontrent qu'au niveau du
**dispatch** de l'action (voir plus bas), jamais dans leurs données.

Deux traits le distinguent de l'historique du brouillon :

- **Il mémorise des régions, pas des instantanés.** `core::LevelDraft` conserve des instantanés
  complets, ce qui est raisonnable pour une grille de tuiles ; une image de 512 × 512 pixels en
  RGBA pèse un mégaoctet, et un coup de pinceau n'en touche qu'une poignée. Une entrée
  (\ref hmi::PixelHistoryEntry "PixelHistoryEntry") retient donc la région affectée et son
  contenu **avant** et **après**. La profondeur reste plafonnée par-dessus.
- **Chaque opération porte un nom** (\ref hmi::PixelOperationKind "PixelOperationKind"), traduit
  par \ref hmi::pixelOperationTranslationKey "pixelOperationTranslationKey" — une clé présente dans
  les deux catalogues, `fr.lang` et `en.lang`. Ce nom sert deux fois : dans le libellé de la
  commande (« Annuler *le remplissage* ») et dans le panneau d'historique visuel
  (`PixelHistoryPanel`), qui liste les opérations et permet de revenir à un point antérieur en un
  clic — pas en appuyant douze fois sur Annuler.

Un **geste complet** produit **une seule** entrée : un glisser de pinceau, si long soit-il, s'annule
d'un coup. Découper par événement de souris rendrait l'annulation inutilisable.

## Le canevas : la géométrie d'abord, le widget ensuite

`HMI/Editor/PixelCanvasGeometry.h` isole les conversions vue ↔ image en fonctions
pures : \ref hmi::imagePixelScreenRect "imagePixelScreenRect" (où dessiner un pixel image),
\ref hmi::screenToImagePixel "screenToImagePixel" (quel pixel est sous le curseur), et les
commandes de zoom. Le zoom est **toujours entier** (`EX-ARCH-022`) : un facteur fractionnaire
donnerait des pixels de largeurs inégales à l'écran, ce qui rend le travail au pixel près
impossible. La grille de pixels n'apparaît qu'au-delà d'un seuil de zoom — en deçà, elle ferait
plus de bruit que de repère.

Cette séparation n'est pas cosmétique : c'est cette géométrie qui détermine le pixel survolé
affiché dans la barre d'état, et elle doit rester juste à tout zoom, tout décalage de vue **et
toute échelle d'affichage**. Elle réutilise pour cela \ref hmi::thumbnailPixelSize
"thumbnailPixelSize" (@ref guide-design-ihm, `LOT-56`) plutôt que de redéfinir sa propre règle de
netteté.

`hmi::PixelCanvas` (`QWidget`) ne fait que le reste : affichage au plus proche voisin, damier de
transparence, gestes de souris. Son fond et son damier sont tirés des jetons de portée
**invariante** (`identityTokens`) : ils représentent l'absence de couleur, pas une surface
d'interface, et ne doivent donc pas changer avec le thème clair/sombre du châssis — sans quoi
l'artiste jugerait ses couleurs sur un fond mouvant.

## Palettes

\ref hmi::PixelPalette "PixelPalette" persiste une palette de projet sur disque.
\ref hmi::extractPalette "extractPalette" recense les couleurs d'une image ouverte (avec leur
fréquence), et \ref hmi::nearestPaletteColor "nearestPaletteColor" sert le mode « contraindre à la
palette », qui interdit de poser une couleur hors palette. C'est la discipline qui donne à un jeu
de sprites son unité : sans contrainte, chaque retouche introduit un nouveau ton presque identique
au précédent.

## Créer un asset à une taille forcément conforme

\ref hmi::validAssetSizes "validAssetSizes" (`PixelAssetIO.h`) propose, pour une famille d'asset,
une courte progression de tailles **dérivées** du contrat d'asset (`hmi::assetDimensionContract`,
`EX-REN-007`) — jamais une seconde description des mêmes règles. L'intention est de rendre une
création non conforme **impossible**, plutôt que de la refuser après coup au chargement. Une
famille à dimensions libres (fond, police) renvoie une liste vide : l'appelant offre alors
une saisie libre.

## Voir le raccord avant de l'avoir posé

\ref hmi::isBitmask16Candidate "isBitmask16Candidate" reconnaît une planche à raccords à ses
dimensions, \ref hmi::bitmaskCellAtPixel "bitmaskCellAtPixel" dit quelle case de voisinage porte un
pixel donné, et \ref hmi::buildAutotileAssemblyPreview "buildAutotileAssemblyPreview" assemble un
**aperçu** de la planche telle qu'elle apparaîtra une fois raccordée. C'est la réponse directe au
problème posé en tête de page : la justesse d'un bord de raccord ne se juge pas case par case, mais
sur l'assemblage.

## Une seule paire Annuler/Refaire, deux cibles

Le canevas et le niveau ont chacun leur historique, mais l'application n'expose **qu'une** action
Annuler et **qu'une** action Refaire (`EX-IHM-062`). Le tri se fait par le contexte d'édition
actif : `hmi::PixelCanvas` implémente \ref hmi::EditContextTarget "EditContextTarget" — la même
interface que `hmi::GameViewport` — et `MainWindow` réassigne sa cible au widget qui reçoit le
focus clavier. Le dispatch existant n'a pas été modifié pour accueillir l'atelier : c'est
précisément le seuil que `LOT-57` avait posé en prévision de ce lot (voir
@ref guide-design-ihm).

De la même façon, les outils du canevas forment un groupe d'actions exclusif **distinct** de celui
des outils de niveau (`EditorActionGroup::PixelTools`), avec ses icônes dessinées par code et sa
barre d'outils dédiée ; et l'état du canevas (asset ouvert, modifications, outil, pixel survolé,
zoom, couleur courante) **étend** le modèle de barre d'état de `LOT-57` d'un contexte d'édition
d'asset, au lieu d'en dupliquer un second.

## Voir aussi
- @ref guide-design-ihm — les jetons, les actions et la barre d'état dont l'atelier hérite.
- @ref guide-editeur — l'éditeur de niveau, son brouillon et son historique **distinct**.
- @ref guide-rendu — les textures, les planches à raccords et le rechargement à chaud que
  l'atelier alimente.
- [Spécification de l'éditeur](@ref spec-editeur) — le *quoi/pourquoi* (`EX-EDIT-045`).

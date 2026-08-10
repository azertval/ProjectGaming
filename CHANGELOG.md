# Changelog

Toutes les évolutions notables du projet sont consignées ici.
Format inspiré de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/) ;
le projet suit le [versionnage sémantique](https://semver.org/lang/fr/).

## [Non publié]

## [0.0.5] - 2026-08-10

> Cinquième jalon : le moteur est **habillé**. Le programme `LOT-40` → `LOT-55`, ouvert juste après
> le jalon précédent, est livré en entier : rendu texturé multicouche avec culling, skins de tuiles
> et **raccords automatiques**, texture par instance, fonds de niveau, **décors libres** hors grille
> avec parallaxe, moteur d'**animation piloté par données**, personnage habillé depuis une
> spritesheet externe, retour du **texte dans la scène** et affichage tête haute, et enfin les
> **ombres du plan physique** (LOT-55) — le tout derrière une bascule `F8` qui restitue à tout
> moment la lecture nue des collisions.
>
> L'**éditeur** change d'échelle en parallèle : bibliothèque d'assets à vignettes avec rechargement
> à chaud (LOT-43), manipulation complète des décors (LOT-50), mode d'inspection par calque
> (LOT-51), puis trois lots qui s'attaquent à l'interface elle-même — un **système de design**
> assumé, thème clair/sombre compris (LOT-56), une **redistribution de l'information** où ce qui
> informe devient permanent et ce qui commande devient unique (LOT-57), et un **atelier pixel art
> intégré** (LOT-54) qui ferme la boucle entre dessiner un asset et le voir dans le niveau.
>
> Cette version retire aussi le dernier morceau d'IHM « maison » (LOT-38 Étape B) : il ne reste
> qu'une application Qt. **943 tests** (541 au jalon précédent).
>
> Voir le détail ci-dessous (LOT-38 Étape B, LOT-39 → LOT-57).

### Ajouté
- **LOT-55 — Ombres du plan physique** (`EX-REN-045`, `EX-ARCH-012`) : **dernier lot du programme
  d'habillage** `LOT-40` → `LOT-55`. Le calque `RenderLayer::Shadow`, réservé sans être utilisé
  depuis le `LOT-40`, s'active enfin — entre `Decor` et `Tile`, donc sous les tuiles et au-dessus du
  fond et des décors d'arrière-plan. L'objectif est de **lecture**, pas d'esthétique : distinguer
  d'un coup d'œil ce qui est **physique** (solide, donc porteur) de ce qui n'est que décor — le
  complément exact du calque de premier plan (`LOT-49`), qui dit l'inverse.
  - `hmi::composeShadows` (`HMI/Graphics/ShadowRenderer.h`) parcourt les mêmes entités que
    `hmi::composeWorldSprites` et n'en retient que celles qui projettent une ombre : pleines
    (`core::isSolid`) ou à silhouette inclinée/courbe (`hmi::hasSilhouette`, `LOT-42`). La région
    échantillonnée est directement `hmi::regionForTile` — le **même** atlas procédural que le mode
    Physique, déjà opaque exactement là où la matière est présente. Teinter ce quad en noir
    semi-transparent, décalé d'un pixel, donne donc l'ombre à sa **forme réelle** (pente, arrondi,
    bloc réduit via `core::tileVisualScale`) sans réimplémenter la moindre géométrie et **sans aucun
    nouveau prédicat de solidité dans `Core`** : une ombre est la projection d'une forme, pas d'un
    degré de solidité, et `Core` expose déjà cette forme sous une version plus riche qu'un booléen.
  - Une **porte** fait exception à la règle « ombre = type statique » : son type reste `Door` quel
    que soit l'état du mécanisme, alors que sa solidité réelle dépend de l'interrupteur.
    `composeShadows` accepte donc une grille de collision optionnelle
    (`MechanismController::collisionMap`, fournie par `GameSession`) et tranche sur l'état
    **courant** — fermée, elle projette ; ouverte, elle ne projette plus. `hmi::DraftRenderer`, qui
    ne simule aucun mécanisme, ne la fournit pas : dans l'éditeur une porte n'a jamais d'ombre, état
    normal et non défaut.
  - Actif en `RenderMode::Texture` **uniquement**, aucun effet sur le gameplay (`EX-ARCH-012`), même
    culling que le reste, masquable par l'axe `Shadow` de `hmi::LayerVisibility` (`LOT-51`). Un bloc
    poussable en mouvement voit son ombre suivre par la même interpolation
    (`hmi::PreviousPosition`) que son propre sprite. Un niveau sans fond ni décor n'a simplement
    aucune surface pour recevoir l'ombre — pas d'erreur, pas de cas particulier.
  - Documenté dans `Documentation/Guide/guide-rendu.md`, qui remplace au passage sa section
    d'orientation « ce qui vient ensuite » (obsolète depuis plusieurs lots) par l'état livré complet
    du programme d'habillage. **943 tests verts**, build `/W4 /WX` propre.

- **LOT-54 — Atelier pixel art intégré** (`EX-EDIT-045`) : créer et modifier les fichiers d'assets
  de texture (skins, planches, fonds, objets, décors) **sans quitter l'application**, en voyant
  immédiatement le résultat dans le niveau. Depuis le `LOT-43`, on savait importer et recharger un
  asset, jamais le **modifier** : corriger un pixel imposait un aller-retour vers un éditeur externe,
  particulièrement coûteux sur une planche à raccords dont la justesse ne se juge qu'une fois
  assemblée. Ce lot s'exécute délibérément **derrière** `LOT-56` et `LOT-57` — le canevas n'a donc à
  inventer ni son habillage, ni ses commandes, ni son affichage d'état, et le budget ainsi libéré
  finance des fonctions d'édition plutôt que de la plomberie d'interface.
  - **TACHE-01 — Écriture d'image** : `hmi::encodeImageFile`, symétrique exact de `decodeImageFile`
    (`LOT-40`), écriture PNG **atomique** (fichier temporaire puis remplacement) depuis un tampon
    RGBA non prémultiplié — le rechargement à chaud surveille le même dossier et pourrait lire un
    fichier à demi écrit. `--export-atlas` passe désormais par ce même chemin. `UnitTests` reste
    constructible sans Qt (le seul test touchant Qt est ajouté conditionnellement).
  - **TACHE-02 — Opérations et historique** : `hmi::PixelOperations` (pinceau, gomme, ligne de
    Bresenham sans trou au glisser rapide, remplissage par zone contiguë **itératif** — jamais
    récursif —, pipette), fonctions pures sur `hmi::DecodedImage`, sans Qt ni GPU.
    `hmi::PixelHistory` : pile d'annulation **locale au canevas**, totalement indépendante de
    `core::LevelDraft` (annuler un coup de pinceau n'annule jamais une pose de tuile), à opérations
    **nommées** (clés présentes dans les deux catalogues de traduction), mémorisant des **régions**
    plutôt que des instantanés complets, profondeur plafonnée, retour à un point antérieur en un
    appel.
  - **TACHE-03 — Canevas** : `hmi::PixelCanvasGeometry` (conversions pures vue ↔ image, zoom
    toujours **entier** pour que les pixels restent carrés), réutilisant `hmi::thumbnailPixelSize`
    (`LOT-56`) pour la netteté à toute échelle d'affichage plutôt que de la redéfinir.
    `hmi::PixelCanvas` (`QWidget`) : plus proche voisin, grille de pixels au-delà d'un seuil de zoom,
    damier de transparence. Fond et damier tirés des jetons de portée **invariante** — un fond qui
    changerait de clarté avec le thème fausserait la perception des couleurs posées, rédhibitoire
    pour l'outil dont c'est le sujet. Un geste complet produit **une** entrée d'historique.
  - **TACHE-04 — Actions, barre d'outils, barre d'état** : quatre outils en actions Qt formant un
    groupe exclusif **distinct** de celui des outils de niveau (`EditorActionGroup::PixelTools`),
    icônes dessinées par code. Annuler/Refaire restent une action **unique** à cible contextuelle :
    `PixelCanvas` implémente `hmi::EditContextTarget` et `MainWindow` réassigne la cible au widget
    qui reçoit le focus clavier — le dispatch livré par `LOT-57` n'est pas modifié.
    `hmi::EditorStatus` et `hmi::PanelFocus` sont **étendus** (`PixelEditStatusInfo`, `PanelId::
    PixelCanvas`/`PixelHistory`), jamais doublés. Nouveau `PixelHistoryPanel`.
  - **TACHE-05 — Ouvrir, créer, enregistrer** : `hmi::validAssetSizes` dérive les tailles proposées
    du contrat d'asset (`EX-REN-007`) plutôt que de les redécrire — une création non conforme devient
    impossible au lieu d'être refusée après coup ; les familles à dimensions libres (fond, décor,
    police) ouvrent une saisie libre. Garde-fou d'écrasement nommant les références concernées.
  - **TACHE-06 — Outils de région** : sélection rectangulaire, déplacement, symétries horizontale et
    verticale, rotations par quart de tour, copier/coller par un presse-papiers autonome — toutes
    fonctions pures, exposées comme des actions.
  - **TACHE-07 — Palettes** : `hmi::PixelPalette` (couleurs nommées persistées dans
    `Assets/palettes.json`, même patron que `SkinCatalog` — fichier absent traité comme une palette
    vide, entrée malformée ignorée plutôt que d'abandonner toute la palette),
    `hmi::extractPalette` (ordre déterministe, occurrences, jamais l'alpha nul) et
    `hmi::nearestPaletteColor` (départage stable, alpha d'origine préservé). Panneau d'édition et
    réglage « contraindre à la palette », persisté. Un sélecteur de couleur libre a été ajouté dans
    la foulée : pipette et pastilles étaient les deux seuls moyens de changer de couleur, aucun ne
    permettant d'en choisir une absente des deux.
  - **TACHE-08 — Aperçu live et planche à raccords** : `MainWindow::updateLivePreview` écrit l'image
    en cours et invalide **ciblément** son entrée de `TextureCache` (`GameViewport::invalidateAsset`,
    prévu depuis `LOT-43`) à chaque geste complet — jamais par pixel. `TileAutotile` est étendu
    (`autotileConfigurationLabelKey`, `autotileAssemblyMasks`) pour que la **même** table canonique
    décrive aussi les seize configurations en langage naturel et fournisse l'assemblage 3×3, jamais
    une seconde table. `hmi::PixelAutotilePreview` (pur) détecte une planche 4×4 et compose l'aperçu
    d'assemblage depuis le tampon en mémoire ; le canevas y ajoute repères de cases et infobulle
    nommant la configuration survolée.
  - **Non livrés, actés dans l'épic** : le point d'entrée depuis le panneau Textures n'est pas câblé
    (TACHE-05), et l'aperçu **d'animation** n'est pas livré faute de temps pour intégrer
    `AnimationCatalog` au canevas (TACHE-08) — l'aperçu de **raccords**, lui, l'est.
  - Documenté dans `Documentation/Guide/guide-atelier-pixel-art.md`. Build `/W4 /WX` propre.

- **LOT-57 — Architecture de l'information de l'éditeur** (`EX-IHM-060` à `EX-IHM-062`) :
  redistribution de l'éditeur — ce qui informe devient permanent, ce qui commande devient unique, ce
  qui ne sert qu'à un outil s'efface quand cet outil n'est pas actif.
  - **TACHE-01 — Barre d'état structurée** : remplace la ligne unique `status.edit_help` (figée à
    l'entrée en mode éditeur, définitivement effacée par le premier message transitoire) par cinq
    zones **permanentes** (`hmi::EditorStatus`, fonction pure sur le patron de `hmi::gameHudLines`,
    `LOT-52`) — niveau ouvert, modifications non enregistrées, outil actif, case survolée, zoom —
    ajoutées à la barre d'état via `addPermanentWidget` (jamais recouvertes par un message). Aide
    contextuelle à l'outil actif, restaurée automatiquement à l'expiration d'un message transitoire
    (`MainWindow::refreshStatusHelp`/`showTransientStatusMessage`, minuteur unique). Case survolée et
    zoom nouvellement exposés par `GameViewport` (`hoveredCell()`/`zoom()`, signaux `hoveredCellChanged`/
    `zoomChanged`, émis seulement sur changement réel). **6 nouveaux tests**, sans Qt/GPU ; build
    `/W4 /WX` propre, suite verte.
  - **TACHE-02 — Regroupement des panneaux, suivi de l'outil actif** : les panneaux Niveaux, Liens et
    Textures sont désormais regroupés en onglets par défaut (`tabifyDockWidget`, disposition v4,
    invalide les dispositions antérieures), chacun restant individuellement déplaçable, détachable et
    refermable. L'onglet pertinent est mis en avant à chaque changement d'outil (`hmi::panelForTool`,
    table pure sur le patron d'`ActionCatalog`) tant que l'utilisateur n'a rien imposé lui-même (choix
    manuel d'onglet, déplacement de panneau) — jamais un masquage, réglable et persisté depuis le menu
    Affichage. **4 nouveaux tests**, sans Qt ; build `/W4 /WX` propre, suite verte.
  - **TACHE-03 — Recentrage du panneau Textures** : l'onglet Calques (mode d'inspection « définition
    des textures », `LOT-51`) quitte le panneau Textures pour le menu Affichage — une entrée par
    calque dans l'ordre de dessin, plus « tout afficher », sans changement de comportement
    (`EX-EDIT-044` inchangée, jamais lue par `GameSession`). L'avertissement permanent qu'imposait sa
    présence dans le panneau devient inutile dans son nouvel emplacement. Le panneau Textures ne
    porte plus que la définition d'apparence (Skins, Fond, Objets, Animations, Décors) ; les deux
    sélecteurs de jeu de skins (session d'édition vs. niveau) portent désormais chacun une infobulle
    distincte. Aucun nouveau test (changements Qt purs) ; build `/W4 /WX` propre, suite verte.
  - **TACHE-04 — Déduplication des commandes et raccourcis** : `hmi::EditorKeyBindings` définissait
    dix actions d'éditeur remappables dont neuf n'étaient jamais lues (raccourcis interceptés en
    dur, non remappables ; Copier/Coller au clavier bypassaient même `EditorKeyBindings`). Toutes
    passent désormais par `hmi::EditorActions` (`hmi::keyBindingIconCatalog`, table pure liant
    action remappable et commande du catalogue) : `EditorActions::applyShortcuts` synchronise le
    raccourci **effectif** de chaque `QAction` depuis les touches remappées, y compris après un
    remappage à chaud. Nouvel onglet « Éditeur » de la page Options (`EditorKeybindingsWidget`,
    même patron que le remappage clavier de jeu). Renommer (F2) renomme désormais le niveau ouvert
    (`GameViewport::renameOpenLevel`, réutilise `LevelFileOperations`/`LevelNameValidation` comme
    `LevelBrowserPanel`) ; l'aide (F1) ouvre un aperçu des raccourcis lisant les touches effectives,
    jamais un texte figé. La bascule Physique/Texture (case dupliquée retirée en TACHE-03) rejoint
    le menu Affichage comme entrée unique de l'action déjà existante (`EX-IHM-062`). Annuler/Refaire/
    Copier/Coller dispatchent désormais via `hmi::EditContextTarget`, interface que `GameViewport`
    implémente — le seuil de dispatch qu'un futur atelier pixel art (`LOT-54`) réutilisera pour sa
    propre cible sans le réécrire. Le doublon de sélecteur de couche de décor (panneau Outils vs.
    onglet Décors) est conservé : les deux ciblent des états distincts (couche du prochain décor
    posé vs. couche du décor sélectionné existant), pas un doublon strict. **3 nouveaux tests** (dont
    un garde-fou cassant si une action est ajoutée sans être branchée) ; build `/W4 /WX` propre, suite
    verte.
  - **Amendement post-essai manuel** : premier essai réel de l'éditeur reconstruit, deux retours
    tranchés dans la foulée. Le panneau « Outils » (`ToolPanel`), qui ne portait déjà plus que le
    strict nécessaire de l'outil Décor depuis `LOT-56`, devient le panneau **Décors**
    (`DecorsPanel`) et regroupe désormais aussi l'inspecteur des décors posés déplacé de l'onglet
    « Décors » du panneau Textures — les deux sélecteurs de couche (placement du prochain décor,
    couche du décor sélectionné) coexistent donc maintenant dans le même panneau, renommés
    explicitement pour lever toute ambiguïté. Le panneau Textures sort du regroupement en onglets de
    TACHE-02 et redevient indépendant, comme Palette/Décors. La barre d'outils du haut, elle, reste
    à sa place (barre globale de la fenêtre principale, hors du panneau Décors). `LAYOUT_VERSION`
    4 → 5.

- **LOT-56 — Système de design de l'IHM Qt** (`EX-IHM-050` à `EX-IHM-055`) : l'éditeur prend enfin
  la main sur sa propre apparence — jusqu'ici le style **natif** de la plate-forme, qui ignorait une
  large part de l'unique feuille de style existante (`theme.qss`, restreinte au menu principal et à
  la page Options par `objectName`, faute de mieux).
  - `hmi::DesignTokens` (`HMI/Interface/DesignTokens.h`) — jetons de design purs (couleurs par rôle,
    espacement, typographie, tailles), en deux portées de structure identique : l'**identité** du
    jeu (menu, Options, jeu — invariante) et le **châssis d'édition** (variable). `hmi::
    ApplicationTheme` choisit le style Qt Fusion avant tout widget, construit la `QPalette`
    complète (actif/inactif/désactivé) et produit `theme.qss` par substitution de marqueurs depuis
    les jetons (`hmi::substituteStyleSheetTemplate`, fonction pure) — plus aucune couleur littérale,
    et une étanchéité entre les deux portées garantie par test. Focus clavier visible partout
    (navigation à la manette, `EX-IHM-040`).
  - Police **Inter** embarquée (`Assets/Fonts/Inter-{Regular,Bold}.ttf`, licence SIL OFL 1.1) avec
    repli sur une famille générique si absente (`hmi::resolveFontFamily`, jamais un second nom codé
    en dur) ; typographie à échelle unique, tailles et marges des `.ui` retirées au profit des
    jetons.
  - Barre d'outils à icônes (`hmi::EditorActions`) remplaçant les boutons radio empilés du panneau
    Outils (`EX-EDIT-015`) : six outils et sept commandes (enregistrer, essayer, annuler, refaire,
    grille, recadrer, mode de rendu) exposés comme des actions Qt **uniques**, simultanément dans le
    menu, la barre d'outils et leur raccourci — plus de double définition. Icônes dessinées par code
    (`hmi::iconGeometry`/`hmi::themeIcon`, géométrie pure + rendu `QPainter`), recolorées depuis les
    jetons.
  - Vignettes de la palette, des grilles d'assets et des lignes du panneau Textures rendues à la
    résolution **réelle** de l'écran (`hmi::thumbnailPixelSize`, fonction pure) : nettes à 100 %,
    125 % et 150 % d'échelle d'affichage, régénérées lors d'un changement d'écran.
  - Thème **clair/sombre de l'éditeur** (`hmi::editorLightTokens`, `hmi::
    resolveEffectiveEditorTheme`), suivant par défaut le réglage du système d'exploitation, réglable
    depuis le menu Affichage (Système/Clair/Sombre) et persisté ; appliqué à chaud (palette, feuille
    de style, icônes) sans redémarrage. Contraste texte/fond vérifié pour les deux thèmes (seuils
    WCAG). L'identité (menu principal, Options, jeu) reste rigoureusement inchangée quel que soit le
    thème actif, y compris après une bascule à chaud — garanti par test.
  - **24 nouveaux tests**, tous sans Qt/GPU (la couche Qt — `ApplicationTheme`, `ThemeIcons`,
    `EditorActions` — reste, comme `BitmapFont`/`GameViewport`, hors `UnitTests` ; seule leur
    logique pure — jetons, gabarit de feuille de style, catalogue d'actions, géométrie d'icônes,
    résolution de thème — y est compilée) ; build `/W4 /WX` propre, suite verte.

- **LOT-52 — Texte, police bitmap et affichage tête haute** (`EX-IHM-003`, re-concrétise
  `EX-REN-032` retirée au `LOT-38`) : le jeu peut de nouveau afficher du texte **dans la scène
  rendue** — les budgets de sauts/dashs (`EX-GP-024`, `LOT-12`) et le nom du tableau, jusqu'ici
  invisibles faute de tout rendu de texte, apparaissent désormais en jeu et en essai.
  - `hmi::ProceduralFont`/`hmi::BitmapFont` — police bitmap chargée depuis `Assets/Fonts/
    font.png` + ses métriques (`font.json`, format JSON versionné, même patron que
    `hmi::AnimationCatalog`), validée par le contrat d'asset (`AssetFamily::Font`, `EX-REN-007`)
    et par sa cohérence avec les dimensions du PNG. Repli **procédural** déterministe si l'atlas
    ou ses métriques sont absents/invalides (glyphes 5×7 pixels, ASCII imprimable + accents
    français `é è à ç ù ê î ô û`, sur le modèle de `hmi::buildProceduralAtlasImage`, `LOT-39`) :
    le jeu reste lisible sans aucun asset de police (`EX-NFR-040`). Aucun asset n'est livré pour
    l'instant (`Source/Elements/Assets/Fonts/README.md`) : le repli procédural est donc le
    rendu actif tant qu'un artiste n'a pas déposé `font.png`/`font.json`. Un caractère non
    couvert est substitué par un glyphe de remplacement, jamais un trou silencieux. Mesure de
    texte pure (`hmi::measureText`), parcourant des **points de code** UTF-8 (pas des octets) —
    piège classique explicitement évité, comme documenté dans l'ancien `hmi::BitmapFont` retiré.
  - `hmi::TextRenderer` (`HMI/Graphics/TextRenderer.h`) — compose une chaîne en `SpriteQuad` sur
    le calque `RenderLayer::UI` (réservé sans être utilisé depuis `LOT-40`), avec ancrage
    paramétrable (gauche/centre/droite, haut/milieu/bas) et positions arrondies au pixel écran
    entier (netteté, `EX-ARCH-022`). **Projection écran dédiée** (`hmi::screenProjectionMatrix`,
    dépendant uniquement des dimensions du viewport) : premier cas du projet où une passe de
    rendu a sa propre projection, indépendante de `Camera2D` — le HUD ne tourne ni ne change de
    taille avec le zoom. Composé dans une `hmi::ComposedScene` **dédiée**, distincte de celle de
    `hmi::SpriteRenderer` et jamais soumise à un cadrage de culling caméra : le texte, en espace
    écran, n'a pas de position monde (`LOT-40` TACHE-05 ne s'y applique pas).
  - `hmi::GameHud`/`hmi::GameSession::renderHud` — fonction **pure** (`hmi::gameHudLines`)
    choisissant les lignes à afficher (compteurs de sauts/dashs, seulement si le budget du
    niveau est **fini** — `-1` = illimité, cas de la grande majorité des tableaux, aucune ligne
    superflue ; nom du tableau) à partir de `core::Player`/`core::Level::name`, testée sans GPU.
    Affichage en jeu et en essai (hérité du point d'entrée unique `hmi::GameSession::render`,
    jamais appelé en édition pure — `hmi::DraftRenderer` reste le seul chemin de l'éditeur) avec
    une ombre portée (décalage d'un pixel) pour rester lisible sur fond clair comme sur fond
    sombre. Nouvelles clés `hud.jumps_remaining`/`hud.dashes_remaining` dans les deux catalogues
    de traduction (`EX-REN-033`).
  - **32 nouveaux tests**, tous sans GPU (`hmi::ProceduralFont`/`hmi::TextRenderer`/`hmi::
    GameHud` sont des fichiers séparés de leurs classes propriétaires de ressources Direct3D —
    `hmi::BitmapFont` n'est pas compilé dans `UnitTests`, comme `hmi::TextureAtlas`) ; build
    `/W4 /WX` propre, suite verte.

- **LOT-51 — Mode d'inspection « définition des textures » par calque** (`EX-EDIT-044`) : nouvel
  onglet « Calques » du panneau Textures — une case à cocher par calque de rendu, dans l'ordre de
  dessin, plus « Physique seul » et « Tout afficher ». Réservé à l'éditeur, **sans aucun effet** ni
  sur `hmi::GameSession` ni sur la bascule `F8` (`LOT-41`), distinction rappelée par une info-bulle.
  `hmi::LayerVisibility` (nouveau) : jeu de visibilités indexé par `RenderLayer`, tout visible par
  défaut, sans persistance entre deux sessions. `hmi::resolveTileAppearance` étend le résolveur
  unique surcharge > skin > damier avec un mode « isoler » : les bits Tuile/Objet pilotent
  désormais les deux axes de résolution d'une tuile plutôt qu'un calque physique distinct, en
  isolant sans repli sur le damier quand un seul axe est actif. `composeWorldSprites`/
  `DraftRenderer::render` filtrent à la **composition** (jamais à la construction de la scène ECS).

- **LOT-50 — Manipulation de décors dans l'éditeur** (`EX-DEC-010`) : outillage d'édition complet
  pour un décor déjà posé (`LOT-49`) — sélectionner, déplacer, redimensionner, pivoter, changer de
  couche et réordonner, en plus du placement/retrait déjà existants.
  - Mutateurs `core::LevelDraft` (`moveDecor`/`resizeDecor`/`rotateDecor`/`setDecorLayer`/
    `bring*Forward`/`send*Backward`/`bring*ToFront`/`send*ToBack`) : position et échelle
    appliquées **atomiquement** pour un redimensionnement, une seule entrée d'historique
    (undo/redo) par geste complet.
  - Géométrie partagée (`hmi::DecorGeometry`) et machine à état pure du geste (`hmi::
    DecorGesture`, même patron que `hmi::LinkGesture`, `LOT-37`) : désignation, distinction
    clic/glisser, poignées (taille écran constante), abandon — jamais de mutation directe du
    brouillon pendant l'aperçu. Remplace `hmi::DecorPlacementGesture` (`LOT-49`), supersedé par la
    détection par rectangle englobant.
  - Section « Décors » du panneau Textures : liste groupée par couche puis par ordre de
    superposition, sélection **croisée** unique avec le canevas, signalement des assets manquants.
  - **Trois défauts corrigés en cours de lot** : `decorWorldBounds` oubliait de convertir les
    pixels de l'asset en unités monde (rectangle englobant, donc poignées et cadre de sélection,
    seize fois trop grands) ; la rotation posée par la poignée dédiée n'avait aucun effet visible
    (`core::Transform::rotation` était ignorée au rendu depuis `LOT-49` — `hmi::SpriteQuad` porte
    désormais une rotation optionnelle, appliquée par `SpriteBatch::draw`, coins tournés autour du
    centre) ; le cadre de sélection et ses poignées restaient alignés aux axes pendant que le
    décor tournait sous eux (`hmi::decorRotatedPoint`, même formule de rotation que le rendu).

- **LOT-49 — Décors libres, rendu multicouche et parallaxe** (`EX-DEC-001`, `EX-DEC-002`,
  `EX-DEC-006`) : `core::Decor` (position/échelle/rotation libres, hors grille, couche,
  statique/manipulable) sérialisé dans le format de niveau versionné (`LOT-44`). Rendu sur les
  calques arrière-plan et premier plan (le premier plan passe **au-dessus** du personnage), repli
  sur le damier magenta pour un asset introuvable ; **parallaxe** relative au centre de la salle
  courante, cohérente avec la caméra à coupure nette entre salles (`LOT-32`). Placement minimal
  dans l'éditeur (poser/supprimer) depuis une nouvelle bibliothèque `Assets/Decors/`.

- **LOT-48 — Personnage habillé depuis une spritesheet externe** : dernier sprite resté hors du
  programme d'habillage — en mode Texture, le personnage retombait jusqu'ici sur le damier
  magenta. Spritesheet externe (`Assets/Player/`) avec repli procédural, taille de l'image
  **découplée** de la hitbox par un point d'ancrage centre-bas (une image plus grande que la
  hitbox ne la déforme donc plus), projection état → clip étendue à la chute, l'atterrissage, la
  glissade murale et le dash (clips prioritaires, résolus sans nouveau champ sur `core::Player`),
  personnage retourné selon son sens de déplacement.

- **LOT-47 — Apparence des mécanismes pilotée par leur état logique** (`EX-REN-006`) : une porte,
  un interrupteur, une plaque de pression et les dangers commuté/temporisé/mobile changent
  désormais d'**apparence** selon leur état en mode Texture, plutôt que la simple modulation
  d'opacité qu'appliquait `GameSession` jusque-là. `hmi::MechanismVisuals` traduit l'état lu dans
  `Core` en clip attendu (infrastructure `LOT-46`) ; un clip manquant retombe proprement sur
  l'image statique. La modulation d'opacité de diagnostic reste réservée au mode **Physique**,
  désormais isolée dans une fonction pure. Nouvelle section « Animations » du panneau Textures
  (asset par défaut par famille de mécanisme, diagnostic des clips manquants, aperçu).

- **LOT-46 — Moteur d'animation générique piloté par données** (précise `EX-REN-012`) : remplace
  l'`enum AnimationClip` figé (`Idle`/`Run`/`Jump`) par des clips-**données**
  (`core::AnimationClip`/`core::ClipSet`) et généralise `AnimationSystem` à **toute** entité
  portant `core::Animation`, plus seulement le personnage. Nouveau format `nom-asset.anim.json`
  (`hmi::AnimationCatalog`) décrivant une spritesheet animée, mis en cache et invalidé
  **conjointement** par `TextureCache`. Anime les skins de tuiles (eau, lave, torche) via une
  horloge **partagée par asset**, résolue à la composition du rendu plutôt qu'écrite par tuile —
  toutes les tuiles d'un même type animé restent ainsi en phase, sans coût par case. Le personnage
  est migré à l'identique (mêmes durées, mêmes images), non-régression attestée par le test de
  référence existant. **Deux défauts corrigés en cours de lot** : le canevas d'édition
  (`hmi::DraftRenderer`, rendu en continu hors essai) composait toujours la première image d'un
  asset animé, sans jamais progresser (horloge partagée factorisée entre pas fixe déterministe du
  jeu et temps réel de l'aperçu d'édition) ; `TextureCache::getAnimation` résolvait le descripteur
  à la racine du dossier `Assets` plutôt que dans `Assets/Skins/`, empêchant silencieusement
  toute tuile de s'animer.

- **LOT-45 — Texture par instance sur les objets interactifs** : assigne une texture à une case
  **précise** d'un niveau, prioritaire sur le skin de son type (`LOT-42`) — `core::
  TileTextureOverride` sur `Level`/`LevelDraft` (JSON, nettoyage, undo/redo), outil `hmi::
  EditorTool::TextureAssign` avec geste pur (`hmi::TextureAssignGesture`) et raccourci `T`,
  résolveur de priorité **unique** (surcharge > skin > damier) dans `hmi::resolveTileAppearance`,
  partagé par le jeu et l'éditeur. Section « Objets » du panneau Textures (choix d'asset, liste
  des surcharges, retrait, surbrillance croisée) et dossier `Assets/Objects/`. Corrige au passage
  une régression silencieuse survenue lors de la réécriture Qt/Direct3D 11 (`LOT-33`-`35`) : le
  pan/zoom manuel de l'éditeur (molette, glisser bouton droit, touche `0`) avait disparu, la
  caméra recalculant un cadrage automatique à chaque image.

- **LOT-44 — Fond de niveau et versionnement du format** : associe un fond à un niveau (calque
  `Background`, ratio préservé, recadrage par le centre, repli en damier si introuvable) et
  introduit le **numéro de version** du format JSON de niveau — première extension de
  `core::Level`/`LevelDraft` du programme d'habillage, pour que les formats suivants (décors,
  surcharges de texture) puissent évoluer sans casser les niveaux déjà écrits. Section « Fond » et
  sélecteur de jeu de skins **du niveau** (distinct du jeu de skins courant d'édition) ajoutés au
  panneau Textures.

- **LOT-43 — Bibliothèque d'assets à vignettes, gestion de fichiers et rechargement à chaud** :
  le panneau Textures affichait les skins par nom de fichier et imposait de passer par
  l'explorateur puis de relancer l'application à chaque retouche d'asset — un coût payé à chaque
  itération des lots d'habillage suivants. Widget de vignettes partagé (`hmi::
  AssetThumbnailView`), import/renommage/duplication/suppression avec **détection des
  références** (`skins.json`), et rechargement à chaud (invalidation `TextureCache` + relecture du
  catalogue) sans reconstruire la scène ni toucher au brouillon en cours d'édition. La
  mémoïsation/invalidation de `TextureCache` est extraite dans `hmi::CacheRegistry`, un registre
  générique testable sans GPU dont `LOT-40` dépendait déjà sans qu'elle soit vérifiée isolément.

- **LOT-42 — Skins de tuiles, raccords automatiques et panneau Textures** : premier lot de
  contenu visuel du programme d'habillage — le mode Texture (`LOT-41`) cesse d'afficher un damier
  partout.
  - `hmi::SkinCatalog` — associe un type de tuile à un asset et un mode de découpage, en **jeux
    nommés** ; format versionné dès sa création (une version supérieure à celle gérée est refusée
    plutôt que lue au mieux), aucune exception ne franchit la lecture (`EX-NFR-040`, patron de
    `core::LevelLoadResult`).
  - `hmi::TileAutotile` — table de **raccords automatiques** par masque de voisinage solide
    (quatre voisins, seize configurations, planche 4×4) : une image unique par type rendait la
    grille visible et le dessus d'une plateforme indiscernable de son intérieur. L'extérieur du
    niveau compte comme solide et le raccord suit la **solidité**, pas le type (un bloc poussable
    jouxtant un mur ne laisse pas de couture) ; les pentes/arrondis, jamais solides, ne
    participent pas au voisinage.
  - Skins appliqués au rendu via `hmi::TileSkinTag` (composant de présentation, même patron que
    `RenderLayerTag`) et le résolveur unique `hmi::resolveTileAppearance` (priorité skin >
    damier) ; le masque de voisinage est calculé **une fois** à la construction, indépendant du
    mode et du jeu de skins, pour que `F8` ou une réassignation se voient à l'image suivante sans
    reconstruire l'ECS.
  - **Détourage** des skins des douze types à silhouette inclinée/courbe (`hmi::
    isInsideSilhouette`, point de vérité unique partagé avec l'atlas procédural) : un skin fourni
    par l'auteur reste une image carrée, le moteur le découpe à la forme exacte de la hitbox
    (`core::slopeSurfaceHeight`), jamais anticrénelée (filtrage *nearest*, `EX-ARCH-022`).
  - Dock **« Textures »**, organisé en onglets dès ce lot bien qu'il n'en compte qu'un — sans
    cette structure, chacun des lots d'habillage suivants (Fond, Objets, Animations, Décors)
    aurait créé son propre panneau. Fichier et mode choisis dans une liste fermée (balayage de
    `Assets/Skins/`, jamais une saisie de chemin).
  - Palette de l'éditeur rendue **fidèle** au mode de rendu courant (`hmi::paletteThumbnail`,
    fonction pure appliquant exactement la même priorité que `hmi::resolveTileAppearance` —
    vérifié par test qu'elles ne divergent jamais) : texture réelle en mode Texture, couleur plate
    en mode Physique, damier pour un type non habillé.
  - **Portabilité du build** corrigée à l'ouverture du lot : sur une configuration neuve, aucun
    exécutable n'était produit (Qt non découvert par `find_package` hors `CMAKE_PREFIX_PATH`, et
    l'architecture du terminal appelant décidait silencieusement de celle du build, x86 depuis une
    « Developer PowerShell »). `scripts/build.ps1` établit désormais lui-même l'environnement x64
    (`vswhere`), et un garde-fou rejette les deux cas à la configuration CMake.

- **LOT-41 — Bascule Physique/Texture (`F8`)** (`EX-REN-046`) : une commande **fixe et non
  remappable** bascule, en édition, en essai **et** en jeu réel, entre le rendu **Physique**
  (couleur plate par type de tuile — la lecture directe des collisions, comportement historique
  strictement inchangé) et le rendu **Texture** (habillage, construit à partir de `LOT-42`).
  - `hmi::RenderMode` et `hmi::resolveTileAppearance` — **point de résolution unique** de
    l'apparence, appelé à la **composition** et non à la construction de la scène : basculer de
    mode ne reconstruit jamais l'ECS, ne coûte aucun pas de simulation et n'a aucun effet rémanent.
    Le mode Texture affiche pour l'instant le damier magenta partout, ce qui est le comportement
    attendu tant qu'aucun skin n'existe (`EX-NFR-040`). La géométrie composée est **identique**
    dans les deux modes : seule la texture échantillonnée change.
  - `F8` est traité en dur dans `hmi::GameViewport::keyPressEvent`, avant toute autre branche pour
    couvrir les trois contextes — même parti pris que `F10` (grille de repère). La touche
    n'apparaît dans **aucune** table de remappage : `hmi::qtKeyToHmiKey` ne traduit pas
    `Qt::Key_F8`, elle ne peut donc structurellement pas être liée à une action (`EX-CTRL-012`).
  - **Défaut `Texture` dans toutes les configurations de build** et **persistance** du dernier
    choix entre deux sessions (`QSettings`, `EX-IHM-011`) : deux binaires du même code ne doivent
    jamais afficher un rendu différent par défaut, et reperdre son mode d'affichage à chaque
    lancement serait une friction quotidienne pendant treize lots. Une préférence absente, vide ou
    corrompue retombe silencieusement sur `Texture`.
  - `hmi::TextureCache` est désormais **câblé en production** (porté par `hmi::GameViewport`,
    propriétaire du damier partagé et point d'entrée des skins à partir de `LOT-42`).

- **LOT-40 — Fondations du rendu texturé : calques nommés, multi-textures, testabilité, culling**
  (`EX-REN-043`, `EX-REN-007`, `EX-NFR-004`, `EX-NFR-005`, précise `EX-REN-014`) : lot **structurel**
  qui ne change aucun pixel affiché, mais lève les quatre verrous qui bloquaient tout le programme
  d'habillage.
  - `hmi::RenderLayer` — ordonnancement de calques **unique et explicite** (fond, décor, ombres,
    tuiles, objets, personnage, premier plan, interface, aides d'édition), réservé en entier dès
    maintenant ; le premier plan est au-dessus du personnage par construction (`EX-DEC-002`). Les
    valeurs de calque magiques (`layer = 100` du joueur, `0` des tuiles) ont disparu au profit du
    composant de présentation `hmi::RenderLayerTag` ; `core::Sprite::layer` conserve son rôle de tri
    **fin à l'intérieur** d'un calque et `Core` continue d'ignorer les calques (`EX-NFR-011`).
  - **Rendu multi-textures** : la composition regroupe les primitives par `(calque, texture)` et le
    rendu émet une passe `begin/end` par groupe contigu, dans l'ordre des calques. Le contrat public
    de `hmi::SpriteBatch` est strictement inchangé ; ses primitives (`SpriteQuad`, `LineQuad`) sont
    extraites dans `HMI/Graphics/Quad.h`, sans dépendance Direct3D.
  - `hmi::ComposedScene` / `hmi::QuadRecorder` — séparation **composition / soumission** : la liste
    ordonnée des primitives d'une image est produite sans device Direct3D, donc **assertable** par un
    test (ordre des calques, contiguïté des groupes de texture, dénombrement, présence d'une
    primitive). Les critères « rendu identique » et « ordre de calque correct » cessent d'être des
    vérifications à l'œil (`EX-NFR-004`).
  - `hmi::TextureCache` — registre de textures chargées à la demande par **nom logique**, bâti sur
    `hmi::TextureLoader`/`hmi::AssetPaths` (LOT-39), avec `invalidate`/`invalidateAll` dès l'origine
    (prérequis du rechargement à chaud, LOT-43) et mémorisation des échecs pour ne pas relire le
    disque à chaque image.
  - `hmi::validateAsset` — **contrat de dimensions par famille d'asset** (atlas, skin de tuile,
    planche à raccords, fond, objet, spritesheet, décor) : validation **pure**, intercalée entre le
    décodage et l'upload, qui refuse un asset non conforme en journalisant le fichier, la dimension
    trouvée et l'attendue (`EX-REN-007`).
  - `hmi::buildMissingTextureImage` — repli **damier magenta** opaque et déterministe, résolu par le
    point d'appel unique `hmi::resolveOrPlaceholder` : un asset manquant se voit, sans jamais
    interrompre le rendu (`EX-NFR-040`).
  - **Culling par cadrage caméra** (`EX-NFR-005`) : seules les primitives intersectant le cadrage de
    la caméra (`hmi::Camera2D::visibleBounds`), élargi d'une marge d'une case, sont soumises ; le
    volume composé, écarté et soumis est observable en journalisation de diagnostic. Le culling est
    purement visuel — une entité écartée reste simulée (`EX-ARCH-012`).

- **LOT-39 — Textures depuis fichiers + repli procédural** (`EX-REN-041`, `EX-REN-042`) : l'atlas de
  tuiles (`hmi::TextureAtlas`) charge désormais `Assets/atlas.png` (à côté de l'exécutable, copié
  comme `Levels`/`Localization`) via un nouveau loader d'image générique (`hmi::TextureLoader`,
  décodage `QImage` → upload Direct3D 11) et une résolution de chemins pure et testable
  (`hmi::AssetPaths`) ; en l'absence d'asset, repli automatique sur la génération procédurale
  historique (`hmi::buildProceduralAtlasImage`, extraite telle quelle, sans régression), sans jamais
  bloquer le rendu (`EX-NFR-040`). Interface publique de `TextureAtlas` inchangée ; `tile`/
  `playerFrameRegion` rendues `static` (pure arithmétique de grille) et testées sans GPU. Un outil de
  développement (`ProjectGaming.exe --export-atlas=<chemin>`) régénère l'atlas de base depuis la
  génération procédurale de référence.

- **Feuille de route LOT-40 → LOT-55 — Programme d'habillage : textures, animations, décors** :
  cadrage documenté (`Documentation/Lot/LOT-40-*` à `LOT-55-*`) pour la suite du travail sur les
  textures, amorcé après LOT-39 puis **entièrement revu** à la suite d'un audit critique du premier
  cadrage. Aucune implémentation de code à ce stade — uniquement le cadrage
  (`epic.md`/`tache-*.md`) et les exigences correspondantes.

  Séquence : fondations du rendu — registre de textures, calques nommés, culling, testabilité
  (LOT-40) ; bascule Physique/Texture (`F8`, LOT-41) ; skin des tuiles avec jeux de skins et
  raccords automatiques (LOT-42) ; bibliothèque d'assets, import et rechargement à chaud (LOT-43) ;
  fond de niveau et versionnement du format (LOT-44) ; texture par objet interactif (LOT-45) ;
  moteur d'animation piloté par données (LOT-46) ; états visuels des mécanismes (LOT-47) ;
  personnage habillé depuis un fichier (LOT-48) ; décors libres et parallaxe (LOT-49) ; édition des
  décors (LOT-50) ; visibilité par calque (LOT-51) ; texte et affichage tête haute (LOT-52) ; effets
  et particules (LOT-53) ; atelier pixel art intégré (LOT-54) ; ombres du plan physique (LOT-55).

  Écarts par rapport au premier cadrage, issus de l'audit : ajout des **animations** (aucun lot n'en
  prévoyait, alors que portes, plaques et pièges n'ont aujourd'hui qu'une modulation d'opacité pour
  signaler leur état) ; ajout du **personnage**, absent du programme ; ajout des **décors libres et
  du calque de premier plan**, spécifiés de longue date (`decors.md`) mais rattachés à aucun lot ;
  ajout du **rendu de texte** (`EX-REN-032`, jamais implémentée — les budgets de sauts/dashs de
  LOT-12 n'étaient visibles nulle part) ; **raccords automatiques** et **jeux de skins** décidés
  avant la fabrication de `skins.json` plutôt qu'après ; **versionnement** du format de niveau au
  premier changement de schéma ; **culling** et **testabilité du rendu sans GPU** intégrés aux
  fondations ; **rechargement à chaud** avancé avant les couches visuelles ; défaut de rendu unifié
  et persisté au lieu de dépendre de la configuration de build.

  Nouveaux ids : `EX-REN-005` à `009`, `EX-EDIT-024` à `027`, `EX-LVL-005`, `EX-NFR-004`/`005`,
  `EX-DEC-006`, `EX-IHM-003` (en plus de `EX-REN-043` à `046` et `EX-EDIT-042` à `045` du premier
  cadrage). Exigences amendées : `EX-REN-012`, `EX-REN-014`, `EX-REN-032`, `EX-REN-046`,
  `EX-EDIT-042`, `EX-EDIT-044`. Les lots `LOT-40` à `LOT-48`, tous non commencés, ont été
  **renumérotés une seule fois** pour que le numéro suive l'ordre d'implémentation ; l'exception est
  actée dans `Documentation/Lot/lots.md`.

- **LOT-37 — Liens de mécanismes par traits/flèches** (`EX-IHM-030`, `EX-IHM-031`) : la liaison
  déclencheur → cible (interrupteur/plaque → porte, danger commuté) était jusqu'ici signalée par une
  simple teinte de case, illisible au-delà de quelques liens. Chaque liaison est désormais dessinée
  comme une **flèche explicite** dans le viewport (`hmi::DraftRenderer::drawLinks`, nouvelle
  primitive de segment orienté `hmi::LineQuad` sur `hmi::SpriteBatch`), alignée au zoom/pan courant.
  Un **outil « Lien » dédié** (`hmi::EditorTool::Link`, panneau Outils) remplace le geste au clic :
  cliquer un déclencheur passe en attente de cible, cliquer une cible crée la liaison (rejouer la
  paire la supprime) ; `Échap` annule une attente en cours. Plusieurs liens partageant un
  déclencheur s'écartent en **éventail** depuis une base commune plutôt que de se superposer. Un
  **panneau « Liens »** (dock Qt) liste toutes les liaisons du niveau, met en surbrillance celle
  sélectionnée et permet de la supprimer. Géométrie et machine à état du geste
  (`hmi::LinkGeometry`, `hmi::LinkGesture`) sont pures et testées sans GPU/Qt (`EX-NFR-010`) ; le
  modèle de liaison (`core::LevelDraft::linkMechanism`/`unlinkMechanism`) est inchangé.

- **LOT-34 → LOT-38 — Refonte de l'IHM vers Qt** (`EX-IHM-*`, `EX-BUILD-010`) : toute l'interface
  hors-jeu (menu, options, remappage clavier/manette, éditeur de niveaux) est désormais une
  application **Qt 6** (`ProjectGaming`), le **rendu in-game restant Direct3D 11** embarqué dans un
  viewport (`hmi::GameViewport`, `QWindow` + `HWND`). Fenêtres dockables (`QDockWidget` : Palette,
  Outils, Niveaux) à disposition persistée (`QSettings`, `EX-IHM-011`), palette en arbre
  (`QTreeView`), navigateur de niveaux (recherche, créer/renommer/dupliquer/supprimer). Mises en
  page éditables hors code (`.ui`/`.qrc`/`.qss` dans `Source/Elements/UI` et `Themes`). Déploiement
  autonome via `windeployqt` (aucune bibliothèque à installer côté utilisateur).
  **Internationalisation** (`EX-REN-033`) : toute l'IHM Qt (menu, options, éditeur, dialogues,
  palette, remappage) est traduite via `hmi::Localization` (catalogues `fr`/`en`, clés en anglais) ;
  l'onglet **Général** des Options offre le **choix de langue** (retraduction à chaud, persistée dans
  `QSettings`) et un bouton **« Enregistrer les journaux »** (`hmi::saveSessionLog`, build dev).

### Modifié
- **Préparation de la release — dette de documentation et de vérification** : audit complet mené
  avant de poser le tag, et correction de ce qu'il a mis au jour.
  - **Une seule source de vérité pour le numéro de version.** `core::Engine::version()` renvoyait
    `"0.1.0"` en dur et `project(… VERSION …)` valait `0.1.0` — aucun des deux n'avait jamais
    correspondu à un tag publié, seul le `Doxyfile` étant bumpé à chaque jalon. Le `project()`
    racine devient l'unique endroit où le numéro est écrit : il alimente `core::Engine::version()`
    par définition de compilation (`PROJECTGAMING_VERSION`), et `scripts/build_docs.py` — déjà
    exécuté par la CI — **échoue** désormais si le `PROJECT_NUMBER` du `Doxyfile` s'en écarte.
  - **Le garde-fou du Cahier de test en était un à moitié.** `generate_cahier_test.py` ne
    reconnaissait que la forme `TEST(` : les cas de test attachés à une *fixture* (`TEST_F`)
    disparaissaient du cahier **sans le moindre message**, dont l'intégralité de
    `test_image_encode.cpp`. Et `--check` ne comparait le fichier qu'au résultat du script, jamais
    au code : un test jamais documenté n'apparaissait d'aucun côté de la comparaison, donc passait.
    Les trois formes sont maintenant reconnues, et un **contrôle de complétude** échoue en listant
    tout test dépourvu de bloc `\castest{}`.
  - **145 tests sur 943 (15 %) n'étaient dans aucun cahier** — des fichiers entiers issus des
    `LOT-46` à `LOT-50`. Tous documentés : le Cahier de test passe de **790 à 943 cas**. Au passage,
    deux blocs de documentation détachés de leur test dans `test_physique_personnage.cpp` (trois
    blocs empilés devant un seul `TEST`) ont été remis en face du test qu'ils décrivent.
  - **Guide du développeur** : `LOT-54`, `LOT-56` et `LOT-57` n'y avaient aucune couverture. Deux
    pages ajoutées — `guide-atelier-pixel-art.md` et `guide-design-ihm.md` — et `guide-ihm-qt.md`,
    qui s'arrêtait au `LOT-36`, recadré et relié aux deux nouvelles ; références à `hmi::ToolPanel`
    (supprimé au `LOT-57`) corrigées dans `guide-editeur.md`.
  - **README et manuel** : la liste des fonctionnalités décrivait l'état d'avant le programme
    d'habillage ; `HMI/` y était encore « fenêtre Win32 » ; le tableau d'intégration continue
    annonçait un runtime « statique » (faux depuis le `LOT-38` — les DLL Qt imposent `/MD`) et
    ignorait le déclencheur `vX.Y.Z`. Le manuel ne connaissait que la préversion roulante et
    documente désormais les **versions publiées**.
- **LOT-38 (Étape B) — Retrait du legacy & réorganisation** : suppression de l'IHM « maison »
  (écrans `IScreen`/`ScreenManager`, widgets d'éditeur, police bitmap, fenêtre Win32) et de
  l'exécutable historique ; `Source/HMI` devient l'unique cible (`ProjectGaming`), code réparti par
  domaine (`Platform`/`Input`/`Graphics`/`Game`/`Localization`/`Interface`/`Editor`). Documentation
  (guides écrans/éditeur/rendu/entrées) et journalisation mises à jour en conséquence.

## [0.0.4] - 2026-07-27

> Quatrième jalon : **fluidité du moteur** au-dessus de 60 Hz (LOT-33) — les entrées sont désormais
> consommées par **pas de simulation** plutôt que par frame de rendu (plus de perte à haut
> framerate), la présentation passe en **flip-model** (latence entrée → image réduite, cadence plus
> régulière) et le rendu **interpole** la position des entités mobiles entre deux pas fixes, sans
> jamais toucher au déterminisme de la simulation. **541 tests** (540 au jalon précédent).
>
> Voir le [CHANGELOG](CHANGELOG.md) pour le détail (LOT-33).

### Modifié
- **LOT-33 — Fluidité du moteur** (`EX-REN-004`, `EX-ARCH-031`, `EX-CTRL-020`, `EX-CTRL-021`) :
  ensemble de corrections de choix techniques boucle/rendu/entrées qui dégradaient le ressenti
  au-dessus de 60 Hz. **Entrées nerveuses** : les fronts (pressée/relâchée) sont désormais consommés
  par **pas de simulation** et non par frame de rendu — un appui capturé sur une frame réelle sans
  pas (rendu > 60 Hz, ≈ 2 sur 3 à 144 Hz) n'est plus perdu (`hmi::Window::beginInputFrame`, nouvelle,
  appelée après chaque pas ; `pumpMessages` n'avance plus les fronts). Les touches ne restent plus
  « collées » à un `Alt+Tab` (`InputState::releaseAll` sur `WM_KILLFOCUS`). Le sondage `XInputGetState`
  d'un slot **sans manette** — coûteux — est throttlé (une frame sur ~120), supprimant les
  micro-saccades chez un joueur clavier. **Présentation flip-model** (`DXGI_SWAP_EFFECT_FLIP_DISCARD`,
  deux back buffers) à la place du modèle *blt* legacy : latence entrée → image réduite et cadence
  plus régulière (la cible de rendu est reliée au back buffer à chaque `clear()`, le flip model la
  dé-liant à `Present`). **Interpolation de rendu** (`EX-ARCH-031`, prévue dès `LOT-01` mais jamais
  exploitée) : nouveau composant de présentation `hmi::PreviousPosition` (dans le `core::World`,
  écrit/lu par `HMI` seul, `Core` intact), rempli au début de chaque pas ; `hmi::SpriteRenderer`
  dessine les entités mobiles (personnage, dangers mobiles, blocs) à `lerp(précédente, courante,
  alpha)` via `core::FixedTimestep::interpolationAlpha`, supprimant le *judder* à haut framerate. La
  simulation reste strictement déterministe (l'interpolation ne touche que l'affichage) ; la caméra
  n'est pas interpolée (coupure nette par salle, `LOT-32`).

## [0.0.3] - 2026-07-25

> Troisième jalon : le prototype devient un **vrai platformer**. Personnage animé (LOT-17/18) et
> **physique newtonienne** (LOT-19) ; **manette** pleinement supportée et **remappage** complet,
> clavier **et** manette (LOT-20, LOT-29, LOT-30) ; bibliothèque de tuiles de plateforme — pentes,
> arrondis (convexes puis concaves), blocs poussables et blocs à taille fractionnaire, sol **et**
> plafond (LOT-21 à LOT-26, LOT-28) ; **dangers avancés** — directionnels, mobiles, commutés,
> temporisés (LOT-31) ; niveaux à **salles** façon *Celeste* (LOT-32) ; palette de l'éditeur
> réorganisée par catégories (LOT-27) et refactoring complet des niveaux de démonstration (LOT-25).
> **540 tests** (292 au jalon précédent).
>
> Voir le [CHANGELOG](CHANGELOG.md) pour le détail par lot (LOT-17 → LOT-32).

### Ajouté
- **LOT-32 — Niveaux à salles** (`EX-REN-015`, `EX-EDIT-023`) : un niveau plus grand qu'une
  **salle** (nouveau `hmi::RoomGrid`, taille fixe en tuiles, `Source/HMI/Graphics`) se joue avec
  une caméra qui cadre la salle **courante** du personnage, au zoom pixel art natif, et bascule
  **nettement** sur la salle voisine dès qu'il en franchit la frontière — façon *Celeste*, sans
  jamais rapetisser le rendu quelle que soit la taille totale du niveau. Le niveau reste une grille
  de tuiles **unique** (aucun format, aucune nouvelle tuile) : une salle a « plusieurs entrées/
  sorties » simplement parce qu'un couloir reste ouvert sur plusieurs de ses bords vers des salles
  voisines. Un niveau qui tient dans une seule salle se comporte à l'identique du cadrage « niveau
  entier » de LOT-16 (non-régression). Dans l'éditeur, un quadrillage de salles (même bascule
  `F10` que le repère fin existant) aide à aligner les couloirs inter-salles, sans changer son
  cadrage (toujours « niveau entier », pan/zoom manuel). `EX-REN-013` reformulée en conséquence
  (scopée aux niveaux tenant dans une seule salle). Nouveau niveau de démonstration
  `demo-salles.json` (2×2 salles) intégré à la séquence jouée.
- **LOT-31 — Blocs de danger avancés** (`EX-GP-050` à `EX-GP-053`) : quatre nouvelles variantes de
  la tuile `Danger` — directionnel (`DangerUp`/`Down`/`Left`/`Right`, mortel uniquement sur une
  bande étroite du bord désigné plutôt que la case entière), mobile (`DangerMover`, aller-retour
  linéaire déterministe autour de sa position de départ), commuté (`DangerSwitched`, mortel
  uniquement quand l'interrupteur/la plaque de pression qui lui est lié est actif — inverse d'une
  porte) et temporisé (`DangerBlink`, alterne mortel/inoffensif selon une période fixe et un
  déphasage par tuile). Nouveau `core::DangerController` (mobile/temporisé) ; `core::
  MechanismController` étendu (commuté, même détection front/continu qu'une porte) ;
  `core::evaluateOutcome` gagne un paramètre `extraDangerBoxes` pour les dangers à état, assemblés
  par `hmi::GameScreen`. Palette de l'éditeur : catégorie « Piège » restructurée (Classique +
  sous-groupe Directionnel + Mobile/Commuté/Clignotant) ; liaison éditeur au même geste qu'une
  porte. Nouveau niveau de démonstration `demo-dangers-avances.json` intégré à la séquence jouée.
- **LOT-30 — Remappage manette** (`EX-CTRL-002`, `EX-CTRL-012`) : troisième sous-menu dans Options
  (« Touches de la manette »), même patron que LOT-29, listant les six actions de jeu avec le
  bouton XInput actuellement lié (`Up`/`Down`/`Left`/`Right`, `A`/`B`/`X`/`Y`, épaules gauche/
  droite) ; capture, échange sur conflit et réinitialisation identiques au remappage clavier.
  Nécessite une manette connectée pour entrer en capture, sinon affiche une invite dédiée.
  `InputState` gagne une piste brute par `GamepadButton`, indépendante de la fusion clavier/manette
  existante sur `Key` (celle-ci reste réservée à la navigation de menu, jamais remappable) ;
  `PlayerInputMapper` vérifie désormais, par action, la touche clavier **ou** le bouton manette
  liés — chaque source remappable indépendamment de l'autre. Le filet de sécurité de LOT-29
  (retomber sur la touche par défaut du clavier pour ne pas casser la manette) devient inutile et
  est retiré : la manette a maintenant sa propre couche de configuration. Persistance dans la même
  section `Settings/keybindings.json` (nouvelle section `"manette"`).
- **LOT-29 — Remappage des touches** (`EX-CTRL-012`) : deux nouveaux sous-menus dans Options
  (« Touches de jeu », « Touches de l'éditeur »), chacun listant ses actions avec la touche
  actuellement liée ; sélectionner puis confirmer une action capture la touche suivante pressée
  (échange automatique en cas de conflit, jamais de doublon), une entrée « Réinitialiser » restaure
  les valeurs par défaut. Six actions de jeu (Gauche/Droite/Sauter/Dash/Viser haut/Viser bas) et un
  sous-ensemble significatif de neuf actions d'éditeur (Sauvegarder/Annuler/Refaire/Copier/Coller/
  Test rapide/Grille/Aide/Renommer) — pas un remappage exhaustif (navigation de menu, `Ctrl+R`,
  redimensionnement par flèches, `"0"`, `Tab`, Maj+clic restent câblés en dur). **Persistance**
  (première du projet) dans `Settings/keybindings.json`, à côté de l'exécutable. Le panneau d'aide
  de l'éditeur (`F1`) reflète désormais les touches réellement liées. Un filet de sécurité
  (`GameKeyBindings::defaultKey` revérifiée en plus du binding courant) garantissait que la manette
  continue de fonctionner après un remap clavier (`EX-CTRL-002`) tant qu'un vrai remappage manette
  restait hors périmètre — devenu inutile et retiré en `LOT-30`, qui apporte ce remappage manette.
- **LOT-28 — Arrondis concaves** (`EX-GP-007`) : quatre nouvelles tuiles, `ConcaveUpRight`/
  `ConcaveUpLeft` (sol) et `ConcaveDownRight`/`ConcaveDownLeft` (plafond) — une seconde famille de
  quart de cercle, **concave** plutôt que **convexe** (`RoundedUpRight`/`RoundedUpLeft` et leurs
  variantes de plafond, `EX-GP-004`/`EX-GP-006`) : centre du cercle du côté **plein** plutôt que du
  côté creux, courbure inversée (tangente horizontale côté creux, verticale côté plein), utile pour
  un raccord en creux entre deux surfaces perpendiculaires. Réutilise l'infrastructure de suivi de
  surface/silhouette posée par `LOT-22`/`LOT-23`/`LOT-26` — la formule de hauteur
  (`core::slopeSurfaceHeight`, deux nouveaux `case` ; `core::ceilingSlopeHeight`, mapping miroir
  étendu) change comme prévu. Sol **et** plafond dans un seul lot (le plafond ne coûtant qu'un
  mapping de deux lignes, infrastructure déjà générique depuis `LOT-26`). Nouveau sous-groupe de
  palette **Concave**, frère de **Arrondi** sous la catégorie **Tuile** (`LOT-27`) ; rendu avec la
  silhouette réelle en gris, logé dans quatre des cases déjà réservées de l'atlas procédural
  (`TextureAtlas::TILES_PER_SIDE` inchangé à `5`).
- **Corrigé en cours de lot (LOT-28)** : `ConcaveDownLeft` visait initialement la case d'atlas
  `(4,4)`, en réalité réservée au damier de transparence (affichait le damier au lieu de sa
  silhouette) — réassignée à `(2,4)`, authentiquement libre.
- **Corrigé en cours de lot (LOT-28), défaut antérieur à ce lot** : `core::resolveSlopeFollow`/
  `core::resolveCeilingSlopeFollow` sélectionnaient la case à consulter uniquement par le **centre**
  de la boîte du personnage — pour la dernière fraction (environ la moitié de la largeur de la
  boîte) de la largeur de toute case pente/arrondi/concave, ce centre atterrit déjà dans la case
  **voisine** ; si celle-ci est non solide (typiquement deux arrondis/concaves posés côte à côte —
  un arc, une voûte), aucun filet ne rattrape le personnage, qui tombe ou saute au travers sans être
  bloqué. Reproduit sur `SlopeDownRight` (`LOT-26`) avec le même symptôme exact : le défaut était
  **latent depuis `LOT-22`**, jamais exposé faute d'avoir déjà chaîné deux tuiles non solides
  adjacentes (une case solide voisine masquait silencieusement le même défaut via la collision
  classique sur grille). Corrigé en élargissant la sélection aux colonnes réellement couvertes par
  la largeur de la boîte, la colonne centrale gardant exactement son calcul d'origine (aucune
  régression pour tout appelant dont la boîte ne dépasse pas d'une seule colonne pertinente).
- **Corrigé en cours de lot (LOT-28), second défaut plus profond** : le correctif ci-dessus ne
  suffisait pas quand le personnage **marche pendant qu'il saute** — la case qui aurait dû bloquer
  pouvait redevenir invisible sur PLUSIEURS pas consécutifs (pas seulement le précédent) avant que
  le seuil vertical de blocage n'y soit atteint, laissant un saut traverser un plafond incliné/
  courbe malgré le premier correctif. Corrigé en faisant mémoriser à `core::CharacterPhysicsSystem`
  l'étendue horizontale couverte par la boîte depuis le **début de la montée courante**
  (`Player::ascentSweepMinX`/`ascentSweepMaxX`), pas seulement le pas précédent. Vérifié généralisé
  aux pentes linéaires de plafond (`LOT-26`), pas spécifique aux arrondis concaves de ce lot. Zéro
  régression sur la suite complète après les deux correctifs.
- **Corrigé en cours de lot (LOT-28), troisième défaut, distinct des deux précédents** : un saut
  bloqué tout près du bord **fin** (silhouette quasi vide) d'un arrondi concave de plafond se
  retrouvait, un pas après le blocage, téléporté au-dessus du plafond — `core::resolveSlopeFollow`
  interprétait à tort le chevauchement résiduel (bord bas du personnage encore dans la case de
  plafond après un blocage par en dessous) comme un atterrissage sur la face du haut de la tuile.
  Corrigé en exigeant que le bord bas ait déjà été au-dessus de la case avant le pas pour qu'un tel
  calage soit accepté (garde-fou restreint aux tuiles de plafond, sans effet sur le calage normal
  d'un atterrissage sur sol plat). Nouvelle infrastructure de journalisation réutilisable
  (`Source/Core/Physics/PhysicsLog.h`, macros `PHYSICS_LOG_*`, réservées aux événements rares) pour
  faciliter le diagnostic d'anomalies similaires à l'avenir. Zéro régression sur la suite complète
  après les trois correctifs.
- **Corrigé en cours de lot (LOT-28)** : `slopeShapePixel` (`TextureAtlas.cpp`) échantillonnait au
  **centre** de chaque pixel, qui n'atteint jamais exactement les bords `0`/`1` de la case (dernier
  pixel plafonné à `≈0,969`) — sans conséquence pour les pentes/arrondis existants, mais visible
  comme une encoche près du bord **plein** (tangente raide) d'un arrondi concave, à l'endroit
  précis où la silhouette doit au contraire être la plus pleine. Corrigé en échantillonnant au bord
  des pixels de coin plutôt qu'à leur centre. Vérifié en jeu : deux arrondis concaves adjacents
  (arche/pic) se traversent désormais en marchant sans chute, silhouette rendue comme un pic net.
- **LOT-27 — Palette de l'éditeur organisée par catégories** (`EX-EDIT-018`) : la palette de
  tuiles, jusqu'ici une liste plate de 19 types, devient un **accordéon à trois niveaux**. Premier
  niveau : deux entrées autonomes toujours visibles (Vide, Piège — ex-Danger, renommé à l'affichage
  seulement) et trois catégories repliables (Tuile, Interactif, Jalon). Deuxième niveau : une
  catégorie dépliée expose ses tuiles directes (ex. Porte, Plaque, Interrupteur) et, pour Tuile/
  Interactif, des **sous-groupes** repliables (Pente, Arrondi, Bloc poussable — familles à
  plusieurs formes/tailles). Troisième niveau : un sous-groupe déplié expose ses variantes
  (orientations, tailles). La hauteur du panneau étant désormais variable, `TilePalette::bottom()`
  remplace le compte fixe (`EditorLayout::PALETTE_TYPE_COUNT`/`TOOLBAR_TOP`, supprimés) ;
  `ToolBar::relayout(top)` repositionne la barre d'outils juste sous la palette à chaque frame,
  quel que soit son état de dépliage. Un mockup HTML/CSS interactif a été itéré avec le demandeur
  (position du dépliage, choix des quatre catégories, troisième niveau d'accordéon) **avant**
  l'implémentation. **Défilement** ajouté après une première revue (« n'est pas complet ») : tout
  déplier en même temps (25 lignes au maximum) pouvait dépasser la hauteur de fenêtre disponible,
  rendant les dernières entrées définitivement inaccessibles à la souris — `TilePalette` expose
  désormais une **fenêtre visible** (`scroll`/`setViewportHeight`), la molette au-dessus du panneau
  latéral la fait défiler (plutôt que de zoomer la caméra), et une barre de défilement (piste +
  curseur, même principe que `LevelPicker`, `LOT-15`) apparaît si le contenu déplié déborde.
  Replier/déplier un en-tête ne le fait **jamais** disparaître de la fenêtre
  (`TilePalette::followRow`, suit automatiquement l'en-tête tout juste basculé — sans quoi le
  défilement aurait simplement déplacé le problème plutôt que de le résoudre). **11 nouveaux
  tests** (10 `TilePalette`, 1 `ToolBar`), aucune régression (406/406 tests verts).

### Corrigé
- **Blocs poussables traversant les pentes/arrondis** (`EX-GP-003`/`EX-GP-004`/`EX-GP-006`/
  `EX-GP-022`) : `BlockController` ne connaît que la solidité statique (`core::isSolid`) pour
  décider si une case est libre — or une pente/arrondi (sol ou plafond) n'est **jamais** solide
  (sa collision passe par un suivi de surface propre au personnage, que `BlockController` ignore
  entièrement). Un bloc suspendu au-dessus d'une pente la traversait donc en tombant, et pouvait
  être poussé dedans. `isFree` traite désormais une case de pente/arrondi comme un **obstacle
  simple** (comme une case solide), cohérent avec le modèle case-par-case des blocs. **2 nouveaux
  tests**, aucune régression (395/395 tests verts).
- **Sélecteur de niveau de l'éditeur illisible au-delà d'une poignée de fichiers** (`EX-EDIT-001`) :
  la liste (« Nouveau niveau » + fichiers `.json` du dossier) s'affichait entièrement, sans
  défilement — au-delà de la hauteur de fenêtre (13 niveaux démo depuis `LOT-25`), les entrées
  en trop restaient invisibles et donc inaccessibles à la souris. `LevelPicker` ne dessine
  désormais que la fenêtre visible (le défilement suit la sélection au clavier, ou la molette
  sans changer la sélection), avec une barre de défilement (piste + curseur) indiquant la
  position dans la liste. 5 nouveaux tests.
- **Pentes/arrondis en jeu dans des couleurs disparates** (`EX-GP-003`/`EX-GP-004`) : la forme
  suit désormais la hitbox réelle (triangle/courbe, correction précédente) mais chaque variante
  gardait sa propre teinte (vert sarcelle, vieux rose, bleu violet, magenta) — incohérent avec un
  simple matériau de plateforme. Rempli en gris (même couleur que `Solid`), comme les blocs
  réduits (`BlockHalf`/`BlockQuarter`, déjà gris) ; la forme (triangle/courbe) reste inchangée.
- **Blocs réduits affichés à taille pleine dans l'éditeur** (`EX-GP-005`) : le canevas de l'éditeur
  (grille du niveau en cours d'édition) dessinait `BlockHalf`/`BlockQuarter` en carré plein
  (1×1 case), sans rapport avec leur boîte de collision réelle (`×0.5`/`×0.25`, centrée) — l'éditeur
  devait rester un aperçu **fidèle** de ce que le jeu affiche, pas une approximation. Réutilise
  désormais la même formule que `core::BlockController::boxAt` (nouvelle fonction partagée
  `core::tileVisualScale`, `Core/Levels/TileType.h`) pour centrer et mettre à l'échelle le sprite,
  côté éditeur **et** côté jeu (`GameScreen::refreshBlockVisuals`, inchangé) — aucune divergence
  possible entre les deux par construction. Les pentes/arrondis, déjà fidèles à leur forme réelle
  (correction précédente), sont inchangés.
- **Pentes et arrondis affichés comme des carrés pleins** (`EX-GP-003`/`EX-GP-004`) : l'atlas
  procédural peignait ces quatre tuiles d'une simple couleur plate, sans rapport avec leur
  hitbox réelle (surface inclinée ou courbe, `core::slopeSurfaceHeight`) — visuellement un mur
  plein là où le personnage pouvait en fait marcher en diagonale au-dessus du vide. `TextureAtlas`
  peint désormais ces quatre cases avec un masque de forme (opaque sous la surface suivie par la
  physique, transparent au-dessus), calculé avec la **même** fonction que la physique : l'affichage
  correspond par construction à la hitbox, sans pouvoir diverger. `hmi::slopeTileGridPosition`
  (`HMI/Graphics/TileVisuals.h`) devient la source de vérité unique des coordonnées d'atlas de ces
  quatre tuiles, partagée entre le rendu (couleur) et la génération du masque.
- **Cahier de test illisible (page unique, 321 cas à plat)** : la page agrégeait tous les cas de
  test sur un seul niveau (mécanisme Doxygen `\xrefitem`, sans aucune section). Remplacée par
  `Documentation/CahierTest.md`, généré par le nouveau `scripts/generate_cahier_test.py` à partir
  des mêmes blocs `\castest{...}` (source de vérité inchangée), structuré selon l'arborescence de
  `Source/Test/` (Tests unitaires par module Core/HMI, Tests d'intégration et Système par fichier)
  — navigable via l'arbre latéral comme toute autre page. Vérifié en CI (`--check`).
- **Accumulateur à pas de temps fixe non utilisé dans `main`** (`EX-NFR-002`) : depuis
  l'intégration du menu principal (LOT-06), la boucle appelait `ScreenManager::update` une seule
  fois par frame réelle avec un delta constant, sans jamais mesurer le temps réel écoulé ni
  appeler `core::FixedTimestep::advance` — la simulation ne restait déterministe que par
  coïncidence, tant que le V-Sync cadençait l'affichage à 60 Hz. La mesure du temps réel
  (`std::chrono::steady_clock`) et la boucle d'accumulateur (rattrapage de plusieurs pas sur une
  frame lente, plafonné par `maximumStepsPerCall`) sont restaurées, conformément à
  `Documentation/Guide/guide-boucle.md`.
- **Sélection de niveau à la souris** (`hmi::LevelPicker`) : le sélecteur de niveau de l'éditeur
  (« Choisir un niveau ») ne répondait qu'au clavier (`↑`/`↓`/`Entrée`) — le survol et le clic
  gauche sélectionnent et confirment désormais un choix, comme le menu principal
  (`hmi::MenuModel`, même mise en page à chasse fixe).
- **Séparateur `*` accepté pour la taille de grille** (`hmi::parseLevelSize`, `Ctrl+R`) : en plus
  de `x`/`X`, un niveau peut être redimensionné en tapant `largeur*hauteur` (ex. `60*40`).

### Ajouté
- **LOT-26 — Pentes/arrondis de plafond** (`EX-GP-006`) : quatre nouvelles tuiles,
  `SlopeDownRight`/`SlopeDownLeft`/`RoundedDownRight`/`RoundedDownLeft` — miroir vertical exact des
  pentes/arrondis de sol existants (`EX-GP-003`/`EX-GP-004`, matière pleine en haut de la case
  plutôt qu'en bas). Comme leurs équivalents de sol, elles ne sont **jamais solides**
  (`core::isSolid`) pour la grille classique : leur collision est résolue par une passe de suivi
  dédiée, miroir de `resolveSlopeFollow` (`core::resolveCeilingSlopeFollow`, déclenchée en
  **montant** plutôt qu'en tombant) — le personnage ne peut **jamais franchir** leur silhouette en
  sautant (bonk précis contre le profil incliné/courbe réel, pas une case pleine uniforme), mais ne
  « marche » jamais dessus non plus (pas de déplacement latéral calé sous un plafond). Leur **face
  du haut**, toujours plate au sommet de la case, supporte normalement un personnage qui tombe
  dessus **par au-dessus** (`core::slopeSurfaceHeight` étendu, hauteur constante `0`, réutilise
  `resolveSlopeFollow` tel quel — sans quoi il serait tombé au travers). Réutilise
  `core::slopeSurfaceHeight`/`core::resolveSlopeFollow` de la variante de sol miroir (aucune formule
  dupliquée, comparaison inversée). Grille de tuiles de l'atlas procédural agrandie
  (`TextureAtlas::TILES_PER_SIDE` `4→5`) pour loger les quatre nouvelles silhouettes, sans décaler
  les couleurs des tuiles existantes (jeu de couleurs recalé explicitement). Placeables depuis la
  palette de l'éditeur (19 types désormais). **12 nouveaux tests** (géométrie miroir, classification
  `isSolid`/`isFollowableSurface`, aller-retour JSON, physique de blocage par en dessous et de
  support par au-dessus), aucune régression (393/393 tests verts).
- **LOT-25 — Refactoring complet des niveaux démo** (`EX-GP-003`/`EX-GP-004`/`EX-GP-005`/
  `EX-GP-015`/`EX-GP-016`/`EX-GP-017`/`EX-GP-020`/`EX-GP-022`/`EX-GP-024`/`EX-GP-025`) : les
  anciens niveaux (`demo.json`…`demo5.json`, accumulés au fil des lots sans repasse d'ensemble —
  `demo5.json`, en particulier, était chargé en jeu mais absent du test système) sont supprimés et
  remplacés par **13 niveaux**, un par mécanique (ou petit groupe cohérent), plus un niveau final
  qui les combine (dash, pente, bloc poussable, interrupteur/porte, double saut). `Source/HMI/
  main.cpp` et `Source/Test/Systeme/test_parcours_complet.cpp` chargent désormais exactement la
  même liste, dans le même ordre — un nouveau script CI, `scripts/check_demo_sequence.py`, échoue
  si les deux divergent de nouveau. La plaque de pression (`EX-GP-025`) a nécessité une conception
  particulière : une géométrie plaque/porte/sortie empilées verticalement s'est révélée
  infranchissable par construction (la porte se referme dès que la boîte du personnage — plus
  petite qu'une case — quitte la plaque, avant d'avoir fini de la traverser) ; le niveau livré
  ouvre à la place une porte au-dessus d'un mur d'une case, franchie par un saut pendant la fenêtre
  où elle est ouverte. **2 nouveaux tests** (franchissement et nécessité du saut), aucune
  régression (376/376 tests verts).
- **LOT-24 — Blocs à taille fractionnaire** (`EX-GP-005`) : deux nouvelles tuiles, `BlockHalf`/
  `BlockQuarter` (`×0.5`/`×0.25` d'une case), gérées par `core::BlockController` au même titre que
  `Block` (poussée/chute identiques, toujours case par case) mais avec une boîte de collision
  **réelle** plus petite et **centrée** dans leur case. La grille classique (`sweepAabb`) ne peut
  pas représenter une occupation partielle de case : nouvelle routine `core::sweepAabbVsAabb`
  (`Core/Physics/AabbVsAabb.h`, balayage continu boîte-contre-boîte, même méthode de clamp direct
  que `SweptCollision.cpp`), composée par `hmi::GameScreen::update` **après** le balayage sur
  grille — la restriction la plus stricte des deux l'emporte toujours. Rendu à l'échelle et centré
  dans l'éditeur, avec exactement la même formule que la collision (aucune divergence possible
  entre le sprite affiché et la boîte testée). **14 nouveaux tests** (unitaires et intégration),
  aucune régression (364/364 tests verts).
- **LOT-23 — Collision arrondie** (`EX-GP-004`) : deux nouvelles tuiles, `RoundedUpRight`/
  `RoundedUpLeft`, variante en **quart de cercle** des pentes de LOT-22 (`h(x) = 1 - sqrt(1 - (1 -
  x)²)` et sa symétrique). Réutilise intégralement l'infrastructure de suivi de surface posée par
  LOT-22 (`core::resolveSlopeFollow`, la correction du balayage horizontal) : un nouveau `case`
  dans `core::slopeSurfaceHeight`/`core::isFollowableSurface` suffit, aucune autre passe n'a été
  modifiée. Palette de l'éditeur (`PALETTE_TYPE_COUNT` 11→13) et couleurs plates distinctes des
  pentes. **7 nouveaux tests** (unitaires et intégration), aucune régression (350/350 tests verts).
- **LOT-22 — Pentes réelles** (`EX-GP-003`) : deux nouvelles tuiles, `SlopeUpRight`/`SlopeUpLeft`
  (pente à 45°, montée pleine sur toute la largeur d'une case), disponibles dans l'éditeur. Jamais
  solides pour le balayage classique (`core::isSolid`) — une nouvelle passe de résolution
  (`core::resolveSlopeFollow`, `Core/Physics/SlopeGeometry.h/.cpp`) cale la position verticale du
  personnage sur la surface de la pente à chaque pas fixe, sauf en cas de saut volontaire (vitesse
  verticale négative). Corrige au passage un piège découvert par les tests d'intégration : le
  balayage horizontal (`sweepX`) traitait comme un mur tout bloc plein partageant une ligne que le
  personnage chevauche déjà en suivant une pente (raccord pente → palier, le cas le plus courant) —
  généralise le principe de la « peau » (`kSkin`) déjà en place pour le sol plat
  (`rowIsSlopeGround`, `Core/Physics/SweptCollision.cpp`). **12 nouveaux tests** (unitaires et
  d'intégration), aucune régression sur la physique existante (343 tests, tous verts).
- **LOT-21 — Bloc poussable** (`EX-GP-022`), nouvelle tuile disponible dans l'éditeur.
  `core::BlockController` (nouveau, `Core/Gameplay`) résout chaque pas fixe, avant la physique du
  personnage : **poussée** horizontale d'une case si la case suivante est libre (ni mur, ni autre
  bloc), et **chute** discrète (une case toutes les `FALL_INTERVAL_STEPS` pas) si le bloc n'est
  plus soutenu par le dessous. Un bloc occupe toujours exactement une case entière, comme les
  autres mécanismes de ce moteur — jamais de position intermédiaire. Un bloc posé sur une plaque
  de pression ne l'active pas encore (évolution à venir, voir `Documentation/Guide/guide-niveaux.md`).
  **10 nouveaux tests unitaires** (331 au total, 321 au jalon précédent).
- **Journalisation étendue pour le diagnostic** : trois nouvelles catégories de log
  (`Core/Gameplay/GameplayLog.h`, `Core/Levels/LevelsLog.h`, `HMI/Editor/EditorLog.h`), sur le
  modèle déjà établi (voir `Documentation/Guide/guide-journalisation.md`). Journalise désormais les
  bascules d'état des mécanismes (interrupteur/plaque de pression), le chargement d'un niveau et
  chaque raison d'échec de validation (`Core/Levels/LevelLoader`), les actions de l'éditeur
  (liaison/déliaison de mécanisme, annuler/refaire, redimensionnement), le nombre de niveaux trouvés
  par le sélecteur, et la connexion/déconnexion de la manette — toujours en dehors des chemins
  exécutés à chaque frame ou à chaque pas fixe (uniquement sur changement d'état réel).
- **LOT-20 — Manette et menu d'options** (`EX-CTRL-002`). Le jeu, le menu et l'éditeur sont
  désormais jouables/navigables à la **manette** (XInput) : D-pad/stick gauche pilotent les mêmes
  directions que les flèches/`Q`/`D`, **A** valide (menu) et saute (jeu), **B**/**Start**
  reviennent/échappent, l'épaule droite dashe. La manette est **fusionnée** dans `InputState` au
  niveau des mêmes touches déjà lues par tout le code existant (deux sources indépendantes
  combinées en lecture, jamais en écriture, pour ne jamais effacer une touche clavier réellement
  maintenue) : aucun consommateur (`MenuModel`, `PlayerInputMapper`, raccourcis de l'éditeur…) n'a
  été modifié. Nouveau **menu d'options** (4ᵉ entrée du menu principal) : bascule **V-Sync**
  (jusqu'ici fixée en dur), bouton de langue, état de connexion de la manette, retour au menu.
  **12 nouveaux tests** (321 au total, 309 au jalon précédent) ; vérifié visuellement dans
  l'application (menu, options, bascule V-Sync/langue).
- **LOT-19 — Physique newtonienne et plaque de pression** (`EX-GP-019`, `EX-GP-025`). Deux
  évolutions liées par le **poids** du personnage : la chute suit désormais un modèle
  **newtonien** — masse (`core::Player::mass`) et traînée proportionnelle à la vitesse, dont
  l'équilibre fait **émerger** une vitesse terminale progressive plutôt qu'un plafond arbitraire
  (`std::min`) ; calibrée pour retomber sur l'ancienne vitesse terminale (25 unités/s) à masse par
  défaut, seule la **courbe** change, pas le résultat final. La montée du saut reste inchangée
  (ressenti LOT-11 non affecté). Nouveau mécanisme de puzzle, la **plaque de pression**
  (`TileType::PressurePlate`) : ouvre la porte liée tant qu'un poids suffisant y repose,
  la referme dès qu'il en part — activation **continue**, à la différence de l'interrupteur à
  bascule (conservé tel quel). Réutilise l'infrastructure de liaison existante (identifiant/
  `opensWith`, éditeur Maj+clic généralisé aux deux types de déclencheur). Niveau de démonstration
  `demo5.json` ajouté à la séquence de jeu. **8 nouveaux tests** (305 au total, 297 au jalon
  précédent) ; vérifié visuellement dans l'application (courbe de chute, plaque de pression,
  éditeur).
- **LOT-18 — Animation du personnage** (`EX-REN-012`). Le personnage anime désormais trois
  **clips** dérivés de son état physique (`core::Player::grounded`, `core::Velocity`) : `Idle`
  (repos, 2 images, au sol et immobile), `Run` (course, 4 images, au sol en mouvement), `Jump`
  (saut, 1 pose fixe, en l'air) — un nouveau composant `core::Animation` et
  `core::AnimationSystem` (logique pure, `Core`, testée sans GPU) déterminent le clip et l'image
  courante chaque pas fixe, **après** `CharacterPhysicsSystem` (qui vient de calculer `grounded`
  pour ce même pas). Côté rendu, `TextureAtlas` expose une grille de 7 images (16×16, toujours
  **carrées** — non-régression du bug d'échelle de LOT-17) et `GameScreen` met à jour la région du
  sprite **à chaque frame** (plus seulement au spawn). Second des deux lots dédiés au personnage
  (`EX-REN-012`, après la silhouette statique de LOT-17). **5 nouveaux tests** d'intégration
  (297 au total, 292 au jalon précédent) ; vérifié visuellement dans l'application (repos, course,
  chute/saut).
- **LOT-17 — Sprite du personnage (statique)** (`EX-REN-011`). Le personnage n'est plus une tuile
  de couleur unie (`_atlas.tile(1, 1)`, cyan) : il affiche désormais une **silhouette humanoïde**
  (tête, cheveux, torse/manches, mains, jambes, chaussures) générée en code, comme le reste de
  l'atlas — aucun fichier image, aucune dépendance externe. La région reste **carrée** (16×16,
  comme une tuile) : c'est l'échelle du `Transform` (`core::playerSize()`, déjà non uniforme,
  0,4×0,8) qui donne au personnage sa proportion finale deux fois plus haute que large — une région
  déjà non carrée aurait doublé cet effet et fait déborder la silhouette de la boîte de collision.
  Elle vit dans la **même** texture que les tuiles (`TextureAtlas`, agrandie d'une bande sous la
  grille), plutôt que dans une classe séparée façon `SaveIcon`/`FlagIcons` : `SpriteRenderer` ne
  dessine qu'une seule texture
  par passe, étendre l'atlas existant évite toute restructuration du rendu. Premier de **deux
  lots** : ce lot livre une **pose statique unique** ; l'animation par séquence d'images
  (repos/course/saut, `EX-REN-012`) est explicitement un lot séparé à venir. Vérifié visuellement
  dans l'application ; `ctest` inchangé (292/292, aucune logique `Core` nouvelle — génération de
  texture non testable hors GPU).

## [0.0.2] - 2026-07-22

> Deuxième jalon : **éditeur de niveaux intégré**, du prototype (LOT-14) à l'outil de production
> (LOT-15 : nommage, garde-fous contre la perte de travail, caméra manuelle, outils de zone,
> panneau latéral, découvrabilité) puis aux grands niveaux (LOT-16 : saisie directe de taille,
> caméra qui cadre un niveau plus grand que la fenêtre). Un level designer peint, lie des
> mécanismes, redimensionne, annule/refait, enregistre et teste un niveau sans écrire de code —
> guide non-codeur inclus pour partager les niveaux via Git sans ligne de commande. **292 tests**
> (215 au jalon précédent), CI verte.
>
> Voir le [CHANGELOG](CHANGELOG.md) pour le détail par lot (LOT-14 → LOT-16).

### Ajouté
- **LOT-16 — Niveaux de grande taille** (`EX-EDIT-017`, `EX-REN-013` corrigée). L'éditeur permet
  désormais de **saisir directement** une taille cible (**Ctrl+R**, format `largeurxhauteur`, ex.
  `60x40`) plutôt que d'incrémenter case par case aux flèches (toujours disponibles pour l'ajustement
  fin) — un **plafond généreux** (100 cases par axe, garde-fou d'usage côté `HMI`, sans limite dans
  `Core`) s'applique aux deux voies, qui passent toutes deux par le même point de redimensionnement
  (`EditorScreen::requestResize`, même confirmation destructrice qu'avant si la nouvelle taille
  perdrait l'entrée/la sortie/une liaison). Le champ de saisie du nom (LOT-15) est généralisé pour
  porter aussi cet usage — un seul mécanisme de saisie de texte, pas un second construit en
  parallèle. **Un niveau plus grand que la fenêtre reste entièrement visible** : la caméra, qui ne
  descendait jamais sous le zoom ×1 (une partie de la grille restait hors champ, dans l'éditeur
  comme en jeu), zoome désormais **en dessous de ×1** quand c'est nécessaire pour cadrer le niveau
  entier — correction unique (`Camera2D::fitZoom`, testée) partagée par l'éditeur et `GameScreen`,
  sans dupliquer le calcul ; le zoom reste entier (netteté pixel art) pour tout niveau tenant déjà
  dans la fenêtre, aucune régression sur les niveaux livrés à ce jour. `EX-REN-013` reformulée pour
  refléter la stratégie réellement implémentée (caméra qui cadre le niveau, ne suit pas le
  personnage — la formulation d'origine ne correspondait déjà plus au code depuis LOT-08). Couvert
  par tests **unitaires** (`LevelSizeValidation`, `Camera2D::fitZoom`) ; vérifié manuellement dans
  l'application (grand niveau entièrement visible à l'ouverture et en essai immédiat, niveaux
  existants inchangés).
- **LOT-15 — Éditeur de niveaux : robustesse et confort d'édition** (`EX-EDIT-009`, `EX-EDIT-012` à
  `EX-EDIT-016`). L'éditeur intégré (LOT-14) se rapproche d'un outil de production : créer un
  niveau **demande un nom** (plus de collision silencieuse sur « Nouveau niveau.json ») et **F2**
  le renomme en cours d'édition ; enregistrer sous un nom qui **écraserait** un autre fichier, un
  **redimensionnement** qui supprimerait l'entrée/la sortie/une liaison, ou **Échap** avec des
  modifications **non enregistrées**, sont désormais **confirmés** avant d'agir. La caméra se
  **déplace** (glisser bouton droit) et **zoome** (molette) indépendamment du cadrage automatique
  (« 0 » y revient), bornée entre ce cadrage (rien à voir au-delà du niveau) et un maximum de
  **4 cases visibles** sur le plus petit axe (précision suffisante). Une **grille de repère**
  (lignes fines sur chaque bord de case) bascule au clavier (`F10`) pour simplifier le repérage
  d'une case avant d'y peindre. Deux nouveaux outils au-delà du pinceau : **Rectangle**
  (remplissage d'une zone glissée) et **Sélection** (`Ctrl+C`/`Ctrl+V`, copier/coller une zone de
  tuiles) — `Tab` fait défiler Pinceau/Rectangle/Sélection, la liaison de mécanismes (`Maj`+clic)
  restant disponible quel que soit l'outil actif. Découvrabilité : la palette et la barre d'outils
  rejoignent un **panneau latéral** vertical fixe (au lieu de bandes empilées pouvant se
  superposer entre elles ou avec la grille), avec un libellé à côté de chaque entrée ; un aperçu
  des raccourcis (`F1`) ; et des liaisons interrupteur↔porte teintées **différemment par
  interrupteur** (au lieu d'une seule teinte cyan partagée).
  Côté dette technique : l'essai immédiat (`P`) transmet désormais le niveau **directement en
  mémoire** à une session de jeu interne (plus de fichier temporaire partagé), et les messages
  d'erreur de validation s'appuient sur un **code d'erreur catégorisé** (`LevelValidationError`)
  plutôt que sur une recherche de sous-chaîne dans le message technique. `LevelDraft::paintRegion`
  (remplissage/collage) réutilise la sémantique cellule-par-cellule de `paintTile`, sans dupliquer
  de règle de niveau (`EX-EDIT-010`), en un seul snapshot undo par opération. Couvert par tests
  **unitaires** (`LevelDraft::paintRegion`/`wouldResizeDropContent`, `TextInputField`,
  `LevelNameValidation`, `ToolBar`, `InputState` — molette/texte tapé, `LevelLoader` — codes
  d'erreur) ; vérifié manuellement dans l'application pour la caméra et le glisser-déposer des
  outils de zone (logique non testable hors GPU, comme le reste d'`EditorScreen`/`GameScreen`).
- **LOT-14 — Éditeur de niveaux intégré** (`EX-EDIT-001` à `EX-EDIT-011`, `EX-EDIT-020` à
  `EX-EDIT-022`, `EX-EDIT-030`/`031`). Le menu **« Mode Édition »** ouvre désormais un véritable
  éditeur, intégré à l'application (même exécutable, même rendu Direct3D 11 que le jeu) : un
  sélecteur propose **« Nouveau niveau »** ou l'un des fichiers déjà enregistrés ; la grille se
  peint à la **souris** depuis une **palette** de tuiles (vide, solide, danger, entrée, sortie,
  interrupteur, porte) ; **Maj**+clic lie un interrupteur à une porte (répéter la paire la délie) ;
  les **flèches** redimensionnent la grille ; **Ctrl+Z**/**Ctrl+Y** annulent/refont n'importe
  quelle mutation (pile d'historique portée par `core::LevelDraft`, LOT-14) ; **Ctrl+S** valide
  puis enregistre le niveau (message clair, non technique, en cas d'échec — aucun fichier écrit) ;
  la touche **P** lance un **essai immédiat** du niveau en cours d'édition dans une session de jeu
  **intégrée** (Échap y met fin et restitue l'éditeur intact, brouillon et historique compris).
  Toute la logique de modèle/sérialisation/validation (`core::LevelWriter`, `core::LevelDraft`)
  vit dans `Core`, pure et testée sans GPU, et réutilise **sans duplication** la validation de
  `LevelLoader` (le brouillon se convertit en niveau en repassant par le même chemin que le
  chargement d'un fichier). Nouveau guide non-codeur (`Documentation/Manuel/partager-un-niveau.md`)
  expliquant, sans ligne de commande, comment créer un niveau et le partager via GitHub Desktop.
  Couvert par tests **unitaires** (`LevelWriter`, `LevelDraft` — mutateurs, undo/redo,
  `LevelPicker`, `TilePalette`) et un test **système** (parcours complet d'édition : peindre, lier
  un mécanisme, annuler, redimensionner, enregistrer, recharger, vérifier que le niveau produit est
  directement jouable). Vérifié manuellement dans l'application (peinture, palette, liaisons,
  redimensionnement, undo/redo, enregistrement sur disque, essai immédiat).

### Corrigé
- **`F10` (éditeur, LOT-15) ne déclenchait rien.** Win32 délivre cette touche (comme les
  combinaisons `Alt`+quelque-chose) via `WM_SYSKEYDOWN`/`WM_SYSKEYUP`, jamais
  `WM_KEYDOWN`/`WM_KEYUP` — une convention historique d'activation du menu, indépendante de
  l'absence de menu dans cette fenêtre. `Window::handleMessage` capture désormais aussi ces deux
  messages (comme les autres touches), en laissant `DefWindowProcW` traiter le comportement
  système par défaut (`Alt+F4`, `Alt+Tab`) — sauf pour `F10` lui-même, absorbé pour éviter
  l'activation visuelle, inutile ici, du (non-)système de menu au relâchement. Découvert lors d'un
  essai interactif par l'utilisateur.

## [0.0.1] - 2026-07-18

> Premier jalon : base stable du **moteur physique 2D** (personnage humanoïde, gravité asymétrique,
> saut/double-saut/wall-jump/dash, niveaux JSON avec mécanismes puzzle et budget de mouvements),
> **215 tests** verts, documentation consolidée (Guide du développeur + Cahier de test).

### Ajouté
- **LOT-13 — Consolidation de la documentation** : mise à jour des pages Doxygen et du `README.md`
  (section « Fonctionnalités du moteur », cibles de test unitaire/intégration/**système**). Nouvelle
  rubrique **Guide du développeur** (`Documentation/Guide/`) : accueil + 6 pages par domaine
  (physique, boucle de simulation, ECS, mathématiques, niveaux, entrées) expliquant chaque notion du
  moteur avec liens **code ↔ mathématiques** (balayage AABB, gravité, normalisation…). Nouvelle
  rubrique **Cahier de test** générée depuis les sources via une balise Doxygen avancée `\castest`
  (alias + `\xrefitem` agrégeant une page unique) : les **215 cas** (unitaires, intégration, système)
  sont annotés (titre, **catégorie**, sous-catégorie, **criticité**, **étapes** détaillées, **résultat
  attendu**), la documentation vivant **à côté du test** pour la maintenabilité. Aucun changement de
  comportement moteur.
- **LOT-12 — Niveau puzzle : mécanismes interrupteur/porte + budget de mouvements** (`EX-GP-020`, `EX-GP-021`, `EX-GP-024`). Les liaisons interrupteur↔porte (chargées depuis le LOT-07 mais inertes) deviennent **fonctionnelles** : un `MechanismController` (Core, pur) **bascule** l'état d'une porte au **contact** de son interrupteur (front) ; une porte **fermée** est **solide** (bloque), **ouverte** franchissable. La physique consomme une **grille de collision** reflétant l'état des portes (la carte du niveau reste la source de vérité). Nouveau **budget de mouvements par tableau** : un niveau peut limiter le nombre de **sauts** et/ou de **dashs** (`jumpBudget`/`dashBudget` dans le JSON, `-1` = illimité) ; à budget épuisé, l'action est **refusée**, et le budget est **réinitialisé** au (re)chargement. Le `GameScreen` exécute les mécanismes, initialise le budget au spawn, recharge le niveau à l'échec (mécanismes + budget remis) et **teinte** les portes selon leur état. Niveau puzzle `demo4.json` (interrupteur ouvrant la porte vers la sortie, budget serré) ajouté à la séquence. Couvert par tests **unitaires** (mécanismes, budget parsing), **intégration** (budget refusé au-delà, porte fermée bloque, `demo4` franchissable via l'interrupteur) et **système** (les 4 niveaux enchaînés).
- **LOT-11 — Ressenti avancé : personnage humanoïde, gravité asymétrique, finitions** (`EX-GP-018`). Le personnage n'est plus un carré 1×1 mais une **silhouette humanoïde** (0,4 × 0,8, collision **et** rendu), centrée dans la tuile d'entrée au spawn (`Core/Physics/PlayerSpawn.h`, source unique taille/placement). La **gravité** devient **asymétrique** : chute plus rapide que la montée (`fallGravityMultiplier`), **flottement à l'apex** (gravité réduite quand la vitesse verticale est faible) et **fast-fall** (maintenir « bas » accélère la chute, via `moveY`). La retombée reste à gravité **constante à multiplicateur près** (`EX-GP-011`). Toute la logique est **pure et déterministe au pas fixe**. Les preuves de franchissabilité (intégration) et le parcours **système** utilisent désormais la **vraie taille** du personnage. Couvert par tests **unitaires** (réglages, taille/spawn), **intégration** (chute > montée, apex hang, fast-fall) et **système** (les 3 niveaux franchis par l'humanoïde).
- **LOT-10 — Mécaniques aériennes avancées : double saut, wall jump, dash** (au-delà du MVP : nouvelles exigences `EX-GP-015`/`016`/`017`, `EX-CTRL-013` ; `EX-GP-013` assouplie). Le saut devient **contextuel** (sol/coyote, mur, ou saut aérien). **Double saut** (`EX-GP-015`) : N sauts aériens paramétrables, rechargés au contact du sol. **Wall jump + wall slide** (`EX-GP-016`) : contre un mur en l'air, le personnage glisse (chute plafonnée) et un saut l'éjecte **en diagonale opposée** au mur (avec verrou horizontal transitoire). **Dash** (`EX-GP-017`) : ruée à vitesse élevée dans l'une des **8 directions** (touches directionnelles, à défaut l'orientation), sur une courte durée, **gravité suspendue**, **une ruée par phase aérienne** rechargée au sol ; touche **Maj** (`EX-CTRL-013`). Toute la logique est **pure et déterministe au pas fixe** dans `Core`, câblée via `toPlayerInput`/`GameScreen`. Un troisième niveau `demo3.json` (couloir bas + fosse, **dash requis**) rejoint la séquence. Couvert par tests **unitaires** (mapping), **d'intégration** (double saut paramétrable, wall slide/jump, dash 8 directions, franchissabilité avec/sans mécanique) et **système** (parcours `demo → demo2 → demo3`).
- **LOT-09 — Saut, game feel et enchaînement de niveaux** : le personnage **saute** (`Espace`/`W`, `EX-GP-011`/`EX-GP-013`) — impulsion au sol, retombée sous **gravité constante** — avec un *game feel* de platformer : **hauteur de saut variable** (relâcher tôt = petit saut), **coyote time** (~80 ms, sauter juste après un bord) et **jump buffering** (~120 ms, saut pré-appuyé honoré à la pose). Les minuteries sont portées par le composant `Player` et décomptées au **pas fixe déterministe** (`EX-NFR-002`) ; l'intention de saut distingue *pressé*/*maintenu* (`EX-CTRL-011`, `toPlayerInput`). Le jeu enchaîne désormais une **séquence ordonnée de niveaux** (`LevelSequence`, `EX-LVL-010`) : à la réussite d'un niveau, le **suivant** se charge automatiquement ; après le **dernier**, retour au titre (`EX-LVL-011`). Deux niveaux livrés — `demo.json` (déplacement/chute) puis `demo2.json` (**marche ascendante, saut requis**). Couvert par tests **unitaires** (mapping saut, `LevelSequence`), **d'intégration** (saut au sol, pas de double saut, hauteur variable, coyote, buffering, franchissabilité avec/sans saut) et un premier test **système** (`Source/Test/Systeme/`) rejouant un **parcours complet** de la séquence sans la couche GPU.
- **LOT-08 — Gameplay personnage (jouable)** : le niveau statique devient **jouable**. Un **personnage** apparaît à l'entrée et est piloté au clavier — **actions logiques** dissociées des touches (`←`/`Q` gauche, `→`/`D` droite, `EX-CTRL-010`, `toPlayerInput`). La physique (`CharacterPhysicsSystem`) applique, **au pas fixe déterministe** (`EX-NFR-002`), le **déplacement horizontal** à vitesse constante (`EX-GP-010`), la **gravité continue** (`EX-GP-012`), et résout les **collisions** par **balayage continu** (`sweepAabb`) contre les tuiles solides — aucune traversée à vitesse élevée, glissement le long des surfaces (`EX-GP-014`). Atteindre la **sortie** revient au menu (`EX-GP-030`) ; toucher un **danger** ou tomber sous le niveau **redémarre** le personnage à l'entrée (`EX-GP-031`, `EX-GP-032`, `evaluateOutcome`). Jeu **par tableaux** : la caméra reste **fixe** et cadre le tableau (adaptation de `EX-REN-013`). Pas de **saut** ni de **mécanismes** (lots ultérieurs). Toute la simulation est **pure dans `Core`** (composants `Player`/`Collider`, `PhysicsConfig`, balayage `Aabb`) et **testée** (balayage, physique, règles, mapping) ; `HMI` ne fait que mapper les touches et afficher. Logique couverte par tests unitaires et d'intégration ; intégration `GameScreen` vérifiée visuellement.
- **Complétude des tests (audit)** : combler des trous de couverture — test unitaire de `approximatelyEqual` (tolérance relative-absolue, grandes magnitudes), test que le **catalogue français livré** (`fr.lang`) se charge et résout ses clés, et un test d'**intégration** « fichier de niveau livré (`demo.json`) → monde ECS » (une entité par tuile non vide). Rééquilibre unit/intégration.
- **LOT-07 — Niveaux (terminé)** : **« Charger niveau »** ouvre désormais un **vrai niveau** chargé depuis `Source/Elements/Levels/demo.json` (format JSON) au lieu de la scène codée en dur. `GameScreen` (TACHE-06) charge le fichier via `LevelLoader`, projette chaque tuile non vide en **entité ECS** (couleur d'atlas par type : solide, danger, entrée, sortie, interrupteur, porte) rendue par le `SpriteRenderer` (`EX-REN-010`, `EX-ARCH-012`), et ajuste la caméra pour cadrer le niveau. Échec de chargement **récupérable** (état neutre à l'écran, pas de plantage, `EX-NFR-040`) ; **Échap** revient au menu. Vérifié de bout en bout (grille de tuiles affichée conforme au fichier).
- **LOT-07 (TACHE-05)** : niveau de démonstration `Source/Elements/Levels/demo.json` (12×8, bords solides, entrée/sortie, danger, paire interrupteur/porte) au format JSON ; copié à côté de l'exécutable par CMake (comme les `.lang`). Un test unitaire vérifie que le niveau **livré se charge et se valide** via `LevelLoader`.
- **LOT-07 (TACHE-04)** : validation du niveau (`EX-LVL-004`) — le chargement rejette proprement, avec message exploitable, un niveau **incohérent** : tuile hors bornes, **positions en double**, entrée/sortie **absente** ou en **plusieurs exemplaires** (unicité), liaison de mécanisme non résolue. Couvert par tests unitaires.
- **LOT-07 (TACHE-03)** : chargement de niveau dans `Core/Levels` — `LevelLoader` lit le format **JSON** (objet `{name, width, height, tiles}`, `tiles` = liste d'objets `{x, y, type, …}`) via nlohmann/json et construit un `Level` ; liaisons interrupteur↔porte résolues par identifiant (`switch.id` ↔ `door.opensWith`). Renvoie un **résultat récupérable** (`LevelLoadResult`, jamais d'exception vers l'appelant, `EX-NFR-040`) : JSON malformé, champ manquant, type inconnu, tuile hors bornes, entrée/sortie absente, liaison non résolue. nlohmann/json reste **confiné au `.cpp`**. Couvert par tests unitaires (`EX-LVL-001`, `EX-LVL-003`).
- **LOT-07 (TACHE-02)** : modèle de niveau dans `Core/Levels` — `TileType` (`Empty`/`Solid`/`Danger`/`Entry`/`Exit`/`Switch`/`Door`), `TileMap` (grille dense typée, accès borné, `isSolid`), `GridPosition` et `Level` (nom + grille + entrée/sortie + mécanismes). Donnée pure, testable sans GPU (`EX-GP-001`, `EX-LVL-002`) ; couvert par tests unitaires.
- **LOT-07 (TACHE-01)** : dépendance **nlohmann/json** (v3.11.3, épinglée via FetchContent, *header-only*) ajoutée pour le parsing des fichiers de niveaux ; liée **en privé** à `Core` (`EX-NFR-031`).
- **Niveau de log configurable au lancement** : le niveau minimum du journaliseur (par défaut `Trace`) se règle via la variable d'environnement `PROJECTGAMING_LOG_LEVEL` ou l'argument `--log-level=<trace|info|warning|error>` (ce dernier prioritaire) ; une valeur non reconnue est ignorée et signalée. Un analyseur pur `core::parseLogLevel` (couvert par tests) fait la conversion. Permet de réduire le bruit en release ou d'augmenter le détail pour déboguer.
- **Bouton d'enregistrement des logs de la session** (**outil de développement, masqué en Release**) : un `MemoryLogSink` capture désormais tous les messages émis, et un **bouton** (icône « télécharger » générée en code) en bas à droite du menu, à gauche du drapeau, écrit la session dans `logs/session_AAAAMMJJ_HHMMSS.log` à côté de l'exécutable. Le bouton n'est présent que dans les builds de développement, via `core::kDeveloperBuild` (`if constexpr`, éliminé en Release). Logique du bouton (`SaveLogButton`) et sérialisation (`serializeSessionLog`) **pures et testées** ; l'action d'enregistrement est injectée dans `MenuScreen` (découplage de la journalisation/fichiers). Écriture **récupérable** (échec signalé, pas de plantage).
- **Journalisation des événements de cycle de vie sur tout le projet** (via l'infrastructure de log du LOT-02). Chaque module a sa **catégorie** de log (en-têtes dédiés, modèle `HMI/HmiLog.h`) : `Core`, `Ecs`, `Graphics`, `Platform`, `HMI`. Sont tracés : création et dimensions de la fenêtre (`Platform`), création des ressources de rendu — device/swap chain, `SpriteBatch`, atlas, police, drapeaux — et redimensionnement (`Graphics`), cadenceur à pas fixe et enregistrement des systèmes ECS (`Core`/`Ecs`), initialisation, **transitions d'écran** (Menu ↔ Jeu ↔ Éditeur), demande de fermeture et changement de langue (`HMI`). Les traces sont **pilotées par événement** — **aucune journalisation dans les chemins par frame** (rendu, mise à jour ECS) — et respectent les niveaux (`INFO` pour les jalons, `TRACE` pour le détail) ; les logs désactivés sont gratuits (garde `isEnabled` avant formatage). Objectif : rendre le déroulé de l'application observable pour le débogage plutôt qu'un fonctionnement silencieux.
- **LOT-06 (TACHE-08)** : sélecteur de langue au menu — un **bouton drapeau** en bas à droite affiche le drapeau de la **langue courante** et bascule entre **français** et **anglais** au clic (`EX-REN-033`). Ajoute le catalogue anglais (`en.lang`), des **icônes de drapeaux générées en code** (`FlagIcons` : France, Royaume-Uni) et une logique `LanguageSelector` **pure, testable sans GPU** (ancrage bas-droit, détection du clic, bascule) ; le `MenuScreen` recharge le catalogue au clic et redessine les libellés dans la nouvelle langue. Bascule **récupérable** (langue conservée si le fichier cible manque). Couvert par tests unitaires ; vérifié visuellement (menu fr ↔ en, drapeaux France / Royaume-Uni).
- **LOT-06 — Menu principal (terminé)** : au lancement, l'exécutable ouvre désormais un **menu principal** au lieu d'une scène codée en dur. `main` (TACHE-07) initialise les ressources partagées (device, `SpriteBatch`, atlas, `BitmapFont`, `Localization` chargée sur le français) et une boucle **pilotée par le `ScreenManager`** démarrant sur le `MenuScreen` : entrées échantillonnées **une fois par frame** (`EX-CTRL-021`), une mise à jour d'écran, puis effacement/rendu/présentation. « Charger niveau » ouvre la scène de démonstration, « Mode Edition » l'écran « à venir », « Quitter » ferme proprement ; **Échap** revient au menu (il ne ferme plus l'application). La scène de démonstration a quitté `main` pour le `GameScreen`. Vérifié de bout en bout (menu → jeu → menu → éditeur → fermeture).
- **LOT-06 (TACHE-06)** : écrans cibles dans `HMI/Interface` — `GameScreen` encapsule la **scène de démonstration** du LOT-05 (grille de tuiles + sprite mobile), avec son `core::World`+`MovementSystem` à pas fixe (`EX-ARCH-030`) et le rendu en lecture seule de l'ECS (`EX-ARCH-012`) ; `EditorScreen` affiche un placeholder « à venir » issu du catalogue de traduction (`EX-REN-032`). Depuis les deux écrans, **Échap** revient au menu. Chaque écran possède ses ressources (RAII), libérées à la transition.
- **LOT-06 (TACHE-05)** : écran de menu principal dans `HMI/Interface` — trois options (**Charger niveau**, **Mode Edition**, **Quitter**) navigables au **clavier** (flèches + Entrée, bouclage) **et à la souris** (survol + clic), l'option sélectionnée mise en évidence (`EX-REN-030`, `EX-CTRL-011`, `EX-CTRL-001`). Les libellés proviennent du **catalogue de traduction** par leur clé (aucun texte en dur). La logique de sélection/transition est isolée dans un `MenuModel` **pur, testable sans GPU** (`EX-NFR-010`), tandis que `MenuScreen` (`IScreen`) se limite au dessin via la police bitmap. Couvert par tests unitaires (navigation, bouclage, survol, clic, actions).
- **LOT-06 (TACHE-04)** : structure d'écrans dans `HMI/Interface` — interface `IScreen` (`update` renvoyant une intention de transition **rester / basculer / quitter**, `render`), `RenderContext` (ressources de rendu partagées) et `ScreenManager` qui détient l'écran courant, applique les transitions et fabrique l'écran suivant via une **fabrique injectée**. Découplé des écrans concrets (aucun écran n'en connaît un autre) et testable sans fenêtre ni GPU (`EX-NFR-010`) ; la demande « quitter » se propage à la boucle (`EX-REN-030`). Couvert par tests unitaires (exécution, basculement, fermeture).
- **LOT-06 (TACHE-03)** : catalogue de traduction (i18n) dans `HMI/Localization` — `Localization` résout tout texte d'interface par **clé** selon la **langue active**, chargée depuis un **fichier par langue** (`Source/Elements/Localization/<langue>.lang`, format `clé = valeur` UTF-8, commentaires `#`). **Repli déterministe** (langue active → langue par défaut → la clé elle-même) : une clé ou un fichier manquant est une erreur **récupérable**, jamais un plantage (`EX-REN-033`, `EX-NFR-040`). Logique pure, testable sans fenêtre ni GPU (`EX-NFR-010`) ; catalogue français fourni (`fr.lang`) et copié à côté de l'exécutable par CMake. Couvert par tests unitaires (analyse, repli, changement de langue, fichier absent).
- **LOT-06 (TACHE-02)** : rendu de texte dans `HMI/Graphics` — `BitmapFont`, police bitmap **monospace générée en code** (glyphes 5×7 intégrés couvrant ASCII imprimable, plus les lettres accentuées françaises composées d'une lettre de base et d'un diacritique). `drawText` dessine une chaîne **UTF-8** en **espace écran** (pixels) via le `SpriteBatch` : un quad par glyphe, teinte appliquée (glyphes blancs colorés par multiplication), échantillonnage *nearest* (`EX-REN-032`, `EX-ARCH-022`) ; les caractères non couverts sont ignorés sans plantage. Fournit aussi `textWidth`/`lineHeight` (centrage) et `screenProjection` (projection pixels → clip de l'interface). Ressources Direct3D en RAII. Vérifié visuellement (libellés du menu, jeu de caractères, accents, teintes).
- **LOT-06 (TACHE-01)** : entrées clavier/souris dans `HMI/Input` — `InputState` échantillonne l'état par frame et expose les fronts **pressée / maintenue / relâchée** (`EX-CTRL-011`) pour le clavier et les boutons souris, plus la position souris. Indépendant de toute fenêtre (aucun `<Windows.h>`), donc testable en isolation (`EX-NFR-010`) ; `hmi::Window` capture les messages Win32 (`WM_KEY*`, `WM_MOUSE*`) et échantillonne l'`InputState` une fois par frame en tête de `pumpMessages` (`EX-CTRL-021`), et expose `input()` + `requestClose()`. Couvert par tests unitaires (fronts déterministes).

### Modifié
- **Pas de console en Release** : l'exécutable est désormais bâti en **sous-système Windows** en Release (aucune fenêtre console — expérience utilisateur plus propre), tout en conservant la **console en Debug** (logs visibles). Le point d'entrée reste le `main()` standard (`/ENTRY:mainCRTStartup`). En conséquence, **aucun sink de log n'est ajouté en Release** (ni console ni capture mémoire) : pas de destination de log, pas de croissance mémoire.
- **LOT-06 (TACHE-01)** : la touche **Échap ne ferme plus** la fenêtre — elle devient une entrée normale (destinée au retour menu) ; la fermeture programmée passe désormais par `Window::requestClose()` (action « Quitter »), la croix continuant de fermer.

### Supprimé
- **Code mort** (audit projet) : retrait de fonctions publiques définies mais **jamais appelées ni testées** — `Camera2D::center()`/`zoom()`, `Logger::minimumLevel()`, `Localization::defaultLanguage()`. Le build `/W4 /WX` couvre déjà locals/paramètres/fonctions-fichier inutilisés ; l'API testée mais consommée au prochain lot (ex. `TileMap::isSolid`, `Level::entry/exit`, `MovementSystem`) et les macros de log uniformes (sans code généré) sont **conservées**.

### Corrigé
- La cible de tests `UnitTests` compile désormais aussi en **Release** : `test_assert.cpp` marquait `[[maybe_unused]]` manquant sur une variable non lue lorsque `PROJECTGAMING_ASSERT` est neutralisé en Release (`/W4 /WX`).

### Ajouté
- **LOT-05 — Rendu 2D (terminé)** : l'exécutable affiche désormais une **scène issue de l'ECS**. La boucle de `main` branche un `core::World` (avec le `MovementSystem`) cadencé à **pas fixe**, puis un **rendu découplé** en lecture seule. `main` construit une scène de démonstration (grille de tuiles + sprite mobile partiellement transparent) rendue via `SpriteRenderer` / `SpriteBatch` / `TextureAtlas` et la `Camera2D`. Première image réelle du jeu (tuiles, transparence, couches, déplacement déterministe).
- **LOT-05 (TACHE-05)** : système de rendu des sprites `SpriteRenderer` dans `HMI/Graphics` — parcourt `world.view<Transform, Sprite>()`, construit le quad monde de chaque entité, résout la région d'atlas en coordonnées de texture, trie par **couche** (tri stable) et dessine via le `SpriteBatch` en appliquant la projection de la caméra. Lecture seule de l'ECS (`EX-ARCH-012`) ; rendu découplé de la simulation.
- **LOT-05 (TACHE-04)** : caméra 2D `Camera2D` dans `HMI/Graphics` — projette le monde vers l'écran (16 px/unité, origine haut-gauche, Y-bas, zoom), fournit la matrice de projection pour le vertex shader et les conversions monde↔écran. Logique pure (DirectXMath), couverte par tests unitaires (conversions réciproques, projection).
- **LOT-05 (TACHE-03)** : atlas de textures procédural `TextureAtlas` dans `HMI/Graphics` — génère en mémoire une grille 4×4 de tuiles 16×16 (couleurs distinctes + une tuile à zones **transparentes** pour valider l'alpha), crée la `ID3D11Texture2D` et sa vue de ressource (RAII), et expose les régions (`tile(colonne, ligne)`). Génération déterministe, remplaçable plus tard par un chargement de fichier.
- **LOT-05 (TACHE-02)** : pipeline de rendu 2D `SpriteBatch` dans `HMI/Graphics` — dessine des quads texturés en Direct3D 11 (shaders HLSL compilés à l'exécution via `D3DCompile`), avec fusion **alpha** (transparence) et échantillonnage **nearest** (pixel art) ; API `begin`/`draw`/`end` avec *batching* (buffers dynamiques, RAII `ComPtr`). `GraphicsDevice` expose `device()`/`context()`.
- **LOT-05 (TACHE-01)** : composant `Sprite` (données pures) dans `Core/Ecs/Components` — région d'atlas (`AtlasRegion`, en pixels), couche de dessin et teinte (`Color` RVBA, blanc opaque par défaut). Agnostique du backend (aucun type DirectX) ; lu par le rendu de `HMI`. Couvert par tests unitaires.
- **LOT-04 — Documentation Doxygen (terminé)** : le site Doxygen est désormais une documentation **navigable** — page d'accueil (`index.md`), rubriques **Spécifications**, **Lots** et **Manuel utilisateur** (page « Télécharger et lancer »), en plus de la référence de code. Navigation hiérarchique (`@page`/`@subpage`) ; l'ordre des spécifications est porté par l'index (pas par des préfixes de fichiers). Garde-fou qualité `WARN_AS_ERROR` sur la génération. **Traçabilité des exigences** : chaque `EX-…` est une ancre Doxygen (`@ref`), avec un lint CI (`scripts/lint_exigences.py`) garantissant l'unicité des identifiants et l'absence de référence orpheline (mode `--next` pour le prochain numéro libre).

### Modifié
- **LOT-04 (TACHE-01)** : réorganisation de l'arborescence documentaire — `Specification/` et `Lot/` déplacés sous `Documentation/` ; le guide de conventions rejoint les spécifications (`Documentation/Specification/conventions.md`) ; les fichiers de spécification perdent leur préfixe numérique (`00-vision.md` → `vision.md`, …), l'ordre étant désormais porté par la navigation Doxygen. Références mises à jour dans tout le dépôt.

### Ajouté
- **Tests d'intégration** : première cible `IntegrationTests` (niveau *Integration*) — la démonstration de mouvement de l'ECS est reclassée depuis les tests unitaires (`test_ecs_mouvement.cpp`), et un test inter-lots vérifie que le cadenceur à pas fixe `FixedTimestep` (LOT-01) pilote correctement la simulation ECS (LOT-03) sans dérive et de façon déterministe (`test_boucle_simulation.cpp`).
- **LOT-03 (TACHE-06)** : premier composant réel et système de démonstration, bouclant l'ECS de bout en bout — composants données pures `Transform` (position, échelle, rotation, en unités monde) et `Velocity` (`Core/Ecs/Components`), et `MovementSystem` (`Core/Ecs/Systems`) qui applique `position += velocity * fixedDelta` aux entités possédant les deux composants. Déterministe, couvert par tests unitaires. **LOT-03 (fondation ECS & math `Core`) terminé.**
- **LOT-03 (TACHE-05)** : façade `World` et systèmes dans `Core/Ecs` — `World` possède les entités (`EntityManager`), les pools de composants (une par type, créée à la demande, via l'interface `IComponentPool`) et les systèmes ; API composants (`addComponent` / `getComponent` / `hasComponent` / `removeComponent`), vues (`view<...>()`) et cycle de vie (`destroyEntity` retire l'entité de **toutes** les pools). Interface `ISystem` (`update(World&, float)`) ; `World::update` exécute les systèmes **dans l'ordre d'enregistrement** au pas de temps fixe. Couvert par tests unitaires.
- **LOT-03 (TACHE-04)** : vue multi-composants dans `Core/Ecs` — `View<Components...>` itère exactement les entités possédant **tous** les composants demandés, pilotée par la plus petite pool (filtrage sur les autres). Deux formes d'usage : `for (auto [entity, ...] : view)` et `view.each([](Entity, A&, B&){ ... })`, avec accès en référence aux composants. Couvert par tests unitaires.
- **LOT-03 (TACHE-03)** : stockage de composants dans `Core/Ecs` — `ComponentPool<T>`, sparse set typé (tableau dense contigu + tableau creux indexé par entité) avec `add` / `get` / `has` / `remove` / `removeIfPresent` ; suppression par swap-and-pop préservant la densité ; `get`/`remove` sur entité absente traités par assertion de précondition. Couvert par tests unitaires.
- **LOT-03 (TACHE-02)** : entités de l'ECS dans `Core/Ecs` — `Entity` (handle générationnel `index`+`generation`, `INVALID_ENTITY`) et `EntityManager` (`create` / `destroy` / `isAlive`, recyclage des index par liste libre avec incrément de génération pour invalider les handles périmés). Couverts par tests unitaires.
- **LOT-03 (TACHE-01)** : types mathématiques de `Core` dans `Core/Math`, sans dépendance DirectX — `Vector2` (opérateurs, produit scalaire, longueur, normalisation, égalité approchée), `Rect` (bords, `contains`, `intersects`, origine haut-gauche / Y-bas) et `MathUtils.h` (`approximatelyEqual`, `kEpsilon`). Couverts par tests unitaires.
- **LOT-02** implémenté : journalisation & diagnostics dans `Core/Diagnostics` — niveaux de log, `Logger` (filtrage + sinks), `ConsoleLogSink` / `MemoryLogSink`, macros `PROJECTGAMING_LOG_*` (horodatage + fichier/ligne) et assertions `PROJECTGAMING_ASSERT` (handler surchargeable, actives en Debug). `main` journalise désormais son démarrage et ses erreurs.
- **LOT-01** implémenté : fenêtre Win32 (`hmi::Window`), initialisation Direct3D 11 en RAII (`hmi::GraphicsDevice`, effacement + présentation V-Sync + redimensionnement) et boucle de jeu à pas de temps fixe déterministe (`core::FixedTimestep`, testée). L'exécutable ouvre une fenêtre stable et se ferme proprement (croix / Échap).
- Arborescence du projet (`Specification/`, `Lot/`, `Documentation/`, `Source/`, `External/`).
- Découpage `Source/` : `Core`, `HMI`, `Elements`, `Test` (`Unit`, `Integration`, `Systeme`).
- Build **CMake** (C++20) avec presets Visual Studio et Ninja ; Visual Studio utilisé via son intégration CMake native.
- **GoogleTest** (FetchContent) et un premier test unitaire.
- **CI GitHub Actions** : configure, build et tests sur `windows-latest`.
- Documentation **Doxygen** (`Doxyfile`) et **guide de conventions** de code.
- Outillage qualité : `.clang-format`, `.clang-tidy`, `.editorconfig`, avertissements `/W4 /WX`, option AddressSanitizer.
- En-têtes précompilés (`Source/pch.h`, option `ENABLE_PCH`).
- CI : couverture de code (OpenCppCoverage, artefact Cobertura + HTML) et génération de la documentation Doxygen (artefact HTML).
- Gouvernance : `CONTRIBUTING.md` (Conventional Commits, trunk-based), `CHANGELOG.md`, `LICENSE`.

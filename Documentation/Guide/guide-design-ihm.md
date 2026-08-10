# Système de design et architecture de l'information {#guide-design-ihm}

> Statut : **livré** (`LOT-56`, `EX-IHM-050` → `EX-IHM-055` ; `LOT-57`, `EX-IHM-060` →
> `EX-IHM-062`). Cette page décrit **comment l'IHM Qt se présente et se répartit** : les jetons de
> design et leur application, puis la distribution de l'information dans l'éditeur. Le socle
> applicatif (fenêtre, viewport, boucle) est en @ref guide-ihm-qt.

## Pourquoi un système de design

De `LOT-34` à `LOT-52`, l'interface a gagné des panneaux, des menus, des grilles et des
inspecteurs — sans qu'aucun lot ne traite jamais son **apparence**. La cause racine était unique et
invisible : l'application n'appelait jamais `setStyle` et tournait donc sur le style **natif** de
la plate-forme, qui ignore une grande partie de toute feuille de style posée par-dessus. C'est
pourquoi `theme.qss` avait dû être restreint par `objectName` au menu principal et à la page
Options, au lieu d'être étendu : ailleurs, il n'avait tout simplement pas d'effet fiable.

Ni `QPalette`, ni `QToolBar`, ni `QActionGroup`, ni `QFontDatabase`, ni `QStyleHints` n'étaient
utilisés ; `devicePixelRatio` n'était pris en compte que par le viewport Direct3D 11, si bien que
les vignettes de la palette et des grilles d'assets étaient floues dès que l'affichage était mis à
l'échelle.

## Les jetons : une seule source, deux portées étanches

\ref hmi::DesignTokens "DesignTokens" (`HMI/Interface/DesignTokens.h`) est la source unique des
couleurs, espacements, tailles et niveaux typographiques. Logique **pure**, sans Qt ni GPU
(`EX-NFR-010`), compilée à la fois dans l'application et directement dans `UnitTests`.

Deux traits méritent l'attention.

**Les couleurs sont nommées par rôle, pas par teinte.** \ref hmi::ColorTokens "ColorTokens" expose
`background`, `surface`, `border`, `text`, `textMuted`, `accent`, `error`… Un jeton `accent`
survit à un changement de couleur ; un jeton qui s'appellerait `ambre` deviendrait un mensonge le
jour où l'accent passe au bleu. Même principe pour la typographie
(\ref hmi::TypographyTokens "TypographyTokens") : `screenTitle`, `sectionTitle`, `body`,
`caption` — jamais une taille ponctuelle hors de cette échelle.

**Il y a deux portées, de structure identique.**

- `identityTokens()` — l'**identité** du jeu : menu principal, page Options, jeu. Invariante.
- `editorDarkTokens()` / `editorLightTokens()` — le **châssis d'édition** : panneaux, barres
  d'outils, barre d'état, boîtes de dialogue ouvertes depuis l'éditeur. Variable.

Réutiliser la même structure pour les deux rend leur symétrie garantie par le système de types
plutôt que par convention : un rôle ajouté à l'une existe nécessairement dans l'autre. Un test
verrouille par ailleurs leur **étanchéité** — l'identité du jeu doit rester rigoureusement
inchangée quel que soit le thème actif du châssis, y compris après une bascule à chaud.

> Piège de plate-forme consigné dans l'en-tête : `<Windows.h>` (via `GraphicsDevice.h`) définit une
> macro `small`, qui casserait silencieusement `SpacingTokens::small`. Le fichier la neutralise.

## De jetons purs à une application habillée

`HMI/Interface/ApplicationTheme.h` est la couche Qt au-dessus des jetons. Elle fait, dans cet
ordre :

1. \ref hmi::applyApplicationStyle "applyApplicationStyle" **choisit le style Fusion avant la
   création du moindre widget** (`EX-IHM-050`) — c'est ce choix, et lui seul, qui rend l'habillage
   prévisible et permet enfin à `theme.qss` de couvrir toute l'application au lieu de deux écrans ;
2. \ref hmi::buildApplicationPalette "buildApplicationPalette" **construit la `QPalette` complète**,
   dans ses trois groupes (actif, inactif, désactivé) : une palette partielle laisse Qt combler les
   trous avec les couleurs du système, ce qui produit des incohérences uniquement visibles sur une
   fenêtre inactive ou un contrôle grisé ;
3. \ref hmi::applyStyleSheet "applyStyleSheet" **produit `theme.qss`** par substitution de
   marqueurs depuis les jetons, via
   \ref hmi::substituteStyleSheetTemplate "substituteStyleSheetTemplate" (fonction pure). Plus
   aucune couleur littérale dans la feuille de style.

Le focus clavier est rendu visible partout — condition de la navigation à la manette
(`EX-IHM-040`), qui n'a pas de pointeur pour dire où elle en est.

> **Invariant à ne pas casser.** Ne jamais poser `titlebar-close-icon` ou
> `titlebar-normal-icon: none` sur un `QDockWidget` dans `theme.qss` : ces propriétés ont provoqué
> un plantage intermittent à la fermeture depuis l'éditeur pendant le `LOT-56`. Masquer un bouton
> de barre de titre se fait par les `features` du dock, pas par la feuille de style.

## Typographie : une police embarquée, un repli qui n'invente rien

La police **Inter** est embarquée (`Assets/Fonts/Inter-{Regular,Bold}.ttf`, licence SIL OFL 1.1).
\ref hmi::resolveFontFamily "resolveFontFamily" décide de la famille effective : la police
enregistrée si elle l'est, sinon une famille **générique** — jamais un second nom de police codé en
dur, qui ne serait qu'un pari sur ce qui est installé chez l'utilisateur. Les tailles et marges
figées dans les fichiers `.ui` ont été retirées au profit des jetons.

## Les commandes : une définition, trois surfaces

\ref hmi::editorActionCatalog "editorActionCatalog" (`HMI/Interface/ActionCatalog.h`) est une
**table pure** décrivant les outils et les commandes de l'éditeur (`hmi::EditorActionSpec`, groupés
par `hmi::EditorActionGroup`). `hmi::EditorActions` construit les `QAction`
depuis cette table, et ces mêmes objets alimentent simultanément le menu, la barre d'outils et le
raccourci clavier (`EX-IHM-055`). C'est ce qui remplace les boutons radio empilés de l'ancien
panneau Outils — et, surtout, ce qui supprime la double définition : avant, une commande présente
au menu et dans une barre existait deux fois, et pouvait diverger.

Les icônes sont **dessinées par code** : \ref hmi::iconGeometry "iconGeometry" décide *quoi*
dessiner (géométrie pure, testable), `hmi::themeIcon` décide *comment* le peindre avec `QPainter`,
recoloré depuis les jetons. Même découpage que `hmi::gameHudLines` (`LOT-52`). Aucun fichier
d'icône à livrer, et un changement de thème recolore tout sans réexporter d'assets.

\ref hmi::EditorActions::applyShortcuts "EditorActions::applyShortcuts" synchronise le raccourci
**effectif** de chaque action depuis les touches remappées, y compris après un remappage à chaud.
C'est ce qui a permis, au `LOT-57`, de rendre réellement remappables dix actions d'éditeur dont
neuf n'étaient jamais lues : leurs raccourcis étaient interceptés en dur, et Copier/Coller au
clavier contournait même complètement `EditorKeyBindings`.

## Netteté à toute échelle d'affichage

\ref hmi::thumbnailPixelSize "thumbnailPixelSize" (fonction pure) donne la taille en pixels
**réels** d'une vignette à partir de sa taille logique et du facteur d'échelle de l'écran. Les
vignettes de la palette, des grilles d'assets et des lignes du panneau Textures sont rendues à
cette taille, et régénérées lors d'un changement d'écran : nettes à 100 %, 125 % et 150 %. Le
canevas de l'atelier pixel art (@ref guide-atelier-pixel-art) réutilise cette même fonction plutôt
que de redéfinir sa propre règle.

## Thème clair/sombre

\ref hmi::resolveEffectiveEditorTheme "resolveEffectiveEditorTheme" traduit un **réglage**
(`EditorThemeSetting` : Système, Clair, Sombre) et l'indication du système d'exploitation en un
**mode** effectif (`EditorThemeMode` : Clair ou Sombre). La séparation compte : le réglage est ce
que l'utilisateur a choisi et qu'on persiste, le mode est ce qu'on applique. « Système » n'est pas
une troisième apparence, c'est une délégation.

Le thème s'applique à chaud — palette, feuille de style, icônes — sans redémarrage. Le contraste
texte/fond est vérifié par test pour les deux thèmes (seuils WCAG ⧉).

## Architecture de l'information : ce qui informe reste, ce qui commande est unique

Le `LOT-57` part d'un constat de terrain : l'éditeur affichait son aide dans une **ligne unique**
de barre d'état, figée à l'entrée en mode éditeur et définitivement effacée par le premier message
transitoire. Passé la première minute, la barre d'état ne disait plus rien.

### Une barre d'état structurée

\ref hmi::editorStatusLines "editorStatusLines" (`HMI/Editor/EditorStatus.h`) est une **fonction
pure** — même patron que `hmi::gameHudLines` (`LOT-52`) — qui décide du contenu de zones
**permanentes** : niveau ouvert, modifications non enregistrées, outil actif, case survolée, zoom,
et une sixième zone (couleur courante) quand le contexte est l'atelier pixel art
(`PixelEditStatusInfo`). Ces zones sont ajoutées par `addPermanentWidget` : un message transitoire
ne peut donc plus les recouvrir. L'aide contextuelle à l'outil actif se restaure automatiquement à
l'expiration du message (`MainWindow::refreshStatusHelp`, minuteur unique).

Le viewport expose pour cela `hoveredCell()` et `zoom()`, avec des signaux émis **seulement sur
changement réel** — sinon la barre d'état se reconstruirait à chaque mouvement de souris.

### Des panneaux groupés, et qui suivent l'outil

Les panneaux sont regroupés en onglets par défaut (`tabifyDockWidget`), chacun restant
individuellement déplaçable, détachable et refermable. \ref hmi::panelForTool "panelForTool" est
une table pure — même patron que le catalogue d'actions — qui dit quel panneau mettre en avant pour
un outil donné ; `panelForPixelTool` fait de même pour les outils du canevas.

La règle de mise en avant a deux garde-fous : elle **n'est jamais un masquage** (on met en avant,
on ne cache rien), et elle **cède dès que l'utilisateur a imposé son choix** (onglet sélectionné à
la main, panneau déplacé). Une interface qui réorganise les panneaux malgré l'utilisateur devient
vite un adversaire.

> La disposition des panneaux est persistée et **versionnée** (`LAYOUT_VERSION`). Chaque
> redistribution incrémente la version, ce qui invalide les dispositions antérieures : sans cela,
> une disposition enregistrée avant le changement rouvrirait des panneaux dans un agencement qui
> n'a plus de sens.

### Un état, un contrôle

`EX-IHM-062` interdit qu'un même état ou une même commande soit exposé à deux endroits. Les
conséquences concrètes : la bascule Physique/Texture est devenue une entrée unique du menu
Affichage ; l'onglet Calques a quitté le panneau Textures pour ce même menu, une entrée par calque
dans l'ordre de dessin ; et Annuler/Refaire/Copier/Coller dispatchent via
\ref hmi::EditContextTarget "EditContextTarget", interface qu'implémentent `hmi::GameViewport`
puis, au `LOT-54`, `hmi::PixelCanvas`. C'est ce seuil de dispatch qui a permis à l'atelier pixel
art d'avoir son propre historique sans réécrire une seule ligne du dispatch existant.

Tout doublon apparent n'en est pas un : les deux sélecteurs de couche de décor ont été
**conservés**, parce qu'ils ciblent des états distincts — la couche du prochain décor posé, et la
couche du décor sélectionné existant. Ils ont été renommés explicitement et rassemblés dans le
panneau Décors, l'ambiguïté se levant à l'écran plutôt que dans la documentation.

## Voir aussi
- @ref guide-ihm-qt — le socle applicatif Qt, le viewport Direct3D 11, la boucle et les entrées.
- @ref guide-atelier-pixel-art — l'atelier qui hérite de ces jetons, actions et zones d'état.
- @ref guide-editeur — l'éditeur de niveau lui-même (brouillon, outils, essai immédiat).
- [Spécification IHM](@ref spec-interface-ihm) — le *quoi/pourquoi* (`EX-IHM-050` → `EX-IHM-062`).

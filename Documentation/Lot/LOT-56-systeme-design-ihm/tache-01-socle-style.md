# TACHE-01 — Socle de style : style Qt, palette applicative et jetons de design {#lot-56-tache-01-socle-style}

**Lot :** [LOT-56](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** non commencé

## Contexte
`Source/HMI/main.cpp` charge une feuille de style et rien d'autre : l'application ne choisit jamais son
style Qt, et s'exécute donc sur le style **natif** de la plate-forme. Ce style dessine la plupart des
contrôles via l'API du système et ignore une large part de la feuille de style — c'est la raison pour
laquelle le thème existant a dû être restreint par `objectName` au menu et aux options.

Aucune `QPalette` n'est définie nulle part dans `Source/HMI`. Les couleurs qui ne passent pas par la
feuille de style — fonds de vues, texte désactivé, couleur de sélection, infobulles, boîtes de dialogue
standard — viennent donc directement du système d'exploitation.

Les grandeurs d'habillage sont par ailleurs éparpillées : taille de vignette dans *AssetThumbnailView*,
autre taille de vignette dans *PalettePanel*, taille d'icône de ligne dans *TexturePanel*, et deux
widgets de remappage jumeaux (*KeybindingsWidget* et *GamepadBindingsWidget*) dont les largeurs
minimales diffèrent sans raison. La couleur d'effacement du viewport, définie dans *GameViewport*, est
**proche mais différente** du fond de la feuille de style : la jointure entre le canevas Direct3D 11 et
les widgets qui l'entourent est visible.

Cette tâche est le socle : les cinq suivantes en dépendent.

## Travail à réaliser
- **Jetons de design** dans un en-tête unique : couleurs nommées **par rôle** (fond, surface, surface
  alternée, bordure, texte, texte atténué, accent, accent au survol, erreur), échelle d'espacement,
  échelle typographique, tailles d'icônes et de vignettes, largeurs de contrôles usuelles.
  Nommer par rôle et non par teinte — un jeton `accent` survit à un changement de couleur, un jeton
  `ambre` non, et la tâche 6 introduira un second jeu de valeurs pour le thème clair.
- **Choix explicite du style Qt** au démarrage, avant toute création de widget.
- **Palette applicative** complète construite à partir des jetons, couvrant les trois groupes de
  Qt (actif, inactif, désactivé) — le groupe désactivé est celui qu'on oublie, et celui qui trahit le
  plus vite un thème incomplet.
- **Couleur d'effacement du viewport dérivée du même jeton** que le fond de l'interface, en
  remplacement de la valeur littérale actuelle de *GameViewport*.
- **Remplacement des constantes de style locales** des widgets par les jetons correspondants, en
  unifiant au passage les deux largeurs minimales divergentes des widgets de remappage.
- **Conversion pure** jeton → chaîne de couleur, exposée comme fonction testable sans instance
  d'application ni GPU (elle servira aussi à produire la feuille de style en TACHE-02).

## Fichiers impactés
- `Source/HMI/Interface/DesignTokens.h` (nouveau) — les jetons et leur conversion.
- `Source/HMI/Interface/ApplicationTheme.{h,cpp}` (nouveau) — construction de la palette, application
  du style.
- `Source/HMI/main.cpp` — choix du style et application de la palette avant la fenêtre principale.
- `Source/HMI/Game/GameViewport.cpp` — couleur d'effacement dérivée du jeton de fond.
- `Source/HMI/Editor/AssetThumbnailView.cpp`, `Source/HMI/Editor/PalettePanel.cpp`,
  `Source/HMI/Editor/TexturePanel.cpp`, `Source/HMI/Interface/KeybindingsWidget.cpp`,
  `Source/HMI/Interface/GamepadBindingsWidget.cpp` — constantes locales remplacées par les jetons.
- `Source/Test/Unit/HMI/Interface/test_design_tokens.cpp` (nouveau).

## Tests (obligatoires)
- **Conversion de couleur** : chaque jeton produit une chaîne de couleur valide et stable ; la
  conversion est **pure** (même entrée, même sortie), testée sans instance d'application.
- **Cohérence viewport / interface** : la couleur d'effacement du viewport et le jeton de fond de
  l'interface dérivent de la même valeur — un test qui échoue si l'une des deux est modifiée seule.
  C'est le seul garde-fou possible contre la réapparition de la couture actuelle.
- **Aucun doublon de grandeur** : les widgets de remappage exposent la même largeur minimale.

## Points d'attention
- **Le style doit être choisi avant la création du moindre widget** : appliqué après, il ne se propage
  pas aux widgets déjà construits.
- **Changer de style change l'apparence de tout**, y compris les boîtes de dialogue standard
  (message, fichier, saisie) et le dialogue de redimensionnement construit à la main dans
  *MainWindow*. Les repasser en revue fait partie de la tâche, pas d'un correctif ultérieur.
- **Les jetons ne remplacent pas les couleurs du monde rendu.** `hmi::TileAppearance` et les
  `core::Color` du pipeline Direct3D 11 décrivent le contenu du jeu, pas l'habillage de l'application :
  ils restent inchangés. Seule la couleur d'effacement du viewport, visuellement contiguë aux widgets,
  entre dans le périmètre.
- Ne pas nommer les jetons d'après leur teinte : la TACHE-06 ajoutera un jeu clair, et un jeton nommé
  d'après sa couleur y deviendra un contresens.

## Définition de fait (DoD)
- L'application choisit explicitement son style et applique une palette complète dérivée d'un jeu
  unique de jetons ; la couleur d'effacement du viewport et le fond de l'interface proviennent du même
  jeton et un test le garantit ; aucune constante de style ne subsiste dans le code des widgets
  concernés ; les boîtes de dialogue standard ont été revues ; `/W4 /WX` propre.

## Exigences
`EX-IHM-050` (style maîtrisé), `EX-IHM-051` (source unique des grandeurs d'habillage) ; réutilise
`EX-IHM-001` (interface hors-jeu en Qt), `EX-NFR-010` (`Core` indépendant de la présentation).

# TACHE-04 — Section « Animations » du panneau « Textures » {#lot-47-tache-04-section-animations}

**Lot :** [LOT-47](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** non commencé

## Contexte
Les clips sont décrits dans les fichiers d'assets (LOT-46) et demandés par la correspondance
état → clip (TACHE-01). Reste à choisir **quel asset animé** chaque famille de mécanisme utilise par
défaut, sans quoi il faudrait assigner une surcharge (LOT-45) sur chaque porte de chaque niveau.

C'est la quatrième section du panneau « Textures », et elle suit la même règle que les précédentes :
elle s'ajoute, elle ne remplace rien.

## Travail à réaliser
- **Section « Animations »** : pour chaque famille de mécanisme (porte, interrupteur, plaque,
  danger commuté, danger temporisé, danger mobile), sélection de l'asset animé par défaut parmi
  `Assets/Objects/`, avec vignettes (LOT-43).
- **Stockage** : dans le jeu de skins courant de `skins.json` (LOT-42) — un mécanisme est un type de
  tuile, son asset par défaut est donc une association type → asset, exactement comme un skin. Aucun
  nouveau fichier de configuration.
- **Diagnostic des clips** : pour l'asset sélectionné, indiquer quels clips attendus par la
  correspondance sont **présents** et lesquels **manquent**. C'est l'information qui permet à
  l'auteur de savoir ce qu'il lui reste à dessiner, sans lancer le jeu.
- **Aperçu** : lecture de l'animation sélectionnée dans le panneau, pour vérifier le rythme sans
  entrer en mode essai.
- Traduction de toutes les chaînes.

## Fichiers impactés
- `Source/HMI/Editor/TexturePanel.{h,cpp}`, `Source/Elements/UI/TexturePanel.ui`.
- `Source/HMI/Graphics/SkinCatalog.{h,cpp}` (aucune extension de format attendue).
- `Source/Elements/Localization/fr.lang`, `en.lang`.

## Tests (obligatoires)
- **Diagnostic des clips** : pour un catalogue d'animation donné et une famille de mécanisme donnée,
  la liste des clips manquants est correcte. Fonction **pure**, testée sans Qt.
- L'assignation d'un asset animé à une famille est bien écrite dans `skins.json` et relue.

## Points d'attention
- **Ne pas créer un second format de configuration.** Réutiliser `skins.json` évite d'avoir deux
  fichiers décrivant l'apparence d'un même type de tuile, avec les incohérences que cela produit.
- L'aperçu doit utiliser le **même** catalogue d'animation que le jeu, pas une lecture parallèle :
  sinon l'aperçu pourrait montrer un rythme différent de celui joué.
- Le diagnostic de clips manquants doit rester **informatif**, pas bloquant : un asset partiel est
  un état de travail légitime.

## Définition de fait (DoD)
- Chaque famille de mécanisme peut recevoir un asset animé par défaut, stocké dans `skins.json` ;
  les clips manquants sont signalés dans le panneau ; l'aperçu joue l'animation ; chaînes
  traduites ; diagnostic testé sans Qt ; `/W4 /WX` propre.

## Exigences
`EX-REN-006` (apparence pilotée par l'état) ; réutilise `EX-EDIT-042` (association type → asset),
`EX-EDIT-024` (jeux de skins), `EX-EDIT-026` (bibliothèque d'assets), `EX-REN-005` (animations par
données), `EX-REN-033` (traduction).

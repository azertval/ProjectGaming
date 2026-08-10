# TACHE-06 — Sélection de niveau côté joueur {#lot-59-tache-06-selection-niveau}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** non commencé

## Contexte
Le menu principal offre quatre entrées, dont un « Jouer » qui ne fait qu'une chose : relancer la
séquence complète depuis le premier tableau. Avec la progression de la `TACHE-05`, ce bouton devient
ambigu — reprend-il ou recommence-t-il ?

Par ailleurs, un joueur ne peut **pas** rejouer un tableau précis, ni jouer un niveau qu'il vient de
créer : la seule façon d'atteindre un niveau arbitraire est l'essai de l'éditeur. L'éditeur possède
pourtant déjà tout ce qu'il faut — `hmi::LevelBrowserPanel` (`LOT-36`) liste, recherche et ouvre les
niveaux d'un dossier.

## Travail à réaliser
- **Remplacer « Jouer »** par trois entrées explicites : *Continuer* (grisée si aucune progression),
  *Nouvelle partie* (avec confirmation si une progression existe), *Choisir un niveau*.
- **Écran de sélection** listant les tableaux de la séquence, avec leur état (terminé, atteint,
  verrouillé). Sont jouables les tableaux **terminés** et le **premier non terminé** ; les suivants
  sont visibles mais verrouillés — sinon la progression qu'on vient d'ajouter ne signifierait rien.
- **Réutiliser `hmi::LevelBrowserPanel`** et `hmi::LevelFileOperations` plutôt que d'écrire un
  second parcours de dossier. Si le panneau est trop lié à l'éditeur, en extraire la partie
  **listage et filtrage** — pas le dupliquer.
- **Accès aux niveaux personnels** : un mode « tous les niveaux du dossier », hors séquence et hors
  progression, pour jouer un niveau créé dans l'éditeur.
- Navigation clavier / souris / manette, libellés traduits.

## Fichiers impactés
- `Source/HMI/Interface/LevelSelectScreen.{h,cpp}` (nouveau), mise en page dans
  `Source/Elements/UI/LevelSelectScreen.ui`.
- `Source/Elements/UI/MainMenu.ui` — entrées du menu principal.
- `Source/HMI/Editor/LevelBrowserPanel.{h,cpp}` — extraction de la partie réutilisable si besoin.
- `Source/HMI/Interface/MainWindow.{h,cpp}`.
- `Source/Elements/Localization/{fr,en}.lang`.
- `Source/Test/Unit/HMI/Game/test_progression.cpp` (étendu : quels tableaux sont jouables).

## Tests (obligatoires)
- Règle de déverrouillage, sous forme de fonction **pure** testée : progression vide → seul le
  premier tableau est jouable ; trois tableaux terminés → les trois plus le quatrième ; séquence
  entièrement terminée → tous.
- *Continuer* est indisponible sans progression, et mène au premier tableau **non terminé** sinon.
- Un tableau verrouillé ne peut pas être lancé, même par le chemin manette.
- Un niveau personnel hors séquence se lance sans toucher à la progression de la séquence.
- Libellés présents dans les deux catalogues.

## Points d'attention
- **La règle de déverrouillage est une fonction pure**, pas une condition disséminée dans le
  widget : c'est elle qu'on teste, l'écran ne fait que l'afficher.
- Jouer un niveau **hors séquence** ne doit rien écrire dans la progression — sans quoi essayer un
  niveau personnel déverrouillerait la campagne.
- Ne pas dupliquer le balayage de dossier : `hmi::LevelBrowserPanel` le fait déjà, et un second
  parcours divergerait au premier changement de convention de nommage.
- Le menu principal est aussi le premier écran vu par un nouveau joueur : trois entrées de jeu plus
  Éditeur, Options et Quitter commencent à faire beaucoup — vérifier la lisibilité à l'essai manuel.

## Définition de fait (DoD)
- Le menu distingue reprendre, recommencer et choisir ; la règle de déverrouillage est pure et
  testée ; un niveau personnel est jouable sans passer par l'éditeur et sans polluer la
  progression ; `hmi::LevelBrowserPanel` est réutilisé, pas dupliqué ; traduit ; `/W4 /WX` propre.

## Exigences
`EX-IHM-005` (sélection de niveau côté joueur) ; réutilise `EX-LVL-014` (progression),
`EX-LVL-013` (séquence), `EX-IHM-020`/`EX-IHM-021` (panneau de gestion des niveaux),
`EX-REN-030` (menu principal), `EX-REN-033` (traduction), `EX-CTRL-012` (manette et clavier).

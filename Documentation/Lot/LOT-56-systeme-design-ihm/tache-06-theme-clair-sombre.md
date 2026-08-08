# TACHE-06 — Thème clair et sombre suivant le système {#lot-56-tache-06-theme-clair-sombre}

**Lot :** [LOT-56](epic.md) · **Emplacement :** `Source/HMI/Interface`, `Source/Elements/Themes` · **Statut :** non commencé

## Contexte
Le thème du projet est sombre, sans alternative ni réglage. Rien dans `Source/HMI` n'interroge le
réglage clair/sombre du système : `QStyleHints` n'y apparaît pas. Une application qui reste sombre sur
un poste configuré en clair détonne, et l'inverse est pire encore pour un outil de création utilisé en
plein jour.

Une fois les jetons en place (TACHE-01) et la feuille de style produite à partir d'eux (TACHE-02), le
coût d'un second jeu de valeurs devient faible : il s'agit de substituer des couleurs, pas de réécrire
un thème. C'est la raison pour laquelle cette tâche vient en dernier — et la raison pour laquelle elle
est **la première à retirer** si le lot doit être resserré : elle emporte alors `EX-IHM-054` avec elle,
sans rien invalider des cinq tâches précédentes.

## Travail à réaliser
- **Second jeu de jetons** pour le thème clair, couvrant exactement les mêmes rôles que le jeu sombre —
  aucun rôle propre à un seul thème, sans quoi la feuille de style diverge.
- **Détection du réglage système** au démarrage et **réaction à son changement** en cours d'exécution.
- **Application à chaud** : changer de thème reconstruit la palette, régénère la feuille de style et
  invalide les icônes dessinées (TACHE-04) et les vignettes (TACHE-05), sans redémarrage.
- **Réglage utilisateur** dans l'onglet Vidéo de la page Options : Système, Clair, Sombre — persisté
  entre deux sessions, à côté des réglages existants.
- **Couleur d'effacement du viewport suivant le thème**, puisqu'elle dérive du jeton de fond
  (TACHE-01).
- **Traduction** des trois libellés du réglage dans les deux catalogues.

## Fichiers impactés
- `Source/HMI/Interface/DesignTokens.h` — second jeu de valeurs.
- `Source/HMI/Interface/ApplicationTheme.{h,cpp}` — détection, résolution et application à chaud.
- `Source/HMI/Interface/OptionsPage.{h,cpp}`, `Source/Elements/UI/OptionsPage.ui` — réglage.
- `Source/HMI/Game/GameViewport.cpp` — couleur d'effacement suivant le thème actif.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Interface/test_application_theme.cpp` — complété.

## Tests (obligatoires)
- **Résolution du thème effectif** : fonction **pure** prenant le réglage utilisateur et le réglage
  système, produisant le thème à appliquer — Système + système clair → clair ; Système + système
  sombre → sombre ; Clair ou Sombre forcé → ignore le système. Testée sans instance d'application.
- **Parité des rôles** : les deux jeux de jetons définissent exactement le même ensemble de rôles — un
  test qui échoue si un rôle est ajouté à l'un sans l'autre.
- **Contraste** : pour chaque thème, le rapport de contraste entre le texte et son fond, et entre le
  texte atténué et son fond, dépasse un seuil de lisibilité fixé — c'est le garde-fou qui évite qu'un
  thème clair soit livré illisible.
- **Persistance** : le réglage est relu au démarrage suivant.

## Points d'attention
- **Un thème clair n'est pas le thème sombre aux couleurs inversées.** Les bordures, les ombres et les
  lignes alternées demandent des écarts de luminosité différents en clair et en sombre ; inverser
  mécaniquement produit un thème plat. Le test de contraste borne le défaut, il ne le remplace pas.
- **Ne pas oublier les icônes et les vignettes** lors du changement à chaud : dessinées depuis les
  jetons, elles restent aux anciennes couleurs si elles ne sont pas régénérées.
- Le réglage vit dans les Options avec les autres réglages persistés, et n'introduit pas son propre
  mécanisme de stockage.
- Vérifier le rendu du viewport pendant le changement : la couleur d'effacement change à la frame
  suivante, ce qui ne doit produire aucun clignotement visible.
- Le contenu du jeu (tuiles, décors, fonds) ne change **jamais** avec le thème : seul l'habillage de
  l'application est concerné.

## Définition de fait (DoD)
- L'application suit le réglage clair/sombre du système par défaut, accepte un forçage depuis les
  Options, persiste ce choix et l'applique sans redémarrage ; les deux jeux de jetons couvrent les
  mêmes rôles et satisfont le seuil de contraste, tous deux vérifiés par des tests purs ; icônes,
  vignettes et couleur d'effacement du viewport suivent le thème actif ; `/W4 /WX` propre.

## Exigences
`EX-IHM-054` (thème clair/sombre suivant le système, réglable et persisté) ; réutilise `EX-IHM-050`
(thème unique), `EX-IHM-051` (source unique des grandeurs), `EX-REN-033` (traduction).

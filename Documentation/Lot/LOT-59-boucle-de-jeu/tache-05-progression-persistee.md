# TACHE-05 — Progression persistée {#lot-59-tache-05-progression-persistee}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/HMI/Game` · **Statut :** non commencé

## Contexte
Rien n'est conservé entre deux lancements. Quinze tableaux se jouent d'une traite ou pas du tout —
fermer l'application ramène au premier. `vision.md` classait la sauvegarde de progression « hors
périmètre MVP » ; le MVP est livré, et une `0.1.0` qui perd la partie à chaque fermeture n'est pas
un jeu qu'on distribue.

Le projet sait déjà persister un réglage : `hmi::GameKeyBindings` et `hmi::EditorKeyBindings`
écrivent un JSON à côté de l'exécutable, tolèrent son absence et ignorent une entrée malformée
plutôt que d'abandonner le fichier entier. La progression suit ce patron, sans en inventer un
second.

## Travail à réaliser
- **Modèle de progression** : identifiant de la séquence, indice du tableau atteint, et ensemble des
  tableaux **terminés** (par nom de fichier, pas par indice — un réordonnancement de la séquence ne
  doit pas rendre la progression fausse).
- **Persistance** dans `Settings/progression.json`, à côté de `Settings/keybindings.json`, via
  `hmi::executableDirectory()`. Lecture **tolérante** : fichier absent = partie neuve ; entrée
  inconnue ou malformée ignorée ; niveau qui n'existe plus simplement écarté.
- **Écriture atomique** (fichier temporaire puis remplacement), comme `hmi::encodeImageFile`
  (`LOT-54`) : une fermeture brutale ne doit pas laisser une progression à demi écrite.
- **Points d'écriture explicites et rares** : à la fin d'un tableau (`TACHE-03`), et nulle part
  ailleurs. Pas d'écriture par image ni par pas.
- **Réinitialisation** : *Nouvelle partie* efface la progression après confirmation.

## Fichiers impactés
- `Source/HMI/Game/Progression.{h,cpp}` (nouveau) — logique pure, sans Qt.
- `Source/HMI/Interface/MainWindow.{h,cpp}` — chargement au démarrage, écriture en fin de tableau.
- `Source/Test/Unit/HMI/Game/test_progression.cpp` (nouveau).
- `Source/Test/CMakeLists.txt` (compiler `Progression.cpp` dans `UnitTests`).
- `.gitignore` — vérifier que `Settings/` produit à l'exécution n'est pas versionné.

## Tests (obligatoires)
- Aller-retour : écrire une progression, la relire, obtenir exactement la même.
- Fichier **absent** → partie neuve, sans erreur.
- Fichier **vide**, **malformé**, ou contenant une entrée inconnue → partie neuve ou progression
  partielle, jamais d'exception ni de plantage.
- Un tableau terminé puis **retiré** de la séquence n'invalide pas le reste de la progression.
- Réordonner la séquence conserve les tableaux terminés (c'est ce que garantit le stockage par nom).
- Tests purs, sans Qt ni GPU.

## Points d'attention
- **Stocker des noms, pas des indices.** Un indice devient faux dès qu'on insère un tableau au
  milieu de la séquence — et l'ajout de tableaux est précisément ce que `TACHE-04` rend facile.
- Le fichier est écrit dans le dossier de l'exécutable : sur une installation en lecture seule
  l'écriture échoue. C'est une erreur **récupérable** (le jeu continue, la progression n'est pas
  conservée, un avertissement est journalisé), pas une condition d'arrêt.
- Ne pas verser la progression dans `QSettings` : le projet y met la **disposition des panneaux**
  (`EX-IHM-011`), qui est un réglage d'outil ; la progression est du contenu de partie, et doit
  vivre à côté de l'exécutable pour partir avec le zip.

## Définition de fait (DoD)
- La progression survit à un redémarrage, se corrompt sans dommage, résiste au réordonnancement de
  la séquence, s'écrit atomiquement et rarement ; couverte par des tests purs ; `/W4 /WX` propre.

## Exigences
`EX-LVL-014` (progression persistée) ; réutilise `EX-LVL-013` (séquence en donnée), `EX-NFR-040`
(erreur récupérable), `EX-NFR-010` (testable sans GPU), `EX-CTRL-012` (patron de persistance des
réglages).

# TACHE-01 — Provisionnement de Qt Multimedia et socle de lecture {#lot-60-tache-01-socle-qt-multimedia}

**Lot :** [LOT-60](epic.md) · **Emplacement :** `Source/HMI/Audio`, `.github/workflows` ·
**Statut :** non commencé

## Contexte
Le projet dépend déjà de Qt (`Widgets`, `Gui`), provisionné sur trois environnements comme
`EX-BUILD-010` l'exige : installeur officiel ou `aqtinstall` en local, `install-qt-action` en CI,
`windeployqt` à la release. Ajouter l'audio revient à **ajouter un module** à une dépendance déjà
maîtrisée, pas à en introduire une nouvelle — c'est tout l'intérêt du choix de Qt Multimedia.

Le travail réel de cette tâche est donc moins le code que le **provisionnement** : si `qtmultimedia`
n'est pas installé en CI, la cible est ignorée ou le build casse ; s'il n'est pas déployé par
`windeployqt`, le zip de release démarre muet ou pas du tout, et personne ne s'en aperçoit avant
qu'un joueur le signale.

## Travail à réaliser
- **Trouver `Qt6::Multimedia`** dans `Source/HMI/CMakeLists.txt`, en composant additionnel, en
  conservant la garde existante : sans Qt, la cible `ProjectGaming` est ignorée sans faire échouer
  la configuration.
- **CI** : ajouter `modules: qtmultimedia` à chaque étape `install-qt-action` de `ci.yml` **et** de
  `release.yml`. Les quatre jobs concernés doivent être traités — en oublier un se manifeste par un
  échec dans un seul job, souvent bien plus tard.
- **`hmi::AudioEngine`** : ouverture du périphérique, lecture d'un échantillon court, volume global,
  arrêt propre. Enveloppe fine autour de `QSoundEffect` — pas une couche d'abstraction spéculative
  au-dessus d'une bibliothèque qu'on ne remplacera pas.
- **Aucune exception à l'absence de périphérique** : l'ouverture qui échoue met le moteur dans un
  état « muet », journalise un avertissement, et toutes les demandes de lecture ultérieures ne font
  rien (`EX-NFR-040`).
- **Vérifier le déploiement** : `windeployqt` (déjà appelé en `POST_BUILD`) doit déposer les
  bibliothèques et les greffons multimédia à côté de l'exécutable. Le contrôle « l'exécutable est
  bien produit » de `ci.yml` a un pendant à écrire ici : les bibliothèques audio sont présentes dans
  le dossier de sortie.

## Fichiers impactés
- `Source/HMI/Audio/AudioEngine.{h,cpp}` (nouveau).
- `Source/HMI/CMakeLists.txt` — composant `Multimedia`, sources.
- `.github/workflows/ci.yml`, `.github/workflows/release.yml` — module `qtmultimedia`.
- `Source/HMI/Audio/README.md` — le dossier cesse d'être un projet d'intention.

## Tests (obligatoires)
- Le moteur en état « muet » (périphérique indisponible) accepte toutes les demandes de lecture sans
  erreur ni plantage — testable en forçant l'état, sans périphérique réel.
- Le volume est borné et normalisé aux extrêmes (valeurs hors plage ramenées, jamais propagées
  telles quelles).
- **`UnitTests`, `IntegrationTests` et `SystemTests` se construisent et passent sans `Qt6::Multimedia`** :
  c'est la garantie que l'audio n'a pas contaminé la logique testable.
- Contrôle en CI de la présence des bibliothèques audio dans le dossier de sortie.

## Points d'attention
- **`QSoundEffect` veut du WAV PCM**, pas un format compressé : le choix de format des assets
  (`TACHE-02`) en dépend directement.
- Un `QSoundEffect` charge son fichier de façon **asynchrone** : jouer immédiatement après
  construction ne produit rien. Les sons doivent être préchargés au démarrage, pas au premier
  déclenchement — sinon le premier saut de la partie est muet, et c'est un défaut difficile à
  attribuer.
- `install-qt-action` a **quatre** points d'appel entre `ci.yml` et `release.yml` : en oublier un
  donne un échec dans un seul job.
- Le module ajoute des mégaoctets au zip de release. Le mesurer et le consigner ; c'est le coût
  assumé du choix de bibliothèque.
- Ne pas construire d'interface abstraite « au cas où l'on changerait de bibliothèque » : le choix
  est arrêté, et une abstraction à une seule implémentation coûte sans rien rendre.

## Définition de fait (DoD)
- `Qt6::Multimedia` est provisionné sur les trois environnements, `hmi::AudioEngine` lit un
  échantillon et dégrade proprement sans périphérique, les tests restent constructibles sans le
  module, le déploiement est vérifié en CI ; `/W4 /WX` propre.

## Exigences
`EX-REN-047` (socle audio dans `HMI`) ; réutilise `EX-BUILD-010` (provisionnement reproductible),
`EX-NFR-031` (version épinglée), `EX-NFR-040` (erreur récupérable), `EX-NFR-010` (`Core` et tests
sans périphérique), `EX-NFR-041` (RAII).

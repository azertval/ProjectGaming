# Exigences non fonctionnelles {#spec-exigences}

> Statut : **brouillon**. Transverse à toutes les specs.

## 1. Performance
- \anchor EX-NFR-001 **EX-NFR-001** — Le jeu doit maintenir **60 images/seconde** sur une configuration de bureau récente pour les niveaux du MVP.
- \anchor EX-NFR-002 **EX-NFR-002** — La simulation doit fonctionner à **pas de temps fixe** et rester déterministe (mêmes entrées → même résultat).
- \anchor EX-NFR-003 **EX-NFR-003** — L'empreinte mémoire doit rester stable dans le temps (aucune fuite ; vérifiable via AddressSanitizer).
- \anchor EX-NFR-005 **EX-NFR-005** — Le rendu ne doit soumettre que les primitives **effectivement
  visibles** : le contenu hors du cadrage de la caméra (`EX-REN-015`, salle courante) est écarté
  avant soumission. Le nombre de primitives émises par image doit rester **borné et observable**,
  l'habillage complet (fond, décors, ombres, tuiles, objets, effets) multipliant le volume par
  rapport au rendu d'origine. Concrétisé en `LOT-40`.

## 2. Architecture & maintenabilité
- \anchor EX-NFR-010 **EX-NFR-010** — La logique (`Core`) doit être **indépendante** de la présentation (`HMI`) et testable sans fenêtre ni GPU.
- \anchor EX-NFR-011 **EX-NFR-011** — Aucune dépendance cyclique entre modules (`HMI → Core`, jamais l'inverse).
- \anchor EX-NFR-012 **EX-NFR-012** — Le code doit respecter le [guide de conventions](conventions.md) (nommage, RAII, documentation, gestion d'erreurs).
- \anchor EX-NFR-013 **EX-NFR-013** — Le code livré doit compiler **sans avertissement** (`/W4 /WX`).

## 3. Qualité & vérification
- \anchor EX-NFR-020 **EX-NFR-020** — Toute logique de gameplay livrée dans `Core` doit être couverte par des **tests unitaires** (GoogleTest).
- \anchor EX-NFR-021 **EX-NFR-021** — Les niveaux du MVP doivent être couverts par un **test système** vérifiant leur franchissabilité.
- \anchor EX-NFR-022 **EX-NFR-022** — La **CI** doit exécuter build, tests et couverture à chaque push/PR et rester verte pour merger.
- \anchor EX-NFR-004 **EX-NFR-004** — La chaîne de rendu doit être **vérifiable sans GPU** : les
  primitives de dessin produites pour une scène donnée doivent pouvoir être **capturées et
  inspectées** par un test (ordre des calques, priorité de résolution des textures, choix des
  raccords automatiques, isolement d'un calque, effets de bord du culling). Un critère d'acceptation
  du type « rendu identique » ou « ordre de calque correct » ne doit pas reposer sur une
  vérification à l'œil quand il peut être asserté. Concrétisé en `LOT-40`.

## 4. Portabilité & reproductibilité
- \anchor EX-NFR-030 **EX-NFR-030** — Le projet doit se construire **exclusivement via CMake**, reproductible sur plusieurs postes (cf. `README`).
- \anchor EX-NFR-031 **EX-NFR-031** — Les dépendances tierces doivent être **épinglées** à une version (ex. GoogleTest).
- \anchor EX-NFR-032 **EX-NFR-032** — Cible actuelle : **Windows/DirectX**. Le `Core` doit toutefois éviter toute dépendance système inutile pour préserver sa testabilité (et une portabilité future).

## 5. Robustesse
- \anchor EX-NFR-040 **EX-NFR-040** — Une erreur récupérable (fichier de niveau invalide, ressource manquante) ne doit pas faire planter le jeu : elle est signalée et gérée (cf. politique d'erreurs des conventions).
- \anchor EX-NFR-041 **EX-NFR-041** — Les ressources (mémoire, handles DirectX) doivent être gérées en **RAII** (libération garantie).

## 6. Build & dépendances
- \anchor EX-BUILD-010 **EX-BUILD-010** — Une dépendance tierce **non gérable par `FetchContent`**
  (SDK volumineux tel que **Qt**) doit être **provisionnée et documentée de façon reproductible** sur
  les trois environnements : poste local (installeur officiel ou `aqtinstall`), **CI** (étape
  d'installation dans le workflow, sur le runner épinglé) et **release** (déploiement des bibliothèques
  dynamiques requises à côté de l'exécutable, ex. `windeployqt`). La version est **épinglée**
  (`EX-NFR-031`) et la licence documentée. Introduit en `LOT-34`.

## Traçabilité
Ces exigences transverses conditionnent l'acceptation de chaque lot. Elles s'appuient sur l'outillage déjà en place (CMake, CI, clang-tidy, ASan, conventions).

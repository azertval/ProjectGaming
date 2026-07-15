# Exigences non fonctionnelles {#spec-exigences}

> Statut : **brouillon**. Transverse à toutes les specs.

## 1. Performance
- \anchor EX-NFR-001 **EX-NFR-001** — Le jeu doit maintenir **60 images/seconde** sur une configuration de bureau récente pour les niveaux du MVP.
- \anchor EX-NFR-002 **EX-NFR-002** — La simulation doit fonctionner à **pas de temps fixe** et rester déterministe (mêmes entrées → même résultat).
- \anchor EX-NFR-003 **EX-NFR-003** — L'empreinte mémoire doit rester stable dans le temps (aucune fuite ; vérifiable via AddressSanitizer).

## 2. Architecture & maintenabilité
- \anchor EX-NFR-010 **EX-NFR-010** — La logique (`Core`) doit être **indépendante** de la présentation (`HMI`) et testable sans fenêtre ni GPU.
- \anchor EX-NFR-011 **EX-NFR-011** — Aucune dépendance cyclique entre modules (`HMI → Core`, jamais l'inverse).
- \anchor EX-NFR-012 **EX-NFR-012** — Le code doit respecter le [guide de conventions](conventions.md) (nommage, RAII, documentation, gestion d'erreurs).
- \anchor EX-NFR-013 **EX-NFR-013** — Le code livré doit compiler **sans avertissement** (`/W4 /WX`).

## 3. Qualité & vérification
- \anchor EX-NFR-020 **EX-NFR-020** — Toute logique de gameplay livrée dans `Core` doit être couverte par des **tests unitaires** (GoogleTest).
- \anchor EX-NFR-021 **EX-NFR-021** — Les niveaux du MVP doivent être couverts par un **test système** vérifiant leur franchissabilité.
- \anchor EX-NFR-022 **EX-NFR-022** — La **CI** doit exécuter build, tests et couverture à chaque push/PR et rester verte pour merger.

## 4. Portabilité & reproductibilité
- \anchor EX-NFR-030 **EX-NFR-030** — Le projet doit se construire **exclusivement via CMake**, reproductible sur plusieurs postes (cf. `README`).
- \anchor EX-NFR-031 **EX-NFR-031** — Les dépendances tierces doivent être **épinglées** à une version (ex. GoogleTest).
- \anchor EX-NFR-032 **EX-NFR-032** — Cible actuelle : **Windows/DirectX**. Le `Core` doit toutefois éviter toute dépendance système inutile pour préserver sa testabilité (et une portabilité future).

## 5. Robustesse
- \anchor EX-NFR-040 **EX-NFR-040** — Une erreur récupérable (fichier de niveau invalide, ressource manquante) ne doit pas faire planter le jeu : elle est signalée et gérée (cf. politique d'erreurs des conventions).
- \anchor EX-NFR-041 **EX-NFR-041** — Les ressources (mémoire, handles DirectX) doivent être gérées en **RAII** (libération garantie).

## Traçabilité
Ces exigences transverses conditionnent l'acceptation de chaque lot. Elles s'appuient sur l'outillage déjà en place (CMake, CI, clang-tidy, ASan, conventions).

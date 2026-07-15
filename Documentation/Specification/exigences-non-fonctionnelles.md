# Exigences non fonctionnelles {#spec-exigences}

> Statut : **brouillon**. Transverse à toutes les specs.

## 1. Performance
- **EX-NFR-001** — Le jeu doit maintenir **60 images/seconde** sur une configuration de bureau récente pour les niveaux du MVP.
- **EX-NFR-002** — La simulation doit fonctionner à **pas de temps fixe** et rester déterministe (mêmes entrées → même résultat).
- **EX-NFR-003** — L'empreinte mémoire doit rester stable dans le temps (aucune fuite ; vérifiable via AddressSanitizer).

## 2. Architecture & maintenabilité
- **EX-NFR-010** — La logique (`Core`) doit être **indépendante** de la présentation (`HMI`) et testable sans fenêtre ni GPU.
- **EX-NFR-011** — Aucune dépendance cyclique entre modules (`HMI → Core`, jamais l'inverse).
- **EX-NFR-012** — Le code doit respecter le [guide de conventions](conventions.md) (nommage, RAII, documentation, gestion d'erreurs).
- **EX-NFR-013** — Le code livré doit compiler **sans avertissement** (`/W4 /WX`).

## 3. Qualité & vérification
- **EX-NFR-020** — Toute logique de gameplay livrée dans `Core` doit être couverte par des **tests unitaires** (GoogleTest).
- **EX-NFR-021** — Les niveaux du MVP doivent être couverts par un **test système** vérifiant leur franchissabilité.
- **EX-NFR-022** — La **CI** doit exécuter build, tests et couverture à chaque push/PR et rester verte pour merger.

## 4. Portabilité & reproductibilité
- **EX-NFR-030** — Le projet doit se construire **exclusivement via CMake**, reproductible sur plusieurs postes (cf. `README`).
- **EX-NFR-031** — Les dépendances tierces doivent être **épinglées** à une version (ex. GoogleTest).
- **EX-NFR-032** — Cible actuelle : **Windows/DirectX**. Le `Core` doit toutefois éviter toute dépendance système inutile pour préserver sa testabilité (et une portabilité future).

## 5. Robustesse
- **EX-NFR-040** — Une erreur récupérable (fichier de niveau invalide, ressource manquante) ne doit pas faire planter le jeu : elle est signalée et gérée (cf. politique d'erreurs des conventions).
- **EX-NFR-041** — Les ressources (mémoire, handles DirectX) doivent être gérées en **RAII** (libération garantie).

## Traçabilité
Ces exigences transverses conditionnent l'acceptation de chaque lot. Elles s'appuient sur l'outillage déjà en place (CMake, CI, clang-tidy, ASan, conventions).

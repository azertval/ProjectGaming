# Conventions de code — ProjectGaming

Règles à respecter dès la première ligne, pour garder un code cohérent et une documentation Doxygen exploitable. Le squelette existant (`Source/Core`, `Source/HMI`) suit déjà ces règles.

---

## 1. Langage & standard
- **C++20** (fixé dans `CMakeLists.txt`, extensions compilateur désactivées).
- Préférer la bibliothèque standard aux API brutes quand c'est possible.
- Pas de `using namespace` en portée de fichier d'en-tête.

## 2. Nommage

| Élément | Convention | Exemple |
|---------|-----------|---------|
| Fichier | PascalCase, `.h` / `.cpp` | `Core.h`, `PlayerController.cpp` |
| Espace de noms | minuscules | `core`, `hmi` |
| Type (class, struct, enum) | PascalCase | `GameState`, `TileMap` |
| Fonction / méthode | camelCase | `version()`, `updatePhysics()` |
| Variable / paramètre | camelCase | `deltaTime`, `tileIndex` |
| Membre de classe | camelCase + suffixe `_` | `position_`, `isGrounded_` |
| Constante (`constexpr`) | UPPER_SNAKE_CASE | `MAX_ENTITIES` |
| Macro (à éviter) | UPPER_SNAKE_CASE | `PG_ASSERT` |

- Un fichier `.h`/`.cpp` porte le nom du type ou module principal qu'il contient.
- Noms en **anglais** pour le code (identifiants), **français** autorisé pour les commentaires et la doc.

## 3. Mise en forme
- Indentation : **4 espaces**, jamais de tabulation.
- Accolades style K&R (ouvrante en fin de ligne) :
  ```cpp
  if (isGrounded_) {
      jump();
  }
  ```
- Une déclaration par ligne. Toujours des accolades, même pour un bloc d'une ligne.
- Longueur de ligne indicative : **100 colonnes**.
- Include guard : **`#pragma once`** (pas de macros de garde manuelles).

## 4. Ordre des `#include`
Dans un `.cpp`, du plus proche au plus général, chaque groupe trié et séparé par une ligne vide :
1. L'en-tête correspondant au fichier (`#include "Core.h"` en premier dans `Core.cpp`).
2. En-têtes standard (`<string>`, `<vector>`, …).
3. En-têtes tiers (`<gtest/gtest.h>`, DirectX…).
4. En-têtes du projet (`"..."`).

## 5. Architecture (dépendances entre modules)
- **`Core`** : logique/moteur, **indépendant** de la présentation. Ne connaît ni DirectX ni la fenêtre.
- **`HMI`** : dépend de `Core`, jamais l'inverse. Contient rendu, fenêtre, entrées.
- **`Elements`** : données/assets statiques, aucun code exécutable.
- Aucune dépendance cyclique. `Core` reste testable sans fenêtre ni GPU.

## 6. Documentation Doxygen
Tout élément public (fichier, type, fonction) est documenté. `JAVADOC_AUTOBRIEF` est actif : la première phrase sert de description courte.

```cpp
/**
 * @file Core.h
 * @brief Point d'entrée de la bibliothèque Core.
 */

/**
 * @brief Met à jour la physique d'une entité.
 * @param entity    Entité à mettre à jour.
 * @param deltaTime Temps écoulé depuis la dernière frame, en secondes.
 * @return true si l'entité a bougé.
 */
bool updatePhysics(Entity& entity, float deltaTime);
```

Balises usuelles : `@file`, `@brief`, `@param`, `@return`, `@note`, `@warning`, `@see`.
Commentaire de membre bref : `///< description` après la déclaration.

## 7. Bonnes pratiques
- `const` par défaut ; `constexpr` quand c'est possible.
- Passage par référence (`const T&`) pour les objets non triviaux.
- RAII et pointeurs intelligents (`std::unique_ptr`) ; éviter `new`/`delete` bruts.
- Initialiser toutes les variables. Préférer l'initialisation par accolades `{}`.
- Pas de nombres magiques : constantes nommées.

## 8. Tests
- Un test = un comportement, nommé `TEST(SuiteName, ComportementTeste)`.
- Fichiers de test : `test_<cible>.cpp` dans `Source/Test/{Unit,Integration,Systeme}/`.
- Toute logique de `Core` livrée s'accompagne de tests unitaires.

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
| Constante (`constexpr`) | UPPER_SNAKE_CASE | `MAXIMUM_ENTITIES` |
| Macro (à éviter) | UPPER_SNAKE_CASE | `PROJECTGAMING_ASSERT` |

- Un fichier `.h`/`.cpp` porte le nom du type ou module principal qu'il contient.
- Noms en **anglais** pour le code (identifiants), commentaires et documentation **en français**.
- **Aucune abréviation** dans les noms de variables et de fonctions : le nom est écrit en toutes lettres (`deltaTime` et non `dt`, `position` et non `pos`, `updatePhysics` et non `updPhys`). Seuls les prénoms d'itérateurs conventionnels (`i`, `j`) et les termes standard du domaine restent tolérés.

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

## 4. Inclusions (`#include`)

### Chemins complets depuis `Source/`
Les en-têtes du projet s'incluent par leur **chemin complet depuis la racine `Source/`**, jamais par nom seul :

```cpp
#include "Core/Core.h"          // ✅ chemin complet
#include "HMI/Window.h"         // ✅
#include "Core.h"               // ❌ nom seul
```

- Séparateur : **slash `/`** (portable), y compris sous Windows — jamais d'antislash.
- La racine d'inclusion `Source/` est fournie par CMake (`target_include_directories(... ${PROJECT_SOURCE_DIR}/Source)`).
- Bénéfice : les dépendances sont explicites et un déplacement de fichier lors d'une refonte d'architecture se repère et se corrige par simple recherche du chemin.

### Ordre des groupes
Dans un `.cpp`, du plus proche au plus général, chaque groupe trié et séparé par une ligne vide :
1. L'en-tête correspondant au fichier (`#include "Core/Core.h"` en premier dans `Core.cpp`).
2. En-têtes standard (`<string>`, `<vector>`, …).
3. En-têtes tiers (`<gtest/gtest.h>`, DirectX…).
4. En-têtes du projet, chemin complet (`"Core/..."`, `"HMI/..."`).

## 5. Architecture (dépendances entre modules)
- **`Core`** : logique/moteur, **indépendant** de la présentation. Ne connaît ni DirectX ni la fenêtre.
- **`HMI`** : dépend de `Core`, jamais l'inverse. Contient rendu, fenêtre, entrées.
- **`Elements`** : données/assets statiques, aucun code exécutable.
- Aucune dépendance cyclique. `Core` reste testable sans fenêtre ni GPU.

### Classes plutôt que fonctions libres
- **Privilégier une classe** (avec état encapsulé et membres privés) à un ensemble de fonctions libres dans un espace de noms. Une classe se dérive et se spécialise : elle permet d'étendre les comportements sans réécrire les appelants.
- Réserver les fonctions libres dans un `namespace` aux utilitaires purs, sans état.
- Exposer le minimum : membres et méthodes internes en `private` par défaut, `public` seulement pour l'interface réellement nécessaire.

### RAII obligatoire
- **Toute classe respecte le RAII** : une ressource (mémoire, handle DirectX, fichier…) est acquise dans le constructeur et libérée dans le destructeur. Aucune méthode `init()` / `cleanup()` manuelle à appeler séparément.
- Un objet construit est immédiatement dans un état valide et utilisable ; sa destruction libère tout.
- Bénéfice recherché : **traçage des bugs et tests facilités** — la durée de vie d'une ressource est celle de l'objet, sans état intermédiaire à moitié initialisé.
- Gérer la copie/déplacement explicitement (règle des 0/3/5) ; préférer `= default` ou `= delete` à une implémentation manuelle quand c'est possible.

## 6. Documentation Doxygen
Tout élément public (fichier, type, fonction) est documenté **en français**. `JAVADOC_AUTOBRIEF` est actif : la première phrase sert de description courte.

### Description dans le `.h` ET le `.cpp`
Le bloc Doxygen de description de la fonction est présent **à la fois** dans la déclaration (`.h`) et dans la définition (`.cpp`) :
- Dans le **`.h`** : le contrat de la fonction (rôle, paramètres, retour) — lu par les appelants.
- Dans le **`.cpp`** : le même bloc, complété par la documentation **du corps** de la fonction.

**Déclaration (`Core.h`)**
```cpp
/**
 * @brief Met à jour la physique d'une entité pour une frame.
 * @param entity    Entité à mettre à jour.
 * @param deltaTime Temps écoulé depuis la dernière frame, en secondes.
 * @return true si l'entité a changé de position.
 */
bool updatePhysics(Entity& entity, float deltaTime);
```

**Définition (`Core.cpp`) — corps documenté**
```cpp
/**
 * @brief Met à jour la physique d'une entité pour une frame.
 * @param entity    Entité à mettre à jour.
 * @param deltaTime Temps écoulé depuis la dernière frame, en secondes.
 * @return true si l'entité a changé de position.
 */
bool updatePhysics(Entity& entity, float deltaTime) {
    const Vector2 ancientPosition = entity.position();

    // Intégration semi-implicite : la vitesse est mise à jour avant la position
    // pour un comportement stable sous gravité constante.
    entity.applyGravity(deltaTime);
    entity.move(entity.velocity() * deltaTime);

    // L'entité est considérée « au sol » uniquement si un contact est détecté
    // sous ses pieds : sinon la gravité continue de s'appliquer à la frame suivante.
    if (entity.detectGroundContact()) {
        entity.resetVerticalVelocity();
    }

    return entity.position() != ancientPosition;
}
```

### Documentation du corps (`.cpp`)
Dans les définitions, commenter en français :
- Le rôle de chaque **branche `if`/`else`** non triviale (pourquoi cette condition).
- Le principe de tout **algorithme complexe** (intention, invariants, cas limites), pas la paraphrase ligne à ligne.
- Les choix non évidents (pourquoi telle formule, telle borne, telle optimisation).

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

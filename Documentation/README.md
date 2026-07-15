# Documentation/

Documentation technique au format **Doxygen** (outils et code).

## Objet
Expliquer *comment* le code et les outils fonctionnent : modules, classes, fonctions, pipeline de build, dépendances DirectX.

## Mise en place (à venir)
- Un fichier `Doxyfile` à la racine de ce dossier configurera la génération.
- Le code source dans `../Source/` sera commenté avec les balises Doxygen (`@brief`, `@param`, `@return`, `@file`, ...).
- La sortie générée (HTML/XML) sera placée dans un sous-dossier ignoré par Git (ex. `generated/`).

## Convention de commentaire
```cpp
/**
 * @brief Description courte.
 * @param nom Description du paramètre.
 * @return Description de la valeur de retour.
 */
```

## Conventions de code
Les règles de style, de nommage et d'architecture sont détaillées dans [`Specification/conventions.md`](Specification/conventions.md).

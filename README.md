# ProjectGaming

Jeu 2D de plateforme / puzzle développé **from scratch** en **C++ / DirectX** (Windows).

## Organisation du dépôt

| Dossier | Rôle |
|---------|------|
| `specification/` | Specs de travail : définition des besoins, contraintes, règles du jeu. |
| `lot/` | Lots de travail. Un sous-dossier par lot, contenant un fichier d'epic et des fichiers de tâches. |
| `Documentation/` | Documentation technique au format **Doxygen** (outils et code). |
| `Source/` | Code source, réparti par fonction. |

### Découpage de `Source/`

| Sous-dossier | Contenu |
|--------------|---------|
| `HMI/` | Code lié aux interfaces (rendu, menus, HUD, interactions utilisateur). |
| `Core/` | Fonctions back : logique de jeu, physique, boucle, gestion d'état, moteur. |
| `Elements/` | Assets et éléments statiques (sprites, tuiles, sons, niveaux, ressources). |

## Statut

Projet en cours d'initialisation — arborescence posée, specs et premiers lots à venir.

# LOT-06 — Menu principal {#lot-06}

> Statut : **à faire**. Le jeu démarre directement sur une scène codée en dur. Ce lot introduit un **menu principal** et la structure d'**écrans** qui l'accompagne, avec les briques nécessaires : entrées clavier/souris et rendu de texte.

## Objectif
Afficher au démarrage un **menu principal** navigable proposant trois options —
**« Charger niveau »**, **« Mode Edition »** et **« Quitter »** — et router l'application vers
l'écran correspondant. En préalable, poser les briques manquantes : un module d'**entrées**
(clavier + souris) et un **rendu de texte** (police bitmap).

À l'issue du lot : au lancement, l'exécutable ouvre le menu ; on navigue au clavier et à la
souris ; « Quitter » ferme l'application, « Charger niveau » ouvre la scène de démonstration
(niveau provisoire) et « Mode Edition » un écran « à venir », tous deux avec retour au menu
par **Échap**.

## Périmètre

### Inclus
- **Entrées** : état clavier et souris exposé par `Window`, avec distinction **pressée /
  maintenue / relâchée** (`EX-CTRL-011`), échantillonné une fois par frame (`EX-CTRL-021`).
- **Rendu de texte** : police **bitmap** intégrée et fonction de dessin de texte via le
  `SpriteBatch` existant (`EX-REN-032`).
- **États d'application (écrans)** : écran courant et transitions (Menu / Jeu / Éditeur).
- **Écran de menu principal** : trois options, surbrillance de la sélection, navigation
  **clavier + souris**, actions câblées (`EX-REN-030`).
- **Écrans cibles** : « Charger niveau » réutilise la scène de démo du LOT-05 (niveau
  provisoire) ; « Mode Edition » affiche un écran « à venir » ; retour au menu par Échap.
- **Intégration `main`** : la boucle pilote l'écran courant ; la scène codée en dur quitte
  `main` pour l'écran de jeu.

### Exclus (lots ultérieurs)
- **Chargement de niveaux depuis fichier** (« Charger niveau » ouvre la démo, pas un fichier).
- **Éditeur** réel (`EX-EDIT-*`) : « Mode Edition » est un placeholder.
- Écrans de **pause** et de **fin de niveau** (`EX-REN-031`).
- Manette (`EX-CTRL-002`), remapping des touches (`EX-CTRL-012`), audio.

## Décisions de cadrage
- **Texte** : police **bitmap** intégrée (dessinée via `SpriteBatch`), cohérente avec le
  pixel art et l'esprit « from scratch ».
- **Navigation** : **clavier + souris** (flèches/Entrée et survol/clic).
- **Actions** : « Charger niveau » → scène de démo (niveau provisoire) ; « Mode Edition » →
  écran « à venir » ; « Quitter » → fermeture. Retour menu par Échap.

## Exigences couvertes
- `EX-REN-030` (menu principal), `EX-REN-032` (texte), `EX-ARCH-012` (le rendu lit l'état).
- `EX-CTRL-011` (pressée/maintenue/relâchée), `EX-CTRL-021` (entrées échantillonnées une
  fois par frame, en amont de la logique).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-entrees-clavier-souris.md) | Entrées clavier & souris | `HMI/Input` | ⬜ Non commencé |
| [TACHE-02](tache-02-rendu-texte-bitmap.md) | Rendu de texte (police bitmap) | `HMI/Graphics` | ⬜ Non commencé |
| [TACHE-03](tache-03-etats-application.md) | États d'application (écrans) | `HMI/Interface` | ⬜ Non commencé |
| [TACHE-04](tache-04-ecran-menu-principal.md) | Écran de menu principal | `HMI/Interface` | ⬜ Non commencé |
| [TACHE-05](tache-05-ecrans-cibles.md) | Écrans cibles (jeu démo + éditeur placeholder) | `HMI/Interface` | ⬜ Non commencé |
| [TACHE-06](tache-06-integration-main.md) | Intégration `main` (boucle pilotée par l'écran) | `HMI/main.cpp` | ⬜ Non commencé |

## Critères d'acceptation du lot
1. Au lancement, l'exécutable affiche le **menu principal** avec les trois options.
2. La sélection se fait **au clavier** (flèches + Entrée) **et à la souris** (survol + clic) ;
   l'option sélectionnée est mise en évidence.
3. **« Quitter »** ferme l'application ; **« Charger niveau »** ouvre la scène de démo ;
   **« Mode Edition »** ouvre l'écran « à venir » ; **Échap** revient au menu depuis ces écrans.
4. Les entrées distinguent pressée/maintenue/relâchée et sont lues **une fois par frame**.
5. Build `/W4 /WX` sans avertissement, tests verts (logique testable : entrées, sélection
   de menu), documentation Doxygen à jour, `CHANGELOG.md` mis à jour.

## Dépendances
- Réutilise le rendu 2D de [LOT-05](@ref lot-05) (`SpriteBatch`, `Camera2D`, `SpriteRenderer`,
  scène de démo) et l'ECS de [LOT-03](@ref lot-03).
- S'appuie sur `hmi::Window` (LOT-01) pour la capture des événements d'entrée.

## Navigation des tâches
- @subpage lot-06-tache-01-entrees-clavier-souris
- @subpage lot-06-tache-02-rendu-texte-bitmap
- @subpage lot-06-tache-03-etats-application
- @subpage lot-06-tache-04-ecran-menu-principal
- @subpage lot-06-tache-05-ecrans-cibles
- @subpage lot-06-tache-06-integration-main

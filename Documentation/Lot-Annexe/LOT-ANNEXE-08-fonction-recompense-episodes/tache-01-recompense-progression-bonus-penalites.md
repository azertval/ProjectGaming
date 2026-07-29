# TACHE-01 — Récompense de progression, bonus de complétion, pénalités {#lot-annexe-08-tache-01-recompense-progression-bonus-penalites}

**Lot :** [LOT-ANNEXE-08](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
`HeadlessLevelEnvironment` (`LOT-ANNEXE-05`) renvoie une position de personnage et une issue brute à
chaque pas, sans aucune notion de récompense. Cette tâche calcule, à partir de ces informations
seules, le signal scalaire que tout algorithme d'apprentissage du programme cherchera à maximiser.

## Travail à réaliser
- **`aisolver::RewardConfig`** (`Source/AiSolver/Env/Reward.h`) : structure de constantes
  documentées — `float progressScale`, `float completionBonus`, `float deathPenalty`, `float
  timePenalty` (par pas), valeurs par défaut choisies pour que `completionBonus` domine
  significativement la somme plausible des autres termes sur un épisode typique (quelques centaines
  de pas).
- **`float computeReward(const RewardConfig&, const Aabb& previousBox, const Aabb& currentBox,
  const core::GridPosition& exit, core::LevelOutcome outcome)`** (`Source/AiSolver/Env/Reward.cpp`) :
  - distance euclidienne du centre de `previousBox`/`currentBox` au centre de la case `exit` ;
  - récompense de progression = `progressScale × (distancePrécédente - distanceCourante)` ;
  - si `outcome == Won` : ajoute `completionBonus` ;
  - si `outcome == Lost` : ajoute `deathPenalty` (négatif) ;
  - toujours : soustrait `timePenalty`.

## Fichiers impactés
- `Source/AiSolver/Env/Reward.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Progression positive** : `currentBox` plus proche de `exit` que `previousBox` → récompense de
  progression strictement positive.
- **Progression négative/nulle** : recul ou stagnation → récompense de progression négative ou
  nulle, jamais positive.
- **Bonus de complétion domine** : sur un épisode synthétique de plusieurs centaines de pas de
  pénalité de temps cumulée, la récompense totale d'un épisode `Won` reste strictement supérieure à
  celle d'un épisode `Lost`/`Playing` interrompu à la même progression partielle.
- **Valeurs par défaut plausibles** : `RewardConfig{}` (constructeur par défaut) ne produit ni
  `NaN` ni valeur aberrante sur des distances et durées typiques des niveaux `demo-*.json`.

## Points d'attention
- **La distance utilisée est euclidienne dans l'espace monde (`core::Vector2`), jamais une distance
  de plus court chemin sur la grille** (décision de cadrage de l'épic) — ne pas tenter d'ajouter un
  pathfinding ici, même partiel.
- **`computeReward` est une fonction pure**, sans état interne ni effet de bord — appelée à chaque
  pas par la boucle d'entraînement (génération 2/3), elle ne doit dépendre que de ses arguments.

## Définition de fait (DoD)
- `RewardConfig`/`computeReward` disponibles et testés (`ctest` vert) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement (récompense, conception d'une fonction de récompense
(*shaping*, *reward hacking*), épisode et horizon), en particulier ses sections 6.1 (récompense
creuse contre dense), 6.3 (*shaping* par potentiel : pourquoi une **différence** de distance et non
une distance) et 6.4 (ordres de grandeur entre les termes).

## Exigences
`EX-IA-009` (nouvelle, partagée avec TACHE-02/03 du même lot).

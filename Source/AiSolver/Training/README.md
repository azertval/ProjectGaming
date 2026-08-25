# AiSolver/Training/

Les **algorithmes d'apprentissage** et la boucle qui les pilote sur un niveau donné.

Quatre familles cohabitent, une par sous-dossier. Elles partagent le même environnement
([`../Env/`](../Env/README.md)), la même récompense et le même format de sortie : ce qui les
distingue est la façon dont elles cherchent, pas ce qu'elles cherchent.

| Sous-dossier | Algorithme |
|---|---|
| `Evolutionary/` | Algorithme **évolutionniste** : sélection par tournoi, croisement, mutation gaussienne, élitisme. Le chemin le plus court vers un agent qui finit réellement un niveau, et la ligne de base de tout le reste. |
| `PolicyGradient/` | **REINFORCE** : collecte de trajectoires, retour actualisé par pas, perte construite comme un graphe d'autodiff. |
| `ActorCritic/` | **Acteur-critique** : réduit la variance en soustrayant au retour une valeur d'état apprise (l'*avantage*). |
| `Advanced/` | **DQN** : réseau de valeur d'action, tampon de rejeu, réseau cible, exploration ε-décroissante. |

Pièces communes, à la racine du dossier :
- `LevelTrainingSession` — boucle d'entraînement pour un unique niveau, avec critère d'arrêt.
- `PolicyGradientLoss` — la formule de perte de *policy gradient*, **partagée** par
  `PolicyGradient/` (poids = retour brut) et `ActorCritic/` (poids = avantage). Elle vit ici, et non
  dans `PolicyGradient/`, précisément parce qu'elle appartient aux deux.
- `ArgmaxRollout` — rejeu déterministe d'une politique en mode `Argmax`. C'est le chemin qui produit
  le rejeu que le joueur regarde ; partagé par `aisolver-cli` et par l'écran Mode IA.
- `DeterministicReplay`, `ReplayExport`, `TrainingResult` — rejeu du meilleur individu, export au
  format v2, et issue d'une session.

Réf. specs : `EX-IA-011` à `EX-IA-015` ; lots [`LOT-ANNEXE-10`](Documentation/Lot-Annexe/LOT-ANNEXE-10-algorithme-evolutionniste/epic.md) à [`LOT-ANNEXE-14`](Documentation/Lot-Annexe/LOT-ANNEXE-14-algorithme-avance/epic.md).

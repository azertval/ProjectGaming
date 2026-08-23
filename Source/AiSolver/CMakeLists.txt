# Bibliothèque AiSolver : calcul numérique maison (tenseurs, RNG) et différentiation automatique
# pour le programme annexe d'IA de résolution autonome (@ref lots-annexe) — aucune dépendance à un
# framework de calcul tiers.
add_library(AiSolver STATIC
    Math/Rng.cpp
    # Graphe de calcul dynamique de l'autodiff maison (LOT-ANNEXE-02) : Node.h/TensorOps.h restent
    # header-only, seule la fabrique generique (variable/unaryOp/binaryOp) est compilee ici.
    Math/Autodiff/Node.cpp
    # Operations differentiables de base (LOT-ANNEXE-02, TACHE-02), construites au-dessus de la
    # fabrique generique ci-dessus.
    Math/Autodiff/Ops.cpp
    # Bibliotheque de reseaux de neurones maison (LOT-ANNEXE-03) : couche dense, activations,
    # composition en reseau, initialisation des poids, serialisation.
    Nn/Dense.cpp
    Nn/WeightInit.cpp
    Nn/Activations.cpp
    Nn/Network.cpp
    Nn/Serialization.cpp
    # Optimiseurs maison (LOT-ANNEXE-04) : relient le gradient accumule par l'autodiff (Node::grad)
    # aux parametres exposes par un reseau (Network::parameters), IOptimizer.h reste header-only.
    Optim/OptimizerUtils.cpp
    Optim/Sgd.cpp
    Optim/Adam.cpp
    # Environnement de simulation headless (LOT-ANNEXE-05) : rejoue un niveau pas a pas, sans
    # fenetre ni GPU, derriere une API reset/step -- lie uniquement a Core (voir ci-dessous).
    Env/HeadlessLevelEnvironment.cpp
    # Encodage de l'observation (LOT-ANNEXE-06) : traduit l'etat lu via HeadlessLevelEnvironment en
    # tenseurs consommables par un reseau de neurones (fenetre de tuiles, etat joueur, mecanismes).
    Env/TileWindowEncoder.cpp
    Env/PlayerStateEncoder.cpp
    Env/MechanismStateEncoder.cpp
    # Espace d'action discret, decodage deterministe/stochastique et format de rejeu v1
    # (LOT-ANNEXE-07) : traduisent une distribution produite par un reseau en core::PlayerInput
    # jouable, et exportent une sequence d'actions gagnante dans un fichier rejouable en jeu.
    Env/ActionSpace.cpp
    Env/ActionDecoding.cpp
    Replay/ReplayFile.cpp
    # Fonction de récompense et classification de fin d'épisode (LOT-ANNEXE-08) : signal
    # d'apprentissage unique et partagé par tout algorithme d'apprentissage du programme
    # (générations 2 et 3), condition de comparabilité de leurs résultats (génération 4).
    Env/Reward.cpp
    Env/Episode.cpp
    # Journalisation CSV incrementale des statistiques d'entrainement (LOT-ANNEXE-09) : format de
    # colonnes unique, reutilise sans changement par toute la generation 2/3 puis par le harnais de
    # benchmark (LOT-ANNEXE-15).
    Stats/MovingAverage.cpp
    Stats/CsvFormat.cpp
    Stats/TrainingStatsRecorder.cpp
    Stats/TrainingRunPath.cpp
    # Assemblage plat des encodeurs de LOT-ANNEXE-06 en un vecteur d'entree de reseau unique,
    # introduit par LOT-ANNEXE-10 (premier consommateur reel de cet assemblage).
    Env/ObservationEncoder.cpp
    # Algorithme evolutionniste maison (LOT-ANNEXE-10) : population de reseaux a poids independants
    # (TACHE-01), evaluation de fitness par run complet sur HeadlessLevelEnvironment (TACHE-02),
    # selection/croisement/mutation/elitisme (TACHE-03), boucle de generation journalisee
    # (TACHE-04) -- premiere brique d'apprentissage du programme, ligne de base sans autodiff.
    Training/Evolutionary/NetworkTopology.cpp
    Training/Evolutionary/Population.cpp
    Training/Evolutionary/FitnessEvaluator.cpp
    Training/Evolutionary/GeneticOperators.cpp
    Training/Evolutionary/EvolutionaryTrainer.cpp
    # Entrainement niveau-par-niveau et export de rejeu (LOT-ANNEXE-11) : applique
    # EvolutionaryTrainer a un fichier de niveau reel avec un vrai critere d'arret
    # (LevelTrainingSession), rejoue le meilleur individu final en mode deterministe
    # (DeterministicReplay), puis exporte la sequence gagnante au format v1 de LOT-ANNEXE-07
    # (ReplayExport) -- premier lot du programme a produire un livrable exploitable hors ligne.
    Training/LevelTrainingSession.cpp
    Training/DeterministicReplay.cpp
    Training/ReplayExport.cpp
    # Policy gradient maison REINFORCE (LOT-ANNEXE-12) : premier algorithme du programme dirige par
    # un gradient calcule exactement (autodiff, TACHE-03), par opposition a la recherche aveugle par
    # population de LOT-ANNEXE-10 -- collecte de trajectoire (TACHE-01), retour actualise
    # (TACHE-02), perte REINFORCE retropropagee (TACHE-03), boucle d'entrainement journalisee
    # (TACHE-04).
    Training/PolicyGradient/TrajectoryCollector.cpp
    Training/PolicyGradient/ReturnCalculator.cpp
    Training/PolicyGradient/ReinforceLoss.cpp
    Training/PolicyGradient/ReinforceTrainer.cpp
)

# Racine d'inclusion = Source/ : les en-têtes s'incluent par chemin complet depuis la racine
# (ex. "AiSolver/Math/Rng.h"), comme Core/HMI.
target_include_directories(AiSolver PUBLIC ${PROJECT_SOURCE_DIR}/Source)

# PUBLIC (pas PRIVATE) : les en-têtes header-only (Tensor.h, TensorOps.h, Matmul.h) utilisent
# PROJECTGAMING_ASSERT (Core/Diagnostics/Assert.h) et sont inclus directement par les consommateurs
# d'AiSolver -- il leur faut donc le chemin d'inclusion et l'édition de liens de Core, pas
# seulement Rng.cpp (seul fichier compilé dans cette bibliothèque).
# PRIVATE : seul Replay/ReplayFile.cpp (LOT-ANNEXE-07) utilise nlohmann::json directement, aucun
# en-tete public d'AiSolver ne l'expose (meme raison que Core PRIVATE ci-dessus).
target_link_libraries(AiSolver PUBLIC Core project_warnings project_options
    PRIVATE nlohmann_json::nlohmann_json)

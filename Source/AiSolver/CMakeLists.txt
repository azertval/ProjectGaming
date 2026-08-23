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

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
    # Voisinage du champ d'objectif rendu visible a l'agent : la recompense mesurait une direction
    # que l'observation ne contenait pas, ce qu'aucune politique sans memoire ne peut deviner sur un
    # niveau a plusieurs salles.
    Env/ObjectiveEncoder.cpp
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
    # Champ de distances de plus court chemin sur la grille (amendement LOT-ANNEXE-08, EX-IA-023) :
    # remplace la distance euclidienne en ligne droite dans Env/Reward.cpp pour les niveaux où un
    # mur force un détour.
    Env/GridDistanceField.cpp
    Env/Episode.cpp
    # Budget de pas et seuil de blocage derives du niveau : la longueur de la chaine d'objectifs
    # varie d'un facteur dix entre demo-wall-jump.json et demo-final.json, une constante globale ne
    # peut donc etre juste aux deux bouts -- elle coupait les episodes des niveaux longs bien avant
    # leur terme.
    Env/StepBudget.cpp
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
    # Rejeu deterministe generalise a n'importe quel eval::TrainedPolicy (LOT-ANNEXE-19, extrait en
    # LOT-ANNEXE-21) : partage entre aisolver-cli (Commands.cpp) et l'IHM (HMI/Ai/TrainingWorker),
    # jamais duplique.
    Training/ArgmaxRollout.cpp
    # Resolution/tracabilite des hyperparametres (LOT-ANNEXE-19, TACHE-02) : dans la bibliotheque
    # (pas seulement l'executable aisolver-cli, cf. add_executable ci-dessous) pour que
    # HMI/Ai/TrainingWorker (LOT-ANNEXE-21) reutilise les memes types/defauts sans les dupliquer.
    Cli/TrainingConfig.cpp
    # Policy gradient maison REINFORCE (LOT-ANNEXE-12) : premier algorithme du programme dirige par
    # un gradient calcule exactement (autodiff, TACHE-03), par opposition a la recherche aveugle par
    # population de LOT-ANNEXE-10 -- collecte de trajectoire (TACHE-01), retour actualise
    # (TACHE-02), perte REINFORCE retropropagee (TACHE-03), boucle d'entrainement journalisee
    # (TACHE-04).
    Training/PolicyGradient/TrajectoryCollector.cpp
    Training/PolicyGradient/ReturnCalculator.cpp
    Training/PolicyGradient/ReinforceLoss.cpp
    Training/PolicyGradient/ReinforceTrainer.cpp
    # Formule de perte de policy gradient commune (LOT-ANNEXE-13), factorisee hors de
    # PolicyGradient/ReinforceLoss.cpp pour etre partagee avec ActorCritic sans dependance d'un
    # sous-dossier de Training vers l'autre.
    Training/PolicyGradientLoss.cpp
    # Acteur-critique (LOT-ANNEXE-13) : reduction de variance du gradient de REINFORCE -- reseau
    # critique estimant la valeur d'etat (TACHE-01), avantage remplacant le retour brut dans la
    # perte de politique (TACHE-02), perte du critique et boucle d'optimisation conjointe
    # acteur/critique (TACHE-03), comparaison chiffree de convergence vs REINFORCE brut (TACHE-04).
    Training/ActorCritic/CriticNetwork.cpp
    Training/ActorCritic/AdvantageCalculator.cpp
    Training/ActorCritic/ActorCriticLoss.cpp
    Training/ActorCritic/CriticLoss.cpp
    Training/ActorCritic/ActorCriticTrainer.cpp
    Eval/ConvergenceComparator.cpp
    # Algorithme avance DQN (LOT-ANNEXE-14) : dernier lot de la generation 3 -- le choix entre PPO
    # et DQN est tranche a l'ouverture du lot (TACHE-01, cf. epic.md) sur la base des resultats de
    # LOT-ANNEXE-13, non concluants sur l'instabilite (niveau de controle trop simple pour la
    # reveler) ; DQN retenu, l'espace d'action discret de LOT-ANNEXE-07 s'y pretant bien. Reseau
    # Q(s,a) et memoire de rejeu (TACHE-01), integration au harnais existant (TACHE-02),
    # comparaison chiffree a quatre series generalisant ConvergenceComparator (TACHE-03).
    Training/Dqn/QNetwork.cpp
    Training/Dqn/ReplayBuffer.cpp
    Training/Dqn/DqnLoss.cpp
    Training/Dqn/DqnTrainer.cpp
    Eval/GenerationComparator.cpp
    # Harnais de benchmark multi-algorithmes (LOT-ANNEXE-15), premier lot de la generation 4 :
    # adaptateur TrainedPolicy uniforme vers chaque famille de modele entraine (TACHE-01),
    # execution automatisee repetee mesurant taux de reussite/pas moyen/variance de la politique
    # elle-meme (pas du rejeu deterministe fige), rapport comparatif CSV par niveau x algorithme
    # (TACHE-02), test de robustesse a un bruit d'observation qui ne touche jamais l'etat reel
    # simule (TACHE-03). Ne depend qu'en lecture de Training/* ; Training/* ne depend jamais d'Eval.
    Eval/TrainedPolicy.cpp
    Eval/EvolutionaryTrainedPolicy.cpp
    Eval/ReinforceTrainedPolicy.cpp
    Eval/ActorCriticTrainedPolicy.cpp
    Eval/AdvancedAlgorithmTrainedPolicy.cpp
    Eval/BenchmarkResult.cpp
    Eval/BenchmarkRunner.cpp
    Eval/BenchmarkReport.cpp
    Eval/NoisyObservation.cpp
    Eval/CrossLevelBenchmark.cpp
    # Validation et stabilisation du format de rejeu v1 (LOT-ANNEXE-17), dernier prealable avant le
    # branchement du rejeu sur le jeu reel (LOT-ANNEXE-18) : empreinte deterministe FNV-1a du
    # fichier de niveau (TACHE-01), validee a la lecture avant toute utilisation du rejeu -- erreur
    # recuperable, jamais un plantage. Format v1 stabilise (TACHE-02, formatVersion 1 -> 2) avec
    # deux champs de commodite (totalDurationSeconds, algorithmId), compatibilite ascendante
    # stricte avec les fichiers deja ecrits par les generations 2/3.
    Replay/LevelFingerprint.cpp
    Replay/ReplayValidation.cpp
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

# Outillage CLI (LOT-ANNEXE-19) : executable distinct de la bibliotheque AiSolver (pas une source
# supplementaire de celle-ci), mince habillage en ligne de commande des sous-commandes
# train/evaluate/export-replay, deleguant integralement aux types deja cadres par les lots amont.
add_executable(aisolver-cli
    Cli/ArgumentParsing.cpp
    Cli/Commands.cpp
    Cli/Main.cpp
)
target_link_libraries(aisolver-cli PRIVATE AiSolver nlohmann_json::nlohmann_json)

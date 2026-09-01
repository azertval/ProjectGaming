var guide_annexe =
[
    [ "Chapitres", "guide-annexe.html#autotoc_md368", null ],
    [ "Convention de notation utilisée dans tout le guide", "guide-annexe.html#autotoc_md369", null ],
    [ "Algèbre tensorielle et calcul numérique", "guide-annexe-algebre-tensorielle.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-algebre-tensorielle.html#autotoc_md275", null ],
      [ "1. Scalaire, vecteur, matrice, tenseur", "guide-annexe-algebre-tensorielle.html#autotoc_md276", null ],
      [ "2. Représenter un tenseur en mémoire : forme, stride, tampon contigu", "guide-annexe-algebre-tensorielle.html#autotoc_md277", [
        [ "2.1. Cas d'une matrice (2 axes)", "guide-annexe-algebre-tensorielle.html#autotoc_md278", null ],
        [ "2.2. Généralisation : le <em>stride</em>", "guide-annexe-algebre-tensorielle.html#autotoc_md279", null ],
        [ "2.3. Ce que ça donne en C++ (illustratif)", "guide-annexe-algebre-tensorielle.html#autotoc_md280", null ]
      ] ],
      [ "3. Opérations élémentaires (élément par élément)", "guide-annexe-algebre-tensorielle.html#autotoc_md281", null ],
      [ "4. Produit matriciel", "guide-annexe-algebre-tensorielle.html#autotoc_md282", null ],
      [ "5. Réductions", "guide-annexe-algebre-tensorielle.html#autotoc_md283", null ],
      [ "6. Pourquoi les erreurs de forme sont des bugs, pas des erreurs à gérer", "guide-annexe-algebre-tensorielle.html#autotoc_md284", null ],
      [ "7. Générateurs pseudo-aléatoires déterministes", "guide-annexe-algebre-tensorielle.html#autotoc_md285", null ],
      [ "Sources", "guide-annexe-algebre-tensorielle.html#autotoc_md286", null ]
    ] ],
    [ "Différentiation automatique et rétropropagation", "guide-annexe-autodiff.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-autodiff.html#autotoc_md314", null ],
      [ "1. La dérivée, intuitivement", "guide-annexe-autodiff.html#autotoc_md315", null ],
      [ "2. La règle de la chaîne", "guide-annexe-autodiff.html#autotoc_md316", null ],
      [ "3. Le graphe de calcul", "guide-annexe-autodiff.html#autotoc_md317", null ],
      [ "4. Mode direct vs mode inverse", "guide-annexe-autodiff.html#autotoc_md318", null ],
      [ "5. Rétropropagation, pas à pas", "guide-annexe-autodiff.html#autotoc_md319", null ],
      [ "6. Pourquoi les gradients s'accumulent (<span class=\"tt\">+=</span>, jamais <span class=\"tt\">=</span>)", "guide-annexe-autodiff.html#autotoc_md320", null ],
      [ "7. Parcours topologique inverse", "guide-annexe-autodiff.html#autotoc_md321", null ],
      [ "8. Vérifier qu'un gradient calculé est correct : les différences finies", "guide-annexe-autodiff.html#autotoc_md322", null ],
      [ "9. Pourquoi une fabrique générique d'opérations (<span class=\"tt\">unaryOp</span>/<span class=\"tt\">binaryOp</span>)", "guide-annexe-autodiff.html#autotoc_md323", null ],
      [ "Sources", "guide-annexe-autodiff.html#autotoc_md324", null ]
    ] ],
    [ "Réseaux de neurones", "guide-annexe-reseaux-neurones.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-reseaux-neurones.html#autotoc_md359", null ],
      [ "1. Le neurone artificiel", "guide-annexe-reseaux-neurones.html#autotoc_md360", null ],
      [ "2. La couche dense", "guide-annexe-reseaux-neurones.html#autotoc_md361", null ],
      [ "3. Pourquoi une fonction d'activation (et pourquoi non linéaire)", "guide-annexe-reseaux-neurones.html#autotoc_md362", null ],
      [ "4. Fonctions d'activation courantes", "guide-annexe-reseaux-neurones.html#autotoc_md363", null ],
      [ "5. Pourquoi l'initialisation des poids compte", "guide-annexe-reseaux-neurones.html#autotoc_md364", null ],
      [ "6. Composer les couches en réseau", "guide-annexe-reseaux-neurones.html#autotoc_md365", null ],
      [ "7. Pourquoi sauvegarder les poids", "guide-annexe-reseaux-neurones.html#autotoc_md366", null ],
      [ "Sources", "guide-annexe-reseaux-neurones.html#autotoc_md367", null ]
    ] ],
    [ "Descente de gradient et optimiseurs", "guide-annexe-optimisation.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-optimisation.html#autotoc_md332", null ],
      [ "1. L'intuition : descendre une pente", "guide-annexe-optimisation.html#autotoc_md333", null ],
      [ "2. Descente de gradient stochastique (SGD) et inertie", "guide-annexe-optimisation.html#autotoc_md334", null ],
      [ "3. Adam : un taux d'apprentissage qui s'adapte", "guide-annexe-optimisation.html#autotoc_md335", null ],
      [ "4. Pourquoi tester la convergence sur des fonctions jouets avant tout usage réel", "guide-annexe-optimisation.html#autotoc_md336", null ],
      [ "Sources", "guide-annexe-optimisation.html#autotoc_md337", null ]
    ] ],
    [ "Vocabulaire de l'apprentissage par renforcement", "guide-annexe-apprentissage-renforcement.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-apprentissage-renforcement.html#autotoc_md295", null ],
      [ "1. Agent, environnement, boucle d'interaction", "guide-annexe-apprentissage-renforcement.html#autotoc_md296", null ],
      [ "2. État, observation", "guide-annexe-apprentissage-renforcement.html#autotoc_md297", null ],
      [ "3. Concevoir une observation : ce que l'agent a le droit de voir", "guide-annexe-apprentissage-renforcement.html#autotoc_md298", [
        [ "3.1. Trois tensions à arbitrer", "guide-annexe-apprentissage-renforcement.html#autotoc_md299", null ],
        [ "3.2. Encoder des catégories : le <em>one-hot</em>", "guide-annexe-apprentissage-renforcement.html#autotoc_md300", null ],
        [ "3.3. Mettre les grandeurs continues à la même échelle", "guide-annexe-apprentissage-renforcement.html#autotoc_md301", null ],
        [ "3.4. Observation partielle et mémoire", "guide-annexe-apprentissage-renforcement.html#autotoc_md302", null ]
      ] ],
      [ "4. Action, politique", "guide-annexe-apprentissage-renforcement.html#autotoc_md303", null ],
      [ "5. Récompense", "guide-annexe-apprentissage-renforcement.html#autotoc_md304", null ],
      [ "6. Concevoir une fonction de récompense : <em>shaping</em> et pièges", "guide-annexe-apprentissage-renforcement.html#autotoc_md305", [
        [ "6.1. Récompense creuse contre récompense dense", "guide-annexe-apprentissage-renforcement.html#autotoc_md306", null ],
        [ "6.2. Le piège : l'agent optimise la lettre, pas l'intention", "guide-annexe-apprentissage-renforcement.html#autotoc_md307", null ],
        [ "6.3. Ajouter du guidage sans changer l'objectif : le <em>shaping</em> par potentiel", "guide-annexe-apprentissage-renforcement.html#autotoc_md308", null ],
        [ "6.4. Ordres de grandeur et conflits entre termes", "guide-annexe-apprentissage-renforcement.html#autotoc_md309", null ]
      ] ],
      [ "7. Épisode, horizon", "guide-annexe-apprentissage-renforcement.html#autotoc_md310", null ],
      [ "8. Le cadre théorique : processus de décision markovien", "guide-annexe-apprentissage-renforcement.html#autotoc_md311", null ],
      [ "9. Pourquoi mesurer un plafond de performance (le CSV de statistiques)", "guide-annexe-apprentissage-renforcement.html#autotoc_md312", null ],
      [ "Sources", "guide-annexe-apprentissage-renforcement.html#autotoc_md313", null ]
    ] ],
    [ "Algorithmes évolutionnistes", "guide-annexe-algorithmes-evolutionnistes.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-algorithmes-evolutionnistes.html#autotoc_md287", null ],
      [ "1. L'idée centrale : sélection artificielle de poids", "guide-annexe-algorithmes-evolutionnistes.html#autotoc_md288", null ],
      [ "2. Sélection : comment choisir les parents", "guide-annexe-algorithmes-evolutionnistes.html#autotoc_md289", null ],
      [ "3. Croisement : combiner deux réseaux", "guide-annexe-algorithmes-evolutionnistes.html#autotoc_md290", null ],
      [ "4. Mutation : introduire de la nouveauté", "guide-annexe-algorithmes-evolutionnistes.html#autotoc_md291", null ],
      [ "5. Pourquoi cette approche fonctionne sans gradient", "guide-annexe-algorithmes-evolutionnistes.html#autotoc_md292", null ],
      [ "6. Pourquoi la reproductibilité à graine fixée est essentielle ici", "guide-annexe-algorithmes-evolutionnistes.html#autotoc_md293", null ],
      [ "Sources", "guide-annexe-algorithmes-evolutionnistes.html#autotoc_md294", null ]
    ] ],
    [ "Policy gradient et REINFORCE", "guide-annexe-reinforce.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-reinforce.html#autotoc_md351", null ],
      [ "1. Le problème : la récompense n'est pas différentiable", "guide-annexe-reinforce.html#autotoc_md352", null ],
      [ "2. L'astuce du log-gradient (<em>log-derivative trick</em>)", "guide-annexe-reinforce.html#autotoc_md353", null ],
      [ "3. La règle de mise à jour REINFORCE", "guide-annexe-reinforce.html#autotoc_md354", null ],
      [ "4. Le retour actualisé", "guide-annexe-reinforce.html#autotoc_md355", null ],
      [ "5. Pourquoi l'exploration stochastique est indispensable pendant l'entraînement", "guide-annexe-reinforce.html#autotoc_md356", null ],
      [ "6. Ce que « rétropropager la perte REINFORCE » signifie concrètement", "guide-annexe-reinforce.html#autotoc_md357", null ],
      [ "Sources", "guide-annexe-reinforce.html#autotoc_md358", null ]
    ] ],
    [ "Acteur-critique", "guide-annexe-acteur-critique.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-acteur-critique.html#autotoc_md268", null ],
      [ "1. D'où vient la variance de REINFORCE", "guide-annexe-acteur-critique.html#autotoc_md269", null ],
      [ "2. L'idée : soustraire une base de comparaison (<em>baseline</em>)", "guide-annexe-acteur-critique.html#autotoc_md270", null ],
      [ "3. Le critique : un réseau qui apprend à estimer <span class=\"tt\">V(s)</span>", "guide-annexe-acteur-critique.html#autotoc_md271", null ],
      [ "4. Ce qui ne change pas par rapport à REINFORCE", "guide-annexe-acteur-critique.html#autotoc_md272", null ],
      [ "5. Pourquoi le critique n'est jamais utilisé à l'évaluation ou à l'export", "guide-annexe-acteur-critique.html#autotoc_md273", null ],
      [ "Sources", "guide-annexe-acteur-critique.html#autotoc_md274", null ]
    ] ],
    [ "PPO et DQN", "guide-annexe-ppo-dqn.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-ppo-dqn.html#autotoc_md338", null ],
      [ "Partie 1 — PPO (Proximal Policy Optimization)", "guide-annexe-ppo-dqn.html#autotoc_md339", [
        [ "1.1. Le problème que PPO résout", "guide-annexe-ppo-dqn.html#autotoc_md340", null ],
        [ "1.2. Le ratio de probabilité", "guide-annexe-ppo-dqn.html#autotoc_md341", null ],
        [ "1.3. Le clip : limiter le ratio", "guide-annexe-ppo-dqn.html#autotoc_md342", null ],
        [ "1.4. Plusieurs passes d'optimisation par lot de trajectoires", "guide-annexe-ppo-dqn.html#autotoc_md343", null ]
      ] ],
      [ "Partie 2 — DQN (Deep Q-Network)", "guide-annexe-ppo-dqn.html#autotoc_md344", [
        [ "2.1. Une approche différente : apprendre une valeur, pas une politique", "guide-annexe-ppo-dqn.html#autotoc_md345", null ],
        [ "2.2. L'équation de Bellman et l'apprentissage par différence temporelle", "guide-annexe-ppo-dqn.html#autotoc_md346", null ],
        [ "2.3. Mémoire de rejeu (<em>replay buffer</em>) et réseau cible", "guide-annexe-ppo-dqn.html#autotoc_md347", null ],
        [ "2.4. Exploration <span class=\"tt\">ε</span>-greedy", "guide-annexe-ppo-dqn.html#autotoc_md348", null ]
      ] ],
      [ "Quelle famille choisir pour LOT-ANNEXE-14 ?", "guide-annexe-ppo-dqn.html#autotoc_md349", null ],
      [ "Sources", "guide-annexe-ppo-dqn.html#autotoc_md350", null ]
    ] ],
    [ "Évaluation et reproductibilité en RL", "guide-annexe-evaluation-rl.html", [
      [ "Pourquoi ce chapitre", "guide-annexe-evaluation-rl.html#autotoc_md325", null ],
      [ "1. Une seule réussite ne prouve pas une compétence fiable", "guide-annexe-evaluation-rl.html#autotoc_md326", null ],
      [ "2. Le taux de réussite n'est qu'une des mesures utiles", "guide-annexe-evaluation-rl.html#autotoc_md327", null ],
      [ "3. Un cas particulier instructif : la politique évolutionniste", "guide-annexe-evaluation-rl.html#autotoc_md328", null ],
      [ "4. Robustesse : un modèle qui « triche » en mémorisant", "guide-annexe-evaluation-rl.html#autotoc_md329", null ],
      [ "5. Le transfert entre niveaux comme mesure, pas comme objectif", "guide-annexe-evaluation-rl.html#autotoc_md330", null ],
      [ "Sources", "guide-annexe-evaluation-rl.html#autotoc_md331", null ]
    ] ]
];
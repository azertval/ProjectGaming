# Vocabulaire de l'apprentissage par renforcement {#guide-annexe-apprentissage-renforcement}

**Lots concernés :** [LOT-ANNEXE-05](@ref lot-annexe-05) à [LOT-ANNEXE-09](@ref lot-annexe-09)
(environnement headless, observation, action, récompense, statistiques).
**Prérequis :** aucun (ce chapitre est le point d'entrée du vocabulaire RL — les chapitres
précédents concernaient uniquement le calcul numérique/réseaux, réutilisables par n'importe quel
domaine d'apprentissage automatique).

## Pourquoi ce chapitre

Jusqu'ici, les chapitres précédents ont posé des outils de calcul génériques (tenseurs, autodiff,
réseaux, optimiseurs) qui existeraient de la même façon pour n'importe quelle tâche d'apprentissage
automatique (reconnaissance d'image, traduction, etc.). Ce chapitre introduit le vocabulaire propre
à l'**apprentissage par renforcement** (*Reinforcement Learning*, RL) — la branche de
l'apprentissage automatique où un agent apprend en **interagissant** avec un environnement, par
essai-erreur, plutôt qu'à partir d'exemples déjà étiquetés (à la différence de l'apprentissage
supervisé, hors sujet ici). C'est le cadre général dans lequel s'inscrivent tous les algorithmes des
générations 2 et 3 du programme.

## 1. Agent, environnement, boucle d'interaction

- L'**agent** est ce qui prend des décisions — dans ce programme, à terme, un réseau de neurones
  (chapitre [Réseaux de neurones](@ref guide-annexe-reseaux-neurones)) qui choisit une action.
- L'**environnement** est le monde avec lequel l'agent interagit — ici, un niveau du jeu, simulé
  par `aisolver::HeadlessLevelEnvironment` (LOT-ANNEXE-05), qui enveloppe fidèlement la même
  simulation (`Core`) que le jeu réel, mais sans fenêtre ni rendu.
- La boucle d'interaction, répétée à chaque **pas** (*step*) :
  1. L'environnement fournit une **observation** de son état courant.
  2. L'agent choisit une **action** à partir de cette observation.
  3. L'environnement applique l'action, avance d'un pas, et renvoie une **récompense** et
     l'observation suivante.
  4. On répète, jusqu'à ce que l'**épisode** se termine.

Cette boucle `reset()`/`step(action) → (observation, récompense, fin ?)` est devenue une
convention quasi universelle en RL logiciel — `HeadlessLevelEnvironment::reset`/`step` en est une
implémentation maison, mais l'idée d'une interface aussi minimale et standardisée doit beaucoup à
la bibliothèque *OpenAI Gym* (voir Sources), qui a popularisé cette forme précise d'API.

## 2. État, observation

L'**état** est la description complète et exacte de la situation dans l'environnement (ici :
position/vitesse exactes du personnage, état de chaque mécanisme, etc. — tout ce qu'il faudrait
pour reproduire exactement la suite de la simulation). L'**observation** est ce que l'agent
**perçoit** de cet état — parfois identique à l'état complet, parfois une version partielle ou
transformée. Dans ce programme, l'observation est le tenseur produit par l'encodeur de
[LOT-ANNEXE-06](@ref lot-annexe-06) (fenêtre de tuiles + état joueur + état des mécanismes), une
transformation de l'état réel de `Core` en une forme exploitable par un réseau (voir
[Réseaux de neurones](@ref guide-annexe-reseaux-neurones)).

## 3. Action, politique

Une **action** est ce que l'agent choisit de faire à un pas donné — ici, un élément de l'espace
d'action discret de [LOT-ANNEXE-07](@ref lot-annexe-07) (direction, saut, dash), converti en
`core::PlayerInput`. Une **politique** (*policy*, souvent notée `π`) est la règle — apprise —
selon laquelle l'agent choisit une action à partir d'une observation. Une politique peut être :
- **déterministe** : toujours la même action pour la même observation (décodage `argmax`,
  [LOT-ANNEXE-07](@ref lot-annexe-07)) ;
- **stochastique** : une distribution de probabilité sur les actions, dont on **tire** une action
  au hasard à chaque pas (décodage `decodeStochastic`) — nécessaire à l'**exploration** pendant
  l'entraînement (voir [REINFORCE](@ref guide-annexe-reinforce)) : une politique toujours certaine
  de son choix ne peut jamais découvrir qu'une autre action ferait mieux.

## 4. Récompense

La **récompense** (*reward*) est un nombre, reçu à chaque pas, qui indique à quel point ce pas a
été « bon ». C'est le **seul** signal à partir duquel l'agent apprend — contrairement à
l'apprentissage supervisé, personne ne dit explicitement à l'agent quelle était la bonne action à
chaque pas, seulement à quel point le résultat était bon ou mauvais. Le choix de la fonction de
récompense (comment traduire « bien jouer » en un nombre) a un impact énorme sur ce que l'agent
finit par apprendre — [LOT-ANNEXE-08](@ref lot-annexe-08) construit cette fonction pour le
programme (progression vers la sortie, bonus de victoire, pénalité de mort/de temps), pensée pour
être **dense** (non nulle à chaque pas) plutôt que **sparse** (seulement à la toute fin) — un signal
dense est bien plus facile à exploiter par un algorithme de gradient (voir
[REINFORCE](@ref guide-annexe-reinforce)), qui a besoin de savoir, à *chaque* pas, si les choses se
sont améliorées ou dégradées.

## 5. Épisode, horizon

Un **épisode** est une séquence complète d'interactions, du début (`reset`) jusqu'à une fin
naturelle — ici, victoire (`Won`), échec (`Lost`), ou une fin artificielle nécessaire pour borner la
durée d'un entraînement automatisé : plafond dur de pas (`TimedOut`) ou blocage détecté (`Stuck`,
absence de progression prolongée) — voir [LOT-ANNEXE-08](@ref lot-annexe-08). La **récompense
cumulée** d'un épisode (parfois appelée le **retour**, *return*) est la somme de toutes les
récompenses reçues pendant cet épisode — c'est, en un sens, ce que tout algorithme d'apprentissage
du programme cherche, in fine, à maximiser (voir
[algorithmes évolutionnistes](@ref guide-annexe-algorithmes-evolutionnistes) et
[REINFORCE](@ref guide-annexe-reinforce) pour deux façons très différentes d'y parvenir).

## 6. Le cadre théorique : processus de décision markovien

Le cadre mathématique standard du RL est le **processus de décision markovien** (*Markov Decision
Process*, MDP) : à tout instant, l'état complet du système détermine entièrement les probabilités
de transition vers l'état suivant (étant donné une action) — l'histoire passée n'ajoute rien de
plus, seul l'état **présent** compte (propriété dite « de Markov »). La simulation de `Core`
respecte cette propriété par construction : elle est déterministe au pas fixe (`EX-NFR-002`), donc
un état donné et une action donnée déterminent **exactement** l'état suivant — un cas particulier
(et le plus simple) de MDP, sans aucune incertitude de transition. Ce chapitre ne développe pas
davantage la théorie formelle du MDP (fonctions de valeur, équation de Bellman) : ces notions sont
introduites précisément là où elles deviennent nécessaires, dans le chapitre
[Acteur-critique](@ref guide-annexe-acteur-critique).

## 7. Pourquoi mesurer un plafond de performance (le CSV de statistiques)

Un entraînement RL ne progresse pas toujours indéfiniment : il finit typiquement par **plafonner**
— la récompense cumulée cesse de s'améliorer d'une génération/d'un épisode à l'autre, signe que
l'agent a extrait tout ce qu'il pouvait de la configuration actuelle (algorithme, réseau,
hyperparamètres). Détecter ce plafond **tôt** (plutôt que de laisser tourner un entraînement
infructueux) est exactement le rôle du CSV de [LOT-ANNEXE-09](@ref lot-annexe-09) — une
préoccupation pratique et opérationnelle, pas seulement théorique, essentielle dès qu'on entraîne
sans supervision constante.

## Sources

- Sutton, R.S., Barto, A.G. (2018). *Reinforcement Learning: An Introduction* (2nd ed.). MIT Press.
  — **la** référence de base de tout ce chapitre et des suivants ; couvre agent/environnement/état/
  action/récompense/politique/épisode/retour et le cadre du MDP en détail. Édition consultable
  gratuitement en ligne (site de l'auteur) une fois de retour avec accès Internet.
- Bellman, R. (1957). *A Markovian Decision Process*. Journal of Mathematics and Mechanics 6(5),
  679–684. — origine du processus de décision markovien.
- Brockman, G., Cheung, V., Pettersson, L., Schneider, J., Schulman, J., Tang, J., Zaremba, W.
  (2016). *OpenAI Gym*. arXiv:1606.01540. — popularise l'interface `reset`/`step` comme standard de
  fait pour un environnement RL logiciel, dont `HeadlessLevelEnvironment` (LOT-ANNEXE-05) s'inspire
  dans sa forme (sans aucune dépendance à cette bibliothèque, réimplémentée entièrement en C++
  maison).
- Puterman, M.L. (1994). *Markov Decision Processes: Discrete Stochastic Dynamic Programming*.
  Wiley. — traitement mathématique approfondi des MDP, au-delà de ce qui est nécessaire ici.

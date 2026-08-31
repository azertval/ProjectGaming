# Regarder l'IA jouer {#manuel-mode-ia}

Le jeu embarque une **IA capable d'apprendre à terminer un niveau toute seule**. Elle n'a rien
d'un robot programmé à l'avance : elle commence en appuyant sur les touches au hasard, et
s'améliore uniquement en constatant ce qui la rapproche de la sortie.

Cette page s'adresse aux joueurs curieux — aucune connaissance en programmation n'est nécessaire.
L'écran s'ouvre depuis l'entrée **Mode IA** du menu principal.

## Ce que fait l'IA, en une phrase

Elle rejoue le même niveau des milliers de fois, garde ce qui marche, et recommence. Rien n'est
calculé pendant que vous regardez : ce que vous voyez à la fin est l'**enregistrement** d'une
partie qu'elle a réellement réussie, rejoué touche par touche.

## Les trois onglets

### Entraînement

Vous choisissez un **niveau** et un **algorithme**, puis vous lancez. Un tableau se remplit au fur
et à mesure : chaque ligne est une étape de l'apprentissage, avec le meilleur score obtenu, la
moyenne, et le pourcentage de tentatives réussies.

Les quatre algorithmes disponibles n'apprennent pas de la même façon :

| Algorithme | Comment il s'y prend |
|---|---|
| **Évolutif** | Fait jouer une population entière, garde les meilleurs, les recombine et les modifie légèrement — comme une sélection naturelle accélérée. C'est le plus simple, et souvent le plus rapide à donner un résultat. |
| **REINFORCE** | Un seul joueur, qui ajuste ses réflexes après chaque partie selon qu'elle s'est bien ou mal terminée. |
| **Acteur-critique** | Comme le précédent, mais avec un second « juge » qui estime si une situation est prometteuse — l'apprentissage est moins erratique. |
| **Avancé (DQN)** | Mémorise ses parties passées et les rejoue mentalement pour en tirer plus de leçons. |

L'entraînement peut durer longtemps. Le bouton **Arrêter** l'interrompt proprement à la fin de
l'étape en cours : le meilleur résultat obtenu jusque-là reste sauvegardé.

**Voir en jeu** rejoue dans la scène du niveau la meilleure tentative du moment. C'est le moyen le
plus parlant de constater les progrès : au début l'IA meurt tout de suite, puis elle apprend à
sauter, puis à contourner, puis à finir.

À la fin d'un entraînement, un bouton **Ouvrir le dossier du run** donne accès aux fichiers produits
(le modèle entraîné, l'historique complet en CSV, la configuration effectivement utilisée).

#### Les réglages de l'onglet Entraînement

L'écran n'affiche que les réglages utiles à l'algorithme choisi — sélectionner **DQN** fait
apparaître son propre groupe de réglages avancés, par exemple, invisible pour les trois autres
algorithmes.

| Réglage | S'applique à | Ce qu'il change |
|---|---|---|
| Taille de population | Évolutif | Nombre de « joueurs » simultanés à chaque étape. Plus grand explore plus de façons de jouer par étape, mais coûte plus cher à calculer. |
| Taux de mutation | Évolutif | Probabilité qu'un réflexe soit légèrement modifié au hasard d'une étape à l'autre — trop bas, l'IA n'explore rien de nouveau ; trop haut, elle oublie ce qui marchait. |
| Épisodes | REINFORCE, Acteur-critique, DQN | Nombre maximal de tentatives avant l'arrêt automatique, même sans résultat. |
| Générations max | Évolutif | Plafond d'étapes : l'entraînement s'arrête là même sans résultat. |
| Réussites stables exigées | Évolutif | Combien d'étapes d'affilée le meilleur joueur doit rester invaincu **et** terminer le niveau pour que l'entraînement s'arrête sur une réussite. |
| Taille du tournoi | Évolutif | Combien de candidats sont tirés au sort pour désigner un « parent ». Plus grand favorise les meilleurs et fait converger plus vite, au prix de la diversité. |
| Force de mutation | Évolutif | De combien un réflexe modifié est modifié. Le taux de mutation dit à quelle fréquence ; la force dit de combien. |
| Taille de couche cachée | Tous | Nombre de neurones de la couche interne du réseau. Plus grand permet des comportements plus fins, mais apprend plus lentement. Un modèle ne se recharge que sur la taille qui l'a produit. |
| Dossier des runs | Tous | Où sont écrits les entraînements. Par défaut, `TrainingRuns/` à côté du jeu. |
| Taux d'apprentissage | REINFORCE, Acteur-critique, DQN | À quelle vitesse l'IA corrige ses réflexes après chaque partie — trop haut, l'apprentissage devient instable. |
| Gamma (actualisation) | REINFORCE, Acteur-critique, DQN | Combien l'IA valorise une récompense lointaine par rapport à une récompense immédiate. |
| Optimiseur | REINFORCE, Acteur-critique, DQN | La méthode de calcul utilisée pour corriger les réflexes (`sgd` ou `adam`, deux variantes classiques). |
| Taux d'apprentissage du critique | Acteur-critique | Séparé de celui ci-dessus, et bien plus élevé : le « critique » doit apprendre à estimer un score qui vaut une centaine de points, là où la politique n'a qu'à départager des préférences proches de l'unité. |
| Épisodes par lot | REINFORCE, Acteur-critique | Combien de parties sont jouées avant chaque correction. Un lot plus grand rend la correction plus fiable, mais en fait beaucoup moins pour le même temps de calcul. |
| Entropie | REINFORCE, Acteur-critique | Récompense l'IA pour rester indécise. Sans ce réglage, une IA finit par toujours rejouer la même partie, à l'identique, et cesse d'apprendre. |
| Exploration minimale | REINFORCE, Acteur-critique | Part de mouvements tirés complètement au hasard, quoi que l'IA préfère. C'est le filet de sécurité de l'entropie : il garantit qu'aucun mouvement ne devienne jamais impossible à essayer. |
| Écrêtage du gradient | REINFORCE, Acteur-critique | Plafonne l'ampleur d'une correction, pour qu'une partie aberrante ne détruise pas d'un coup ce qui a été appris. |
| Images par décision | REINFORCE, Acteur-critique, DQN | Combien d'images de jeu une décision est maintenue. À une image, les directions tirées au hasard se compensent et l'IA n'avance pas ; quelques images rendent son exploration réellement directionnelle. |
| Budget de pas | Tous | Durée maximale d'une partie, en images. **Laissé à zéro, il est calculé à partir du niveau** : le trajet à parcourir y est mesuré case par case, mécanismes compris. Un niveau court reçoit un petit budget, `demo-final` en reçoit plusieurs milliers. |
| Seuil de blocage | Tous | Au bout de combien d'images sans progrès une partie est abandonnée. Laissé à zéro, il est lui aussi déduit du budget. |
| Taux de croisement | Évolutif | Fréquence à laquelle un enfant mélange deux parents plutôt que de copier l'un d'eux. Copier permet à une trouvaille de survivre intacte d'une étape à l'autre. |
| Graine | Tous | Fixe le hasard de départ : relancer un entraînement avec la même graine reproduit exactement le même déroulement. |
| Groupe DQN avancé | DQN uniquement | Réglages de la mémoire des parties passées que DQN rejoue mentalement (capacité, taille des lots, rythme des mises à jour) et de son exploration (probabilité de tenter un mouvement au hasard plutôt que celui qu'elle croit le meilleur, décroissante au fil de l'entraînement). |

#### Comment fonctionne la population (algorithme Évolutif)

À chaque étape, la population entière (sa taille est le réglage ci-dessus) rejoue le niveau une
fois chacune. Le meilleur joueur de l'étape est conservé tel quel pour l'étape suivante — il ne
peut jamais être perdu. Le reste de la population suivante est reconstitué en tirant au sort des
« parents » parmi les meilleurs de l'étape (les plus mauvais ont moins de chances d'être choisis),
en mélangeant leurs réflexes, puis en y appliquant le taux et la force de mutation ci-dessus. L'étape
suivante rejoue alors avec cette nouvelle population, et ainsi de suite jusqu'à ce qu'un joueur
termine le niveau de façon stable pendant le nombre d'étapes exigé, ou jusqu'au plafond de
générations — deux réglages de l'écran.

Pendant l'entraînement, l'indicateur **Générations stables** montre où en est cette série (par
exemple « 3 / 5 »), et une estimation du **temps restant** s'affiche sous la barre de progression.
Pour l'algorithme DQN, l'indicateur **Epsilon courant** montre la probabilité, décroissante, que
l'IA tente encore un mouvement au hasard.

#### Enregistrer et réutiliser des réglages

Trois boutons, sous les réglages : **Enregistrer la configuration** écrit tous les réglages dans un
fichier ; **Charger une configuration** les relit ; **Réinitialiser aux défauts** revient aux
valeurs d'origine. C'est le même format de fichier que celui déposé dans chaque dossier de run —
donc l'onglet Validation propose aussi **Reprendre les réglages de ce run**, qui recharge dans
l'onglet Entraînement les réglages exacts d'un entraînement passé, pour le relancer à l'identique
ou n'en faire varier qu'un seul.

#### Le fichier `stats.csv`

Chaque étape écrit une ligne dans un fichier `stats.csv` (une ligne par étape, jamais par joueur
individuel) : c'est ce qui alimente le tableau et le graphique de suivi pendant l'entraînement, et
ce qui reste consultable après coup depuis le dossier du run. Ce fichier est toujours écrit — son
coût ne dépend pas de la taille de la population (une population de 500 ne produit pas plus de
lignes qu'une population de 32, seulement plus de calcul par étape).

#### Traces du moteur pendant l'entraînement

Séparément de ce fichier `stats.csv`, le moteur du jeu journalise lui-même certains événements
(chargement de niveau, déclenchement d'un mécanisme…) — utile pendant une partie normale, mais
chaque tentative d'un individu pendant l'entraînement rejoue le niveau depuis le début, ce qui
produisait auparavant une trace par tentative, sans rapport avec une partie réellement jouée, et
ralentissait l'entraînement pour une grande population. Ceci est désormais automatique : le jeu
réduit lui-même ces traces le temps d'un entraînement ou d'une évaluation, sans réglage à faire.

### Validation & sauvegarde

Reprend un entraînement passé et mesure ce qu'il vaut vraiment, en le faisant rejouer plusieurs
fois d'affilée. On y lit le taux de réussite et le nombre de pas moyen — un modèle qui gagne une
fois sur deux n'est pas un modèle qui a compris le niveau.

La mesure tourne en arrière-plan : la fenêtre reste utilisable, une barre montre l'avancement, et
le bouton **Annuler** l'interrompt à la fin de la répétition en cours (les répétitions déjà jouées
restent comptées).

Choisir un entraînement dans la liste remplit le modèle et le niveau, mais **rien n'oblige à les
garder** : mesurer un modèle sur un *autre* niveau que celui qui l'a entraîné dit s'il a compris
quelque chose de général ou seulement appris un tableau par cœur. Un modèle sauvegardé ailleurs
peut aussi être ouvert directement.

Les réglages de mesure sont le nombre de **répétitions**, le **budget de pas** au-delà duquel un
essai est abandonné, la **graine** (même graine, mêmes essais), et le **mode de décodage** :
*argmax* joue toujours le coup jugé le meilleur, *stochastic* tire au sort selon les préférences du
modèle. **Exporter le rapport CSV** enregistre le résultat pour le comparer ailleurs.

**Exporter comme rejeu publié** refait jouer le modèle et enregistre la partie dans
`Elements/Replays/`, d'où l'onglet Rejeu la relit. Un rejeu publié ne peut être qu'une réussite : si
le modèle ne termine pas le niveau, le jeu vous le dit et n'écrit rien.

### Rejeu

Liste les parties enregistrées livrées avec le jeu, et les rejoue. Un enregistrement est lié au
niveau exact sur lequel il a été produit : si ce niveau a été modifié depuis, le jeu **refuse** de
le rejouer plutôt que de vous montrer une partie qui déraille au bout de dix secondes.

## Questions fréquentes

**L'IA triche-t-elle ?** Non. Elle joue avec les mêmes touches que vous, dans le même moteur, à la
même cadence. Elle voit ce qui l'entoure sur quelques cases, sa propre vitesse, et l'état des
mécanismes proches — pas le plan du niveau.

**Pourquoi l'entraînement est-il lent ?** Parce qu'elle apprend en essayant. Les premiers milliers
de tentatives servent surtout à découvrir que tomber dans le vide est une mauvaise idée.

**Est-ce que ça marche sur n'importe quel niveau ?** Sur les niveaux courts, oui, assez vite. Plus
un tableau est long et demande d'enchaîner des mécanismes différents, plus il lui faut de temps —
et rien ne garantit qu'elle y arrive.

**Mes entraînements sont-ils conservés ?** Oui, dans un dossier `TrainingRuns/` à côté du jeu. Ils
ne sont pas envoyés ailleurs et ne partent pas avec le jeu si vous le partagez.

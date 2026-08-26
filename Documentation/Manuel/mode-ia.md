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
| Épisodes / générations max | Tous sauf Évolutif (plafond de générations pour Évolutif, non réglable ici) | Nombre maximal de tentatives avant l'arrêt automatique, même sans résultat. |
| Taux d'apprentissage | REINFORCE, Acteur-critique, DQN | À quelle vitesse l'IA corrige ses réflexes après chaque partie — trop haut, l'apprentissage devient instable. |
| Gamma (actualisation) | REINFORCE, Acteur-critique, DQN | Combien l'IA valorise une récompense lointaine par rapport à une récompense immédiate. |
| Optimiseur | REINFORCE, Acteur-critique, DQN | La méthode de calcul utilisée pour corriger les réflexes (`sgd` ou `adam`, deux variantes classiques). |
| Graine | Tous | Fixe le hasard de départ : relancer un entraînement avec la même graine reproduit exactement le même déroulement. |
| Groupe DQN avancé | DQN uniquement | Réglages de la mémoire des parties passées que DQN rejoue mentalement (capacité, taille des lots, rythme des mises à jour) et de son exploration (probabilité de tenter un mouvement au hasard plutôt que celui qu'elle croit le meilleur, décroissante au fil de l'entraînement). |

#### Comment fonctionne la population (algorithme Évolutif)

À chaque étape, la population entière (sa taille est le réglage ci-dessus) rejoue le niveau une
fois chacune. Le meilleur joueur de l'étape est conservé tel quel pour l'étape suivante — il ne
peut jamais être perdu. Le reste de la population suivante est reconstitué en tirant au sort des
« parents » parmi les meilleurs de l'étape (les plus mauvais ont moins de chances d'être choisis),
en mélangeant leurs réflexes, puis en y appliquant le taux de mutation ci-dessus. L'étape suivante
rejoue alors avec cette nouvelle population, et ainsi de suite jusqu'à ce qu'un joueur termine le
niveau de façon stable plusieurs étapes d'affilée, ou jusqu'au plafond de générations.

#### La case « Pas de trace CSV pour ce run »

Chaque étape écrit une ligne dans un fichier `stats.csv` (une ligne par étape, jamais par joueur
individuel) : c'est ce qui alimente le tableau et le graphique de suivi pendant l'entraînement.
Ce coût ne dépend donc pas de la taille de la population — une population de 500 ne produit pas
plus de lignes qu'une population de 32, seulement plus de calcul par étape. La case sert plutôt
pour un entraînement à très grand nombre d'étapes, ou lancé depuis un disque contraint : elle
supprime l'écriture du fichier, sans rien changer au tableau et au graphique affichés à l'écran.

### Validation & sauvegarde

Reprend un entraînement passé et mesure ce qu'il vaut vraiment, en le faisant rejouer plusieurs
fois d'affilée. On y lit le taux de réussite et le nombre de pas moyen — un modèle qui gagne une
fois sur deux n'est pas un modèle qui a compris le niveau.

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

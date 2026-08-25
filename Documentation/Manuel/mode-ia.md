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

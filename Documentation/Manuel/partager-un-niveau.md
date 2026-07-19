# Créer et partager un niveau (sans ligne de commande) {#manuel-partager-niveau}

Ce guide s'adresse aux **créateurs de niveaux** (level design) sans connaissance en
programmation. Il explique comment créer un niveau dans l'éditeur intégré, puis le partager avec
le reste de l'équipe via une interface Git graphique — **jamais de ligne de commande**.

## 1. Récupérer le projet

1. Installer [GitHub Desktop](https://desktop.github.com/) (gratuit).
2. Ouvrir GitHub Desktop, se connecter avec un compte GitHub (en créer un si besoin — gratuit).
3. **File → Clone repository**, choisir `azertval/ProjectGaming`, puis un dossier sur votre
   ordinateur. Le bouton **Clone** télécharge tout le projet.

## 2. Lancer l'éditeur

Récupérez l'exécutable du jeu (`ProjectGaming.exe`) — voir
[Télécharger et lancer le jeu](@ref manuel-telecharger) si vous n'avez pas encore compilé le
projet vous-même. Depuis le menu principal, choisissez **Mode Édition**.

## 3. Créer un niveau

À l'ouverture de l'éditeur, une liste apparaît : **Nouveau niveau**, ou l'un des niveaux déjà
enregistrés. Naviguez avec les flèches **↑**/**↓**, validez avec **Entrée**.

Une fois dans la grille :

| Action | Comment |
|--------|---------|
| Peindre une tuile | Cliquer une case de la **palette** (en haut à gauche) pour choisir son type, puis cliquer (ou cliquer-glisser) sur la grille. |
| Placer l'entrée / la sortie | Choisir *Entrée* ou *Sortie* dans la palette, puis cliquer la case voulue — l'ancienne position se déplace automatiquement (un niveau n'a qu'une seule entrée et une seule sortie). |
| Relier un interrupteur à une porte | Peindre d'abord un *Interrupteur* et une *Porte*, puis maintenir **Maj** et cliquer l'interrupteur, puis (Maj toujours enfoncé) cliquer la porte. Recommencer la même paire retire la liaison. |
| Agrandir / réduire la grille | Flèches **←**/**→** pour la largeur, **↑**/**↓** pour la hauteur. |
| Annuler / refaire | **Ctrl+Z** / **Ctrl+Y**. |
| Tester le niveau | Touche **P** — le niveau se joue directement dans l'éditeur ; **Échap** pour revenir à l'édition, rien n'est perdu. |
| Enregistrer | **Ctrl+S** — un message en bas de l'écran confirme l'enregistrement, ou explique le problème si le niveau n'est pas encore jouable (par exemple : il manque une sortie). |
| Quitter l'éditeur | **Échap** (hors essai immédiat) — retour au menu. |

Le niveau est enregistré à côté de l'exécutable, dans le dossier `Levels`.

## 4. Publier votre niveau

1. Copiez le fichier de votre niveau (`Levels\<nom>.json`, à côté de `ProjectGaming.exe`) dans le
   dossier `Source/Elements/Levels/` de votre copie du projet (celle clonée à l'étape 1).
2. Ouvrez **GitHub Desktop** : votre nouveau fichier apparaît dans la liste des changements.
3. En bas à gauche, donnez un court résumé (ex. « Ajout du niveau Foret-1 ») et cliquez
   **Commit to main**.
4. Cliquez **Push origin** (en haut) : votre niveau est envoyé sur GitHub, visible par toute
   l'équipe.

## 5. Récupérer les niveaux des autres

Dans GitHub Desktop, cliquez **Fetch origin** puis **Pull origin** : les niveaux ajoutés par
d'autres membres de l'équipe apparaissent dans votre dossier `Source/Elements/Levels/`, prêts à
être ouverts dans l'éditeur.

## En cas de problème

- **L'enregistrement affiche un message d'erreur** : le niveau n'a pas encore d'entrée et/ou de
  sortie, ou une porte n'est pas correctement reliée — le message précise le problème. Corrigez et
  recommencez **Ctrl+S**.
- **GitHub Desktop signale un conflit** : deux personnes ont modifié le *même* fichier de niveau.
  Donnez des **noms de fichiers différents** à vos niveaux pour éviter ce cas.

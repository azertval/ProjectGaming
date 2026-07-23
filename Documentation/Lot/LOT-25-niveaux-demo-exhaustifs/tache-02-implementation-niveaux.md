# TACHE-02 — Implémentation des niveaux {#lot-25-tache-02-implementation-niveaux}

**Lot :** [LOT-25](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** à faire

## Contexte
Traduit le tableau mécanique → niveau(x) de `TACHE-01` en fichiers de niveaux réels, via
l'éditeur intégré (`@ref guide-editeur`) plutôt qu'en JSON écrit à la main — cohérent avec
`EX-EDIT-*` (l'édition passe par l'outil, pas par le texte brut).

## Travail à réaliser
- Pour chaque niveau **nouveau** identifié en `TACHE-01` : le concevoir dans l'éditeur intégré
  (peinture, mécanismes, essai immédiat `P`), l'enregistrer dans `Source/Elements/Levels/`.
- Pour chaque niveau **existant** à compléter : l'ouvrir dans l'éditeur, ajouter les éléments
  requis (ex. une plateforme nécessitant le double saut), en s'assurant que les mécaniques déjà
  exercées par ce niveau restent nécessaires (ne pas régresser un niveau existant en le modifiant).
- Nommer les fichiers de façon **traçable** à leur mécanique (ex. `demo-double-saut.json`,
  `demo-pente.json`) plutôt que de poursuivre la numérotation `demoN.json` arbitraire — plus lisible
  une fois le nombre de niveaux étendu au-delà de 5.

## Fichiers impactés
- `Source/Elements/Levels/*.json` (nouveaux et modifiés, selon le tableau de `TACHE-01`).

## Tests (obligatoires)
- Chaque niveau, testé immédiatement dans l'éditeur (`P`) : franchissable en utilisant la mécanique
  visée, **infranchissable** sans elle (vérification manuelle, cohérente avec le critère
  d'acceptation 1 de l'épic).

## Points d'attention
- **Un niveau qui ne teste rien n'a pas sa place** : si un niveau existant s'avère franchissable
  sans jamais utiliser la mécanique qu'il était censé isoler (dérive constatée en le rejouant),
  le corriger avant de passer à `TACHE-03` plutôt que de livrer un test système qui ne teste rien
  en pratique.
- Conserver la convention de nommage des fichiers de niveaux **stable** une fois choisie en début
  de tâche : `TACHE-03` (séquence + test système) référence ces noms directement.

## Définition de fait (DoD)
- Tous les niveaux du tableau de `TACHE-01` existent, sont enregistrés, et vérifiés manuellement
  franchissables **avec** la mécanique visée.

## Exigences
Aucune exigence propre — production de contenu, pas de code moteur.

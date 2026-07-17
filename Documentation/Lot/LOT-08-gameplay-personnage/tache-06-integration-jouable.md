# TACHE-06 — Intégration jouable dans `GameScreen` (caméra, succès / échec) {#lot-08-tache-06-integration-jouable}

**Lot :** [LOT-08](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** à faire

## Contexte
Dernière tâche : assembler les briques `Core` (physique, balayage, règles) et le mapping d'entrée
dans le `GameScreen`, qui n'affiche pour l'instant qu'un niveau **statique** (LOT-07). C'est une
**brique GPU/écran** : vérifiée **visuellement**, non testée en unitaire (précédent du projet), la
logique sous-jacente étant déjà couverte par TACHE-02 à TACHE-05.

## Travail à réaliser
- **Apparition du personnage** : à l'entrée du niveau (`Level::entry`), créer une entité
  `Player` + `Transform` + `Velocity` + `Collider` + `Sprite` (région d'atlas du personnage,
  `EX-REN-011`).
- **Boucle de simulation** (dans `update`, à pas fixe) :
  1. mapper l'`InputState` → `PlayerInput` (TACHE-05) ;
  2. exécuter `CharacterPhysicsSystem` (TACHE-03) ;
  3. évaluer `LevelOutcome` (TACHE-04) sur la boîte du personnage.
- **Verdict** : `Won` → transition **retour menu** ; `Lost` → **réinitialiser** le niveau à son
  état initial (reconstruire monde + personnage depuis le `Level` conservé en mémoire, sans relire
  le fichier — `EX-GP-032`). `Playing` → continuer.
- **Caméra qui suit** (`EX-REN-013`) : centrer la caméra sur le personnage, **bornée** aux limites
  du niveau (pas de hors-champ). Réutilise `Camera2D` (LOT-05).
- **Rendu** : le personnage est dessiné par le `SpriteRenderer` existant, par-dessus la grille.
- **Échap** : conserve le retour au menu (comportement LOT-07).

## Fichiers impactés
- `Source/HMI/Interface/GameScreen.h`/`.cpp` (conserver le `Level` chargé, ajouter personnage,
  simulation, caméra suiveuse, gestion succès/échec).
- Éventuellement `Source/HMI/Graphics/Camera2D` (helper de suivi borné, si absent).

## Vérification (visuelle, pas de test unitaire)
- Le personnage **tombe** à l'apparition et **repose** sur le premier sol.
- `←`/`→` (et `Q`/`D`) le déplacent ; il **bute** sur les murs sans les traverser.
- La **caméra suit** et reste **dans** le niveau (pas de bande hors-carte aux bords).
- Atteindre la **sortie** → retour menu ; toucher un **danger** ou **tomber dans le vide** →
  niveau **redémarré** au point d'entrée.
- Capture d'écran de contrôle (procédure visuelle habituelle) jointe à la revue.

## Points d'attention
- **Frontière** : aucune logique de simulation nouvelle ici — le `GameScreen` **orchestre** des
  briques `Core` déjà testées. S'il faut ajouter de la logique testable, la remonter dans `Core`.
- **Réinitialisation propre** : repartir d'un monde **neuf** à l'échec (pas d'état résiduel) —
  d'où l'intérêt de garder le `Level` (données immuables) en mémoire.
- **Caméra bornée** : gérer les niveaux plus petits que la vue (centrage) sans dépasser.
- Câblage des **assets** (région d'atlas du personnage) cohérent avec le LOT-05.

## Définition de fait (DoD)
- `GameScreen` jouable : déplacement + gravité + collisions + caméra suiveuse + succès/échec
  fonctionnels et **vérifiés visuellement** ; build `/W4 /WX`, `CHANGELOG.md` et Doxygen à jour.

## Exigences
`EX-GP-010`, `EX-GP-012`, `EX-GP-014`, `EX-GP-030`, `EX-GP-031`, `EX-GP-032`, `EX-REN-013`,
`EX-REN-011`, `EX-CTRL-010`.

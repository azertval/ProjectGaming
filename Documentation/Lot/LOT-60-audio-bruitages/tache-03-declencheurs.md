# TACHE-03 — Déclencheurs depuis les transitions d'état {#lot-60-tache-03-declencheurs}

**Lot :** [LOT-60](epic.md) · **Emplacement :** `Source/HMI/Game`, `Source/HMI/Interface` ·
**Statut :** non commencé

## Contexte
C'est ici que se joue le respect de `EX-ARCH-012` : la présentation ne modifie jamais la simulation.
La tentation naturelle est d'appeler la lecture d'un son **depuis** le code de gameplay, à l'endroit
exact où le saut est décidé — c'est une ligne, et elle marche. Elle mettrait une dépendance
périphérique dans `Core`, briserait la testabilité sans matériel (`EX-NFR-010`) et ferait dépendre
la simulation de l'ordre d'exécution de la présentation.

Le projet a déjà résolu ce problème pour le rendu : `Core` **expose un état**, `HMI` l'observe et
en déduit ce qu'il dessine. `hmi::MechanismVisuals` (`LOT-47`) fait exactement cela — état logique
d'un mécanisme → clip d'animation, fonction pure. Les sons suivent la même route.

Ces déclencheurs sont par ailleurs **partagés avec [LOT-53](@ref lot-53)** : les particules
réagissent aux mêmes transitions. Ils sont donc écrits ici sous une forme réutilisable, une fois.

## Travail à réaliser
- **Détection de transitions** : à partir de l'état de `core::Player` et des contrôleurs de
  mécanismes au pas courant et au pas précédent, produire la liste des **événements** survenus
  (a sauté, a atterri, a dashé, a touché un mur, est mort, a terminé). Fonction **pure**, sans Qt ni
  périphérique, dans `HMI`.
- **Table événement → son**, distincte de la détection : ce qui **est arrivé** et ce que **ça
  produit** sont deux questions séparées — c'est ce qui permettra à `LOT-53` de brancher des
  particules sur la même détection sans la modifier.
- **Événements de mécanismes** : interrupteur basculé, porte ouverte/fermée, plaque enfoncée/
  relâchée, bloc poussé. Lus depuis `core::MechanismController` et `core::BlockController`, sans
  les modifier.
- **Événements d'interface** : déplacement dans un menu, validation, retour, ouverture de la pause,
  fin de tableau, fin de séquence — ces derniers sur les écrans livrés par
  [LOT-59](@ref lot-59).
- **Un son par événement et par pas** : une transition détectée deux fois (rendu plus rapide que la
  simulation) ne doit pas jouer deux fois. Les événements se lisent **par pas de simulation**, comme
  les fronts d'entrée depuis le `LOT-33`.

## Fichiers impactés
- `Source/HMI/Game/GameEvents.{h,cpp}` (nouveau) — détection pure des transitions.
- `Source/HMI/Audio/SoundTriggers.{h,cpp}` (nouveau) — table événement → son, pure.
- `Source/HMI/Game/GameSession.{h,cpp}` et `GameViewport.cpp` — câblage.
- `Source/HMI/Interface/MainWindow.cpp` — sons d'interface.
- `Source/Test/Unit/HMI/Game/test_game_events.cpp`, `Source/Test/Unit/HMI/Audio/test_sound_triggers.cpp`
  (nouveaux), `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Chaque transition produit **exactement un** événement : un saut donne un événement « a sauté », ni
  zéro ni deux.
- Un état **stable** ne produit aucun événement (courir au sol pendant cent pas : aucun
  atterrissage).
- **Aucun événement dupliqué** quand plusieurs images de rendu tombent dans le même pas de
  simulation — le piège exact que `LOT-33` a déjà traité pour les entrées.
- La table événement → son est **exhaustive** : tout événement de l'énumération a une entrée
  (vérifié par un test qui parcourt l'énumération, pas par relecture).
- **Aucun effet sur la simulation** : les tests de gameplay et de franchissabilité passent
  inchangés, et la détection ne prend l'état qu'en lecture.
- Tests purs, sans périphérique.

## Points d'attention
- **La détection doit être `const`** vis-à-vis de l'état de jeu. Si elle a besoin d'écrire quelque
  part, c'est dans sa propre mémoire du pas précédent, jamais dans `Core`.
- **Mémoriser l'état précédent, pas un booléen « déjà joué »** : un drapeau posé dans le composant
  serait une mutation de la simulation par la présentation.
- La mort et le redémarrage se suivent immédiatement : vérifier que le son de mort n'est pas
  supprimé par le rechargement du niveau intervenant au pas suivant.
- Un dash au contact d'un mur peut produire trois transitions dans le même pas ; décider et
  documenter la priorité plutôt que de jouer trois sons superposés.
- Ne pas dupliquer cette détection dans `LOT-53` : elle est écrite ici pour être réutilisée.

## Définition de fait (DoD)
- Les événements de jeu et d'interface sont détectés par une fonction pure et testée, associés aux
  sons par une table exhaustive, sans aucun doublon ni aucun effet sur la simulation ; la structure
  est directement réutilisable par `LOT-53` ; `/W4 /WX` propre.

## Exigences
`EX-REN-047` (déclenchement par les transitions d'état) ; lève `EX-REN-040` pour sa partie
déclenchement ; réutilise `EX-ARCH-012` (présentation sans effet sur la simulation), `EX-NFR-010`
(testable sans périphérique), `EX-NFR-002` (déterminisme), `EX-REN-021` (pas fixe), `EX-GP-020`
à `EX-GP-025` (mécanismes), `EX-GP-030`/`EX-GP-031` (fin de niveau).

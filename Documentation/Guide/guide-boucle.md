# Boucle de jeu et pas de temps fixe {#guide-boucle}

Cette page part de zéro : elle explique ce qu'est une boucle de jeu pour qui n'en a jamais écrit,
puis détaille comment ce moteur la construit.

## Qu'est-ce qu'une boucle de jeu ?

Une application classique (un formulaire, un outil en ligne de commande) est **passive** : elle
attend un événement (clic, entrée clavier, requête réseau) puis réagit, et ne fait rien entre deux
événements. Un jeu vidéo, lui, doit donner l'illusion d'un monde **vivant** en continu : un
personnage tombe sous l'effet de la gravité même si le joueur n'appuie sur aucune touche. Il faut
donc une boucle qui tourne **en permanence**, tant que le jeu est ouvert, et qui à chaque tour :

1. **lit** les entrées disponibles (clavier, souris, fenêtre) ;
2. **fait avancer** l'état du jeu (« mettre à jour la logique ») ;
3. **dessine** l'état courant à l'écran (« rendre ») ;
4. **affiche** l'image dessinée (« présenter »).

Un tour de cette boucle correspond à une **frame** (image). Un jeu qui tourne à 60 *frames per
second* (FPS) exécute ces quatre étapes 60 fois par seconde, soit un tour toutes les ~16,7 ms. Dans
ce moteur, cette boucle vit dans le viewport Qt (`hmi::GameViewport`, cadencé par
`QEvent::UpdateRequest`) et enchaîne : sondage manette + événements Qt → mise à jour → rendu →
présentation (@ref guide-ihm-qt).

## Le piège du framerate variable

La difficulté est que la durée réelle d'une frame **varie** : selon la machine, la charge du
système, ou une fenêtre minimisée un instant, une frame peut durer 16 ms ou 200 ms. Une écriture
naïve de la mise à jour ressemblerait à :

```cpp
while (running) {
    float deltaTime = measureTimeSinceLastFrame();  // variable, imprévisible
    update(deltaTime);                               // avance la simulation de deltaTime
    render();
}
```

Ici, un personnage qui tombe se déplacerait de `vitesse × deltaTime` à chaque frame. Sur une
machine rapide (deltaTime petit), la chute avancerait par petits pas fréquents ; sur une machine
lente (deltaTime grand), par grands pas rares. Le résultat numérique final **diffère** selon la
vitesse de la machine : mêmes entrées, trajectoires différentes. Deux conséquences concrètes :

- **le jeu n'est pas reproductible** : un test automatisé qui rejoue une séquence d'entrées peut
  obtenir un résultat différent d'une exécution à l'autre, ou d'une machine à l'autre ;
- **le *game feel* change avec la machine** : un saut peut être plus haut ou plus court, une
  collision peut « traverser » un mur plus facilement quand `deltaTime` grandit (un grand pas
  saute par-dessus un obstacle fin — voir @ref guide-physique, §1).

C'est ce défaut que le **pas de temps fixe** élimine.

## Le principe du pas de temps fixe

Idée : au lieu de faire avancer la logique de la durée réelle (variable) de la frame, on la fait
avancer par **incréments constants**, par exemple toujours 1/60 s. Le nombre d'incréments à
exécuter à chaque frame dépend du temps réel écoulé, mais **chaque incrément individuel** voit
toujours exactement la même durée. La logique de jeu (physique, mécanismes) ne connaît donc
**jamais** de `deltaTime` variable — seulement des pas identiques et répétés — ce qui garantit le
**déterminisme** : mêmes entrées → **exactement** la même trajectoire, quelle que soit la vitesse
de la machine (`EX-NFR-002`, `EX-ARCH-030`). C'est indispensable pour des tests reproductibles et
pour un ressenti de jeu stable.

Le rendu, lui, n'a pas besoin de cette contrainte : dessiner une fois de plus ou de moins par
seconde ne casse aucun invariant logique. Il reste donc cadencé sur le temps réel, une fois par
frame — c'est le **découplage** entre logique (fixe) et rendu (variable).

## L'accumulateur : \ref core::FixedTimestep "core::FixedTimestep"

Le mécanisme qui convertit un temps réel variable en un nombre entier de pas fixes s'appelle un
**accumulateur**. Principe :

1. à chaque frame, on **ajoute** le temps réel écoulé à un compteur (`_accumulator`) ;
2. tant que ce compteur contient **au moins** un pas fixe complet, on **retranche** un pas fixe du
   compteur et on compte un pas à exécuter ;
3. ce qui **reste** dans le compteur (moins d'un pas fixe) est conservé pour la frame suivante — il
   ne se perd jamais, sinon les erreurs d'arrondi s'accumuleraient et la simulation dériverait.

`core::FixedTimestep::advance(elapsedSeconds)` implémente exactement ce mécanisme et renvoie le
nombre de pas à exécuter :

```cpp
core::FixedTimestep clock;               // pas fixe par défaut : 1/60 s
const int steps = clock.advance(elapsedSeconds);
for (int i = 0; i < steps; ++i) {
    updateCurrentScreen(clock.fixedDeltaSeconds());  // toujours 1/60 s, jamais elapsedSeconds
}
render();  // une seule fois, quel que soit le nombre de pas
```

### Exemple chiffré

Avec un pas fixe de 1/60 s (≈ 0,01667 s) :

| Frame | Temps réel écoulé | Accumulateur avant | Accumulateur après | Pas exécutés |
|-------|--------------------|---------------------|----------------------|--------------|
| 1     | 0,020 s            | 0,000 s              | 0,003 s               | 1            |
| 2     | 0,015 s            | 0,003 s              | 0,018 s → 0,001 s     | 1            |
| 3     | 0,040 s            | 0,001 s              | 0,041 s → 0,008 s     | 2            |

À la frame 3, le temps réel écoulé (0,040 s) dépasse deux pas fixes (2 × 1/60 ≈ 0,033 s) : la
boucle exécute **deux** mises à jour logiques avant de rendre une seule image. C'est normal et
voulu — le rendu peut « sauter » plusieurs pas de simulation sans jamais désynchroniser la logique.

### La « spirale de la mort »

Si une frame est anormalement longue (fenêtre déplacée, point d'arrêt du débogueur, machine
gelée un instant), le temps écoulé peut représenter des **dizaines** de pas fixes d'un coup.
Exécuter tous ces pas d'affilée prend du temps réel, ce qui retarde la frame suivante, qui devra à
son tour rattraper encore plus de pas — un cercle vicieux appelé la **spirale de la mort**
(*spiral of death*), qui peut figer le jeu indéfiniment. `FixedTimestep` s'en protège avec
`maximumStepsPerCall` : le nombre de pas renvoyé par `advance()` est **plafonné** (5 par défaut),
quitte à ce que la simulation « perde » du temps réel dans un cas extrême plutôt que de bloquer.

### \ref core::FixedTimestep::interpolationAlpha "interpolationAlpha"

Après avoir consommé tous les pas fixes disponibles, il peut rester une fraction de pas dans
l'accumulateur (entre 0 et 1 pas). `interpolationAlpha()` l'expose comme un facteur dans `[0, 1[` :
le rendu s'en sert pour **interpoler** visuellement entre la position du pas précédent et celle du
pas courant, afin d'afficher un mouvement lisse même quand le framerate de rendu dépasse la
fréquence des pas fixes (`EX-ARCH-031`, concrétisé en `LOT-33`). Sans cette interpolation, une
entité mobile resterait figée à sa dernière position simulée pendant plusieurs frames de rendu puis
« sauterait » d'un coup au pas suivant — un *judder* en marches d'escalier visible dès qu'un écran
dépasse 60 Hz. La mécanique côté rendu est détaillée dans @ref guide-rendu (composant
`hmi::PreviousPosition`, `hmi::SpriteRenderer`).

### Les frames sans pas de simulation et les entrées

Quand le rendu dépasse 60 Hz, certaines frames réelles n'exécutent **aucun** pas fixe (`steps == 0`)
— l'accumulateur n'a pas encore atteint un pas complet. Ces frames dessinent quand même (le rendu
est découplé), en interpolant grâce à `interpolationAlpha`. Elles imposent en revanche une
précaution sur les **entrées** : un appui capturé sur une telle frame doit **survivre** jusqu'à ce
qu'un pas de simulation le lise, au lieu d'être effacé par la frame suivante. C'est pourquoi la
boucle avance la ligne de base des fronts d'entrée (`hmi::InputState::beginFrame`) **après chaque
pas consommé**, et non à chaque frame de rendu — détaillé dans @ref guide-entrees. Sans cette
précaution, à 144 Hz environ deux appuis sur trois seraient perdus (bug corrigé en `LOT-33`).

## Conséquence pratique pour tout le code de simulation

**Toute** la logique de jeu (`core::CharacterPhysicsSystem`, les mécanismes) reçoit `fixedDelta`
— une constante — et **jamais** le temps réel. C'est pour cela que les tests peuvent appeler
`update(..., 1.0f/60.0f)` en boucle, sans horloge ni minuteur : c'est **exactement** ce que fait le
jeu en exécution normale, pas une approximation. Un système qui lirait le temps réel directement
romprait cette garantie et deviendrait, par construction, non déterministe et difficile à tester.

## Voir aussi
- `core::FixedTimestep`.
- `hmi::GameViewport` (boucle cadencée par Qt), @ref guide-ihm-qt, @ref guide-ecrans (navigation).
- @ref guide-ecs — la logique exécutée à chaque pas fixe.
- @ref guide-physique — le système le plus sensible au déterminisme du pas fixe.

# Écrans et navigation {#guide-ecrans}

Cette page explique comment l'application passe du menu au jeu, à l'éditeur ou aux options — le
squelette de navigation qui relie entre eux tous les écrans décrits dans les autres pages
(@ref guide-entrees pour le menu principal et le menu d'options, @ref guide-niveaux et
@ref guide-editeur pour ce que le jeu et l'éditeur affichent une fois actifs).

## Le problème : plusieurs écrans, une seule boucle

@ref guide-boucle décrit une boucle de jeu qui met à jour puis dessine « l'état du jeu », comme
s'il n'y en avait qu'un seul. Ce moteur a en réalité **quatre** états mutuellement exclusifs — menu,
jeu, éditeur, options — et un seul est actif à la fois. Une écriture naïve testerait un indicateur
`enum` global au début de chaque fonction (« si on est dans le menu, fais ceci ; si on est en jeu,
fais cela ») : chaque nouvel écran ajouterait une branche à **chaque** point de la boucle qui en a
besoin (entrées, mise à jour, rendu), et un écran ne pourrait jamais être testé sans compiler et
faire tourner tous les autres en même temps. Ce moteur choisit à la place le patron **stratégie** :
chaque écran est un objet interchangeable qui sait se mettre à jour et se dessiner lui-même, derrière
une interface commune.

## Le contrat d'un écran : \ref hmi::IScreen "IScreen" et \ref hmi::ScreenTransition "ScreenTransition"

```cpp
class IScreen {
public:
    virtual ~IScreen() = default;

    // Fait progresser la logique d'un pas fixe et renvoie l'intention de transition.
    [[nodiscard]] virtual ScreenTransition update(const InputState& input, float fixedDelta) = 0;

    // Dessine l'écran à partir des ressources partagées du RenderContext.
    virtual void render(RenderContext& context) = 0;
};
```

Chaque écran (menu, jeu, éditeur, options) implémente cette interface minimale. Le point capital
est que `update` ne **renvoie qu'une intention** — jamais un écran ne bascule lui-même vers un
autre :

```cpp
struct ScreenTransition {
    enum class Kind { None, Switch, Quit };
    Kind kind = Kind::None;
    ScreenId target = ScreenId::Menu;  // pertinent seulement si kind == Switch

    static ScreenTransition none() noexcept { return {}; }
    static ScreenTransition switchTo(ScreenId id) noexcept { return {Kind::Switch, id}; }
    static ScreenTransition quit() noexcept { return {Kind::Quit, ScreenId::Menu}; }
};
```

Par exemple, `MenuModel::update` renvoie `ScreenTransition::switchTo(ScreenId::Game)` quand
« Jouer » est validé, mais ne construit **jamais** de `GameScreen` lui-même — il n'a même pas
connaissance de la classe `GameScreen`. Ce découplage a deux conséquences pratiques : un écran est
testable en isolation (lui passer un `InputState` construit à la main et vérifier la transition
renvoyée, sans fenêtre ni GPU, `EX-NFR-010`), et ajouter un nouvel écran ne demande de modifier
**aucun** écran existant — seulement la fabrique, ci-dessous.

## Qui applique les transitions : \ref hmi::ScreenManager "ScreenManager"

`ScreenManager` détient l'écran courant et **applique** l'intention qu'il renvoie, chaque frame :

```cpp
bool ScreenManager::update(const InputState& input, float fixedDelta) {
    if (_quit || _current == nullptr) return true;

    const ScreenTransition transition = _current->update(input, fixedDelta);
    switch (transition.kind) {
        case ScreenTransition::Kind::None:
            break;                                  // rester sur l'écran courant
        case ScreenTransition::Kind::Switch:
            _current = _factory(transition.target);  // detruit l'ancien, construit le nouveau
            _currentId = transition.target;
            break;
        case ScreenTransition::Kind::Quit:
            _current.reset();
            _quit = true;
            break;
    }
    return _quit;
}
```

Un point souvent contre-intuitif : `_current = _factory(transition.target)` **détruit** l'ancien
écran (le `unique_ptr` précédent est libéré à l'affectation) et en construit un **tout neuf** — il
n'y a pas de pile d'écrans ni de mise en pause : quitter l'éditeur pour retourner au menu ne
« suspend » pas l'éditeur, il le **détruit** (voir plus bas comment `EditorScreen` contourne cela
pour l'essai immédiat, sans passer par ce mécanisme). La transition est appliquée **immédiatement**,
dans le même appel à `update` : le rendu de la **même** frame dessine donc déjà le nouvel écran,
jamais un ancien écran sur le point de disparaître.

`ScreenManager` ne sait pas construire un écran concret lui-même — cette responsabilité est
**injectée** sous forme d'une fabrique (`ScreenManager::Factory`, une fonction
`ScreenId → std::unique_ptr<IScreen>`). Le gestionnaire ne connaît donc ni `GameScreen`, ni
`EditorScreen`, ni leurs dépendances de rendu (atlas, police…) : il est testable sans fenêtre ni GPU,
en lui passant une fabrique qui produit de faux écrans (`test_screen_manager.cpp` fait exactement
cela).

## La fabrique réelle : assembler les écrans dans `main`

C'est `Source/HMI/main.cpp` qui fournit la fabrique **réelle**, dans un simple `switch` sur
`ScreenId` :

```cpp
hmi::ScreenManager::Factory factory = [&](hmi::ScreenId id) -> std::unique_ptr<hmi::IScreen> {
    switch (id) {
        case hmi::ScreenId::Menu:
            return std::make_unique<hmi::MenuScreen>(localization, saveLogAction);
        case hmi::ScreenId::Game:
            return std::make_unique<hmi::GameScreen>(spriteBatch, atlas, width, height,
                                                       levelSequence);
        case hmi::ScreenId::Editor:
            return std::make_unique<hmi::EditorScreen>(spriteBatch, atlas, width, height);
        case hmi::ScreenId::Options:
            return std::make_unique<hmi::OptionsScreen>(localization, graphics);
    }
    return nullptr;
};
hmi::ScreenManager screens(std::move(factory), hmi::ScreenId::Menu);
```

Chaque branche construit l'écran concret avec **ses propres** dépendances (le `MenuScreen` reçoit le
catalogue de traduction, le `GameScreen` la séquence de niveaux à jouer) — des dépendances que
`ScreenManager` lui-même ne voit jamais, puisqu'elles sont capturées par la lambda (`[&]`) au moment
de la construction de la fabrique, pas transmises à chaque appel. Cette fabrique est le **seul**
endroit du programme qui connaît la liste complète des écrans concrets : ajouter un cinquième écran
revient à ajouter une branche ici et un cas à `ScreenId`, sans toucher à `ScreenManager` ni aux
écrans existants — exactement la promesse du patron stratégie évoquée plus haut.

## Où ça s'insère dans la boucle de jeu

@ref guide-boucle explique l'accumulateur à pas fixe ; voici comment il pilote concrètement les
écrans, dans `main.cpp` :

```cpp
core::FixedTimestep timestep;
const int steps = timestep.advance(elapsedSeconds);
for (int step = 0; step < steps; ++step) {
    if (screens.update(window.input(), timestep.fixedDeltaSeconds())) {
        // ScreenTransition::quit() a été demandé : arrêter la boucle sans dessiner.
    }
}
screens.render(context);  // une seule fois, quel que soit le nombre de pas ci-dessus
```

`ScreenManager::update` est donc appelé **autant de fois qu'il y a de pas de simulation fixes** dans
la frame (typiquement un, parfois plusieurs sur une frame lente) — exactement comme
`core::World::update` pour l'ECS (@ref guide-ecs). Un écran de menu, qui ne fait que lire des fronts
d'entrée (@ref guide-entrees), n'a en pratique presque jamais qu'un seul pas à traiter (une frame
lente au point d'accumuler plusieurs pas est rare hors gameplay) ; un `GameScreen`, lui, en a
structurellement besoin pour que la physique reste déterministe même sur une frame lente. `render`
n'est, à l'inverse, **jamais** appelé plus d'une fois par frame réelle — dessiner l'état courant une
fois de plus n'apporterait rien, contrairement à la simulation qui doit, elle, rattraper chaque pas
manqué.

## Les ressources partagées : \ref hmi::RenderContext "RenderContext"

Tous les écrans dessinent avec les **mêmes** ressources de rendu (lot de sprites, atlas de
textures, police bitmap, catalogue de traduction, icônes) — les recréer par écran serait coûteux et
inutile, puisqu'elles vivent le temps de l'application, pas le temps d'un écran. `RenderContext`
les regroupe et les transmet en un seul paramètre à `IScreen::render`, sans qu'aucun écran n'en
possède le cycle de vie :

```cpp
struct RenderContext {
    SpriteBatch& spriteBatch;
    const TextureAtlas& atlas;
    const BitmapFont& font;
    const Localization& localization;
    const FlagIcons& flags;
    const SaveIcon& saveIcon;
    int viewportWidth = 0;
    int viewportHeight = 0;
};
```

`main.cpp` reconstruit ce contexte à **chaque** frame (ses champs sont des références/valeurs par
copie bon marché, pas des ressources possédées) juste avant `screens.render(context)` — les
dimensions du *viewport*, en particulier, doivent refléter un éventuel redimensionnement de fenêtre
survenu **cette** frame (@ref guide-rendu).

## Enchaîner des niveaux : \ref hmi::LevelSequence "LevelSequence"

`GameScreen` ne joue pas un niveau isolé mais une **séquence** ordonnée
(`Source/Elements/Levels/demo.json` → `demo2.json` → … → `demo5.json`, un ordre de difficulté
maîtrisé, `EX-LVL-010`) : `LevelSequence` est une logique pure (chemins de fichiers, indice
courant) qui répond à deux questions — quel niveau charger maintenant (`current()`), et existe-t-il
un niveau après (`hasNext()`) pour enchaîner à la réussite plutôt que revenir au menu
(`EX-LVL-011`). Cette logique ne dépend d'aucun rendu ni fichier réel au sens fort : elle manipule
des chemins, pas leur contenu, ce qui la rend testable sans écrire de fichiers de niveau sur disque.
`GameScreen` observe `core::evaluateOutcome` (@ref guide-niveaux) à chaque pas : une issue `Won`
appelle `_sequence.advance()` puis recharge le niveau `current()` **sans** passer par
`ScreenManager` (l'écran reste le même, seul son niveau interne change) ; après le dernier niveau,
`GameScreen` renvoie lui-même `ScreenTransition::switchTo(ScreenId::Menu)` — c'est le **seul** cas où
un enchaînement de niveaux se traduit en une vraie transition d'écran.

## Un cas particulier : l'essai immédiat de l'éditeur

`EditorScreen` (@ref guide-editeur) a besoin d'incruster une **vraie** partie jouable (`P`) puis d'y
revenir exactement où l'édition en était (`Échap`) — un besoin que `ScreenManager` ne peut **pas**
satisfaire tel quel, puisqu'une transition `Switch` **détruit** l'écran quitté (section
ci-dessus) : revenir à l'éditeur créerait une instance neuve, brouillon perdu. `EditorScreen`
contourne le problème en restant **le seul** écran géré par `ScreenManager` pendant tout l'essai : il
embarque un `std::unique_ptr<GameScreen>` optionnel et lui délègue `update`/`render` tant qu'il
existe, sans jamais renvoyer lui-même de transition vers `ScreenId::Game`. C'est une brique de bas
niveau (`GameScreen` en tant qu'objet réutilisable), pas un contournement du contrat `IScreen` —
voir @ref guide-editeur pour le détail complet.

## Voir aussi
- `hmi::IScreen`, `hmi::ScreenTransition`, `hmi::ScreenId`, `hmi::ScreenManager`, `hmi::RenderContext`, `hmi::LevelSequence`.
- @ref guide-entrees — `hmi::MenuModel`/`hmi::MenuScreen` et `hmi::OptionsModel`/`hmi::OptionsScreen`, deux écrans concrets construits par la fabrique.
- @ref guide-niveaux, @ref guide-editeur — ce que `GameScreen` et `EditorScreen` font une fois actifs, et comment l'éditeur réutilise `GameScreen` sans passer par `ScreenManager`.
- @ref guide-boucle — l'accumulateur à pas fixe qui pilote combien de fois `ScreenManager::update` est appelé par frame.
- @ref guide-rendu — la construction du `RenderContext` et l'assemblage complet d'une frame de rendu dans `main`.

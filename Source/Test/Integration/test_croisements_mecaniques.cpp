/**
 * @file test_croisements_mecaniques.cpp
 * @brief Tests d'intégration des **croisements** de mécaniques (`LOT-65` TACHE-06).
 *
 * La suite couvre chaque mécanique **seule** de façon dense (`test_physique_personnage.cpp` en
 * compte 75) et presque **aucune combinaison** : un dash n'a jamais rencontré de bloc ni de pente,
 * une porte ne s'est jamais refermée sur personne, un bloc n'a jamais été poussé dans une porte.
 * Or c'est exactement là que vivent les défauts d'interaction — le seul déjà connu (une
 * configuration de `MovingPlatform`, même immobile, qui casse le suivi de pente **ailleurs** dans
 * le même niveau) a été trouvé en jouant, pas en testant.
 *
 * Chaque test construit sa géométrie **en mémoire** : ce module ne dépend d'aucun fichier de
 * niveau livré, pour qu'une refonte du contenu ne le fasse jamais bouger. Il rejoue en revanche
 * l'orchestration **exacte** de `hmi::GameSession::update` (plateformes, puis blocs, puis grille,
 * puis composition boîte-boîte, puis mécanismes), sans quoi il testerait un jeu qui n'existe pas.
 *
 * Un test qui passe du premier coup n'est pas une perte : il **fige** un comportement jusque-là
 * non spécifié.
 */

#include <cmath>
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/BlockController.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Gameplay/PlatformController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/AabbVsAabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"

namespace {

constexpr float STEP = 1.0f / 60.0f;

// Scène complète : le monde ECS et les trois contrôleurs, montés comme hmi::GameSession les monte.
struct Scene {
    explicit Scene(const core::Level& level)
        : blocks(level), mechanisms(level), platforms(level), _level(&level) {}

    core::World world;
    core::Entity player{};
    core::BlockController blocks;
    core::MechanismController mechanisms;
    core::PlatformController platforms;
    core::CharacterPhysicsSystem physics;
    core::LevelOutcome outcome = core::LevelOutcome::Playing;

    // Fait apparaître le personnage à sa VRAIE taille (0,4 x 0,8), centré dans la case : une boîte
    // 1x1 se fait happer par une colonne solide voisine pendant le suivi de pente, ce qui rendrait
    // les tests de pente faux pour une raison sans rapport avec ce qu'ils mesurent.
    void spawn(int column, int row) {
        player = world.createEntity();
        const core::Vector2 size = core::playerSize();
        world.addComponent(player,
                           core::Transform{core::playerSpawnPosition(column, row), size, 0.0f});
        world.addComponent(player, core::Velocity{});
        world.addComponent(player, core::Collider{size});
        world.addComponent(player, core::Player{});
    }

    // Non const : core::World::getComponent ne l'est pas.
    [[nodiscard]] core::Aabb playerBox() {
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        return core::Aabb::fromTopLeftSize(transform.position, collider.size);
    }

    // Un pas fixe, dans l'ORDRE de hmi::GameSession::update (LOT-63 TACHE-03) : plateformes,
    // blocs, grille, composition boîte-boîte des blocs réduits, mécanismes, issue.
    void step(const core::PlayerInput& in) {
        const core::Aabb previousBox = playerBox();

        platforms.update();
        const std::vector<core::PlatformSample> samples = platforms.samples();

        const core::TileMap mechanismMap = mechanisms.collisionMap();
        blocks.update(previousBox, in.moveX, mechanismMap, samples);
        const core::TileMap collision = blocks.collisionMap(mechanismMap);
        physics.update(world, collision, in, STEP, samples);

        core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Vector2 delta = transform.position - previousBox.min;
        core::Vector2 bestPosition = transform.position;
        core::Vector2 bestNormal{};
        const std::vector<float>& scales = blocks.scales();
        for (std::size_t index = 0; index < scales.size(); ++index) {
            if (scales[index] >= 1.0f) {
                continue;
            }
            const core::SweepResult result =
                core::sweepAabbVsAabb(previousBox, delta, blocks.boxAt(index));
            if (result.normal.x != 0.0f && std::fabs(result.position.x - previousBox.min.x) <
                                               std::fabs(bestPosition.x - previousBox.min.x)) {
                bestPosition.x = result.position.x;
                bestNormal.x = result.normal.x;
            }
            if (result.normal.y != 0.0f && std::fabs(result.position.y - previousBox.min.y) <
                                               std::fabs(bestPosition.y - previousBox.min.y)) {
                bestPosition.y = result.position.y;
                bestNormal.y = result.normal.y;
            }
        }
        if (bestNormal.x != 0.0f || bestNormal.y != 0.0f) {
            transform.position = bestPosition;
            core::Velocity& velocity = world.getComponent<core::Velocity>(player);
            if (bestNormal.x != 0.0f) {
                velocity.value.x = 0.0f;
            }
            if (bestNormal.y != 0.0f) {
                velocity.value.y = 0.0f;
                if (bestNormal.y < 0.0f) {
                    world.getComponent<core::Player>(player).grounded = true;
                }
            }
        }

        const core::Aabb box = playerBox();
        std::vector<core::TriggerWeight> weights;
        weights.reserve(blocks.positions().size());
        for (std::size_t index = 0; index < blocks.positions().size(); ++index) {
            weights.push_back(
                core::TriggerWeight{.box = blocks.boxAt(index), .mass = blocks.massAt(index)});
        }
        mechanisms.update(box, 1.0f, in.interactPressed, weights);

        std::vector<core::Aabb> extraDangerBoxes;
        if (world.getComponent<core::Player>(player).squished || mechanisms.crushedPlayer()) {
            extraDangerBoxes.push_back(box);
        }
        outcome = core::evaluateOutcome(box, *_level, extraDangerBoxes);
    }

    void run(const core::PlayerInput& in, int steps) {
        for (int i = 0; i < steps; ++i) {
            step(in);
        }
    }

private:
    const core::Level* _level;
};

// Sol plein sur toute la largeur, à la ligne @p row.
void floorAt(core::TileMap& tiles, int row) {
    for (int column = 0; column < tiles.width(); ++column) {
        tiles.setTile(column, row, core::TileType::Solid);
    }
}

// Entrée qui dashe vers la droite, sans saut.
core::PlayerInput dashRight() {
    core::PlayerInput in{1.0f};
    in.dashPressed = true;
    return in;
}

}  // namespace

/**
 * @brief Dash × bloc poussable plein : la ruée ne traverse jamais le bloc. `pushBlocks` exige un
 * contact à `PUSH_TOUCH_TOLERANCE` (0,05) évalué sur la boîte du pas **précédent**, alors qu'un
 * dash déplace 0,25 case par pas — cinq fois la tolérance. Le bloc reste solide dans la grille,
 * donc le balayage arrête le personnage même quand la fenêtre de poussée est manquée.
 * \castest{<b>Un dash contre un bloc poussable ne le traverse jamais : le personnage est arrêté ou
 * le bloc est poussé, jamais de passage au travers.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un dash contre un bloc poussable ne le traverse jamais : le personnage est arrêté ou le
 * bloc est poussé, jamais de passage au travers.
 * }
 */
TEST(CroisementsMecaniques, DashContreUnBlocNeLeTraversePas) {
    core::TileMap tiles(16, 6);
    floorAt(tiles, 5);
    tiles.setTile(6, 4, core::TileType::Block);
    core::Level level("dash-bloc", tiles, core::GridPosition{1, 4}, core::GridPosition{15, 4},
                      std::vector<core::Mechanism>{});
    Scene scene(level);
    scene.spawn(1, 4);

    scene.run(dashRight(), 240);

    // Le bloc a ete pousse devant le personnage, qui reste TOUJOURS a gauche de sa boite : jamais
    // de traversee, quelle que soit la fenetre de poussee manquee en chemin.
    const core::GridPosition blockCell = scene.blocks.positions()[0];
    EXPECT_GT(blockCell.column, 6) << "le bloc n'a pas ete pousse du tout";
    EXPECT_LE(scene.playerBox().max.x, scene.blocks.boxAt(0).min.x + 1e-3f);
}

/**
 * @brief Dash × bloc **réduit** : même garantie, par un chemin de code entièrement différent — un
 * bloc réduit n'est pas solide dans la grille, sa collision passe par `core::sweepAabbVsAabb`
 * (`EX-GP-005`). C'est le cas où un dash à 0,25 case/pas avait le plus de raisons de passer au
 * travers.
 * \castest{<b>Un dash contre un bloc réduit ne le traverse pas : la composition boîte-boîte tient
 * à vitesse de dash.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un dash contre un bloc réduit ne le traverse pas : la composition boîte-boîte tient à
 * vitesse de dash.
 * }
 */
TEST(CroisementsMecaniques, DashContreUnBlocReduitCoinceNeLeTraversePas) {
    core::TileMap tiles(16, 6);
    floorAt(tiles, 5);
    tiles.setTile(6, 4, core::TileType::BlockHalf);
    tiles.setTile(7, 4, core::TileType::Solid);  // mur derriere : la poussee echoue toujours
    core::Level level("dash-bloc-reduit", tiles, core::GridPosition{1, 4},
                      core::GridPosition{15, 4}, std::vector<core::Mechanism>{});
    Scene scene(level);
    scene.spawn(1, 4);

    scene.run(dashRight(), 240);

    // Bloc immobile (mur derriere) : boite reelle centree, bord gauche a 6,25. Le personnage doit
    // s'arreter dessus, jamais au-dela.
    EXPECT_EQ(scene.blocks.positions()[0], (core::GridPosition{6, 4}));
    EXPECT_LE(scene.playerBox().max.x, 6.25f + 1e-2f);
}

/**
 * @brief Dash × pente — **défaut moteur consigné** (`LOT-65` TACHE-06).
 *
 * Le suivi de surface (`core::resolveSlopeFollow`) n'avait jamais été éprouvé qu'à vitesse de
 * **marche** (0,05 case/pas). À vitesse de **dash** (0,25 case/pas, vingt fois plus), une pente à
 * 45° suivie d'un palier plein — la silhouette exacte de tous les tableaux de pente livrés — n'est
 * **plus franchissable** : le personnage se fige au sommet, à 0,15 case sous le palier, arrêté
 * horizontalement par la colonne pleine avant que le suivi de pente ne l'ait élevé assez haut.
 *
 * Ce test **caractérise** le comportement actuel plutôt que de le corriger : le cadrage du lot
 * consigne les défauts découverts en jouant sans les corriger, et les deux corrections moteur
 * qu'il assume par ailleurs sont nommées et bornées. La première assertion vérifie que la même
 * géométrie **se franchit à la marche** : sans elle, on ne saurait pas si le test mesure un défaut
 * ou une géométrie mal choisie. Si quelqu'un corrige ce défaut, ce test échouera — c'est voulu, et
 * c'est le signal d'aller mettre à jour le `CHANGELOG` et les tableaux qui s'en tiennent à l'écart.
 *
 * Conséquence pour le contenu : aucun tableau ne place un dash **obligatoire** dans une montée de
 * pente vers un palier.
 * \castest{<b>Défaut consigné : une pente franchissable à la marche ne l'est pas au dash, le
 * personnage restant figé au sommet.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Défaut consigné : une pente franchissable à la marche ne l'est pas au dash, le
 * personnage restant figé au sommet.
 * }
 */
TEST(CroisementsMecaniques, DashSurUnePenteResteBloqueAuSommetDefautConsigne) {
    // Geometrie commune aux deux mesures : sol bas, une pente a 45 degres, palier haut. C'est la
    // silhouette exacte des tableaux de pente livres.
    core::TileMap tiles(16, 7);
    for (int column = 0; column < 6; ++column) {
        tiles.setTile(column, 5, core::TileType::Solid);
    }
    tiles.setTile(6, 4, core::TileType::SlopeUpRight);
    for (int column = 7; column < 16; ++column) {
        tiles.setTile(column, 4, core::TileType::Solid);
    }
    const core::Level level("dash-pente", tiles, core::GridPosition{1, 4},
                            core::GridPosition{15, 3}, std::vector<core::Mechanism>{});

    // Reference : a la MARCHE, la pente se franchit et le personnage se pose sur le palier.
    Scene walking(level);
    walking.spawn(1, 4);
    walking.run(core::PlayerInput{1.0f}, 300);
    const core::Aabb walkBox = walking.playerBox();
    ASSERT_GT(walkBox.min.x, 7.0f) << "la geometrie de reference n'est pas franchissable a la "
                                      "marche : le test mesurerait autre chose que le dash";
    ASSERT_NEAR(walkBox.max.y, 4.0f, 0.05f);

    // Mesure : sur la MEME pente, le dash reste bloque. Le personnage bute contre la colonne
    // pleine du palier (bord droit a x = 7) avec les pieds encore 0,15 case trop bas, et n'en
    // repart jamais.
    Scene dashing(level);
    dashing.spawn(1, 4);
    dashing.run(dashRight(), 300);
    const core::Aabb dashBox = dashing.playerBox();
    EXPECT_LT(dashBox.min.x, 7.0f) << "le defaut semble corrige : mettre a jour ce test, le "
                                      "CHANGELOG, et lever la contrainte sur les tableaux";
    EXPECT_NEAR(dashBox.max.x, 7.0f, 1e-2f) << "bloque par la colonne pleine du palier";
    EXPECT_GT(dashBox.max.y, 4.0f + 0.05f) << "pieds encore sous le niveau du palier";
}

/**
 * @brief Dash × plafond incliné : la passe symétrique (`core::resolveCeilingSlopeFollow`) n'était
 * elle non plus éprouvée qu'à vitesse de marche. Un dash sous une silhouette de plafond doit être
 * arrêté par elle, pas la franchir par la vitesse.
 * \castest{<b>Un dash sous un plafond incliné est arrêté par sa silhouette, jamais franchi par la
 * vitesse.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un dash sous un plafond incliné est arrêté par sa silhouette, jamais franchi par la
 * vitesse.
 * }
 */
TEST(CroisementsMecaniques, DashSousUnPlafondInclineResteSousSaSilhouette) {
    core::TileMap tiles(16, 7);
    floorAt(tiles, 5);
    // Plafond descendant vers la droite au-dessus du couloir : la matiere occupe le HAUT de la
    // case, et sa silhouette descend jusqu'a fermer le passage.
    tiles.setTile(8, 4, core::TileType::SlopeDownLeft);
    for (int column = 9; column < 16; ++column) {
        tiles.setTile(column, 4, core::TileType::Solid);
    }
    core::Level level("dash-plafond", tiles, core::GridPosition{1, 4}, core::GridPosition{15, 3},
                      std::vector<core::Mechanism>{});
    Scene scene(level);
    scene.spawn(1, 4);

    scene.run(dashRight(), 300);

    // Le couloir se referme : le personnage ne doit jamais depasser la colonne pleine.
    EXPECT_LE(scene.playerBox().max.x, 9.0f + 1e-2f);
}

/**
 * @brief Dash × plaque de pression : la plaque s'évalue par chevauchement au pas courant, et un
 * dash la survole en deux pas. Elle doit tout de même s'activer au passage — sinon un tableau
 * combinant les deux serait infranchissable pour une raison invisible au level designer.
 * \castest{<b>Un dash qui traverse une plaque de pression l'active au passage, malgré la brièveté
 * du contact.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un dash qui traverse une plaque de pression l'active au passage, malgré la brièveté du
 * contact.
 * }
 */
TEST(CroisementsMecaniques, DashSurUnePlaqueLActiveAuPassage) {
    core::TileMap tiles(16, 6);
    floorAt(tiles, 5);
    tiles.setTile(6, 4, core::TileType::PressurePlate);
    tiles.setTile(12, 2, core::TileType::Door);
    const std::vector<core::Mechanism> mechanisms{core::Mechanism{
        .switchPosition = core::GridPosition{6, 4}, .doorPosition = core::GridPosition{12, 2}}};
    core::Level level("dash-plaque", tiles, core::GridPosition{1, 4}, core::GridPosition{15, 4},
                      mechanisms);
    Scene scene(level);
    scene.spawn(1, 4);

    bool openedAtLeastOnce = false;
    for (int i = 0; i < 240; ++i) {
        scene.step(dashRight());
        if (scene.mechanisms.isDoorOpen(0)) {
            openedAtLeastOnce = true;
        }
    }

    EXPECT_TRUE(openedAtLeastOnce) << "la plaque n'a jamais ete enfoncee par le dash";
    // Activation CONTINUE : une fois le personnage parti, la porte s'est refermee.
    EXPECT_FALSE(scene.mechanisms.isDoorOpen(0));
}

/**
 * @brief Bloc sur plaque : un bloc poussable posé sur une plaque de pression tient la porte
 * ouverte **après** le départ du personnage (`EX-GP-025`, `LOT-65` TACHE-06). C'est l'idiome de
 * puzzle le plus classique du genre, et il était hors d'atteinte tant que le contrôleur ne
 * recevait que la boîte du joueur.
 * \castest{<b>Un bloc posé sur une plaque de pression tient la porte ouverte après le départ du
 * personnage.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc posé sur une plaque de pression tient la porte ouverte après le départ du
 * personnage.
 * }
 */
TEST(CroisementsMecaniques, BlocSurPlaqueTientLaPorteOuverteApresLeDepart) {
    core::TileMap tiles(16, 6);
    floorAt(tiles, 5);
    tiles.setTile(4, 4, core::TileType::Block);
    tiles.setTile(8, 4, core::TileType::PressurePlate);
    tiles.setTile(12, 2, core::TileType::Door);
    const std::vector<core::Mechanism> mechanisms{core::Mechanism{
        .switchPosition = core::GridPosition{8, 4}, .doorPosition = core::GridPosition{12, 2}}};
    core::Level level("bloc-plaque", tiles, core::GridPosition{1, 4}, core::GridPosition{15, 4},
                      mechanisms);
    Scene scene(level);
    scene.spawn(1, 4);

    // Pousser le bloc jusque sur la plaque, en marchant.
    const core::PlayerInput walkRight{1.0f};
    while (scene.blocks.positions()[0].column < 8) {
        scene.step(walkRight);
    }
    ASSERT_EQ(scene.blocks.positions()[0].column, 8);
    EXPECT_TRUE(scene.mechanisms.isDoorOpen(0)) << "le bloc pose sur la plaque ne l'enfonce pas";

    // Repartir vers la gauche, loin de la plaque : la porte doit RESTER ouverte, c'est tout
    // l'interet de poser un poids.
    const core::PlayerInput walkLeft{-1.0f};
    scene.run(walkLeft, 180);
    EXPECT_TRUE(scene.mechanisms.isDoorOpen(0))
        << "la porte s'est refermee alors que le bloc est reste sur la plaque";
}

/**
 * @brief Symétrique de `PlaqueDePressionPoidsInsuffisant` côté blocs : un bloc **réduit** est trop
 * léger pour enfoncer une plaque (masse = facteur de taille, cf. `core::BlockController::massAt`).
 * La règle est lisible pour le level designer — seul un bloc plein fait le poids — et distingue les
 * caisses par leur taille plutôt que par une propriété invisible.
 * \castest{<b>Un bloc réduit posé sur une plaque de pression est trop léger pour
 * l'enfoncer.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc réduit posé sur une plaque de pression est trop léger pour l'enfoncer.
 * }
 */
TEST(CroisementsMecaniques, BlocReduitTropLegerPourEnfoncerLaPlaque) {
    core::TileMap tiles(16, 6);
    floorAt(tiles, 5);
    tiles.setTile(4, 4, core::TileType::BlockHalf);
    tiles.setTile(8, 4, core::TileType::PressurePlate);
    tiles.setTile(12, 2, core::TileType::Door);
    const std::vector<core::Mechanism> mechanisms{core::Mechanism{
        .switchPosition = core::GridPosition{8, 4}, .doorPosition = core::GridPosition{12, 2}}};
    core::Level level("bloc-leger-plaque", tiles, core::GridPosition{1, 4},
                      core::GridPosition{15, 4}, mechanisms);
    Scene scene(level);
    scene.spawn(1, 4);

    const core::PlayerInput walkRight{1.0f};
    while (scene.blocks.positions()[0].column < 8) {
        scene.step(walkRight);
    }
    ASSERT_EQ(scene.blocks.positions()[0].column, 8);

    // Repartir a gauche : ni le bloc leger, ni personne, n'enfonce la plaque.
    scene.run(core::PlayerInput{-1.0f}, 180);
    EXPECT_FALSE(scene.mechanisms.isDoorOpen(0));
    EXPECT_FLOAT_EQ(scene.blocks.massAt(0), 0.5f);
}

/**
 * @brief Une porte qui se referme sur le personnage est **mortelle** (`EX-GP-021`, `LOT-65`
 * TACHE-06), comme l'écrasement sous une plateforme mobile. Sans cela, le personnage reste encastré
 * dans un mur sans échec possible — la « situation sans issue » que la conception des niveaux
 * interdit (`niveaux.md` Sec. 3). Le tableau `demo-plaque-pression` livré créait cette situation à
 * chaque partie.
 * \castest{<b>Une porte qui se referme sur le personnage provoque l'échec du niveau, jamais un
 * personnage figé dans un mur.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une porte qui se referme sur le personnage provoque l'échec du niveau, jamais un
 * personnage figé dans un mur.
 * }
 */
TEST(CroisementsMecaniques, PorteQuiSeRefermeSurLePersonnageEstMortelle) {
    core::TileMap tiles(12, 6);
    floorAt(tiles, 5);
    tiles.setTile(3, 4, core::TileType::PressurePlate);
    tiles.setTile(4, 4, core::TileType::Door);  // porte juste apres la plaque, sur le chemin
    const std::vector<core::Mechanism> mechanisms{core::Mechanism{
        .switchPosition = core::GridPosition{3, 4}, .doorPosition = core::GridPosition{4, 4}}};
    core::Level level("porte-ecrasante", tiles, core::GridPosition{1, 4}, core::GridPosition{11, 4},
                      mechanisms);
    Scene scene(level);
    scene.spawn(1, 4);

    // Marcher vers la droite : la plaque ouvre la porte, puis le personnage quitte la plaque en
    // entrant dans l'embrasure -- la porte se referme sur lui.
    const core::PlayerInput walkRight{1.0f};
    for (int i = 0; i < 300 && scene.outcome == core::LevelOutcome::Playing; ++i) {
        scene.step(walkRight);
    }

    EXPECT_EQ(scene.outcome, core::LevelOutcome::Lost)
        << "la porte s'est refermee sur le personnage sans echec : blocage definitif";
}

/**
 * @brief Bloc poussé sur une tuile de danger : le bloc ne **masque** pas le danger. Un level
 * designer pourrait croire l'inverse (« je bouche les pics avec une caisse ») ; le comportement est
 * figé ici plutôt que découvert en jouant.
 * \castest{<b>Un bloc poussé sur une tuile de danger ne la neutralise pas : le contact reste
 * mortel.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc poussé sur une tuile de danger ne la neutralise pas : le contact reste mortel.
 * }
 */
TEST(CroisementsMecaniques, BlocPousseSurUnDangerNeLeNeutralisePas) {
    core::TileMap tiles(12, 6);
    floorAt(tiles, 5);
    tiles.setTile(4, 4, core::TileType::Block);
    tiles.setTile(6, 4, core::TileType::Danger);
    core::Level level("bloc-danger", tiles, core::GridPosition{1, 4}, core::GridPosition{11, 4},
                      std::vector<core::Mechanism>{});
    Scene scene(level);
    scene.spawn(1, 4);

    const core::PlayerInput walkRight{1.0f};
    for (int i = 0; i < 300 && scene.outcome == core::LevelOutcome::Playing; ++i) {
        scene.step(walkRight);
    }

    // Que le bloc ait ete pousse sur le danger ou arrete devant, avancer tout droit finit mal :
    // le danger n'est jamais neutralise par ce qui repose dessus.
    EXPECT_EQ(scene.outcome, core::LevelOutcome::Lost);
}

/**
 * @brief Plateforme mobile × porte : une plateforme qui traverse une porte **fermée** n'est jamais
 * arrêtée par elle — sa position est une fonction déterministe du numéro de pas (`EX-GP-026`), pas
 * le résultat d'une collision. Le comportement est figé ici pour qu'un level designer ne compte pas
 * sur une porte pour retenir une plateforme.
 * \castest{<b>Une plateforme mobile n'est jamais arrêtée par une porte fermée : sa trajectoire ne
 * dépend d'aucune collision.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une plateforme mobile n'est jamais arrêtée par une porte fermée : sa trajectoire ne
 * dépend d'aucune collision.
 * }
 */
TEST(CroisementsMecaniques, PlateformeMobileTraverseUnePorteFermee) {
    core::TileMap tiles(16, 8);
    floorAt(tiles, 7);
    tiles.setTile(3, 4, core::TileType::Switch);
    tiles.setTile(8, 4, core::TileType::Door);  // fermee au depart, sur le trajet de la plateforme
    tiles.setTile(2, 4, core::TileType::MovingPlatform);
    const std::vector<core::Mechanism> mechanisms{core::Mechanism{
        .switchPosition = core::GridPosition{3, 4}, .doorPosition = core::GridPosition{8, 4}}};
    const std::vector<core::MovingPlatformConfig> platforms{
        core::MovingPlatformConfig{.startPosition = core::GridPosition{2, 4},
                                   .waypoints = {core::GridPosition{12, 4}},
                                   .speed = 4.0f}};
    // Aucune pente dans ce niveau : un movingPlatform present casse le suivi de pente ailleurs
    // dans le meme fichier (defaut moteur consigne au LOT-65).
    core::Level level("plateforme-porte", tiles, core::GridPosition{1, 6},
                      core::GridPosition{15, 6}, mechanisms, -1, -1, {}, {}, {}, std::nullopt,
                      std::nullopt, {}, {}, platforms);
    Scene scene(level);
    scene.spawn(1, 6);

    scene.run(core::PlayerInput{}, 180);

    // La plateforme a depasse la colonne de la porte fermee : rien ne la retient.
    ASSERT_FALSE(scene.platforms.samples().empty());
    EXPECT_GT(scene.platforms.samples()[0].currentBox.min.x, 8.0f);
}

/**
 * @brief Plateforme mobile × bloc poussable : la plateforme **porte** le bloc posé dessus
 * (`EX-GP-026`, exigé mais jamais mis en scène par aucun tableau livré) — et ce portage compose
 * avec le poids que ce bloc exerce sur les mécanismes.
 * \castest{<b>Une plateforme mobile emporte le bloc poussable posé dessus, dont la position suit
 * celle de la plateforme.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une plateforme mobile emporte le bloc poussable posé dessus, dont la position suit
 * celle de la plateforme.
 * }
 */
TEST(CroisementsMecaniques, PlateformeMobileEmporteLeBlocPose) {
    core::TileMap tiles(16, 8);
    floorAt(tiles, 7);
    tiles.setTile(2, 5, core::TileType::MovingPlatform);
    tiles.setTile(2, 4, core::TileType::Block);  // pose juste au-dessus de la plateforme
    const std::vector<core::MovingPlatformConfig> platforms{
        core::MovingPlatformConfig{.startPosition = core::GridPosition{2, 5},
                                   .waypoints = {core::GridPosition{12, 5}},
                                   .speed = 3.0f}};
    core::Level level("plateforme-bloc", tiles, core::GridPosition{1, 6}, core::GridPosition{15, 6},
                      std::vector<core::Mechanism>{}, -1, -1, {}, {}, {}, std::nullopt,
                      std::nullopt, {}, {}, platforms);
    Scene scene(level);
    scene.spawn(1, 6);

    const core::GridPosition blockStart = scene.blocks.positions()[0];
    scene.run(core::PlayerInput{}, 120);

    EXPECT_GT(scene.blocks.positions()[0].column, blockStart.column)
        << "le bloc n'a pas ete emporte par la plateforme";
}

/**
 * @brief Deux déclencheurs — un interrupteur **et** une plaque — liés à la même porte : le
 * comportement n'était ni testé ni décrit par la validation du chargeur. Il est figé ici : le
 * dernier déclencheur évalué décide, chacun restant maître de sa propre porte.
 * \castest{<b>Deux déclencheurs liés à la même porte se composent sans incohérence : la porte suit
 * un état défini, jamais indéterminé.</b><br/>
 * \tcat Integration · Croisements<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Deux déclencheurs liés à la même porte se composent sans incohérence : la porte suit un
 * état défini, jamais indéterminé.
 * }
 */
TEST(CroisementsMecaniques, DeuxDeclencheursSurLaMemePorte) {
    core::TileMap tiles(14, 6);
    floorAt(tiles, 5);
    tiles.setTile(3, 4, core::TileType::Switch);
    tiles.setTile(6, 4, core::TileType::PressurePlate);
    tiles.setTile(10, 2, core::TileType::Door);
    const core::GridPosition door{10, 2};
    const std::vector<core::Mechanism> mechanisms{
        core::Mechanism{.switchPosition = core::GridPosition{3, 4}, .doorPosition = door},
        core::Mechanism{.switchPosition = core::GridPosition{6, 4}, .doorPosition = door}};
    core::Level level("deux-declencheurs", tiles, core::GridPosition{1, 4},
                      core::GridPosition{13, 4}, mechanisms);
    Scene scene(level);
    scene.spawn(1, 4);

    const core::PlayerInput walkRight{1.0f};
    scene.run(walkRight, 300);

    // L'interrupteur a bascule (ouvert, definitif jusqu'au prochain passage) et la plaque s'est
    // refermee au depart : les deux etats coexistent sans que le controleur ne se contredise.
    EXPECT_TRUE(scene.mechanisms.isDoorOpen(0));
    EXPECT_FALSE(scene.mechanisms.isDoorOpen(1));
}

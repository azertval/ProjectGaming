/**
 * @file test_render_budget.cpp
 * @brief Test de non-régression du volume de primitives et de l'efficacité du culling (`LOT-62`
 *        TACHE-01, `EX-NFR-005`, `EX-NFR-004`).
 */

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/Decor.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelScene.h"
#include "Core/Levels/TileMap.h"
#include "Core/Physics/PlayerSpawn.h"
#include "HMI/Graphics/BackgroundRenderer.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/DecorVisuals.h"
#include "HMI/Graphics/ParticleRenderer.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/RoomGrid.h"
#include "HMI/Graphics/ShadowRenderer.h"
#include "HMI/Graphics/TileAutotile.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"

namespace {

// Textures factices : la composition ne fait que comparer des identites (cf. test_quad_recorder).
// Aucun skin ni decor n'est charge : tout retombe sur l'atlas (Physique) ou le damier (Texture,
// EX-NFR-040) -- comportement normal du programme d'habillage, sans consequence sur le VOLUME de
// primitives, seule chose que ce test mesure.
int atlasStorage = 0;
int missingStorage = 0;
int backgroundStorage = 0;
hmi::TextureHandle atlasTexture = &atlasStorage;
hmi::TextureHandle missingTexture = &missingStorage;
hmi::TextureHandle backgroundTexture = &backgroundStorage;

/// Textures de reference des tests : atlas 5x5 cases de 16px (meme geometrie que
/// hmi::TextureAtlas, cf. test_shadow_render.cpp) et damier de repli.
hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.atlas = atlasTexture;
    textures.atlasWidth = 80;
    textures.atlasHeight = 80;
    textures.missing = missingTexture;
    textures.missingWidth = 16;
    textures.missingHeight = 16;
    return textures;
}

/// Charge un niveau livre depuis son fichier JSON (PROJECTGAMING_LEVELS_DIR).
core::Level loadDeliveredLevel(const std::string& fileName) {
    const std::filesystem::path path = std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / fileName;
    const core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
    if (!result.ok()) {
        ADD_FAILURE() << "Echec de chargement de " << fileName << " : " << result.error;
        return core::Level{"invalide",
                           core::TileMap{1, 1},
                           core::GridPosition{0, 0},
                           core::GridPosition{0, 0},
                           {}};
    }
    return *result.level;
}

/// Peuple un monde a partir d'un niveau, EXACTEMENT comme hmi::GameSession::loadLevel (tuiles +
/// decors + personnage a l'entree), sans aucune dependance GPU : c'est ce qui rend la scene
/// composee representative de ce que le joueur voit reellement.
core::World buildWorld(const core::Level& level) {
    core::World world;
    const core::TileMap& map = level.tileMap();
    core::buildLevelScene(
        world, level, [](core::TileType type) { return hmi::regionForTile(type); },
        [&](core::Entity entity, core::TileType type, int column, int row) {
            world.addComponent(
                entity, hmi::TileSkinTag{type, hmi::solidNeighborMask(map, column, row),
                                         hmi::textureOverrideAt(level.textureOverrides(),
                                                                core::GridPosition{column, row})});
        },
        [&](core::Entity entity, const core::Decor& decor, std::size_t) {
            world.addComponent(entity, hmi::DecorVisualTag{decor.assetName, decor.layer});
            world.addComponent(entity, hmi::RenderLayerTag{hmi::decorRenderLayer(decor.layer)});
        });

    // Personnage a l'entree, meme construction que hmi::GameSession::spawnPlayer (les champs
    // d'animation/collision n'affectent pas la composition, omis ici).
    const core::Entity player = world.createEntity();
    const core::Vector2 size = core::playerSize();
    world.addComponent(
        player, core::Transform{core::playerSpawnPosition(level.entry().column, level.entry().row),
                                size, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 80, 16, 16};
    world.addComponent(player, sprite);
    hmi::PlayerSpriteTag playerTag;
    playerTag.quadSize = size;  // approximation suffisante : seul le VOLUME est mesure ici.
    world.addComponent(player, playerTag);
    world.addComponent(player, hmi::RenderLayerTag{hmi::RenderLayer::Player});
    return world;
}

/// Camera de reference, reproductible : cadree sur la salle de l'ENTREE du niveau, comme
/// hmi::GameSession::render (LOT-32, EX-REN-015) -- jamais un etat par defaut susceptible de
/// changer (cf. points d'attention de la tache).
hmi::Camera2D referenceCamera(const core::Level& level) {
    constexpr int VIEWPORT_WIDTH = 1280;
    constexpr int VIEWPORT_HEIGHT = 720;
    constexpr float ROOM_ZOOM_MARGIN = 0.92f;  // meme marge que GameSession::render.

    const core::TileMap& map = level.tileMap();
    const hmi::RoomGrid rooms(map.width(), map.height());
    const hmi::RoomBounds bounds = rooms.roomBounds(rooms.roomIndexAt(level.entry()));

    hmi::Camera2D camera(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    camera.setCenter(
        core::Vector2{static_cast<float>(bounds.column) + static_cast<float>(bounds.width) * 0.5f,
                      static_cast<float>(bounds.row) + static_cast<float>(bounds.height) * 0.5f});
    camera.setZoom(hmi::Camera2D::fitZoom(
        static_cast<float>(VIEWPORT_WIDTH), static_cast<float>(VIEWPORT_HEIGHT),
        static_cast<float>(bounds.width), static_cast<float>(bounds.height), ROOM_ZOOM_MARGIN));
    return camera;
}

/// Compose UNE PASSE d'image complete d'un niveau dans @p scene (jamais videe : la composition est
/// cumulative, meme contrat que hmi::composeWorldSprites), au meme patron que
/// hmi::SpriteRenderer::render mais sans aucune soumission GPU : fond, ombres, sprites du monde,
/// particules (aucune ici). Appelable plusieurs fois de suite sur la meme scene pour simuler une
/// emission en double (cf. le test negatif).
void composeIntoScene(hmi::ComposedScene& scene, core::World& world, const core::Level& level,
                      hmi::RenderMode mode, const hmi::Camera2D& camera) {
    const hmi::SceneTextures textures = testTextures();

    hmi::BackgroundTexture background;
    if (level.background().has_value()) {
        background = hmi::BackgroundTexture{backgroundTexture, 64, 64};
    }
    hmi::composeBackground(scene, background, level.tileMap().width(), level.tileMap().height(),
                           mode);
    hmi::composeShadows(scene, world, mode, textures, 0.0f);
    hmi::composeWorldSprites(scene, world, mode, textures, 0.0f, &camera);
    hmi::composeParticles(scene, world, mode, textures);
}

/// Compose une image complete d'un niveau (une seule passe, triee) : voir composeIntoScene.
hmi::ComposedScene composeLevelScene(core::World& world, const core::Level& level,
                                     hmi::RenderMode mode, const hmi::Camera2D& camera) {
    hmi::ComposedScene scene;
    scene.setVisibleBounds(camera.visibleBounds());
    composeIntoScene(scene, world, level, mode, camera);
    scene.sort();
    return scene;
}

/// Plafond nommé du volume de primitives d'un niveau livré, dans les deux modes de rendu
/// (`EX-NFR-005`). Valeurs larges (cf. points d'attention de la tâche) : elles attrapent un facteur
/// deux accidentel, pas un chiffre exact — voir la mesure de référence datée du `LOT-62`
/// (`Documentation/Guide/guide-rendu.md`).
struct LevelBudget {
    std::string fileName;
    /// Plafond des primitives EXAMINEES (avant culling) : proportionnel au contenu total du
    /// niveau, pas a la salle visible seule.
    int consideredCeiling;
    /// Plafond des primitives SOUMISES (apres culling) : borne par la taille d'une salle
    /// (`hmi::RoomGrid::ROOM_WIDTH_TILES` x `ROOM_HEIGHT_TILES`), plus marge/fond/personnage.
    int submittedCeiling;
};

/// Les niveaux livrés (`Source/Elements/Levels/sequence-demo.json`, `LOT-25`, refondus `LOT-65`),
/// avec leur plafond. Un plafond par niveau, jamais global (`demo-salles` et `demo-deplacement`
/// n'ont rien de comparable) — voir `epic.md`.
const std::vector<LevelBudget>& deliveredLevelBudgets() {
    static const std::vector<LevelBudget> budgets = {
        {"demo-deplacement.json", 160, 160},
        {"demo-saut.json", 210, 210},
        {"demo-double-saut.json", 140, 140},
        {"demo-wall-jump.json", 90, 90},
        {"demo-dash.json", 260, 260},
        {"demo-mouvement.json", 300, 300},
        {"demo-interrupteur.json", 175, 175},
        {"demo-plaque-pression.json", 235, 235},
        {"demo-cle.json", 190, 190},
        {"demo-bloc.json", 190, 190},
        {"demo-budget.json", 230, 160},
        {"demo-pente.json", 190, 190},
        {"demo-pente-gauche.json", 190, 190},
        {"demo-concave.json", 260, 260},
        {"demo-plafond.json", 270, 270},
        {"demo-bloc-reduit.json", 190, 190},
        {"demo-bloc-quart.json", 215, 215},
        {"demo-plateforme.json", 140, 140},
        {"demo-dangers-avances.json", 340, 230},
        {"demo-dangers-directionnels.json", 380, 270},
        {"demo-final.json", 650, 200},
        {"demo-salles.json", 900, 350},
    };
    return budgets;
}

}  // namespace

/**
 * @brief Chaque niveau livré reste sous son plafond de primitives composées et soumises, en mode
 *        Physique et en mode Texture.
 * \castest{<b>Chaque niveau livre reste sous son plafond, dans les deux modes de rendu.</b><br/>
 * \tcat Unitaire · Budget de rendu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger chaque niveau livre.<br/>2. Composer sa scene sur la salle d'entree, en mode
 * Physique puis Texture.<br/>
 * \tattendu Les primitives examinees et soumises restent sous le plafond nomme du niveau, dans
 * les deux modes.
 * }
 */
TEST(RenderBudgetTest, ChaqueNiveauLivreResteSousSonPlafond) {
    for (const LevelBudget& budget : deliveredLevelBudgets()) {
        SCOPED_TRACE(budget.fileName);
        const core::Level level = loadDeliveredLevel(budget.fileName);
        const hmi::Camera2D camera = referenceCamera(level);

        for (const hmi::RenderMode mode : {hmi::RenderMode::Physique, hmi::RenderMode::Texture}) {
            SCOPED_TRACE(mode == hmi::RenderMode::Physique ? "Physique" : "Texture");
            core::World world = buildWorld(level);
            const hmi::ComposedScene scene = composeLevelScene(world, level, mode, camera);

            hmi::QuadRecorder recorder;
            recorder.record(scene);
            const hmi::SceneStatistics& stats = recorder.statistics();

            EXPECT_LE(stats.considered, budget.consideredCeiling)
                << "Primitives examinees au-dela du plafond -- ventilation par calque :\n"
                << recorder.describe() << "\nTile=" << recorder.countOnLayer(hmi::RenderLayer::Tile)
                << " Shadow=" << recorder.countOnLayer(hmi::RenderLayer::Shadow)
                << " Decor=" << recorder.countOnLayer(hmi::RenderLayer::Decor)
                << " Player=" << recorder.countOnLayer(hmi::RenderLayer::Player);
            EXPECT_LE(stats.submitted, budget.submittedCeiling)
                << "Primitives soumises au-dela du plafond -- ventilation par calque :\n"
                << recorder.describe() << "\nTile=" << recorder.countOnLayer(hmi::RenderLayer::Tile)
                << " Shadow=" << recorder.countOnLayer(hmi::RenderLayer::Shadow)
                << " Decor=" << recorder.countOnLayer(hmi::RenderLayer::Decor)
                << " Player=" << recorder.countOnLayer(hmi::RenderLayer::Player);
        }
    }
}

/**
 * @brief Émettre accidentellement un calque en double fait dépasser le plafond du niveau : le
 *        contrôle est sensible, pas simplement très large.
 * \castest{<b>Composer un calque deux fois depasse le plafond du niveau.</b><br/>
 * \tcat Unitaire · Budget de rendu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer normalement demo-deplacement en mode Texture.<br/>2. Composer une seconde
 * fois dans la meme scene (double emission).<br/>
 * \tattendu Le volume double depasse le plafond nomme du niveau, alors que la composition simple
 * restait dessous.
 * }
 */
TEST(RenderBudgetTest, CalqueComposeDeuxFoisDepasseLePlafond) {
    const LevelBudget budget = deliveredLevelBudgets().front();  // demo-deplacement.json
    const core::Level level = loadDeliveredLevel(budget.fileName);
    const hmi::Camera2D camera = referenceCamera(level);
    core::World world = buildWorld(level);

    hmi::ComposedScene scene;
    scene.setVisibleBounds(camera.visibleBounds());
    composeIntoScene(scene, world, level, hmi::RenderMode::Texture, camera);
    const int singleSubmitted = scene.statistics().submitted;
    ASSERT_LE(singleSubmitted, budget.submittedCeiling)
        << "La composition simple devrait deja rester sous le plafond.";

    // Toute la scene composee une seconde fois (bug type : un fond redessine par salle plutot que
    // par ecran, un calque emis deux fois) : jamais videe entre les deux passes, exactement comme
    // un tel bug le ferait a l'insu de l'appelant (composeBackground/composeShadows/
    // composeWorldSprites/composeParticles sont tous cumulatifs par contrat).
    composeIntoScene(scene, world, level, hmi::RenderMode::Texture, camera);
    const int doubledSubmitted = scene.statistics().submitted;

    EXPECT_EQ(doubledSubmitted, singleSubmitted * 2);
    EXPECT_GT(doubledSubmitted, budget.submittedCeiling)
        << "Une double emission doit depasser le plafond -- sinon le plafond ne verifie rien.";
}

/**
 * @brief Sur un grand niveau à plusieurs salles, le culling écarte une fraction significative des
 *        primitives : c'est ce qui distingue un culling qui fonctionne d'un culling qui compile.
 * \castest{<b>Le culling ecarte une fraction significative sur un grand niveau.</b><br/>
 * \tcat Unitaire · Budget de rendu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger demo-salles (plusieurs salles).<br/>2. Composer sa scene sur la salle
 * d'entree, en mode Texture.<br/>
 * \tattendu Au moins la moitie des primitives examinees sont ecartees par le culling.
 * }
 */
TEST(RenderBudgetTest, CullingEcarteUneFractionSignificativeSurUnGrandNiveau) {
    constexpr float MINIMUM_CULLED_FRACTION = 0.5f;

    const core::Level level = loadDeliveredLevel("demo-salles.json");
    const hmi::Camera2D camera = referenceCamera(level);
    core::World world = buildWorld(level);
    const hmi::ComposedScene scene =
        composeLevelScene(world, level, hmi::RenderMode::Texture, camera);

    const hmi::SceneStatistics stats = scene.statistics();
    ASSERT_GT(stats.considered, 0);
    const float culledFraction =
        static_cast<float>(stats.culled) / static_cast<float>(stats.considered);

    EXPECT_GE(culledFraction, MINIMUM_CULLED_FRACTION)
        << hmi::formatSceneStatistics(stats) << " (fraction ecartee : " << culledFraction << ")";
}

/**
 * @brief Le volume de primitives d'une scène est déterministe : composer deux fois la même scène,
 *        avec la même caméra, donne exactement les mêmes compteurs.
 * \castest{<b>Deux compositions de la meme scene donnent le meme volume.</b><br/>
 * \tcat Unitaire · Budget de rendu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer deux fois la scene de demo-salles, en mode Texture.<br/>
 * \tattendu Les compteurs (examinees, ecartees, soumises, passes) sont strictement identiques.
 * }
 */
TEST(RenderBudgetTest, DeuxCompositionsDeLaMemeSceneDonnentLeMemeVolume) {
    const core::Level level = loadDeliveredLevel("demo-salles.json");
    const hmi::Camera2D camera = referenceCamera(level);

    core::World firstWorld = buildWorld(level);
    const hmi::ComposedScene first =
        composeLevelScene(firstWorld, level, hmi::RenderMode::Texture, camera);

    core::World secondWorld = buildWorld(level);
    const hmi::ComposedScene second =
        composeLevelScene(secondWorld, level, hmi::RenderMode::Texture, camera);

    EXPECT_EQ(first.statistics().considered, second.statistics().considered);
    EXPECT_EQ(first.statistics().culled, second.statistics().culled);
    EXPECT_EQ(first.statistics().submitted, second.statistics().submitted);
    EXPECT_EQ(first.statistics().batches, second.statistics().batches);
}

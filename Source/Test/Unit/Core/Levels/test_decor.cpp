/**
 * @file test_decor.cpp
 * @brief Tests unitaires du décor libre (LOT-49, `EX-DEC-001`).
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/Decor.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Math/Vector2.h"

namespace {

using core::Decor;
using core::DecorLayer;
using core::LevelDraft;
using core::Vector2;

}  // namespace

/**
 * @brief addDecor ajoute un décor en fin de vecteur, préservant l'ordre d'ajout.
 * \castest{<b>addDecor ajoute un décor en fin de vecteur.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu addDecor ajoute un décor en fin de vecteur, préservant l'ordre d'ajout.
 * }
 */
TEST(DecorTest, AddDecorAjouteEnFinDeVecteur) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    draft.addDecor(Decor{"bush.png", Vector2{2.5f, 3.25f}});
    draft.addDecor(Decor{"torch.png", Vector2{4.0f, 1.0f}});

    ASSERT_EQ(draft.decors().size(), 2u);
    EXPECT_EQ(draft.decors()[0].assetName, "bush.png");
    EXPECT_EQ(draft.decors()[1].assetName, "torch.png");
}

/**
 * @brief removeDecor retire le décor au rang donné, sans effet si le rang est hors bornes.
 * \castest{<b>removeDecor retire le décor au rang donné.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu removeDecor retire le décor au rang donné, sans effet si le rang est hors bornes.
 * }
 */
TEST(DecorTest, RemoveDecorRetireAuRangDonne) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{2.5f, 3.25f}});
    draft.addDecor(Decor{"torch.png", Vector2{4.0f, 1.0f}});

    draft.removeDecor(0);

    ASSERT_EQ(draft.decors().size(), 1u);
    EXPECT_EQ(draft.decors()[0].assetName, "torch.png");

    draft.removeDecor(42);  // rang hors bornes : sans effet
    EXPECT_EQ(draft.decors().size(), 1u);
}

/**
 * @brief Un niveau sans décor round-trip sans écrire le champ "decors".
 * \castest{<b>Un niveau sans décor n'écrit pas le champ decors.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau sans décor round-trip sans écrire le champ "decors".
 * }
 */
TEST(DecorTest, SansDecorLeChampDecorsEstAbsentDuJson) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    EXPECT_TRUE(loaded.level->decors().empty());

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("\"decors\""), std::string::npos);
}

/**
 * @brief Un niveau existant sans champ "decors" se charge sans erreur (rétrocompatibilité,
 * `EX-LVL-005`).
 * \castest{<b>Un niveau sans champ decors se charge sans erreur.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau existant sans champ decors se charge sans erreur ni décor.
 * }
 */
TEST(DecorTest, NiveauSansChampDecorsSeChargeSansDecor) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [
          {"x":0,"y":0,"type":"entry"},
          {"x":1,"y":1,"type":"solid"},
          {"x":2,"y":2,"type":"exit"}
        ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    EXPECT_TRUE(loaded.level->decors().empty());
}

/**
 * @brief Un décor unique (position fractionnaire, rotation et échelle non par défaut, couche
 * premier plan, manipulable) round-trip exactement.
 * \castest{<b>Un décor unique round-trip exactement via le JSON.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un décor unique (position fractionnaire, rotation et échelle non par défaut, couche
 * premier plan, manipulable) round-trip exactement.
 * }
 */
TEST(DecorTest, UnDecorRoundTripExactement) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.setEntry(0, 0);
    draft.setExit(9, 9);
    Decor decor{"tree.png", Vector2{3.5f, 7.25f}};
    decor.scale = Vector2{2.0f, 0.5f};
    decor.rotation = 1.5f;
    decor.layer = DecorLayer::Foreground;
    decor.manipulable = true;
    draft.addDecor(decor);

    const core::LevelLoadResult built = draft.toLevel();
    ASSERT_TRUE(built.ok()) << built.error;
    const std::string json = core::LevelWriter::toJsonString(*built.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    ASSERT_EQ(reloaded.level->decors().size(), 1u);
    const Decor& roundTripped = reloaded.level->decors().front();
    EXPECT_EQ(roundTripped.assetName, "tree.png");
    EXPECT_EQ(roundTripped.position, (Vector2{3.5f, 7.25f}));
    EXPECT_EQ(roundTripped.scale, (Vector2{2.0f, 0.5f}));
    EXPECT_FLOAT_EQ(roundTripped.rotation, 1.5f);
    EXPECT_EQ(roundTripped.layer, DecorLayer::Foreground);
    EXPECT_TRUE(roundTripped.manipulable);
}

/**
 * @brief Plusieurs décors sur des couches différentes préservent leur ordre à l'écriture et à la
 * relecture.
 * \castest{<b>L'ordre des décors est préservé à l'écriture et à la relecture.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Plusieurs décors sur des couches différentes préservent leur ordre à l'écriture et à
 * la relecture.
 * }
 */
TEST(DecorTest, OrdrePreserveALEcritureEtALaRelecture) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.setEntry(0, 0);
    draft.setExit(9, 9);
    Decor background{"mountain.png", Vector2{1.0f, 1.0f}};
    background.layer = DecorLayer::Background;
    Decor decorLayer{"bush.png", Vector2{2.0f, 2.0f}};
    decorLayer.layer = DecorLayer::Decor;
    Decor foreground{"branch.png", Vector2{3.0f, 3.0f}};
    foreground.layer = DecorLayer::Foreground;
    draft.addDecor(background);
    draft.addDecor(decorLayer);
    draft.addDecor(foreground);

    const core::LevelLoadResult built = draft.toLevel();
    ASSERT_TRUE(built.ok()) << built.error;
    const std::string json = core::LevelWriter::toJsonString(*built.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    ASSERT_EQ(reloaded.level->decors().size(), 3u);
    EXPECT_EQ(reloaded.level->decors()[0].assetName, "mountain.png");
    EXPECT_EQ(reloaded.level->decors()[1].assetName, "bush.png");
    EXPECT_EQ(reloaded.level->decors()[2].assetName, "branch.png");
}

/**
 * @brief Redimensionner la grille conserve les décors hors des nouvelles bornes (choix assumé,
 * différent des autres données annexes).
 * \castest{<b>Redimensionner conserve les décors hors bornes.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Redimensionner la grille conserve les décors hors des nouvelles bornes, contrairement
 * aux autres données annexes.
 * }
 */
TEST(DecorTest, RedimensionnementConserveLesDecorsHorsBornes) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"branch.png", Vector2{9.5f, 9.5f}});

    draft.resize(2, 2);

    ASSERT_EQ(draft.decors().size(), 1u);
    EXPECT_EQ(draft.decors()[0].position, (Vector2{9.5f, 9.5f}));
}

/**
 * @brief undo annule un ajout de décor ; redo le rétablit.
 * \castest{<b>undo/redo couvrent un ajout de décor.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu undo annule un ajout de décor ; redo le rétablit.
 * }
 */
TEST(DecorTest, UndoRedoCouvrentUnAjout) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    draft.addDecor(Decor{"bush.png", Vector2{2.0f, 2.0f}});
    ASSERT_EQ(draft.decors().size(), 1u);

    ASSERT_TRUE(draft.undo());
    EXPECT_TRUE(draft.decors().empty());

    ASSERT_TRUE(draft.redo());
    ASSERT_EQ(draft.decors().size(), 1u);
    EXPECT_EQ(draft.decors()[0].assetName, "bush.png");
}

/**
 * @brief undo annule une suppression de décor (le restitue tel quel, au même rang).
 * \castest{<b>undo restitue un décor supprimé.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu undo annule une suppression de décor (le restitue tel quel, au même rang).
 * }
 */
TEST(DecorTest, UndoRestitueUnDecorSupprime) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{2.0f, 2.0f}});
    draft.addDecor(Decor{"torch.png", Vector2{4.0f, 4.0f}});

    draft.removeDecor(0);
    ASSERT_EQ(draft.decors().size(), 1u);

    ASSERT_TRUE(draft.undo());
    ASSERT_EQ(draft.decors().size(), 2u);
    EXPECT_EQ(draft.decors()[0].assetName, "bush.png");
    EXPECT_EQ(draft.decors()[1].assetName, "torch.png");
}

/**
 * @brief fromLevel restitue les décors d'un niveau déjà chargé, dans leur ordre.
 * \castest{<b>fromLevel restitue les décors d'un niveau chargé.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu fromLevel restitue les décors d'un niveau déjà chargé, dans leur ordre.
 * }
 */
TEST(DecorTest, FromLevelRestitueLesDecors) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 4,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 3, "y": 3, "type": "exit" }
      ],
      "decors": [
        { "asset": "bush.png", "x": 1.5, "y": 2.5, "layer": "background" },
        { "asset": "branch.png", "x": 0.5, "y": 0.5, "layer": "foreground" }
      ]
    })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const LevelDraft draft = LevelDraft::fromLevel(*loaded.level);

    ASSERT_EQ(draft.decors().size(), 2u);
    EXPECT_EQ(draft.decors()[0].assetName, "bush.png");
    EXPECT_EQ(draft.decors()[0].layer, DecorLayer::Background);
    EXPECT_EQ(draft.decors()[1].assetName, "branch.png");
    EXPECT_EQ(draft.decors()[1].layer, DecorLayer::Foreground);
}

/**
 * @brief Sans champ "layer" explicite, un décor est lu sur la couche Decor (valeur par défaut).
 * \castest{<b>Sans champ layer, la couche par défaut est Decor.</b><br/>
 * \tcat Unitaire · Décor<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans champ "layer" explicite, un décor est lu sur la couche Decor (valeur par
 * défaut).
 * }
 */
TEST(DecorTest, SansChampLayerLaCoucheParDefautEstDecor) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 4,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 3, "y": 3, "type": "exit" }
      ],
      "decors": [ { "asset": "bush.png", "x": 1.0, "y": 1.0 } ]
    })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    ASSERT_EQ(loaded.level->decors().size(), 1u);
    EXPECT_EQ(loaded.level->decors().front().layer, DecorLayer::Decor);
}

/**
 * @file test_decor_list_model.cpp
 * @brief Tests unitaires de la construction des lignes de la section « Décors » (LOT-50
 *        TACHE-04).
 */

#include <gtest/gtest.h>

#include <vector>

#include "Core/Levels/Decor.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/DecorListModel.h"

namespace {

using core::Decor;
using core::DecorLayer;
using core::Vector2;

Decor makeDecor(const char* asset, DecorLayer layer) {
    Decor decor{asset, Vector2{}};
    decor.layer = layer;
    return decor;
}

}  // namespace

/**
 * @brief Les lignes sont groupées par couche (arrière-plan, puis décor, puis premier plan).
 * \castest{<b>buildDecorListRows groupe les lignes par couche.</b><br/>
 * \tcat Unitaire · Decor List Model<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire des decors sur les trois couches, dans un ordre entremele.<br/>
 * \tattendu Les lignes sont regroupees par couche : Background, puis Decor, puis Foreground.
 * }
 */
TEST(DecorListModelTest, GroupeLesLignesParCouche) {
    const std::vector<Decor> decors{
        makeDecor("a.png", DecorLayer::Foreground),
        makeDecor("b.png", DecorLayer::Background),
        makeDecor("c.png", DecorLayer::Decor),
        makeDecor("d.png", DecorLayer::Background),
    };

    const std::vector<hmi::DecorListRow> rows = hmi::buildDecorListRows(decors, {});

    ASSERT_EQ(rows.size(), 4u);
    EXPECT_EQ(rows[0].assetName, "b.png");
    EXPECT_EQ(rows[0].layer, DecorLayer::Background);
    EXPECT_EQ(rows[1].assetName, "d.png");
    EXPECT_EQ(rows[1].layer, DecorLayer::Background);
    EXPECT_EQ(rows[2].assetName, "c.png");
    EXPECT_EQ(rows[2].layer, DecorLayer::Decor);
    EXPECT_EQ(rows[3].assetName, "a.png");
    EXPECT_EQ(rows[3].layer, DecorLayer::Foreground);
}

/**
 * @brief À l'intérieur d'une couche, l'ordre de superposition (rang croissant) est préservé.
 * \castest{<b>L'ordre de superposition est preserve a l'interieur d'une couche.</b><br/>
 * \tcat Unitaire · Decor List Model<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire trois decors de la meme couche.<br/>
 * \tattendu Les lignes conservent l'ordre du vecteur d'origine.
 * }
 */
TEST(DecorListModelTest, PreserveLOrdreDeSuperpositionALInterieurDUneCouche) {
    const std::vector<Decor> decors{
        makeDecor("first.png", DecorLayer::Decor),
        makeDecor("second.png", DecorLayer::Decor),
        makeDecor("third.png", DecorLayer::Decor),
    };

    const std::vector<hmi::DecorListRow> rows = hmi::buildDecorListRows(decors, {});

    ASSERT_EQ(rows.size(), 3u);
    EXPECT_EQ(rows[0].assetName, "first.png");
    EXPECT_EQ(rows[1].assetName, "second.png");
    EXPECT_EQ(rows[2].assetName, "third.png");
}

/**
 * @brief Chaque ligne porte le rang d'origine du décor, indépendamment du tri d'affichage.
 * \castest{<b>Chaque ligne porte le rang d'origine du decor.</b><br/>
 * \tcat Unitaire · Decor List Model<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire des decors dont le tri d'affichage change l'ordre.<br/>
 * \tattendu Le champ index de chaque ligne reste le rang dans le vecteur d'origine.
 * }
 */
TEST(DecorListModelTest, ChaqueLigneConserveSonRangDOrigine) {
    const std::vector<Decor> decors{
        makeDecor("foreground.png", DecorLayer::Foreground),  // rang 0
        makeDecor("background.png", DecorLayer::Background),  // rang 1
    };

    const std::vector<hmi::DecorListRow> rows = hmi::buildDecorListRows(decors, {});

    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0].assetName, "background.png");
    EXPECT_EQ(rows[0].index, 1u);
    EXPECT_EQ(rows[1].assetName, "foreground.png");
    EXPECT_EQ(rows[1].index, 0u);
}

/**
 * @brief Un décor dont l'asset ne figure pas parmi les assets connus est marqué manquant.
 * \castest{<b>Un asset introuvable est marque manquant.</b><br/>
 * \tcat Unitaire · Decor List Model<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un decor dont l'asset n'est pas dans la liste des assets connus.<br/>
 * \tattendu La ligne correspondante porte assetMissing a true.
 * }
 */
TEST(DecorListModelTest, AssetIntrouvableEstMarqueManquant) {
    const std::vector<Decor> decors{makeDecor("ghost.png", DecorLayer::Decor)};
    const std::vector<std::string> availableAssets{"bush.png", "branch.png"};

    const std::vector<hmi::DecorListRow> rows = hmi::buildDecorListRows(decors, availableAssets);

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_TRUE(rows[0].assetMissing);
}

/**
 * @brief Un décor dont l'asset figure parmi les assets connus n'est pas marqué manquant.
 * \castest{<b>Un asset present n'est pas marque manquant.</b><br/>
 * \tcat Unitaire · Decor List Model<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un decor dont l'asset est dans la liste des assets connus.<br/>
 * \tattendu La ligne correspondante porte assetMissing a false.
 * }
 */
TEST(DecorListModelTest, AssetPresentNEstPasMarqueManquant) {
    const std::vector<Decor> decors{makeDecor("bush.png", DecorLayer::Decor)};
    const std::vector<std::string> availableAssets{"bush.png", "branch.png"};

    const std::vector<hmi::DecorListRow> rows = hmi::buildDecorListRows(decors, availableAssets);

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_FALSE(rows[0].assetMissing);
}

/**
 * @brief Un niveau sans décor produit une liste vide.
 * \castest{<b>Sans decor, la liste est vide.</b><br/>
 * \tcat Unitaire · Decor List Model<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Appeler buildDecorListRows sur un vecteur vide.<br/>
 * \tattendu La liste renvoyee est vide.
 * }
 */
TEST(DecorListModelTest, SansDecorLaListeEstVide) {
    const std::vector<hmi::DecorListRow> rows = hmi::buildDecorListRows({}, {});

    EXPECT_TRUE(rows.empty());
}

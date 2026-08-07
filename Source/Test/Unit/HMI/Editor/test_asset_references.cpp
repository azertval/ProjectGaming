/**
 * @file test_asset_references.cpp
 * @brief Tests unitaires de la détection des références à un asset (LOT-43 TACHE-02).
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Editor/AssetReferences.h"
#include "HMI/Graphics/SkinCatalog.h"

TEST(AssetReferencesTest, AssetCiteParUnJeuEstTrouve) {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});

    const std::vector<hmi::AssetReference> references =
        hmi::findSkinCatalogReferences(catalog, "stone.png");

    ASSERT_EQ(references.size(), 1U);
    EXPECT_EQ(references.front().setName, "foret");
}

TEST(AssetReferencesTest, AssetCiteParPlusieursJeuxEtTypesEstTrouveAChaqueFois) {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});
    catalog.assign("foret", core::TileType::Block,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Single});
    catalog.assign("grotte", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});

    const std::vector<hmi::AssetReference> references =
        hmi::findSkinCatalogReferences(catalog, "stone.png");

    EXPECT_EQ(references.size(), 3U);
}

TEST(AssetReferencesTest, AssetNonReferenceNeDonneAucuneReference) {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});

    EXPECT_TRUE(hmi::findSkinCatalogReferences(catalog, "grass.png").empty());
}

TEST(AssetReferencesTest, NomProcheMaisDifferentNeDeclenchePasDeFauxPositif) {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone2.png", hmi::SkinMode::Bitmask16});

    EXPECT_TRUE(hmi::findSkinCatalogReferences(catalog, "stone.png").empty());
}

TEST(AssetReferencesTest, DescribeReferencesEstVideSansReference) {
    EXPECT_TRUE(hmi::describeReferences({}).empty());
}

TEST(AssetReferencesTest, DescribeReferencesNommeLesJeuxEtLesTypes) {
    const std::vector<hmi::AssetReference> references{
        {"foret", "solid"},
        {"foret", "block"},
        {"grotte", "solid"},
    };

    const std::string message = hmi::describeReferences(references);

    EXPECT_NE(message.find("foret"), std::string::npos);
    EXPECT_NE(message.find("grotte"), std::string::npos);
    EXPECT_NE(message.find("solid"), std::string::npos);
    EXPECT_NE(message.find("block"), std::string::npos);
}

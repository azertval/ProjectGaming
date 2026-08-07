/**
 * @file test_asset_library.cpp
 * @brief Tests unitaires du balayage/filtrage partagé par le panneau « Textures » (LOT-43 TACHE-01).
 */

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Editor/AssetLibrary.h"

namespace {

// Dossier temporaire vierge par test (créé/supprimé automatiquement).
class AssetLibraryTest : public ::testing::Test {
protected:
    std::filesystem::path dir;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("pg_asset_library_" + std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        std::filesystem::create_directories(dir);
    }
    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(dir, error);
    }

    void touch(const std::string& name) {
        std::ofstream file(dir / name);
    }
};

}  // namespace

TEST_F(AssetLibraryTest, NeRetientQueLesImagesTrieesParOrdreAlphabetique) {
    touch("stone.png");
    touch("crate.png");
    touch("notes.txt");
    touch("README.md");

    const std::vector<std::string> assets = hmi::listAssetFiles(dir);

    EXPECT_EQ(assets, (std::vector<std::string>{"crate.png", "stone.png"}));
}

TEST_F(AssetLibraryTest, ExtensionInsensibleALaCasse) {
    touch("stone.PNG");
    touch("crate.Png");

    const std::vector<std::string> assets = hmi::listAssetFiles(dir);

    EXPECT_EQ(assets, (std::vector<std::string>{"crate.Png", "stone.PNG"}));
}

TEST_F(AssetLibraryTest, FiltreParSousChaineInsensibleALaCasse) {
    touch("stone.png");
    touch("crate.png");
    touch("stone_cracked.png");

    EXPECT_EQ(hmi::listAssetFiles(dir, "STONE"),
             (std::vector<std::string>{"stone.png", "stone_cracked.png"}));
    EXPECT_EQ(hmi::listAssetFiles(dir, "crate"), (std::vector<std::string>{"crate.png"}));
    EXPECT_TRUE(hmi::listAssetFiles(dir, "inexistant").empty());
}

TEST_F(AssetLibraryTest, DossierAbsentListeVide) {
    EXPECT_TRUE(
        hmi::listAssetFiles(std::filesystem::temp_directory_path() / "pg_asset_library_absent")
            .empty());
}

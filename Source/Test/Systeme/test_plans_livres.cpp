// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_plans_livres.cpp
 * @brief Tests système du contenu livré côté plans picturaux : plus aucun décor dans les
 *        niveaux, et tout plan référencé existe aux dimensions que sa densité impose
 *        (`EX-LVL-009`, `EX-DEC-040`, `EX-DEC-041`, `EX-NFR-031`, LOT-69 TACHE-10).
 *
 * Le contrôle porte sur les **fichiers du dépôt**, pas sur une structure en mémoire : c'est le
 * seul endroit qui puisse attraper un plan déclaré mais jamais généré, ou généré à une taille que
 * le format ne relira pas. Les dimensions sont lues dans l'en-tête PNG à la main, sans passer par
 * `HMI` : les tests système ne lient que `Core` (aucune dépendance GPU, `EX-NFR-004`), et
 * treize octets d'IHDR ne justifient pas d'y attacher tout le module de rendu.
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/Plane.h"
#include "Core/Levels/TileMap.h"

namespace {

/// Dossier des niveaux livrés (chemin **source**, pas la copie posée à côté de l'exécutable).
std::filesystem::path levelsDirectory() {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR);
}

/// @return Les fichiers `demo-*.json` du dépôt, triés — l'ordre rend les échecs reproductibles.
std::vector<std::filesystem::path> deliveredLevelFiles() {
    std::vector<std::filesystem::path> files;
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(levelsDirectory())) {
        const std::string name = entry.path().filename().string();
        if (entry.is_regular_file() && name.rfind("demo-", 0) == 0 &&
            entry.path().extension() == ".json") {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

/// Dimensions lues dans l'en-tête PNG (IHDR, toujours le premier bloc, à un décalage fixe).
struct PngSize {
    int width = 0;
    int height = 0;
};

/// @return Les dimensions du PNG @p path, `{0, 0}` si le fichier est absent ou n'est pas un PNG.
PngSize readPngSize(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return {};
    }
    std::array<unsigned char, 24> header{};
    file.read(reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(header.size()));
    if (file.gcount() != static_cast<std::streamsize>(header.size())) {
        return {};
    }
    constexpr std::array<unsigned char, 8> SIGNATURE{0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    for (std::size_t index = 0; index < SIGNATURE.size(); ++index) {
        if (header[index] != SIGNATURE[index]) {
            return {};
        }
    }
    const auto readBigEndian = [&header](std::size_t offset) {
        return static_cast<int>((static_cast<std::uint32_t>(header[offset]) << 24) |
                                (static_cast<std::uint32_t>(header[offset + 1]) << 16) |
                                (static_cast<std::uint32_t>(header[offset + 2]) << 8) |
                                static_cast<std::uint32_t>(header[offset + 3]));
    };
    return PngSize{readBigEndian(16), readBigEndian(20)};
}

}  // namespace

/**
 * @brief Plus aucun niveau livré ne porte le champ `decors`, retiré au `LOT-69`.
 *
 * Le chargeur l'ignore avec un avertissement plutôt que de le rejeter (`EX-LVL-009`) : un niveau
 * oublié se chargerait donc **sans erreur**, simplement dépouillé de son habillage. C'est
 * exactement le genre de régression qu'aucun test de comportement n'attrape.
 * \castest{<b>Aucun niveau livre ne porte encore le champ 'decors'.</b><br/>
 * \tcat Systeme · Contenu livre<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire le texte de chaque fichier demo-*.json du depot.<br/>
 * \tattendu Aucun ne contient la cle "decors".
 * }
 */
TEST(PlansLivresSysteme, AucunNiveauNePorteEncoreDesDecors) {
    const std::vector<std::filesystem::path> files = deliveredLevelFiles();
    ASSERT_FALSE(files.empty()) << "Aucun niveau demo-*.json trouve dans " << levelsDirectory();

    for (const std::filesystem::path& path : files) {
        std::ifstream file(path, std::ios::binary);
        ASSERT_TRUE(file) << "Niveau illisible : " << path;
        const std::string text((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
        EXPECT_EQ(text.find("\"decors\""), std::string::npos)
            << path.filename().string()
            << " porte encore le champ 'decors' : il serait charge SANS ERREUR, mais son "
               "habillage serait silencieusement perdu (EX-LVL-009).";
    }
}

/**
 * @brief Tout plan référencé par un niveau livré existe, et à la taille exacte que sa densité
 *        impose.
 *
 * `Core` ne vérifie pas l'existence d'un fichier de plan (`EX-NFR-011`) et `HMI` replie sur un
 * damier visible (`EX-NFR-040`) : un plan manquant ou mal dimensionné ne casse donc **rien**, il
 * se contente de ne pas s'afficher comme prévu. Le contrôle appartient à ce test, et à lui seul.
 * \castest{<b>Tout plan referencé existe et a les dimensions de sa densite.</b><br/>
 * \tcat Systeme · Contenu livre<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger chaque niveau livre.<br/>2. Pour chacun de ses plans, ouvrir le PNG dans
 * Levels/Plans et lire ses dimensions.<br/>
 * \tattendu Le fichier existe et mesure largeur x densite par hauteur x densite.
 * }
 */
TEST(PlansLivresSysteme, ToutPlanReferenceExisteAuxDimensionsAttendues) {
    const std::filesystem::path plansDirectory = levelsDirectory() / "Plans";
    const std::vector<std::filesystem::path> files = deliveredLevelFiles();
    ASSERT_FALSE(files.empty());

    int checked = 0;
    for (const std::filesystem::path& path : files) {
        const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
        ASSERT_TRUE(loaded.ok()) << path.filename().string() << " : " << loaded.error;
        const core::Level& level = *loaded.level;

        for (const core::Plane& plane : level.planes()) {
            SCOPED_TRACE(path.filename().string() + " -> " + plane.fileName);
            const PngSize size = readPngSize(plansDirectory / plane.fileName);
            EXPECT_EQ(size.width, level.tileMap().width() * plane.pixelsPerUnit);
            EXPECT_EQ(size.height, level.tileMap().height() * plane.pixelsPerUnit);
            ++checked;
        }
    }
    EXPECT_GT(checked, 0) << "Aucun plan verifie : le contenu livre n'en declare plus un seul, ce "
                             "qui viderait ce test de son sens.";
}

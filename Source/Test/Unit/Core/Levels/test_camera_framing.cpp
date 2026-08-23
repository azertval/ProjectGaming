// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_camera_framing.cpp
 * @brief Tests unitaires du cadrage de caméra porté par le niveau : modes, règle de repli et
 *        validation (`EX-LVL-006`, `EX-REN-016`, LOT-64).
 */

#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/LevelLoader.h"

/**
 * @brief Le nom JSON d'un mode et sa reconnaissance sont symétriques pour les trois modes.
 * \castest{<b>Le nom JSON d'un mode et sa reconnaissance sont symétriques.</b><br/>
 * \tcat Unitaire · Camera Framing<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Pour chacun des trois modes, convertir en nom JSON puis reconnaître ce nom.<br/>
 * \tattendu Le mode reconnu est identique au mode d'origine.
 * }
 */
TEST(CameraFramingTest, NomEtReconnaissanceSontSymetriques) {
    for (const core::CameraFramingMode mode :
         {core::CameraFramingMode::WholeLevel, core::CameraFramingMode::PerRoom,
          core::CameraFramingMode::Follow}) {
        const std::string_view name = core::cameraFramingModeName(mode);
        const std::optional<core::CameraFramingMode> parsed = core::parseCameraFramingMode(name);
        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(*parsed, mode);
    }
}

/**
 * @brief Un nom de mode non reconnu ne produit aucun mode.
 * \castest{<b>Un nom de mode non reconnu ne produit aucun mode.</b><br/>
 * \tcat Unitaire · Camera Framing<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Reconnaître un nom de mode arbitraire, non défini.<br/>
 * \tattendu `parseCameraFramingMode` renvoie `std::nullopt`.
 * }
 */
TEST(CameraFramingTest, NomInconnuNeProduitAucunMode) {
    EXPECT_FALSE(core::parseCameraFramingMode("zoomInfini").has_value());
    EXPECT_FALSE(core::parseCameraFramingMode("").has_value());
}

/**
 * @brief Un cadrage déclaré est résolu tel quel, sans substitution.
 * \castest{<b>Un cadrage déclaré est résolu tel quel.</b><br/>
 * \tcat Unitaire · Camera Framing<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre un cadrage explicitement déclaré (`Follow`).<br/>
 * \tattendu Le mode résolu est exactement celui déclaré, quelles que soient les dimensions.
 * }
 */
TEST(CameraFramingTest, CadrageDeclareEstResoluTelQuel) {
    const std::optional<core::CameraFramingConfig> declared =
        core::CameraFramingConfig{.mode = core::CameraFramingMode::Follow};
    // Dimensions qui, sans declaration, resoudraient en PerRoom -- la declaration doit primer.
    const core::CameraFramingConfig resolved = core::resolveCameraFraming(declared, 100, 100);
    EXPECT_EQ(resolved.mode, core::CameraFramingMode::Follow);
}

/**
 * @brief La règle de repli (`EX-LVL-006`) reproduit exactement la règle historique : niveau entier
 * s'il tient dans une salle de taille par défaut (bornes incluses), par salle sinon.
 * \castest{<b>La règle de repli reproduit la règle historique aux bornes.</b><br/>
 * \tcat Unitaire · Camera Framing<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre l'absence de cadrage pour un niveau tenant exactement dans une salle par
 * défaut.<br/>2. Résoudre pour un niveau un cran plus large, puis un cran plus haut.<br/>
 * \tattendu Le niveau qui tient exactement resout en *niveau entier* ; chaque dépassement d'un
 * seul cran sur un axe resout en *par salle*, sans taille personnalisée.
 * }
 */
TEST(CameraFramingTest, RegleDeRepliReproduitLaRegleHistoriqueAuxBornes) {
    const core::CameraFramingConfig exact = core::resolveCameraFraming(
        std::nullopt, core::kDefaultRoomWidthTiles, core::kDefaultRoomHeightTiles);
    EXPECT_EQ(exact.mode, core::CameraFramingMode::WholeLevel);

    const core::CameraFramingConfig tooWide = core::resolveCameraFraming(
        std::nullopt, core::kDefaultRoomWidthTiles + 1, core::kDefaultRoomHeightTiles);
    EXPECT_EQ(tooWide.mode, core::CameraFramingMode::PerRoom);
    EXPECT_FALSE(tooWide.roomWidthTiles.has_value());

    const core::CameraFramingConfig tooTall = core::resolveCameraFraming(
        std::nullopt, core::kDefaultRoomWidthTiles, core::kDefaultRoomHeightTiles + 1);
    EXPECT_EQ(tooTall.mode, core::CameraFramingMode::PerRoom);
}

/**
 * @brief La validation nomme le champ fautif : taille de vue nulle/négative, taille supérieure au
 * niveau, paramètre étranger au mode retenu (`EX-LVL-004`). `roomWidthTiles`/`roomHeightTiles`
 * sont valides pour `PerRoom` **et** `Follow` (la taille de vue du suivi, `EX-REN-017`).
 * \castest{<b>La validation nomme le champ fautif pour chaque cas invalide.</b><br/>
 * \tcat Unitaire · Camera Framing<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Valider une taille de salle nulle, puis supérieure au niveau, puis un paramètre de
 * taille de vue sur le mode *niveau entier*.<br/>2. Valider une taille de vue sur le mode
 * *suivi*.<br/>
 * \tattendu Chaque cas invalide renvoie un message d'erreur non vide ; un cadrage cohérent (dont
 * une taille de vue en mode suivi) ne renvoie aucune erreur.
 * }
 */
TEST(CameraFramingTest, ValidationNommeLeChampFautif) {
    const core::CameraFramingConfig zeroWidth{.mode = core::CameraFramingMode::PerRoom,
                                              .roomWidthTiles = 0};
    EXPECT_TRUE(core::validateCameraFramingConfig(zeroWidth, 20, 20).has_value());

    const core::CameraFramingConfig exceedsLevel{.mode = core::CameraFramingMode::PerRoom,
                                                 .roomHeightTiles = 30};
    EXPECT_TRUE(core::validateCameraFramingConfig(exceedsLevel, 20, 20).has_value());

    const core::CameraFramingConfig foreignParameter{.mode = core::CameraFramingMode::WholeLevel,
                                                     .roomWidthTiles = 10};
    EXPECT_TRUE(core::validateCameraFramingConfig(foreignParameter, 20, 20).has_value());

    const core::CameraFramingConfig valid{
        .mode = core::CameraFramingMode::PerRoom, .roomWidthTiles = 10, .roomHeightTiles = 10};
    EXPECT_FALSE(core::validateCameraFramingConfig(valid, 20, 20).has_value());

    // La taille de vue est desormais valide en mode suivi (EX-REN-017) : reglable comme en mode
    // par salle, ce n'est plus un parametre etranger.
    const core::CameraFramingConfig followWithSize{
        .mode = core::CameraFramingMode::Follow, .roomWidthTiles = 10, .roomHeightTiles = 8};
    EXPECT_FALSE(core::validateCameraFramingConfig(followWithSize, 20, 20).has_value());
}

/**
 * @brief Les zones de caméra dessinées à la main sont valides uniquement en mode *par salle*,
 * entièrement dans les bornes du niveau, et de taille non nulle (`EX-LVL-004`, `EX-LVL-007`).
 * \castest{<b>La validation des zones de caméra nomme le champ fautif.</b><br/>
 * \tcat Unitaire · Camera Framing<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Valider une zone sur un mode qui n'est pas *par salle*.<br/>2. Valider une zone de
 * largeur nulle, une zone à position négative, une zone dépassant le niveau.<br/>3. Valider un
 * cadrage *par salle* avec des zones valides, y compris chevauchantes.<br/>
 * \tattendu Chaque cas invalide échoue ; un cadrage cohérent, zones chevauchantes comprises,
 * réussit.
 * }
 */
TEST(CameraFramingTest, ValidationDesZonesNommeLeChampFautif) {
    const core::CameraFramingConfig wrongMode{
        .mode = core::CameraFramingMode::WholeLevel,
        .zones = {core::CameraZone{.x = 0, .y = 0, .width = 5, .height = 5}}};
    EXPECT_TRUE(core::validateCameraFramingConfig(wrongMode, 20, 20).has_value());

    const core::CameraFramingConfig zeroWidth{
        .mode = core::CameraFramingMode::PerRoom,
        .zones = {core::CameraZone{.x = 0, .y = 0, .width = 0, .height = 5}}};
    EXPECT_TRUE(core::validateCameraFramingConfig(zeroWidth, 20, 20).has_value());

    const core::CameraFramingConfig negativePosition{
        .mode = core::CameraFramingMode::PerRoom,
        .zones = {core::CameraZone{.x = -1, .y = 0, .width = 5, .height = 5}}};
    EXPECT_TRUE(core::validateCameraFramingConfig(negativePosition, 20, 20).has_value());

    const core::CameraFramingConfig exceedsLevel{
        .mode = core::CameraFramingMode::PerRoom,
        .zones = {core::CameraZone{.x = 15, .y = 0, .width = 10, .height = 5}}};
    EXPECT_TRUE(core::validateCameraFramingConfig(exceedsLevel, 20, 20).has_value());

    // Zones chevauchantes : valide (la priorite se joue a la resolution, pas a la validation).
    const core::CameraFramingConfig overlapping{
        .mode = core::CameraFramingMode::PerRoom,
        .zones = {core::CameraZone{.x = 0, .y = 0, .width = 10, .height = 10},
                  core::CameraZone{.x = 5, .y = 5, .width = 10, .height = 10}}};
    EXPECT_FALSE(core::validateCameraFramingConfig(overlapping, 20, 20).has_value());
}

namespace {

// Un niveau livré et son mode de cadrage attendu. Chaque niveau déclare désormais un cadrage
// explicite (LOT-65 TACHE-02, EX-LVL-006) plutôt que de dépendre de la règle de repli par
// dimensions -- ce test vérifie le choix effectivement livré, pas la règle elle-même (couverte par
// ailleurs dans ce fichier avec des dimensions synthétiques). Même liste, dans le même esprit, que
// le parcours complet (Source/Test/Systeme/test_parcours_complet.cpp) : toute divergence doit se
// retrouver ici.
struct ExpectedFraming {
    const char* file;
    core::CameraFramingMode mode;
    /// Le niveau règle-t-il lui-même sa taille de salle / de suivi (`EX-REN-017`) ? Aucun tableau
    /// ne le faisait avant le `LOT-65` TACHE-09 : la mécanique était livrée et jamais employée,
    /// ce qu'un contrôle portant sur le seul `mode` ne pouvait pas voir.
    bool customRoomSize = false;
};

}  // namespace

/**
 * @brief Chaque niveau livré déclare un cadrage **explicite** (`LOT-65`) : la plupart tiennent
 * dans une salle par défaut et se voient entiers, `demo-mouvement.json` -- la traversée continue
 * la plus longue -- **suit** le personnage, et `demo-final.json` découpe ses quatre salles *par
 * salle*, avec des zones dessinées à la main et une taille de salle qui lui est propre.
 * \castest{<b>Chaque niveau livré déclare le cadrage explicitement choisi pour son contenu.</b>
 * <br/>
 * \tcat Unitaire · Camera Framing<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger les niveaux livrés (`Source/Elements/Levels`).<br/>2. Vérifier le mode de
 * cadrage résolu de chacun.<br/>
 * \tattendu La plupart résolvent en *niveau entier*, `demo-mouvement.json` en *suivi* et
 * `demo-final.json` en *par salle*.
 * }
 */
TEST(CameraFramingTest, NiveauxLivresReproduisentLeurComportementActuel) {
    const std::vector<ExpectedFraming> levels = {
        {"demo-deplacement.json", core::CameraFramingMode::WholeLevel},
        {"demo-saut.json", core::CameraFramingMode::WholeLevel},
        {"demo-double-saut.json", core::CameraFramingMode::WholeLevel},
        {"demo-wall-jump.json", core::CameraFramingMode::WholeLevel},
        {"demo-dash.json", core::CameraFramingMode::WholeLevel},
        {"demo-mouvement.json", core::CameraFramingMode::Follow, /*customRoomSize=*/true},
        {"demo-interrupteur.json", core::CameraFramingMode::WholeLevel},
        {"demo-plaque-pression.json", core::CameraFramingMode::WholeLevel},
        {"demo-cle.json", core::CameraFramingMode::WholeLevel},
        {"demo-bloc.json", core::CameraFramingMode::WholeLevel},
        {"demo-budget.json", core::CameraFramingMode::WholeLevel},
        {"demo-pente.json", core::CameraFramingMode::WholeLevel},
        {"demo-bloc-reduit.json", core::CameraFramingMode::WholeLevel},
        {"demo-plateforme.json", core::CameraFramingMode::WholeLevel},
        {"demo-dangers-avances.json", core::CameraFramingMode::WholeLevel},
        {"demo-final.json", core::CameraFramingMode::PerRoom, /*customRoomSize=*/true},
    };

    for (const ExpectedFraming& expected : levels) {
        const std::filesystem::path path =
            std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / expected.file;
        const core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
        ASSERT_TRUE(result.ok()) << expected.file << " : " << result.error;
        EXPECT_EQ(result.level->cameraFraming().mode, expected.mode) << expected.file;
        // Taille de salle : declaree seulement la ou le tableau la choisit, jamais subie ailleurs.
        EXPECT_EQ(result.level->cameraFraming().roomWidthTiles.has_value(), expected.customRoomSize)
            << expected.file;
        EXPECT_EQ(result.level->cameraFraming().roomHeightTiles.has_value(),
                  expected.customRoomSize)
            << expected.file;
    }
}

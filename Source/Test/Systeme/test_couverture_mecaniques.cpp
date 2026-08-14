/**
 * @file test_couverture_mecaniques.cpp
 * @brief Garde-fou de couverture des mécaniques par le contenu livré (`EX-LVL-015`, `LOT-65`
 *        TACHE-01).
 *
 * `scripts/check_demo_sequence.py` protège déjà la **séquence** (le jeu et ce module énumèrent les
 * mêmes fichiers) ; il ne dit rien de la **couverture** : rien n'empêche qu'un type de tuile livré,
 * testé unitairement, n'apparaisse dans aucun niveau réellement joué. Ce module comble ce trou en
 * dérivant l'inventaire attendu des **énumérations du code** (`core::TileType`,
 * `core::CameraFramingMode`), jamais d'une liste recopiée à la main — une liste recopiée se périme
 * en silence au premier type ajouté sans qu'on pense à la mettre à jour.
 *
 * « Couvert » signifie seulement « présent dans un niveau de la séquence livrée » : ce contrôle ne
 * vérifie pas la franchissabilité (rôle du test système, `ParcoursCompletSysteme`,
 * `EX-NFR-021`) — une mécanique posée dans un coin inaccessible serait « couverte » sans jamais
 * être jouée. Les deux contrôles sont nécessaires et complémentaires, jamais l'un substitut de
 * l'autre.
 */

#include <filesystem>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/Decor.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelSequence.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Levels/TileTypeName.h"

namespace {

// Enumere tous les TileType significatifs pour le contenu (tout sauf Empty, jamais emis dans un
// niveau -- cf. TileTypeName.cpp). Meme borne (`TileType::MovingPlatform`, le dernier enumerateur)
// et meme technique que `core::parseTileType` (Core/Levels/TileTypeName.cpp) : un type ajoute APRES
// MovingPlatform exigerait de bouger cette borne aux DEUX endroits, exactement comme
// `parseTileType` -- ce n'est pas une limite propre a ce garde-fou. Un type INSERE avant
// MovingPlatform (le cas courant) est en revanche pris en compte sans aucune modification ici : ni
// liste recopiee, ni compteur a incrementer a la main.
std::vector<core::TileType> allContentTileTypes() {
    std::vector<core::TileType> types;
    for (int raw = 0; raw <= static_cast<int>(core::TileType::MovingPlatform); ++raw) {
        const auto type = static_cast<core::TileType>(raw);
        if (type == core::TileType::Empty) {
            continue;  // jamais emis dans un niveau : les cases vides sont omises de 'tiles'.
        }
        types.push_back(type);
    }
    return types;
}

// Enumere les trois modes de cadrage (Core/Levels/CameraFraming.h) : trois valeurs seulement, pas
// de sentinelle de fin a maintenir -- une liste explicite ici reste sure tant que ce petit enum ne
// change pas de convention (contrairement a TileType, jamais recopiee ailleurs dans ce module).
std::vector<core::CameraFramingMode> allCameraFramingModes() {
    return {core::CameraFramingMode::WholeLevel, core::CameraFramingMode::PerRoom,
            core::CameraFramingMode::Follow};
}

/**
 * @brief Exclusions nommées et justifiées : aucune aujourd'hui.
 *
 * Chaque type de `core::TileType` et chaque mode de `core::CameraFramingMode` correspond à un
 * contenu légitimement plaçable dans un niveau de démonstration -- il n'existe pas de mécanique
 * livrée qui ne PEUT pas apparaître dans un tableau joué. La liste reste ici, vide plutôt
 * qu'absente, pour que la prochaine exclusion (si elle devient un jour nécessaire) ait un
 * emplacement évident, commenté, plutôt que d'être bricolée ailleurs.
 */
std::set<core::TileType> excludedTileTypes() {
    return {};
}

// État de couverture accumulé en parcourant la séquence livrée.
struct CoverageState {
    std::set<core::TileType> presentTypes;
    std::set<core::CameraFramingMode> presentFramingModes;
    bool dangerBlinkDephased = false;   // au moins un DangerBlink avec phase != 0.
    bool dangerMoverVertical = false;   // au moins un DangerMover avec axis == Vertical.
    bool movementBudgetBounded = false; // au moins un niveau avec jumpBudget ou dashBudget borné.
    bool instanceTextureOverride = false;  // au moins une texture assignée par instance.
    bool foregroundDecor = false;          // au moins un décor de premier plan.
};

// Parcourt la séquence livrée (même fichier que le jeu et `test_parcours_complet.cpp`) et relève
// les mécaniques présentes. Une séquence absente/vide/invalide (état de `TACHE-00`, avant que
// `TACHE-01`/`TACHE-02`/`TACHE-03` ne reconstruisent le contenu) donne un état entièrement vide --
// c'est-à-dire tout, sauf les exclusions nommées, apparaît comme manquant. C'est le résultat rouge
// attendu tant que le contenu n'a pas été reconstruit, pas un défaut de ce garde-fou.
CoverageState scanDeliveredSequence() {
    CoverageState state;
    const std::filesystem::path sequencePath =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "sequence-demo.json";
    const core::LevelSequenceLoadResult sequenceResult =
        core::LevelSequenceLoader::loadFromFile(sequencePath);
    if (!sequenceResult.ok()) {
        return state;
    }

    for (const std::string& file : sequenceResult.sequence->levels) {
        const std::filesystem::path levelPath = std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
        const core::LevelLoadResult levelResult = core::LevelLoader::loadFromFile(levelPath);
        if (!levelResult.ok()) {
            // Un niveau invalide est un défaut distinct, déjà signalé ailleurs (chargement,
            // `check_demo_sequence.py`) : ce garde-fou ne mesure que la couverture, pas la
            // validité -- il continue avec les niveaux qui restent.
            continue;
        }
        const core::Level& level = *levelResult.level;
        const core::TileMap& map = level.tileMap();
        for (int row = 0; row < map.height(); ++row) {
            for (int column = 0; column < map.width(); ++column) {
                state.presentTypes.insert(map.tile(column, row));
            }
        }
        state.presentFramingModes.insert(level.cameraFraming().mode);
        if (level.jumpBudget() >= 0 || level.dashBudget() >= 0) {
            state.movementBudgetBounded = true;
        }
        if (!level.textureOverrides().empty()) {
            state.instanceTextureOverride = true;
        }
        for (const core::Decor& decor : level.decors()) {
            if (decor.layer == core::DecorLayer::Foreground) {
                state.foregroundDecor = true;
            }
        }
        for (const core::DangerBlinkConfig& blink : level.blinkConfigs()) {
            if (blink.phase != 0) {
                state.dangerBlinkDephased = true;
            }
        }
        for (const core::DangerMoverConfig& mover : level.moverConfigs()) {
            if (mover.axis == core::DangerMoverAxis::Vertical) {
                state.dangerMoverVertical = true;
            }
        }
    }
    return state;
}

// Types attendus mais absents de `covered`, exclusions nommées mises à part -- fonction pure,
// testée directement (test négatif) sans dépendre de niveaux réels sur disque.
std::vector<core::TileType> missingTileTypes(const std::set<core::TileType>& covered,
                                             const std::set<core::TileType>& excluded) {
    std::vector<core::TileType> missing;
    for (const core::TileType type : allContentTileTypes()) {
        if (excluded.contains(type)) {
            continue;
        }
        if (!covered.contains(type)) {
            missing.push_back(type);
        }
    }
    return missing;
}

std::vector<core::CameraFramingMode> missingFramingModes(
    const std::set<core::CameraFramingMode>& covered) {
    std::vector<core::CameraFramingMode> missing;
    for (const core::CameraFramingMode mode : allCameraFramingModes()) {
        if (!covered.contains(mode)) {
            missing.push_back(mode);
        }
    }
    return missing;
}

std::string describeMissing(const CoverageState& state) {
    std::ostringstream out;
    const std::vector<core::TileType> missingTypes =
        missingTileTypes(state.presentTypes, excludedTileTypes());
    if (!missingTypes.empty()) {
        out << "types de tuile non couverts : ";
        for (const core::TileType type : missingTypes) {
            out << core::tileTypeName(type) << " ";
        }
        out << "; ";
    }
    const std::vector<core::CameraFramingMode> missingModes =
        missingFramingModes(state.presentFramingModes);
    if (!missingModes.empty()) {
        out << "modes de cadrage non couverts : ";
        for (const core::CameraFramingMode mode : missingModes) {
            out << core::cameraFramingModeName(mode) << " ";
        }
        out << "; ";
    }
    if (!state.dangerBlinkDephased) {
        out << "aucun DangerBlink dephase (phase != 0) ; ";
    }
    if (!state.dangerMoverVertical) {
        out << "aucun DangerMover vertical ; ";
    }
    if (!state.movementBudgetBounded) {
        out << "aucun budget de mouvements borne (jumpBudget/dashBudget) ; ";
    }
    if (!state.instanceTextureOverride) {
        out << "aucune texture par instance ; ";
    }
    if (!state.foregroundDecor) {
        out << "aucun decor de premier plan ; ";
    }
    return out.str();
}

}  // namespace

/**
 * @brief L'inventaire dérive de l'énumération, pas d'une liste recopiée : chaque type produit par
 * `allContentTileTypes` a un nom (`core::tileTypeName`) qui se relit exactement au même type
 * (`core::parseTileType`) -- même garantie structurelle que celle déjà démontrée pour
 * `core::parseTileType` lui-même, appliquée à l'énumération utilisée par ce garde-fou.
 * \castest{<b>L'inventaire des types de tuile dérive de l'énumération du code : chaque type
 * produit a un nom qui se relit exactement au même type.</b><br/>
 * \tcat Systeme · Couverture Mecaniques<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'inventaire des types de tuile dérive de l'énumération du code : chaque type produit
 * a un nom qui se relit exactement au même type.
 * }
 */
TEST(CouvertureMecaniques, InventaireDeriveDeLEnumeration) {
    const std::vector<core::TileType> types = allContentTileTypes();
    ASSERT_FALSE(types.empty());
    for (const core::TileType type : types) {
        EXPECT_NE(type, core::TileType::Empty);
        const std::string name = core::tileTypeName(type);
        const std::optional<core::TileType> roundTrip = core::parseTileType(name);
        ASSERT_TRUE(roundTrip.has_value()) << "nom sans type : " << name;
        EXPECT_EQ(*roundTrip, type);
    }
}

/**
 * @brief Test négatif : retirer d'un ensemble couvert la seule occurrence d'un type le fait
 * apparaître comme manquant, sans toucher à l'inventaire lui-même (`allContentTileTypes`) --
 * démontre que le garde-fou est sensible à une régression de couverture, indépendamment de tout
 * fichier de niveau réel sur disque.
 * \castest{<b>Retirer d'un tableau la seule occurrence d'un type de tuile fait échouer le
 * garde-fou de couverture, qui le nomme précisément.</b><br/>
 * \tcat Systeme · Couverture Mecaniques<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Retirer d'un tableau la seule occurrence d'un type de tuile fait échouer le garde-fou
 * de couverture, qui le nomme précisément.
 * }
 */
TEST(CouvertureMecaniques, GardeFouSensibleAUneMecaniqueManquante) {
    std::set<core::TileType> covered;
    for (const core::TileType type : allContentTileTypes()) {
        covered.insert(type);
    }
    covered.erase(core::TileType::Solid);  // seule occurrence retirée.

    const std::vector<core::TileType> missing = missingTileTypes(covered, excludedTileTypes());
    ASSERT_EQ(missing.size(), 1U);
    EXPECT_EQ(missing.front(), core::TileType::Solid);

    // Même démonstration côté modes de cadrage.
    std::set<core::CameraFramingMode> coveredModes = {core::CameraFramingMode::WholeLevel,
                                                       core::CameraFramingMode::PerRoom,
                                                       core::CameraFramingMode::Follow};
    coveredModes.erase(core::CameraFramingMode::Follow);
    const std::vector<core::CameraFramingMode> missingModes = missingFramingModes(coveredModes);
    ASSERT_EQ(missingModes.size(), 1U);
    EXPECT_EQ(missingModes.front(), core::CameraFramingMode::Follow);
}

/**
 * @brief Garde-fou principal : chaque type de tuile, chaque mode de cadrage et chaque variante
 * significative (danger temporisé déphasé, danger mobile vertical, budget de mouvements borné,
 * texture par instance, décor de premier plan) livrés apparaissent dans au moins un niveau de la
 * séquence réellement livrée (`sequence-demo.json`) -- pas dans un test unitaire isolé.
 * \castest{<b>Chaque mécanique livrée (type de tuile, mode de cadrage, variante significative)
 * apparaît dans au moins un niveau de la séquence livrée.</b><br/>
 * \tcat Systeme · Couverture Mecaniques<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Chaque mécanique livrée (type de tuile, mode de cadrage, variante significative)
 * apparaît dans au moins un niveau de la séquence livrée.
 * }
 */
TEST(CouvertureMecaniques, ChaqueMecaniqueLivreeEstCouverteParLaSequence) {
    const CoverageState state = scanDeliveredSequence();
    const std::vector<core::TileType> missingTypes =
        missingTileTypes(state.presentTypes, excludedTileTypes());
    const std::vector<core::CameraFramingMode> missingModes =
        missingFramingModes(state.presentFramingModes);

    const bool allCovered = missingTypes.empty() && missingModes.empty() &&
                            state.dangerBlinkDephased && state.dangerMoverVertical &&
                            state.movementBudgetBounded && state.instanceTextureOverride &&
                            state.foregroundDecor;
    EXPECT_TRUE(allCovered) << describeMissing(state);
}

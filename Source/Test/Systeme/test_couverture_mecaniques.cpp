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
 * « Couvert » signifie ici « posé au moins `MIN_OCCURRENCES` fois dans la séquence livrée »
 * (`LOT-65` TACHE-05). La version initiale de ce garde-fou se contentait de la **présence**, et
 * cela n'a pas suffi : la séquence livrée en `TACHE-03` le satisfaisait avec un exemplaire unique
 * de presque chaque mécanique, dont plusieurs hors d'atteinte du personnage. Le seuil
 * d'occurrences répare la première moitié du défaut ; la seconde — « posé » ne vaut pas
 * « atteignable » — relève du test système (`ParcoursCompletSysteme`, `EX-NFR-021`), qui relève
 * désormais la trajectoire réelle et refuse une mécanique hors de portée. Les deux contrôles sont
 * nécessaires et complémentaires, jamais l'un substitut de l'autre.
 */

#include <filesystem>
#include <map>
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

/**
 * @brief Nombre minimal d'occurrences d'un type de tuile dans l'ensemble de la séquence
 *        (`LOT-65` TACHE-05, doctrine de profondeur — `niveaux.md` Sec. 3).
 *
 * Une occurrence unique prouve qu'un type se **charge**, pas qu'il se **joue** : c'est exactement
 * l'état qu'avait produit la première séquence du `LOT-65`, où 941 tuiles `Solid` côtoyaient un
 * exemplaire unique de presque chaque mécanique. Trois suffisent à distinguer « posé » de
 * « pratiqué » (montrer, pratiquer, varier) sans imposer de remplissage. Constante nommée plutôt
 * que littéral dispersé : le seuil est un choix de conception, pas une vérité.
 */
constexpr int MIN_OCCURRENCES = 3;

// État de couverture accumulé en parcourant la séquence livrée.
struct CoverageState {
    std::map<core::TileType, int> typeCounts;  // occurrences cumulées, pas simple présence.
    std::set<core::CameraFramingMode> presentFramingModes;
    bool dangerBlinkDephased = false;  // au moins un DangerBlink avec phase != 0.
    bool dangerMoverVertical = false;  // au moins un DangerMover avec axis == Vertical.
    // Les deux budgets sont comptés SÉPARÉMENT : un « jumpBudget ou dashBudget » laissait passer
    // une séquence entière sans le moindre budget de dash, ce qui était le cas jusqu'au LOT-65
    // TACHE-05.
    bool jumpBudgetBounded = false;
    bool dashBudgetBounded = false;
    bool instanceTextureOverride = false;  // au moins une texture assignée par instance.
    bool foregroundDecor = false;          // au moins un décor de premier plan.
    // Variantes de cadrage du LOT-64 invisibles d'un contrôle portant sur le seul `mode` :
    // rectangles de caméra dessinés à la main (EX-LVL-007) et taille de salle/suivi choisie par le
    // niveau plutôt que subie (EX-REN-017).
    bool cameraZonesDeclared = false;
    bool explicitRoomSize = false;
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
                ++state.typeCounts[map.tile(column, row)];
            }
        }
        const core::CameraFramingConfig& framing = level.cameraFraming();
        state.presentFramingModes.insert(framing.mode);
        if (!framing.zones.empty()) {
            state.cameraZonesDeclared = true;
        }
        if (framing.roomWidthTiles.has_value() || framing.roomHeightTiles.has_value()) {
            state.explicitRoomSize = true;
        }
        if (level.jumpBudget() >= 0) {
            state.jumpBudgetBounded = true;
        }
        if (level.dashBudget() >= 0) {
            state.dashBudgetBounded = true;
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

// Types attendus dont les occurrences cumulées restent sous `minOccurrences`, exclusions nommées
// mises à part -- fonction pure, testée directement (test négatif) sans dépendre de niveaux réels
// sur disque. Un type absent compte zéro occurrence : « manquant » et « posé une seule fois » sont
// le même défaut à des degrés différents, et le même contrôle les couvre.
std::vector<core::TileType> insufficientTileTypes(const std::map<core::TileType, int>& counts,
                                                  const std::set<core::TileType>& excluded,
                                                  int minOccurrences = MIN_OCCURRENCES) {
    std::vector<core::TileType> insufficient;
    for (const core::TileType type : allContentTileTypes()) {
        if (excluded.contains(type)) {
            continue;
        }
        const auto found = counts.find(type);
        const int count = found == counts.end() ? 0 : found->second;
        if (count < minOccurrences) {
            insufficient.push_back(type);
        }
    }
    return insufficient;
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
        insufficientTileTypes(state.typeCounts, excludedTileTypes());
    if (!missingTypes.empty()) {
        out << "types de tuile posés moins de " << MIN_OCCURRENCES << " fois : ";
        for (const core::TileType type : missingTypes) {
            const auto found = state.typeCounts.find(type);
            out << core::tileTypeName(type) << "(" << (found == state.typeCounts.end() ? 0 : found->second)
                << ") ";
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
    if (!state.jumpBudgetBounded) {
        out << "aucun budget de sauts borne (jumpBudget) ; ";
    }
    if (!state.dashBudgetBounded) {
        out << "aucun budget de dashs borne (dashBudget) ; ";
    }
    if (!state.instanceTextureOverride) {
        out << "aucune texture par instance ; ";
    }
    if (!state.foregroundDecor) {
        out << "aucun decor de premier plan ; ";
    }
    if (!state.cameraZonesDeclared) {
        out << "aucune zone de camera dessinee (cameraFraming.zones) ; ";
    }
    if (!state.explicitRoomSize) {
        out << "aucune taille de salle/suivi choisie par un niveau (roomWidthTiles/"
               "roomHeightTiles) ; ";
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
 * @brief Test négatif : retirer les occurrences d'un type, ou n'en laisser qu'une, le fait
 * apparaître comme insuffisamment couvert, sans toucher à l'inventaire lui-même
 * (`allContentTileTypes`) -- démontre que le garde-fou est sensible à une régression de couverture
 * **comme** à une régression de profondeur, indépendamment de tout fichier de niveau réel sur
 * disque. La frontière du seuil est vérifiée dans les deux sens.
 * \castest{<b>Retirer les occurrences d'un type de tuile, ou n'en laisser qu'une, fait
 * échouer le garde-fou de couverture, qui le nomme précisément.</b><br/>
 * \tcat Systeme · Couverture Mecaniques<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Retirer d'un tableau la seule occurrence d'un type de tuile fait échouer le garde-fou
 * de couverture, qui le nomme précisément.
 * }
 */
TEST(CouvertureMecaniques, GardeFouSensibleAUneMecaniqueManquante) {
    std::map<core::TileType, int> counts;
    for (const core::TileType type : allContentTileTypes()) {
        counts[type] = MIN_OCCURRENCES;
    }
    counts.erase(core::TileType::Solid);  // toutes les occurrences retirées.

    const std::vector<core::TileType> missing = insufficientTileTypes(counts, excludedTileTypes());
    ASSERT_EQ(missing.size(), 1U);
    EXPECT_EQ(missing.front(), core::TileType::Solid);

    // Profondeur (LOT-65 TACHE-05) : un type POSÉ, mais une seule fois, est signalé exactement
    // comme un type absent -- c'est le défaut que le contrôle par simple présence laissait passer.
    counts[core::TileType::Solid] = MIN_OCCURRENCES;
    counts[core::TileType::Switch] = 1;
    const std::vector<core::TileType> shallow = insufficientTileTypes(counts, excludedTileTypes());
    ASSERT_EQ(shallow.size(), 1U);
    EXPECT_EQ(shallow.front(), core::TileType::Switch);

    // Et le seuil est bien une frontière : à MIN_OCCURRENCES pile, plus rien n'est signalé.
    counts[core::TileType::Switch] = MIN_OCCURRENCES;
    EXPECT_TRUE(insufficientTileTypes(counts, excludedTileTypes()).empty());

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
 * @brief Garde-fou principal : chaque type de tuile livré est posé au moins `MIN_OCCURRENCES` fois
 * dans la séquence réellement livrée (`sequence-demo.json`), chaque mode de cadrage y apparaît, et
 * chaque variante significative est employée — danger temporisé déphasé, danger mobile vertical,
 * budget de sauts **et** budget de dashs (comptés séparément), texture par instance, décor de
 * premier plan, zones de caméra dessinées et taille de salle choisie par le niveau.
 * \castest{<b>Chaque type de tuile livré est posé au moins trois fois dans la séquence, et chaque
 * mode de cadrage comme chaque variante significative y est employé.</b><br/>
 * \tcat Systeme · Couverture Mecaniques<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Chaque type de tuile livré est posé au moins trois fois dans la séquence, et chaque
 * mode de cadrage comme chaque variante significative y est employé.
 * }
 */
TEST(CouvertureMecaniques, ChaqueMecaniqueLivreeEstCouverteParLaSequence) {
    const CoverageState state = scanDeliveredSequence();
    const std::vector<core::TileType> missingTypes =
        insufficientTileTypes(state.typeCounts, excludedTileTypes());
    const std::vector<core::CameraFramingMode> missingModes =
        missingFramingModes(state.presentFramingModes);

    const bool allCovered = missingTypes.empty() && missingModes.empty() &&
                            state.dangerBlinkDephased && state.dangerMoverVertical &&
                            state.jumpBudgetBounded && state.dashBudgetBounded &&
                            state.instanceTextureOverride && state.foregroundDecor &&
                            state.cameraZonesDeclared && state.explicitRoomSize;
    EXPECT_TRUE(allCovered) << describeMissing(state);
}

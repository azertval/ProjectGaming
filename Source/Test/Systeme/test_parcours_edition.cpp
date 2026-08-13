/**
 * @file test_parcours_edition.cpp
 * @brief Parcours système : créer un niveau dans l'éditeur, l'enregistrer, le recharger, et
 *        vérifier qu'il est immédiatement jouable — sans la couche GPU (LOT-14).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

/**
 * @brief Parcours complet d'édition : peindre, lier un mécanisme, annuler une erreur,
 *        redimensionner, enregistrer, recharger, et confirmer que le niveau produit est
 *        directement jouable — exactement ce qu'un level designer fait dans l'éditeur
 *        (`EX-EDIT-002` à `EX-EDIT-011`).
 * \castest{<b>Parcours complet d'édition : peindre, lier un mécanisme, annuler une erreur,
 * redimensionner, enregistrer, recharger, et confirmer que le niveau produit est directement
 * jouable.</b><br/>
 * \tcat Système · Éditeur de niveaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peindre un niveau (entrée, sortie, interrupteur/porte liés) dans un
 * `LevelDraft`.<br/>2. Peindre une tuile par erreur puis l'annuler (`undo`).<br/>3. Redimensionner
 * la grille.<br/>4. Valider, enregistrer sur disque, recharger.<br/>5. Vérifier le contenu
 * rechargé et l'issue (`evaluateOutcome`) à l'entrée et à la sortie.<br/>
 * \tattendu Le niveau rechargé restitue exactement le contenu édité (dont l'annulation) et se
 * comporte comme un niveau livré normalement : `Playing` à l'entrée, `Won` à la sortie.
 * }
 */
TEST(ParcoursEditionSysteme, CreerEditerEnregistrerRechargerEtJouer) {
    // 1. Un level designer peint un petit niveau avec un mécanisme.
    core::LevelDraft draft = core::LevelDraft::empty("Parcours systeme", 5, 3);
    draft.setEntry(0, 1);
    draft.setExit(4, 1);
    draft.paintTile(2, 0, core::TileType::Switch);
    draft.paintTile(2, 2, core::TileType::Door);
    draft.linkMechanism(core::GridPosition{2, 0}, core::GridPosition{2, 2});
    draft.setJumpBudget(2);

    // 2. Une erreur de peinture, annulée (EX-EDIT-005) : l'historique fait partie du geste
    // normal d'édition, pas seulement d'un scénario isolé.
    draft.paintTile(1, 1, core::TileType::Danger);
    ASSERT_TRUE(draft.undo());
    EXPECT_EQ(draft.tileMap().tile(1, 1), core::TileType::Empty);

    // 3. Redimensionnement : la grille s'agrandit sans perdre le contenu déjà posé.
    draft.resize(6, 3);
    ASSERT_EQ(draft.tileMap().width(), 6);
    EXPECT_EQ(draft.tileMap().tile(4, 1), core::TileType::Exit);

    // 4. Enregistrement puis rechargement (round-trip disque, EX-EDIT-011).
    const core::LevelLoadResult validated = draft.toLevel();
    ASSERT_TRUE(validated.ok()) << validated.error;

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "projectgaming_systeme_edition.json";
    ASSERT_TRUE(core::LevelWriter::saveToFile(*validated.level, path));

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromFile(path);
    std::filesystem::remove(path);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    const core::Level& level = *reloaded.level;
    EXPECT_EQ(level.entry(), (core::GridPosition{0, 1}));
    EXPECT_EQ(level.exit(), (core::GridPosition{4, 1}));
    EXPECT_EQ(level.jumpBudget(), 2);
    EXPECT_EQ(level.tileMap().tile(1, 1), core::TileType::Empty);  // l'annulation a survecu
    ASSERT_EQ(level.mechanisms().size(), 1u);
    EXPECT_EQ(level.mechanisms().front().switchPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(level.mechanisms().front().doorPosition, (core::GridPosition{2, 2}));

    // 5. "Pret a jouer" : le niveau reconstruit reagit exactement comme le jeu l'attend.
    const core::Aabb atEntry =
        core::Aabb::fromTopLeftSize(core::Vector2{0.0f, 1.0f}, core::Vector2{0.9f, 0.9f});
    EXPECT_EQ(core::evaluateOutcome(atEntry, level), core::LevelOutcome::Playing);

    const core::Aabb atExit =
        core::Aabb::fromTopLeftSize(core::Vector2{4.0f, 1.0f}, core::Vector2{0.9f, 0.9f});
    EXPECT_EQ(core::evaluateOutcome(atExit, level), core::LevelOutcome::Won);
}

/**
 * @brief Parcours complet d'édition des dangers avancés : peindre les sept nouveaux types, lier
 * un danger commuté à un interrupteur, configurer un danger mobile et un danger temporisé,
 * enregistrer, recharger — round-trip fidèle de bout en bout (`EX-GP-050` à `053`, `EX-EDIT-011`).
 * \castest{<b>Parcours d'édition des dangers avancés : peindre, lier, configurer, enregistrer,
 * recharger.</b><br/>
 * \tcat Système · Éditeur de niveaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peindre les sept nouveaux types de danger dans un `LevelDraft`.<br/>2. Lier un
 * interrupteur à un danger commuté (même geste qu'une porte).<br/>3. Configurer l'axe/la portée
 * d'un danger mobile et la période/le déphasage d'un danger temporisé.<br/>4. Enregistrer sur
 * disque, recharger.<br/>
 * \tattendu Le niveau rechargé restitue exactement les sept types, la liaison de danger commuté et
 * les deux configurations explicites.
 * }
 */
TEST(ParcoursEditionSysteme, EditeConfigureEnregistreEtRechargeLesDangersAvances) {
    // 1. Un level designer peint les sept nouveaux types dans un brouillon.
    core::LevelDraft draft = core::LevelDraft::empty("Dangers avances (edition)", 8, 4);
    draft.setEntry(0, 0);
    draft.setExit(7, 3);
    draft.paintTile(1, 0, core::TileType::DangerUp);
    draft.paintTile(2, 0, core::TileType::DangerRight);
    draft.paintTile(3, 0, core::TileType::Switch);
    draft.paintTile(4, 0, core::TileType::DangerSwitched);
    draft.paintTile(5, 0, core::TileType::DangerMover);
    draft.paintTile(6, 0, core::TileType::DangerBlink);

    // 2. Liaison interrupteur -> danger commuté, même geste que déclencheur -> porte.
    draft.linkMechanism(core::GridPosition{3, 0}, core::GridPosition{4, 0});

    // 3. Configuration explicite du mobile (axe/portée) et du temporisé (période/déphasage/durée
    // active) — sans configuration, valeurs de conception par défaut (couvert par
    // `test_level_loader.cpp`, non répété ici).
    draft.setMoverConfig(core::GridPosition{5, 0}, core::DangerMoverAxis::Vertical, 2);
    draft.setBlinkConfig(core::GridPosition{6, 0}, 90, 15, 30);

    // 4. Enregistrement puis rechargement (round-trip disque, EX-EDIT-011).
    const core::LevelLoadResult validated = draft.toLevel();
    ASSERT_TRUE(validated.ok()) << validated.error;

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "projectgaming_systeme_edition_dangers.json";
    ASSERT_TRUE(core::LevelWriter::saveToFile(*validated.level, path));

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromFile(path);
    std::filesystem::remove(path);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    const core::Level& level = *reloaded.level;
    EXPECT_EQ(level.tileMap().tile(1, 0), core::TileType::DangerUp);
    EXPECT_EQ(level.tileMap().tile(2, 0), core::TileType::DangerRight);
    EXPECT_EQ(level.tileMap().tile(4, 0), core::TileType::DangerSwitched);
    EXPECT_EQ(level.tileMap().tile(5, 0), core::TileType::DangerMover);
    EXPECT_EQ(level.tileMap().tile(6, 0), core::TileType::DangerBlink);

    ASSERT_EQ(level.dangerLinks().size(), 1u);
    EXPECT_EQ(level.dangerLinks().front().triggerPosition, (core::GridPosition{3, 0}));
    EXPECT_EQ(level.dangerLinks().front().dangerPosition, (core::GridPosition{4, 0}));

    ASSERT_EQ(level.moverConfigs().size(), 1u);
    EXPECT_EQ(level.moverConfigs().front().axis, core::DangerMoverAxis::Vertical);
    EXPECT_EQ(level.moverConfigs().front().range, 2);

    ASSERT_EQ(level.blinkConfigs().size(), 1u);
    EXPECT_EQ(level.blinkConfigs().front().period, 90);
    EXPECT_EQ(level.blinkConfigs().front().phase, 15);
    EXPECT_EQ(level.blinkConfigs().front().activeDuration, 30);
}

/**
 * @brief Parcours complet d'édition du cadrage de caméra : choisir un mode, l'annuler, en choisir
 * un autre avec une taille de salle personnalisée, enregistrer, recharger -- exactement le geste
 * du sélecteur de la section « Cadrage » (`EX-EDIT-028`, LOT-64).
 * \castest{<b>Parcours complet d'édition du cadrage de caméra : choisir, annuler, enregistrer,
 * recharger.</b><br/>
 * \tcat Système · Éditeur de niveaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Choisir le mode *suivi* sur un brouillon.<br/>2. Annuler (`undo`) : retour au
 * cadrage par défaut.<br/>3. Choisir le mode *par salle* avec une taille personnalisée.<br/>4.
 * Enregistrer sur disque, recharger.<br/>
 * \tattendu Le niveau rechargé restitue exactement le mode et la taille de salle choisis en
 * dernier ; l'annulation intermédiaire n'y figure pas (historique linéaire normal).
 * }
 */
TEST(ParcoursEditionSysteme, EditeAnnuleEnregistreEtRechargeLeCadrageDeCamera) {
    // 1. Le level designer choisit le mode "suivi" dans la section "Cadrage".
    core::LevelDraft draft = core::LevelDraft::empty("Cadrage (edition)", 30, 20);
    draft.setEntry(0, 0);
    draft.setExit(29, 19);
    draft.setCameraFraming(core::CameraFramingConfig{.mode = core::CameraFramingMode::Follow});
    EXPECT_EQ(draft.cameraFraming().mode, core::CameraFramingMode::Follow);

    // 2. Changement d'avis, annulé (EX-EDIT-005) : restitue le cadrage par défaut d'un brouillon
    // vierge (LevelDraft::empty, niveau entier) -- valeur de départ du champ, pas la règle de
    // repli d'EX-LVL-006 (qui ne s'applique qu'au CHARGEMENT d'un fichier sans champ déclaré).
    ASSERT_TRUE(draft.undo());
    EXPECT_EQ(draft.cameraFraming().mode, core::CameraFramingMode::WholeLevel);

    // 3. Choix definitif : mode "par salle" avec une taille de salle personnalisee.
    draft.setCameraFraming(core::CameraFramingConfig{
        .mode = core::CameraFramingMode::PerRoom, .roomWidthTiles = 10, .roomHeightTiles = 8});

    // 4. Enregistrement puis rechargement (round-trip disque, EX-EDIT-011).
    const core::LevelLoadResult validated = draft.toLevel();
    ASSERT_TRUE(validated.ok()) << validated.error;

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "projectgaming_systeme_edition_cadrage.json";
    ASSERT_TRUE(core::LevelWriter::saveToFile(*validated.level, path));

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromFile(path);
    std::filesystem::remove(path);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    const core::Level& level = *reloaded.level;
    EXPECT_EQ(level.cameraFraming().mode, core::CameraFramingMode::PerRoom);
    ASSERT_TRUE(level.cameraFraming().roomWidthTiles.has_value());
    EXPECT_EQ(*level.cameraFraming().roomWidthTiles, 10);
    ASSERT_TRUE(level.cameraFraming().roomHeightTiles.has_value());
    EXPECT_EQ(*level.cameraFraming().roomHeightTiles, 8);
}

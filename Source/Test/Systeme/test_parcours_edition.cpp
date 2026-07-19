/**
 * @file test_parcours_edition.cpp
 * @brief Parcours système : créer un niveau dans l'éditeur, l'enregistrer, le recharger, et
 *        vérifier qu'il est immédiatement jouable — sans la couche GPU (LOT-14).
 */

#include <filesystem>

#include <gtest/gtest.h>

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

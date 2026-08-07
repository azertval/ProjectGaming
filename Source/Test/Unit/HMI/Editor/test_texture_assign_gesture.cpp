#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"
#include "HMI/Editor/TextureAssignGesture.h"

namespace {

using core::GridPosition;
using core::TileType;
using hmi::resolveTextureAssignClick;
using hmi::TextureAssignAction;

}  // namespace

// Case vide : toujours ignoree, quel que soit l'etat de selection/override.
TEST(TextureAssignGesture, CaseVideIgnoree) {
    const auto decision = resolveTextureAssignClick(GridPosition{1, 1}, TileType::Empty,
                                                     std::nullopt, std::string{"crate.png"}, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Ignore);
}

// Case sans override, aucun asset selectionne : ignoree (rien a assigner).
TEST(TextureAssignGesture, AucunAssetSelectionneIgnore) {
    const auto decision =
        resolveTextureAssignClick(GridPosition{1, 1}, TileType::Door, std::nullopt, std::nullopt, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Ignore);
}

// Case sans override, un asset est selectionne : assignation.
TEST(TextureAssignGesture, CaseSansOverrideAssigneLAssetSelectionne) {
    const auto decision = resolveTextureAssignClick(
        GridPosition{2, 3}, TileType::Door, std::nullopt, std::string{"door_red.png"}, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Assign);
    EXPECT_EQ(decision.cell, (GridPosition{2, 3}));
    EXPECT_EQ(decision.assetName, "door_red.png");
}

// Case avec un override different de l'asset selectionne : remplacement (Assign).
TEST(TextureAssignGesture, CaseAvecOverrideDifferentRemplace) {
    const auto decision =
        resolveTextureAssignClick(GridPosition{2, 3}, TileType::Door, std::string{"door_blue.png"},
                                  std::string{"door_red.png"}, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Assign);
    EXPECT_EQ(decision.assetName, "door_red.png");
}

// Case avec le MEME override que l'asset selectionne : clic gauche retire (bascule).
TEST(TextureAssignGesture, ReclicSurLeMemeAssetRetire) {
    const auto decision =
        resolveTextureAssignClick(GridPosition{2, 3}, TileType::Door, std::string{"door_red.png"},
                                  std::string{"door_red.png"}, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Remove);
    EXPECT_EQ(decision.cell, (GridPosition{2, 3}));
}

// Clic droit sur une case avec override : retrait explicite.
TEST(TextureAssignGesture, ClicDroitAvecOverrideRetire) {
    const auto decision = resolveTextureAssignClick(
        GridPosition{2, 3}, TileType::Door, std::string{"door_red.png"}, std::nullopt, true);
    EXPECT_EQ(decision.action, TextureAssignAction::Remove);
    EXPECT_EQ(decision.cell, (GridPosition{2, 3}));
}

// Clic droit sur une case sans override : sans effet.
TEST(TextureAssignGesture, ClicDroitSansOverrideIgnore) {
    const auto decision = resolveTextureAssignClick(GridPosition{2, 3}, TileType::Door, std::nullopt,
                                                     std::string{"door_red.png"}, true);
    EXPECT_EQ(decision.action, TextureAssignAction::Ignore);
}

// Clic droit sur une case vide : ignore avant meme de regarder l'override (aucune case a retirer).
TEST(TextureAssignGesture, ClicDroitSurCaseVideIgnore) {
    const auto decision = resolveTextureAssignClick(GridPosition{2, 3}, TileType::Empty,
                                                     std::string{"door_red.png"}, std::nullopt, true);
    EXPECT_EQ(decision.action, TextureAssignAction::Ignore);
}

// Tout type non-Empty est eligible : aucune liste blanche par type (contrairement aux liens).
TEST(TextureAssignGesture, ToutTypeNonVideEstEligible) {
    const auto decision = resolveTextureAssignClick(
        GridPosition{0, 0}, TileType::SlopeUpRight, std::nullopt, std::string{"deco.png"}, false);
    EXPECT_EQ(decision.action, TextureAssignAction::Assign);
}

#include "HMI/Graphics/MechanismVisuals.h"

#include <gtest/gtest.h>

namespace hmi {
namespace {

// Chaque famille de mecanisme, chaque etat : la correspondance renvoie le clip attendu (LOT-47
// TACHE-01) -- table petite et figee, donc testable exhaustivement.

TEST(MechanismVisualsTest, DoorTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::Door, false), MECHANISM_CLIP_DOOR_CLOSED);
    EXPECT_EQ(mechanismTargetClip(core::TileType::Door, true), MECHANISM_CLIP_DOOR_OPEN);
}

TEST(MechanismVisualsTest, SwitchTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::Switch, false), MECHANISM_CLIP_SWITCH_INACTIVE);
    EXPECT_EQ(mechanismTargetClip(core::TileType::Switch, true), MECHANISM_CLIP_SWITCH_ACTIVE);
}

TEST(MechanismVisualsTest, PressurePlateTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::PressurePlate, false),
              MECHANISM_CLIP_PLATE_RELEASED);
    EXPECT_EQ(mechanismTargetClip(core::TileType::PressurePlate, true), MECHANISM_CLIP_PLATE_PRESSED);
}

TEST(MechanismVisualsTest, DangerSwitchedTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerSwitched, false),
              MECHANISM_CLIP_DANGER_SWITCHED_INACTIVE);
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerSwitched, true),
              MECHANISM_CLIP_DANGER_SWITCHED_ACTIVE);
}

TEST(MechanismVisualsTest, DangerBlinkTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerBlink, false),
              MECHANISM_CLIP_DANGER_BLINK_HARMLESS);
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerBlink, true),
              MECHANISM_CLIP_DANGER_BLINK_LETHAL);
}

TEST(MechanismVisualsTest, DangerMoverHasSingleClipIrrespectiveOfActive) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerMover, false),
              MECHANISM_CLIP_DANGER_MOVER_IDLE);
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerMover, true),
              MECHANISM_CLIP_DANGER_MOVER_IDLE);
}

TEST(MechanismVisualsTest, StatelessTileProducesNoClipRequest) {
    EXPECT_FALSE(mechanismTargetClip(core::TileType::Solid, false).has_value());
    EXPECT_FALSE(mechanismTargetClip(core::TileType::Block, true).has_value());
    EXPECT_FALSE(mechanismTargetClip(core::TileType::Empty, false).has_value());
    EXPECT_FALSE(isStatefulMechanism(core::TileType::Solid));
    EXPECT_FALSE(isStatefulMechanism(core::TileType::Block));
}

TEST(MechanismVisualsTest, StatefulMechanismsAreRecognized) {
    EXPECT_TRUE(isStatefulMechanism(core::TileType::Door));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::Switch));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::PressurePlate));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::DangerSwitched));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::DangerBlink));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::DangerMover));
}

TEST(MechanismVisualsTest, DoorTransitionsOnStateChangeOnly) {
    const auto opening = mechanismTransitionClip(core::TileType::Door, false, true);
    ASSERT_TRUE(opening.has_value());
    EXPECT_EQ(*opening, MECHANISM_CLIP_DOOR_OPENING);

    const auto closing = mechanismTransitionClip(core::TileType::Door, true, false);
    ASSERT_TRUE(closing.has_value());
    EXPECT_EQ(*closing, MECHANISM_CLIP_DOOR_CLOSING);

    EXPECT_FALSE(mechanismTransitionClip(core::TileType::Door, true, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::Door, false, false).has_value());
}

TEST(MechanismVisualsTest, OnlyDoorTransitionsOtherFamiliesSnapDirectly) {
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::Switch, false, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::PressurePlate, false, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::DangerSwitched, false, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::DangerBlink, false, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::DangerMover, false, true).has_value());
}

TEST(MechanismVisualsTest, ExpectedClipsMatchTargetAndTransitionNames) {
    const std::vector<std::string> doorClips = mechanismExpectedClips(core::TileType::Door);
    EXPECT_EQ(doorClips, (std::vector<std::string>{MECHANISM_CLIP_DOOR_CLOSED,
                                                    MECHANISM_CLIP_DOOR_OPENING,
                                                    MECHANISM_CLIP_DOOR_OPEN,
                                                    MECHANISM_CLIP_DOOR_CLOSING}));
    EXPECT_EQ(mechanismExpectedClips(core::TileType::DangerMover),
              (std::vector<std::string>{MECHANISM_CLIP_DANGER_MOVER_IDLE}));
    EXPECT_TRUE(mechanismExpectedClips(core::TileType::Solid).empty());
}

// Modulation d'opacite de diagnostic (LOT-47 TACHE-03) : 1.0 en mode Texture, quelle que soit
// l'etat -- c'est la garantie qu'"aucun quad de mecanisme n'a une teinte alpha differente de 1"
// en mode Texture (critere d'acceptation du lot).
TEST(MechanismVisualsTest, TextureModeAlwaysFullyOpaque) {
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Texture, true, 0.25f, 1.0f), 1.0f);
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Texture, false, 0.25f, 1.0f), 1.0f);
}

TEST(MechanismVisualsTest, PhysiqueModeUsesTheProvidedAlphaPair) {
    // Porte : ouverte (active) plus TRANSPARENTE.
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Physique, true, 0.25f, 1.0f), 0.25f);
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Physique, false, 0.25f, 1.0f), 1.0f);
    // Danger : actif plus OPAQUE -- sens oppose, meme fonction.
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Physique, true, 1.0f, 0.35f), 1.0f);
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Physique, false, 1.0f, 0.35f), 0.35f);
}

}  // namespace
}  // namespace hmi

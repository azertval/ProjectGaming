/**
 * @file test_animation_clip.cpp
 * @brief Tests unitaires du modèle de clip d'animation (`core::AnimationClip`/`core::ClipSet`,
 *        LOT-46 TACHE-01) : données pures, sans GPU ni fichier.
 */

#include <gtest/gtest.h>

#include "Core/Ecs/AnimationClip.h"

namespace {
core::AnimationClip makeClip(std::string name, std::vector<int> frames, float duration,
                              core::ClipEndMode endMode = core::ClipEndMode::Loop,
                              std::string next = {}) {
    core::AnimationClip clip;
    clip.name = std::move(name);
    clip.frames = std::move(frames);
    clip.frameDuration = duration;
    clip.endMode = endMode;
    clip.nextClip = std::move(next);
    return clip;
}
}  // namespace

TEST(AnimationClipTest, ClipExistantResoluParNom) {
    core::ClipSet clips;
    clips.addClip(makeClip("idle", {0, 1}, 0.5f));
    clips.addClip(makeClip("run", {2, 3, 4, 5}, 0.1f));

    EXPECT_EQ(clips.clipCount(), 2);
    ASSERT_GE(clips.indexOf("idle"), 0);
    ASSERT_GE(clips.indexOf("run"), 0);
    EXPECT_EQ(clips.clipAt(clips.indexOf("idle")).name, "idle");
    EXPECT_EQ(clips.clipAt(clips.indexOf("run")).name, "run");
}

TEST(AnimationClipTest, ClipInexistantRepliDeterministe) {
    core::ClipSet clips;
    clips.addClip(makeClip("idle", {0, 1}, 0.5f));

    EXPECT_EQ(clips.indexOf("inconnu"), -1);
    // Un index hors bornes (celui renvoye par un nom absent, ou n'importe quel entier invalide)
    // ne plante pas et retombe sur le premier clip (EX-NFR-040).
    EXPECT_EQ(clips.clipAt(-1).name, "idle");
    EXPECT_EQ(clips.clipAt(42).name, "idle");
}

TEST(AnimationClipTest, JeuVideRepliSurUnClipParDefaut) {
    const core::ClipSet clips;
    EXPECT_EQ(clips.clipCount(), 0);
    const core::AnimationClip& fallback = clips.clipAt(0);
    EXPECT_TRUE(fallback.name.empty());
    ASSERT_FALSE(fallback.frames.empty());
    EXPECT_EQ(fallback.frameDuration, 0.0f);
}

TEST(AnimationClipTest, ClipAUneSeuleImage) {
    core::ClipSet clips;
    clips.addClip(makeClip("jump", {6}, 0.1f));
    const core::AnimationClip& clip = clips.clipAt(clips.indexOf("jump"));
    EXPECT_EQ(clip.frames.size(), 1u);
}

TEST(AnimationClipTest, DureesInegalesEntreClips) {
    core::ClipSet clips;
    clips.addClip(makeClip("idle", {0, 1}, 0.5f));
    clips.addClip(makeClip("run", {2, 3, 4, 5}, 0.1f));

    EXPECT_FLOAT_EQ(clips.clipAt(clips.indexOf("idle")).frameDuration, 0.5f);
    EXPECT_FLOAT_EQ(clips.clipAt(clips.indexOf("run")).frameDuration, 0.1f);
}

TEST(AnimationClipTest, ClipJoueUneFoisAvecClipSuivant) {
    core::ClipSet clips;
    clips.addClip(
        makeClip("opening", {1, 2, 3, 4}, 0.06f, core::ClipEndMode::OneShot, "open"));
    clips.addClip(makeClip("open", {5}, 0.0f));

    const core::AnimationClip& opening = clips.clipAt(clips.indexOf("opening"));
    EXPECT_EQ(opening.endMode, core::ClipEndMode::OneShot);
    EXPECT_EQ(opening.nextClip, "open");
    EXPECT_GE(clips.indexOf(opening.nextClip), 0);
}

TEST(AnimationClipTest, AjouterUnClipDeMemeNomLeRemplace) {
    core::ClipSet clips;
    clips.addClip(makeClip("idle", {0}, 1.0f));
    const int firstIndex = clips.indexOf("idle");
    clips.addClip(makeClip("idle", {0, 1, 2}, 0.25f));

    EXPECT_EQ(clips.clipCount(), 1);
    EXPECT_EQ(clips.indexOf("idle"), firstIndex);
    EXPECT_EQ(clips.clipAt(firstIndex).frames.size(), 3u);
}

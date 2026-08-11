#include <gtest/gtest.h>

#include "HMI/Audio/AudioEngine.h"

namespace {

TEST(AudioEngine, MutedEngineAcceptsPreloadAndPlayWithoutError) {
    hmi::AudioEngine engine(hmi::AudioEngine::ForceMuted::Yes);
    EXPECT_TRUE(engine.muted());

    engine.preload("saut", "chemin/inexistant.wav");
    engine.play("saut");
    engine.play("evenement_jamais_precharge");
    // Aucune exception, aucun plantage : le test reussit s'il atteint ce point.
}

TEST(AudioEngine, VolumeIsClampedToUnitRange) {
    hmi::AudioEngine engine(hmi::AudioEngine::ForceMuted::Yes);

    engine.setVolume(-5.0f);
    EXPECT_FLOAT_EQ(engine.volume(), 0.0f);

    engine.setVolume(5.0f);
    EXPECT_FLOAT_EQ(engine.volume(), 1.0f);

    engine.setVolume(0.42f);
    EXPECT_FLOAT_EQ(engine.volume(), 0.42f);
}

TEST(AudioEngine, DefaultVolumeIsFull) {
    hmi::AudioEngine engine(hmi::AudioEngine::ForceMuted::Yes);
    EXPECT_FLOAT_EQ(engine.volume(), 1.0f);
}

}  // namespace

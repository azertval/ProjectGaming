/**
 * @file test_player_input_mapper.cpp
 * @brief Tests unitaires de la traduction clavier → intention (`toPlayerInput`).
 */

#include <gtest/gtest.h>

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/PlayerInputMapper.h"

namespace {

// Construit un état clavier avec un ensemble de touches enfoncées.
hmi::InputState withKeys(std::initializer_list<hmi::Key> keys) {
    hmi::InputState input;
    for (const hmi::Key key : keys) {
        input.onKeyDown(key);
    }
    return input;
}

}  // namespace

/**
 * @brief Flèche gauche seule → intention vers la gauche (-1).
 * \castest{<b>Flèche gauche seule → intention vers la gauche (-1).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Flèche gauche seule → intention vers la gauche (-1).
 * }
 */
TEST(PlayerInputMapperTest, FlecheGauche) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Left})).moveX, -1.0f);
}

/**
 * @brief Flèche droite seule → intention vers la droite (+1).
 * \castest{<b>Flèche droite seule → intention vers la droite (+1).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Flèche droite seule → intention vers la droite (+1).
 * }
 */
TEST(PlayerInputMapperTest, FlecheDroite) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Right})).moveX, 1.0f);
}

/**
 * @brief Touches alternatives ZQSD : Q → gauche, D → droite.
 * \castest{<b>Touches alternatives ZQSD : Q → gauche, D → droite.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Touches alternatives ZQSD : Q → gauche, D → droite.
 * }
 */
TEST(PlayerInputMapperTest, TouchesAlternativesQetD) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Q})).moveX, -1.0f);
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::D})).moveX, 1.0f);
}

/**
 * @brief Aucune touche → intention nulle (immobile).
 * \castest{<b>Aucune touche → intention nulle (immobile).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Aucune touche → intention nulle (immobile).
 * }
 */
TEST(PlayerInputMapperTest, AucuneTouche) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(hmi::InputState{}).moveX, 0.0f);
}

/**
 * @brief Gauche et droite simultanées → neutralisation (0).
 * \castest{<b>Gauche et droite simultanées → neutralisation (0).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Gauche et droite simultanées → neutralisation (0).
 * }
 */
TEST(PlayerInputMapperTest, GaucheEtDroiteSeNeutralisent) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Left, hmi::Key::Right})).moveX, 0.0f);
}

/**
 * @brief Espace fraîchement enfoncée → saut **pressé** (front) et **maintenu**.
 * \castest{<b>Espace fraîchement enfoncée → saut **pressé** (front) et **maintenu**.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Espace fraîchement enfoncée → saut **pressé** (front) et **maintenu**.
 * }
 */
TEST(PlayerInputMapperTest, EspacePresseeDeclencheLeSaut) {
    const core::PlayerInput input = hmi::toPlayerInput(withKeys({hmi::Key::Space}));
    EXPECT_TRUE(input.jumpPressed);
    EXPECT_TRUE(input.jumpHeld);
}

/**
 * @brief `W` équivaut à Espace pour le saut.
 * \castest{<b>`W` équivaut à Espace pour le saut.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `W` équivaut à Espace pour le saut.
 * }
 */
TEST(PlayerInputMapperTest, WEquivautEspacePourLeSaut) {
    EXPECT_TRUE(hmi::toPlayerInput(withKeys({hmi::Key::W})).jumpPressed);
}

/**
 * @brief Saut **maintenu** sans nouveau front → `jumpHeld` vrai mais `jumpPressed` faux.
 * \castest{<b>Saut **maintenu** sans nouveau front → `jumpHeld` vrai mais `jumpPressed`
 * faux.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Saut **maintenu** sans nouveau front → `jumpHeld` vrai mais `jumpPressed` faux.
 * }
 */
TEST(PlayerInputMapperTest, SautMaintenuN_estPasUnFront) {
    hmi::InputState input;
    input.onKeyDown(hmi::Key::Space);  // frame 1 : le front a déjà eu lieu
    input.beginFrame();                // frame 2 : l'état courant devient l'état précédent
    input.onKeyDown(hmi::Key::Space);  // toujours enfoncée, mais plus au front

    const core::PlayerInput mapped = hmi::toPlayerInput(input);
    EXPECT_FALSE(mapped.jumpPressed);
    EXPECT_TRUE(mapped.jumpHeld);
}

/**
 * @brief Aucune touche de saut → ni pressé ni maintenu.
 * \castest{<b>Aucune touche de saut → ni pressé ni maintenu.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Aucune touche de saut → ni pressé ni maintenu.
 * }
 */
TEST(PlayerInputMapperTest, PasDeSaut) {
    const core::PlayerInput input = hmi::toPlayerInput(hmi::InputState{});
    EXPECT_FALSE(input.jumpPressed);
    EXPECT_FALSE(input.jumpHeld);
}

/**
 * @brief Déplacement et saut sont indépendants (axes distincts).
 * \castest{<b>Déplacement et saut sont indépendants (axes distincts).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Déplacement et saut sont indépendants (axes distincts).
 * }
 */
TEST(PlayerInputMapperTest, DeplacementEtSautIndependants) {
    const core::PlayerInput input =
        hmi::toPlayerInput(withKeys({hmi::Key::Right, hmi::Key::Space}));
    EXPECT_FLOAT_EQ(input.moveX, 1.0f);
    EXPECT_TRUE(input.jumpPressed);
}

/**
 * @brief Maj fraîchement enfoncée → dash **pressé** (front).
 * \castest{<b>Maj fraîchement enfoncée → dash **pressé** (front).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Maj fraîchement enfoncée → dash **pressé** (front).
 * }
 */
TEST(PlayerInputMapperTest, MajDeclencheLeDash) {
    EXPECT_TRUE(hmi::toPlayerInput(withKeys({hmi::Key::Shift})).dashPressed);
    EXPECT_FALSE(hmi::toPlayerInput(hmi::InputState{}).dashPressed);
}

/**
 * @brief Visée verticale (y vers le bas) : Bas → +1, Haut → -1, les deux → 0.
 * \castest{<b>Visée verticale (y vers le bas) : Bas → +1, Haut → -1, les deux → 0.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Visée verticale (y vers le bas) : Bas → +1, Haut → -1, les deux → 0.
 * }
 */
TEST(PlayerInputMapperTest, ViseeVerticale) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Down})).moveY, 1.0f);
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Up})).moveY, -1.0f);
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Up, hmi::Key::Down})).moveY, 0.0f);
}

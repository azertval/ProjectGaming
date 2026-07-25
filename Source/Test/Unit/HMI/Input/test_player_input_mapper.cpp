/**
 * @file test_player_input_mapper.cpp
 * @brief Tests unitaires de la traduction clavier → intention (`toPlayerInput`).
 */

#include <gtest/gtest.h>

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/GameKeyBindings.h"
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

// Traduit avec les bindings par défaut : la plupart des tests ci-dessous ne portent pas sur le
// remappage lui-même (couvert par les tests dédiés en fin de fichier), seulement sur la
// traduction touche -> intention.
core::PlayerInput mapWithDefaults(const hmi::InputState& input) {
    return hmi::toPlayerInput(input, hmi::GameKeyBindings{});
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
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Left})).moveX, -1.0f);
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
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Right})).moveX, 1.0f);
}

/**
 * @brief Sans remap, `Q`/`D`/`W` ne déclenchent plus rien (aucun alias fixe depuis `LOT-29`).
 * \castest{<b>Sans remap, Q/D/W ne déclenchent plus rien (aucun alias fixe).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans remap, Q/D/W ne déclenchent plus rien.
 * }
 */
TEST(PlayerInputMapperTest, SansRemapQDWNeDeclenchentRien) {
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Q})).moveX, 0.0f);
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::D})).moveX, 0.0f);
    EXPECT_FALSE(mapWithDefaults(withKeys({hmi::Key::W})).jumpPressed);
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
    EXPECT_FLOAT_EQ(mapWithDefaults(hmi::InputState{}).moveX, 0.0f);
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
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Left, hmi::Key::Right})).moveX, 0.0f);
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
    const core::PlayerInput input = mapWithDefaults(withKeys({hmi::Key::Space}));
    EXPECT_TRUE(input.jumpPressed);
    EXPECT_TRUE(input.jumpHeld);
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

    const core::PlayerInput mapped = mapWithDefaults(input);
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
    const core::PlayerInput input = mapWithDefaults(hmi::InputState{});
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
    const core::PlayerInput input = mapWithDefaults(withKeys({hmi::Key::Right, hmi::Key::Space}));
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
    EXPECT_TRUE(mapWithDefaults(withKeys({hmi::Key::Shift})).dashPressed);
    EXPECT_FALSE(mapWithDefaults(hmi::InputState{}).dashPressed);
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
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Down})).moveY, 1.0f);
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Up})).moveY, -1.0f);
    EXPECT_FLOAT_EQ(mapWithDefaults(withKeys({hmi::Key::Up, hmi::Key::Down})).moveY, 0.0f);
}

/**
 * @brief Une action remappée (`LOT-29`) réagit à sa nouvelle touche, en plus de la touche par
 *        défaut (jamais désactivée : filet de sécurité manette, `EX-CTRL-002`).
 * \castest{<b>Une action remappée réagit à sa nouvelle touche, en plus de la touche par défaut
 * (filet de sécurité manette).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une action remappée réagit à sa nouvelle touche ET à la touche par défaut.
 * }
 */
TEST(PlayerInputMapperTest, RemapperUneActionAjouteLaNouvelleTouche) {
    hmi::GameKeyBindings bindings;
    bindings.setKey(hmi::GameAction::Dash, hmi::Key::F1);

    EXPECT_TRUE(hmi::toPlayerInput(withKeys({hmi::Key::F1}), bindings).dashPressed);
    // Maj (touche par defaut de Dash) continue de declencher le dash apres remap : sans ce filet,
    // le bouton manette RB (qui n'alimente que Key::Shift, Window::pollGamepad) cesserait de
    // fonctionner des qu'un joueur remappe Dash au clavier (EX-CTRL-002).
    EXPECT_TRUE(hmi::toPlayerInput(withKeys({hmi::Key::Shift}), bindings).dashPressed);
}

/**
 * @brief Un bouton manette (source distincte du clavier) continue de déclencher une action même
 *        après un remappage clavier — la manette n'alimente que la touche par défaut.
 * \castest{<b>Un bouton manette continue de déclencher une action après un remappage clavier
 * (`EX-CTRL-002`).</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bouton manette (source gamepad) continue de déclencher l'action après remap clavier.
 * }
 */
TEST(PlayerInputMapperTest, BoutonManetteContinueDeFonctionnerApresRemapClavier) {
    hmi::GameKeyBindings bindings;
    bindings.setKey(hmi::GameAction::Jump, hmi::Key::F1);  // remap clavier : Sauter -> F1

    hmi::InputState input;
    input.onGamepadKeyDown(hmi::Key::Space);  // Window::pollGamepad : bouton A -> Key::Space (fixe)

    EXPECT_TRUE(hmi::toPlayerInput(input, bindings).jumpPressed);
}

/**
 * @brief Remapper Gauche et Droite chacune sur une touche arbitraire et distincte produit un
 *        mouvement dans le bon sens pour chacune, sans neutralisation croisée.
 * \castest{<b>Remapper Gauche et Droite sur des touches distinctes ne les neutralise pas l'une
 * l'autre.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Remapper Gauche sur D et Droite sur Q produit le mouvement attendu pour chacune.
 * }
 */
TEST(PlayerInputMapperTest, RemapperGaucheEtDroiteSurDesTouchesDistinctesNeLesAnnulePas) {
    // Aucune touche n'est jamais partagee par deux actions : chaque action ne repond qu'a sa
    // propre touche liee (plus sa touche par defaut si elle reste libre, voir toPlayerInput) ;
    // deux touches distinctes pour deux actions opposees ne peuvent donc jamais s'annuler.
    hmi::GameKeyBindings bindings;
    bindings.setKey(hmi::GameAction::MoveLeft, hmi::Key::D);
    bindings.setKey(hmi::GameAction::MoveRight, hmi::Key::Q);

    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::D}), bindings).moveX, -1.0f);
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Q}), bindings).moveX, 1.0f);
}

/**
 * @brief Le filet de sécurité manette ne revérifie pas la touche par défaut d'une action si une
 *        AUTRE action se l'est appropriée : sinon les deux actions se déclenchent à la fois.
 * \castest{<b>Le filet de sécurité manette ne revérifie pas la touche par défaut si une autre
 * action se l'est appropriée.</b><br/>
 * \tcat Unitaire · Player Input Mapper<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Échanger Gauche/Droite (flèches) déplace dans le bon sens, sans neutralisation.
 * }
 */
TEST(PlayerInputMapperTest, FiletDeSecuriteNIgnoreQuandToucheDefautReprise) {
    // setKey echange : Gauche <- Fleche droite (defaut de Droite), Droite <- Fleche gauche
    // (defaut de Gauche). Sans la garde isKeyClaimedByOtherAction, le filet de securite
    // reverifierait la touche par defaut de CHAQUE action (Gauche -> Fleche gauche, Droite ->
    // Fleche droite) meme si elle appartient desormais a l'autre action remappee : les deux
    // touches redeclencheraient alors les deux actions a la fois, neutralisant tout mouvement.
    hmi::GameKeyBindings bindings;
    bindings.setKey(hmi::GameAction::MoveLeft, hmi::Key::Right);

    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::Right);
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveRight), hmi::Key::Left);  // echange automatique

    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Right}), bindings).moveX, -1.0f);
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Left}), bindings).moveX, 1.0f);
}

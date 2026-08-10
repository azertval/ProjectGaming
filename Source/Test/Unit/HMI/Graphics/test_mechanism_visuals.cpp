#include "HMI/Graphics/MechanismVisuals.h"

#include <gtest/gtest.h>

namespace hmi {
namespace {

// Chaque famille de mecanisme, chaque etat : la correspondance renvoie le clip attendu (LOT-47
// TACHE-01) -- table petite et figee, donc testable exhaustivement.

/**
 * @brief La porte demande le clip « fermée » à l'état inactif et « ouverte » à l'état actif : la
 * table est petite et figée, donc vérifiée famille par famille, exhaustivement.
 * \castest{<b>La porte demande le clip fermé ou ouvert selon son état logique.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, DoorTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::Door, false), MECHANISM_CLIP_DOOR_CLOSED);
    EXPECT_EQ(mechanismTargetClip(core::TileType::Door, true), MECHANISM_CLIP_DOOR_OPEN);
}

/**
 * @brief L'interrupteur demande le clip inactif ou actif selon son état.
 * \castest{<b>L'interrupteur demande le clip inactif ou actif selon son état.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, SwitchTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::Switch, false), MECHANISM_CLIP_SWITCH_INACTIVE);
    EXPECT_EQ(mechanismTargetClip(core::TileType::Switch, true), MECHANISM_CLIP_SWITCH_ACTIVE);
}

/**
 * @brief La plaque de pression demande le clip relâché ou enfoncé selon son état.
 * \castest{<b>La plaque de pression demande le clip relâché ou enfoncé selon son état.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, PressurePlateTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::PressurePlate, false),
              MECHANISM_CLIP_PLATE_RELEASED);
    EXPECT_EQ(mechanismTargetClip(core::TileType::PressurePlate, true), MECHANISM_CLIP_PLATE_PRESSED);
}

/**
 * @brief Le danger commutable demande le clip inactif ou actif selon son état.
 * \castest{<b>Le danger commutable demande le clip inactif ou actif selon son état.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, DangerSwitchedTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerSwitched, false),
              MECHANISM_CLIP_DANGER_SWITCHED_INACTIVE);
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerSwitched, true),
              MECHANISM_CLIP_DANGER_SWITCHED_ACTIVE);
}

/**
 * @brief Le danger clignotant demande le clip inoffensif ou mortel selon la phase courante — c'est
 * ce qui rend sa fenêtre de létalité **lisible** avant de s'y engager.
 * \castest{<b>Le danger clignotant demande le clip inoffensif ou mortel selon sa phase.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, DangerBlinkTargetClips) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerBlink, false),
              MECHANISM_CLIP_DANGER_BLINK_HARMLESS);
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerBlink, true),
              MECHANISM_CLIP_DANGER_BLINK_LETHAL);
}

/**
 * @brief Le danger mobile n'a qu'un seul clip, quel que soit l'état passé : il n'a pas d'état
 * logique à refléter, et lui inventer une seconde apparence induirait le joueur en erreur.
 * \castest{<b>Le danger mobile a un seul clip, quel que soit l'état passé.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, DangerMoverHasSingleClipIrrespectiveOfActive) {
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerMover, false),
              MECHANISM_CLIP_DANGER_MOVER_IDLE);
    EXPECT_EQ(mechanismTargetClip(core::TileType::DangerMover, true),
              MECHANISM_CLIP_DANGER_MOVER_IDLE);
}

/**
 * @brief Une tuile sans état ne demande aucun clip et n'est pas reconnue comme mécanisme : c'est ce
 * qui évite d'installer une horloge d'animation sur chaque mur du niveau.
 * \castest{<b>Une tuile sans état ne demande aucun clip et n'est pas un mécanisme.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, StatelessTileProducesNoClipRequest) {
    EXPECT_FALSE(mechanismTargetClip(core::TileType::Solid, false).has_value());
    EXPECT_FALSE(mechanismTargetClip(core::TileType::Block, true).has_value());
    EXPECT_FALSE(mechanismTargetClip(core::TileType::Empty, false).has_value());
    EXPECT_FALSE(isStatefulMechanism(core::TileType::Solid));
    EXPECT_FALSE(isStatefulMechanism(core::TileType::Block));
}

/**
 * @brief Les six familles à état sont toutes reconnues comme mécanismes : le pendant positif du
 * test précédent, sans lequel le prédicat pourrait être trop restrictif sans que rien ne le montre.
 * \castest{<b>Les six familles de mécanismes à état sont reconnues.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, StatefulMechanismsAreRecognized) {
    EXPECT_TRUE(isStatefulMechanism(core::TileType::Door));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::Switch));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::PressurePlate));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::DangerSwitched));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::DangerBlink));
    EXPECT_TRUE(isStatefulMechanism(core::TileType::DangerMover));
}

/**
 * @brief La porte ne demande un clip de transition que sur un **changement** d'état — ouverture ou
 * fermeture — et rien quand l'état est stable : c'est ce qui empêche de rejouer l'ouverture en
 * boucle sur une porte qui reste ouverte.
 * \castest{<b>La porte ne demande une transition que sur un changement d'état.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Aucune autre famille ne demande de transition : interrupteur, plaque et dangers basculent
 * directement. Chercher une transition partout produirait des avertissements pour des assets
 * pourtant conformes.
 * \castest{<b>Seule la porte demande une transition ; les autres familles basculent
 * directement.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, OnlyDoorTransitionsOtherFamiliesSnapDirectly) {
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::Switch, false, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::PressurePlate, false, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::DangerSwitched, false, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::DangerBlink, false, true).has_value());
    EXPECT_FALSE(mechanismTransitionClip(core::TileType::DangerMover, false, true).has_value());
}

/**
 * @brief La liste des clips attendus par famille couvre exactement les états **et** les transitions
 * qu'utilisent les deux fonctions précédentes, et reste vide pour une tuile sans état. C'est elle
 * qui sert au contrat d'asset : une divergence ferait valider des assets incomplets.
 * \castest{<b>Les clips attendus par famille correspondent aux états et transitions réellement
 * demandés.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief En mode Texture, l'opacité de diagnostic vaut toujours 1 : le rendu habillé montre
 * l'apparence de l'asset, jamais une modulation de débogage héritée du mode Physique.
 * \castest{<b>En mode Texture, l'opacité de diagnostic vaut toujours 1.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(MechanismVisualsTest, TextureModeAlwaysFullyOpaque) {
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Texture, true, 0.25f, 1.0f), 1.0f);
    EXPECT_FLOAT_EQ(mechanismDiagnosticAlpha(RenderMode::Texture, false, 0.25f, 1.0f), 1.0f);
}

/**
 * @brief En mode Physique, la paire d'opacités fournie est utilisée telle quelle, dans un sens
 * comme dans l'autre : une porte ouverte devient plus transparente, un danger actif plus opaque —
 * deux conventions opposées servies par la même fonction, sans cas particulier codé en dur.
 * \castest{<b>En mode Physique, la paire d'opacités fournie est utilisée dans les deux
 * sens.</b><br/>
 * \tcat Unitaire · Apparence des mécanismes<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @file test_decor_placement_gesture.cpp
 * @brief Tests unitaires de la recherche du décor le plus proche d'un clic (LOT-49 TACHE-04).
 */

#include <gtest/gtest.h>

#include <cstddef>
#include <optional>
#include <vector>

#include "Core/Levels/Decor.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/DecorPlacementGesture.h"

namespace {

using core::Decor;
using core::Vector2;

}  // namespace

/**
 * @brief nearestDecorAt renvoie le rang du décor le plus proche, dans le rayon de détection.
 * \castest{<b>nearestDecorAt renvoie le decor le plus proche.</b><br/>
 * \tcat Unitaire · Decor Placement Gesture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer deux decors a des distances differentes du point clique.<br/>
 * \tattendu Le rang du decor le plus proche est renvoye.
 * }
 */
TEST(DecorPlacementGestureTest, RenvoieLeDecorLePlusProche) {
    std::vector<Decor> decors;
    decors.push_back(Decor{"far.png", Vector2{5.0f, 5.0f}});
    decors.push_back(Decor{"near.png", Vector2{0.1f, 0.0f}});

    const std::optional<std::size_t> found = hmi::nearestDecorAt(Vector2{0.0f, 0.0f}, decors);

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, 1u);
}

/**
 * @brief Aucun décor à portée (hors du rayon de détection) renvoie `std::nullopt`.
 * \castest{<b>Hors du rayon de detection, aucun decor n'est trouve.</b><br/>
 * \tcat Unitaire · Decor Placement Gesture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer un decor loin du point clique.<br/>
 * \tattendu nearestDecorAt renvoie std::nullopt.
 * }
 */
TEST(DecorPlacementGestureTest, HorsDuRayonRenvoieNullopt) {
    std::vector<Decor> decors;
    decors.push_back(Decor{"far.png", Vector2{10.0f, 10.0f}});

    const std::optional<std::size_t> found = hmi::nearestDecorAt(Vector2{0.0f, 0.0f}, decors);

    EXPECT_FALSE(found.has_value());
}

/**
 * @brief Un vecteur de décors vide renvoie toujours `std::nullopt`.
 * \castest{<b>Sans decor, nearestDecorAt renvoie toujours nullopt.</b><br/>
 * \tcat Unitaire · Decor Placement Gesture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appeler nearestDecorAt sur un vecteur vide.<br/>
 * \tattendu std::nullopt est renvoye.
 * }
 */
TEST(DecorPlacementGestureTest, SansDecorRenvoieNullopt) {
    const std::optional<std::size_t> found = hmi::nearestDecorAt(Vector2{0.0f, 0.0f}, {});

    EXPECT_FALSE(found.has_value());
}

/**
 * @brief À distance égale, le décor de rang le plus élevé (posé le plus récemment) est préféré —
 * cohérent avec l'ordre de superposition intra-couche.
 * \castest{<b>A distance egale, le rang le plus eleve est prefere.</b><br/>
 * \tcat Unitaire · Decor Placement Gesture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer deux decors a exactement la meme distance du point clique.<br/>
 * \tattendu Le rang le plus eleve (le dernier pose) est renvoye.
 * }
 */
TEST(DecorPlacementGestureTest, ADistanceEgaleLeRangLePlusEleveGagne) {
    std::vector<Decor> decors;
    decors.push_back(Decor{"first.png", Vector2{0.2f, 0.0f}});
    decors.push_back(Decor{"second.png", Vector2{-0.2f, 0.0f}});

    const std::optional<std::size_t> found = hmi::nearestDecorAt(Vector2{0.0f, 0.0f}, decors);

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, 1u);
}

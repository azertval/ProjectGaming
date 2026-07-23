/**
 * @file test_slope_geometry.cpp
 * @brief Tests unitaires de la fonction de hauteur des tuiles suivables (`slopeSurfaceHeight`,
 *        `EX-GP-003`).
 */

#include <cmath>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "Core/Physics/SlopeGeometry.h"

/**
 * @brief `SlopeUpRight` monte de gauche à droite : haut (0) au bord droit, bas (1) au bord gauche.
 * \castest{<b>SlopeUpRight monte de gauche à droite : haut au bord droit, bas au bord gauche.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `slopeSurfaceHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs attendues.<br/>
 * \tattendu Hauteur 1 au bord gauche, 0 au bord droit, 0,5 au centre.
 * }
 */
TEST(SlopeGeometryTest, SlopeUpRightMonteDeGaucheADroite) {
    const auto left = core::slopeSurfaceHeight(core::TileType::SlopeUpRight, 0.0f);
    const auto center = core::slopeSurfaceHeight(core::TileType::SlopeUpRight, 0.5f);
    const auto right = core::slopeSurfaceHeight(core::TileType::SlopeUpRight, 0.999f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_FLOAT_EQ(*left, 1.0f);
    EXPECT_FLOAT_EQ(*center, 0.5f);
    EXPECT_NEAR(*right, 0.0f, 1e-3f);
}

/**
 * @brief `SlopeUpLeft` monte de droite à gauche : haut (0) au bord gauche, bas (1) au bord droit.
 * \castest{<b>SlopeUpLeft monte de droite à gauche : haut au bord gauche, bas au bord droit.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `slopeSurfaceHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs attendues.<br/>
 * \tattendu Hauteur 0 au bord gauche, 1 au bord droit, 0,5 au centre.
 * }
 */
TEST(SlopeGeometryTest, SlopeUpLeftMonteDeDroiteAGauche) {
    const auto left = core::slopeSurfaceHeight(core::TileType::SlopeUpLeft, 0.0f);
    const auto center = core::slopeSurfaceHeight(core::TileType::SlopeUpLeft, 0.5f);
    const auto right = core::slopeSurfaceHeight(core::TileType::SlopeUpLeft, 0.999f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 0.0f, 1e-6f);
    EXPECT_FLOAT_EQ(*center, 0.5f);
    EXPECT_NEAR(*right, 1.0f, 1e-3f);
}

/**
 * @brief Un type de tuile sans surface à suivre renvoie `std::nullopt`.
 * \castest{<b>Un type de tuile sans surface à suivre renvoie nullopt.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `slopeSurfaceHeight` pour Solid, Empty, Block.<br/>2. Vérifier l'absence de
 * valeur.<br/>
 * \tattendu `std::nullopt` pour les trois types.
 * }
 */
TEST(SlopeGeometryTest, TypeNonSuivableRenvoieNullopt) {
    EXPECT_FALSE(core::slopeSurfaceHeight(core::TileType::Solid, 0.5f).has_value());
    EXPECT_FALSE(core::slopeSurfaceHeight(core::TileType::Empty, 0.5f).has_value());
    EXPECT_FALSE(core::slopeSurfaceHeight(core::TileType::Block, 0.5f).has_value());
}

/**
 * @brief `isSolid` renvoie `false` pour les deux orientations de pente.
 * \castest{<b>isSolid renvoie false pour les deux orientations de pente.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appeler `core::isSolid` sur `SlopeUpRight` et `SlopeUpLeft`.<br/>2. Vérifier
 * l'assertion.<br/>
 * \tattendu `false` pour les deux — la solidité d'une pente est gérée par le suivi de surface, pas
 * par `isSolid`.
 * }
 */
TEST(SlopeGeometryTest, PenteNestPasSolideStatiquement) {
    EXPECT_FALSE(core::isSolid(core::TileType::SlopeUpRight));
    EXPECT_FALSE(core::isSolid(core::TileType::SlopeUpLeft));
}

/**
 * @brief `isFollowableSurface` distingue les tuiles suivables des autres.
 * \castest{<b>isFollowableSurface distingue les tuiles suivables des autres.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Appeler `isFollowableSurface` sur les deux pentes et sur `Solid`.<br/>2. Vérifier
 * l'assertion.<br/>
 * \tattendu `true` pour les deux pentes, `false` pour `Solid`.
 * }
 */
TEST(SlopeGeometryTest, IsFollowableSurfaceDistingueLesTypes) {
    EXPECT_TRUE(core::isFollowableSurface(core::TileType::SlopeUpRight));
    EXPECT_TRUE(core::isFollowableSurface(core::TileType::SlopeUpLeft));
    EXPECT_FALSE(core::isFollowableSurface(core::TileType::Solid));
}

/**
 * @brief `RoundedUpRight` (quart de cercle, `EX-GP-004`) : hauteur 1 au bord gauche, 0 au bord
 * droit, ~0,134 au centre (valeur calculée à la main : `1 - sqrt(1 - 0,5²)`).
 * \castest{<b>RoundedUpRight suit un profil de quart de cercle, haut à droite.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `slopeSurfaceHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs calculées à la main.<br/>
 * \tattendu Hauteur 1 au bord gauche, 0 au bord droit, environ 0,134 au centre (courbe, pas 0,5
 * comme une pente linéaire).
 * }
 */
TEST(SlopeGeometryTest, RoundedUpRightSuitUnQuartDeCercle) {
    const auto left = core::slopeSurfaceHeight(core::TileType::RoundedUpRight, 0.0f);
    const auto center = core::slopeSurfaceHeight(core::TileType::RoundedUpRight, 0.5f);
    const auto right = core::slopeSurfaceHeight(core::TileType::RoundedUpRight, 0.999f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 1.0f, 1e-6f);
    EXPECT_NEAR(*center, 1.0f - std::sqrt(0.75f), 1e-4f);  // ~0,1339746
    EXPECT_NEAR(*right, 0.0f, 1e-3f);
}

/**
 * @brief `RoundedUpLeft` (quart de cercle, `EX-GP-004`) : symétrique de `RoundedUpRight`.
 * \castest{<b>RoundedUpLeft suit un profil de quart de cercle, haut à gauche.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `slopeSurfaceHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs calculées à la main.<br/>
 * \tattendu Hauteur 0 au bord gauche, 1 au bord droit, environ 0,134 au centre.
 * }
 */
TEST(SlopeGeometryTest, RoundedUpLeftSuitUnQuartDeCercle) {
    const auto left = core::slopeSurfaceHeight(core::TileType::RoundedUpLeft, 0.0f);
    const auto center = core::slopeSurfaceHeight(core::TileType::RoundedUpLeft, 0.5f);
    // Bord droit évalué exactement en 1,0 (et non 0,999 comme pour la pente linéaire) : la
    // tangente y est VERTICALE (dérivée infinie, cercle) — à 0,999 la hauteur ne vaut encore
    // qu'environ 0,955, loin de 1 (contrairement à une pente linéaire, à pente bornée). Seule la
    // valeur EXACTE au bord (formule pure, sans restriction de domaine) est fiable ici.
    const auto right = core::slopeSurfaceHeight(core::TileType::RoundedUpLeft, 1.0f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 0.0f, 1e-6f);
    EXPECT_NEAR(*center, 1.0f - std::sqrt(0.75f), 1e-4f);
    EXPECT_NEAR(*right, 1.0f, 1e-6f);
}

/**
 * @brief `isSolid` renvoie `false` pour les deux orientations d'arrondi (même raisonnement que les
 * pentes).
 * \castest{<b>isSolid renvoie false pour les deux orientations d'arrondi.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appeler `core::isSolid` sur `RoundedUpRight` et `RoundedUpLeft`.<br/>2. Vérifier
 * l'assertion.<br/>
 * \tattendu `false` pour les deux.
 * }
 */
TEST(SlopeGeometryTest, ArrondiNestPasSolideStatiquement) {
    EXPECT_FALSE(core::isSolid(core::TileType::RoundedUpRight));
    EXPECT_FALSE(core::isSolid(core::TileType::RoundedUpLeft));
}

/**
 * @brief `isFollowableSurface` reconnaît aussi les deux orientations d'arrondi.
 * \castest{<b>isFollowableSurface reconnaît les deux orientations d'arrondi.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Appeler `isFollowableSurface` sur `RoundedUpRight` et `RoundedUpLeft`.<br/>2. Vérifier
 * l'assertion.<br/>
 * \tattendu `true` pour les deux.
 * }
 */
TEST(SlopeGeometryTest, IsFollowableSurfaceReconnaitLesArrondis) {
    EXPECT_TRUE(core::isFollowableSurface(core::TileType::RoundedUpRight));
    EXPECT_TRUE(core::isFollowableSurface(core::TileType::RoundedUpLeft));
}

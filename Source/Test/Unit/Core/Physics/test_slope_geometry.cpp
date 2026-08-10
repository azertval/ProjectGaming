/**
 * @file test_slope_geometry.cpp
 * @brief Tests unitaires de la fonction de hauteur des tuiles suivables (`slopeSurfaceHeight`,
 *        `EX-GP-003`/`EX-GP-004`/`EX-GP-007`).
 */

#include <cmath>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "Core/Physics/SlopeGeometry.h"

/**
 * @brief `SlopeUpRight` monte de gauche à droite : haut (0) au bord droit, bas (1) au bord gauche.
 * \castest{<b>SlopeUpRight monte de gauche à droite : haut au bord droit, bas au bord
 * gauche.</b><br/> \tcat Unitaire · Slope Geometry<br/> \tcrit Majeur<br/> \tetapes 1. Évaluer
 * `slopeSurfaceHeight` aux deux bords et au centre de la case.<br/>2. Comparer aux valeurs
 * attendues.<br/> \tattendu Hauteur 1 au bord gauche, 0 au bord droit, 0,5 au centre.
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
 * \castest{<b>SlopeUpLeft monte de droite à gauche : haut au bord gauche, bas au bord
 * droit.</b><br/> \tcat Unitaire · Slope Geometry<br/> \tcrit Majeur<br/> \tetapes 1. Évaluer
 * `slopeSurfaceHeight` aux deux bords et au centre de la case.<br/>2. Comparer aux valeurs
 * attendues.<br/> \tattendu Hauteur 0 au bord gauche, 1 au bord droit, 0,5 au centre.
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
 * \tetapes 1. Appeler `isFollowableSurface` sur `RoundedUpRight` et `RoundedUpLeft`.<br/>2.
 * Vérifier l'assertion.<br/> \tattendu `true` pour les deux.
 * }
 */
TEST(SlopeGeometryTest, IsFollowableSurfaceReconnaitLesArrondis) {
    EXPECT_TRUE(core::isFollowableSurface(core::TileType::RoundedUpRight));
    EXPECT_TRUE(core::isFollowableSurface(core::TileType::RoundedUpLeft));
}

/**
 * @brief Les pentes/arrondis de **plafond** (`SlopeDownRight`/`SlopeDownLeft`/`RoundedDownRight`/
 * `RoundedDownLeft`, `EX-GP-006`) ne sont **jamais solides** pour la grille classique, comme leurs
 * équivalents de sol — leur collision est résolue par une passe de suivi dédiée
 * (`core::resolveCeilingSlopeFollow`), pas par `core::isSolid`.
 * \castest{<b>Les pentes/arrondis de plafond ne sont pas solides pour la grille classique, comme
 * leurs équivalents de sol.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appeler `core::isSolid` sur les quatre types de plafond.<br/>2. Vérifier
 * l'assertion.<br/>
 * \tattendu `false` pour les quatre — sous peine de transformer leur bord bas en mur invisible
 * pour le balayage classique.
 * }
 */
TEST(SlopeGeometryTest, PenteEtArrondiDePlafondNeSontPasSolidesStatiquement) {
    EXPECT_FALSE(core::isSolid(core::TileType::SlopeDownRight));
    EXPECT_FALSE(core::isSolid(core::TileType::SlopeDownLeft));
    EXPECT_FALSE(core::isSolid(core::TileType::RoundedDownRight));
    EXPECT_FALSE(core::isSolid(core::TileType::RoundedDownLeft));
}

/**
 * @brief Les pentes/arrondis de plafond n'offrent **jamais de déplacement latéral calé** en
 * marchant (`EX-GP-006`) : `isFollowableSurface` reste `false`. Leur **face du haut** est en
 * revanche une surface plate au sommet de la case (`slopeSurfaceHeight` y renvoie `0.0f`,
 * constant) — un personnage qui tombe **dessus** (par le dessus, pas en sautant depuis en dessous)
 * s'y pose comme sur un sol normal, sans quoi il tomberait au travers.
 * \castest{<b>Les pentes/arrondis de plafond ont une face du haut plate (0,0), sans déplacement
 * latéral calé.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appeler `isFollowableSurface` et `slopeSurfaceHeight` sur les quatre types de
 * plafond.<br/>2. Vérifier les valeurs.<br/>
 * \tattendu `isFollowableSurface` renvoie `false` ; `slopeSurfaceHeight` renvoie `0.0f` (face du
 * haut plate) pour les quatre types.
 * }
 */
TEST(SlopeGeometryTest, PenteEtArrondiDePlafondOntUneFaceDuHautPlate) {
    for (const core::TileType type :
         {core::TileType::SlopeDownRight, core::TileType::SlopeDownLeft,
          core::TileType::RoundedDownRight, core::TileType::RoundedDownLeft}) {
        EXPECT_FALSE(core::isFollowableSurface(type));
        const auto height = core::slopeSurfaceHeight(type, 0.5f);
        ASSERT_TRUE(height.has_value());
        EXPECT_FLOAT_EQ(*height, 0.0f);
    }
}

/**
 * @brief `isCeilingSlope` reconnaît exactement les quatre types de plafond, aucun autre
 * (`EX-GP-006`).
 * \castest{<b>isCeilingSlope reconnaît exactement les quatre types de plafond, aucun
 * autre.</b><br/> \tcat Unitaire · Slope Geometry<br/> \tcrit Mineur<br/> \tetapes 1. Appeler
 * `isCeilingSlope` sur les quatre types de plafond et sur `SlopeUpRight`/ `Solid`.<br/>2. Vérifier
 * l'assertion.<br/> \tattendu `true` pour les quatre types de plafond, `false` pour `SlopeUpRight`
 * et `Solid`.
 * }
 */
TEST(SlopeGeometryTest, IsCeilingSlopeReconnaitExactementLesQuatreTypes) {
    EXPECT_TRUE(core::isCeilingSlope(core::TileType::SlopeDownRight));
    EXPECT_TRUE(core::isCeilingSlope(core::TileType::SlopeDownLeft));
    EXPECT_TRUE(core::isCeilingSlope(core::TileType::RoundedDownRight));
    EXPECT_TRUE(core::isCeilingSlope(core::TileType::RoundedDownLeft));
    EXPECT_FALSE(core::isCeilingSlope(core::TileType::SlopeUpRight));
    EXPECT_FALSE(core::isCeilingSlope(core::TileType::Solid));
}

/**
 * @brief `ceilingSlopeHeight` est le miroir vertical exact de `slopeSurfaceHeight` pour les quatre
 * types de plafond (`EX-GP-006`) : `SlopeDownRight` (miroir de `SlopeUpRight`) monte en épaisseur
 * de gauche à droite.
 * \castest{<b>ceilingSlopeHeight(SlopeDownRight) est le miroir vertical exact de
 * slopeSurfaceHeight(SlopeUpRight).</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `ceilingSlopeHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs attendues.<br/>
 * \tattendu Hauteur 0 au bord gauche, 1 au bord droit, 0,5 au centre.
 * }
 */
TEST(SlopeGeometryTest, CeilingSlopeDownRightEstLeMiroirDeSlopeUpRight) {
    const auto left = core::ceilingSlopeHeight(core::TileType::SlopeDownRight, 0.0f);
    const auto center = core::ceilingSlopeHeight(core::TileType::SlopeDownRight, 0.5f);
    const auto right = core::ceilingSlopeHeight(core::TileType::SlopeDownRight, 0.999f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 0.0f, 1e-6f);
    EXPECT_FLOAT_EQ(*center, 0.5f);
    EXPECT_NEAR(*right, 1.0f, 1e-3f);
}

/**
 * @brief `ceilingSlopeHeight(SlopeDownLeft)` est le miroir vertical exact de
 * `slopeSurfaceHeight(SlopeUpLeft)`.
 * \castest{<b>ceilingSlopeHeight(SlopeDownLeft) est le miroir vertical exact de
 * slopeSurfaceHeight(SlopeUpLeft).</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `ceilingSlopeHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs attendues.<br/>
 * \tattendu Hauteur 1 au bord gauche, 0 au bord droit, 0,5 au centre.
 * }
 */
TEST(SlopeGeometryTest, CeilingSlopeDownLeftEstLeMiroirDeSlopeUpLeft) {
    const auto left = core::ceilingSlopeHeight(core::TileType::SlopeDownLeft, 0.0f);
    const auto center = core::ceilingSlopeHeight(core::TileType::SlopeDownLeft, 0.5f);
    const auto right = core::ceilingSlopeHeight(core::TileType::SlopeDownLeft, 0.999f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 1.0f, 1e-6f);
    EXPECT_FLOAT_EQ(*center, 0.5f);
    EXPECT_NEAR(*right, 0.0f, 1e-3f);
}

/**
 * @brief `ceilingSlopeHeight(RoundedDownRight)` est le miroir vertical exact de
 * `slopeSurfaceHeight(RoundedUpRight)` (quart de cercle).
 * \castest{<b>ceilingSlopeHeight(RoundedDownRight) est le miroir vertical exact de
 * slopeSurfaceHeight(RoundedUpRight).</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `ceilingSlopeHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs calculées à la main.<br/>
 * \tattendu Hauteur 0 au bord gauche, 1 au bord droit, environ 0,866 au centre (`sqrt(0,75)`).
 * }
 */
TEST(SlopeGeometryTest, CeilingRoundedDownRightEstLeMiroirDeRoundedUpRight) {
    const auto left = core::ceilingSlopeHeight(core::TileType::RoundedDownRight, 0.0f);
    const auto center = core::ceilingSlopeHeight(core::TileType::RoundedDownRight, 0.5f);
    const auto right = core::ceilingSlopeHeight(core::TileType::RoundedDownRight, 1.0f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 0.0f, 1e-6f);
    EXPECT_NEAR(*center, std::sqrt(0.75f), 1e-4f);  // ~0,8660254
    EXPECT_NEAR(*right, 1.0f, 1e-6f);
}

/**
 * @brief `ceilingSlopeHeight(RoundedDownLeft)` est le miroir vertical exact de
 * `slopeSurfaceHeight(RoundedUpLeft)`.
 * \castest{<b>ceilingSlopeHeight(RoundedDownLeft) est le miroir vertical exact de
 * slopeSurfaceHeight(RoundedUpLeft).</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `ceilingSlopeHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs calculées à la main.<br/>
 * \tattendu Hauteur 1 au bord gauche, 0 au bord droit, environ 0,866 au centre.
 * }
 */
TEST(SlopeGeometryTest, CeilingRoundedDownLeftEstLeMiroirDeRoundedUpLeft) {
    const auto left = core::ceilingSlopeHeight(core::TileType::RoundedDownLeft, 0.0f);
    const auto center = core::ceilingSlopeHeight(core::TileType::RoundedDownLeft, 0.5f);
    // Bord droit évalué exactement en 1,0 (comme slopeSurfaceHeight(RoundedUpLeft), voir son test
    // dédié) : la tangente y est VERTICALE (cercle), à 0,999 la hauteur ne vaut encore qu'environ
    // 0,045, loin de 0 — seule la valeur EXACTE au bord (formule pure) est fiable ici.
    const auto right = core::ceilingSlopeHeight(core::TileType::RoundedDownLeft, 1.0f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 1.0f, 1e-6f);
    EXPECT_NEAR(*center, std::sqrt(0.75f), 1e-4f);
    EXPECT_NEAR(*right, 0.0f, 1e-6f);
}

/**
 * @brief `ceilingSlopeHeight` renvoie `std::nullopt` pour un type sans silhouette de plafond.
 * \castest{<b>ceilingSlopeHeight renvoie nullopt pour un type sans silhouette de plafond.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Évaluer `ceilingSlopeHeight` pour `Solid` et `SlopeUpRight` (variante de sol).<br/>
 * 2. Vérifier l'absence de valeur.<br/>
 * \tattendu `std::nullopt` pour les deux.
 * }
 */
TEST(SlopeGeometryTest, CeilingSlopeHeightNulloptSiPasDePlafond) {
    EXPECT_FALSE(core::ceilingSlopeHeight(core::TileType::Solid, 0.5f).has_value());
    EXPECT_FALSE(core::ceilingSlopeHeight(core::TileType::SlopeUpRight, 0.5f).has_value());
}

/**
 * @brief `ConcaveUpRight` (quart de cercle **concave**, `EX-GP-007`) : mêmes bords que
 * `RoundedUpRight` (1 à gauche, 0 à droite), mais courbure inversée — au centre, `sqrt(0,75) ≈
 * 0,866`, bien distinct du `1 - sqrt(0,75) ≈ 0,134` de l'arrondi convexe (les deux valeurs sont
 * complémentaires, somme 1, ce qui rend une inversion accidentelle de formule facile à détecter).
 * \castest{<b>ConcaveUpRight suit un profil de quart de cercle concave, mêmes bords que
 * RoundedUpRight, courbure inversée.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `slopeSurfaceHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs calculées à la main.<br/>
 * \tattendu Hauteur 1 au bord gauche, 0 au bord droit, environ 0,866 au centre — distinct du 0,134
 * de l'arrondi convexe de même orientation.
 * }
 */
TEST(SlopeGeometryTest, ConcaveUpRightSuitUnQuartDeCercleConcave) {
    const auto left = core::slopeSurfaceHeight(core::TileType::ConcaveUpRight, 0.0f);
    const auto center = core::slopeSurfaceHeight(core::TileType::ConcaveUpRight, 0.5f);
    const auto right = core::slopeSurfaceHeight(core::TileType::ConcaveUpRight, 1.0f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 1.0f, 1e-6f);
    EXPECT_NEAR(*center, std::sqrt(0.75f), 1e-4f);  // ~0,8660254
    EXPECT_NEAR(*right, 0.0f, 1e-6f);
}

/**
 * @brief `ConcaveUpLeft` (`EX-GP-007`) : symétrique de `ConcaveUpRight`, mêmes bords que
 * `RoundedUpLeft`.
 * \castest{<b>ConcaveUpLeft suit un profil de quart de cercle concave, haut à gauche.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `slopeSurfaceHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs calculées à la main.<br/>
 * \tattendu Hauteur 0 au bord gauche, 1 au bord droit, environ 0,866 au centre.
 * }
 */
TEST(SlopeGeometryTest, ConcaveUpLeftSuitUnQuartDeCercleConcave) {
    const auto left = core::slopeSurfaceHeight(core::TileType::ConcaveUpLeft, 0.0f);
    const auto center = core::slopeSurfaceHeight(core::TileType::ConcaveUpLeft, 0.5f);
    const auto right = core::slopeSurfaceHeight(core::TileType::ConcaveUpLeft, 1.0f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 0.0f, 1e-6f);
    EXPECT_NEAR(*center, std::sqrt(0.75f), 1e-4f);
    EXPECT_NEAR(*right, 1.0f, 1e-6f);
}

/**
 * @brief `isSolid` renvoie `false` pour les deux orientations d'arrondi concave (même raisonnement
 * que l'arrondi convexe).
 * \castest{<b>isSolid renvoie false pour les deux orientations d'arrondi concave.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appeler `core::isSolid` sur `ConcaveUpRight` et `ConcaveUpLeft`.<br/>2. Vérifier
 * l'assertion.<br/>
 * \tattendu `false` pour les deux.
 * }
 */
TEST(SlopeGeometryTest, ConcaveNestPasSolideStatiquement) {
    EXPECT_FALSE(core::isSolid(core::TileType::ConcaveUpRight));
    EXPECT_FALSE(core::isSolid(core::TileType::ConcaveUpLeft));
}

/**
 * @brief `isFollowableSurface` reconnaît aussi les deux orientations d'arrondi concave.
 * \castest{<b>isFollowableSurface reconnaît les deux orientations d'arrondi concave.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Appeler `isFollowableSurface` sur `ConcaveUpRight` et `ConcaveUpLeft`.<br/>2.
 * Vérifier l'assertion.<br/> \tattendu `true` pour les deux.
 * }
 */
TEST(SlopeGeometryTest, IsFollowableSurfaceReconnaitLesConcaves) {
    EXPECT_TRUE(core::isFollowableSurface(core::TileType::ConcaveUpRight));
    EXPECT_TRUE(core::isFollowableSurface(core::TileType::ConcaveUpLeft));
}

/**
 * @brief Les arrondis concaves de **plafond** (`ConcaveDownRight`/`ConcaveDownLeft`, `EX-GP-007`)
 * ne sont **jamais solides** pour la grille classique, comme leurs équivalents de sol, et n'offrent
 * jamais de déplacement latéral calé — leur face du haut, en revanche, est plate (`0.0f`), comme
 * les autres variantes de plafond (`EX-GP-006`).
 * \castest{<b>Les arrondis concaves de plafond ne sont pas solides, n'offrent pas de déplacement
 * latéral calé, et ont une face du haut plate.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appeler `isSolid`/`isFollowableSurface`/`slopeSurfaceHeight` sur les deux types.<br/>
 * 2. Vérifier les valeurs.<br/>
 * \tattendu `isSolid` et `isFollowableSurface` renvoient `false` ; `slopeSurfaceHeight` renvoie
 * `0.0f` pour les deux.
 * }
 */
TEST(SlopeGeometryTest, ConcaveDePlafondNeSontPasSolidesEtOntUneFaceDuHautPlate) {
    for (const core::TileType type :
         {core::TileType::ConcaveDownRight, core::TileType::ConcaveDownLeft}) {
        EXPECT_FALSE(core::isSolid(type));
        EXPECT_FALSE(core::isFollowableSurface(type));
        const auto height = core::slopeSurfaceHeight(type, 0.5f);
        ASSERT_TRUE(height.has_value());
        EXPECT_FLOAT_EQ(*height, 0.0f);
    }
}

/**
 * @brief `isCeilingSlope` reconnaît aussi les deux types de plafond concave (`EX-GP-007`).
 * \castest{<b>isCeilingSlope reconnaît les deux types de plafond concave.</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Appeler `isCeilingSlope` sur `ConcaveDownRight`/`ConcaveDownLeft` et sur
 * `ConcaveUpRight`.<br/>2. Vérifier l'assertion.<br/>
 * \tattendu `true` pour les deux types de plafond, `false` pour `ConcaveUpRight` (variante de sol).
 * }
 */
TEST(SlopeGeometryTest, IsCeilingSlopeReconnaitAussiLesConcaves) {
    EXPECT_TRUE(core::isCeilingSlope(core::TileType::ConcaveDownRight));
    EXPECT_TRUE(core::isCeilingSlope(core::TileType::ConcaveDownLeft));
    EXPECT_FALSE(core::isCeilingSlope(core::TileType::ConcaveUpRight));
}

/**
 * @brief `ceilingSlopeHeight(ConcaveDownRight)` est le miroir vertical exact de
 * `slopeSurfaceHeight(ConcaveUpRight)` (`EX-GP-007`).
 * \castest{<b>ceilingSlopeHeight(ConcaveDownRight) est le miroir vertical exact de
 * slopeSurfaceHeight(ConcaveUpRight).</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `ceilingSlopeHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs calculées à la main.<br/>
 * \tattendu Hauteur 0 au bord gauche, 1 au bord droit, environ 0,134 au centre (`1 - sqrt(0,75)`).
 * }
 */
TEST(SlopeGeometryTest, CeilingConcaveDownRightEstLeMiroirDeConcaveUpRight) {
    const auto left = core::ceilingSlopeHeight(core::TileType::ConcaveDownRight, 0.0f);
    const auto center = core::ceilingSlopeHeight(core::TileType::ConcaveDownRight, 0.5f);
    const auto right = core::ceilingSlopeHeight(core::TileType::ConcaveDownRight, 1.0f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 0.0f, 1e-6f);
    EXPECT_NEAR(*center, 1.0f - std::sqrt(0.75f), 1e-4f);  // ~0,1339746
    EXPECT_NEAR(*right, 1.0f, 1e-6f);
}

/**
 * @brief `ceilingSlopeHeight(ConcaveDownLeft)` est le miroir vertical exact de
 * `slopeSurfaceHeight(ConcaveUpLeft)`.
 * \castest{<b>ceilingSlopeHeight(ConcaveDownLeft) est le miroir vertical exact de
 * slopeSurfaceHeight(ConcaveUpLeft).</b><br/>
 * \tcat Unitaire · Slope Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Évaluer `ceilingSlopeHeight` aux deux bords et au centre de la case.<br/>2. Comparer
 * aux valeurs calculées à la main.<br/>
 * \tattendu Hauteur 1 au bord gauche, 0 au bord droit, environ 0,134 au centre.
 * }
 */
TEST(SlopeGeometryTest, CeilingConcaveDownLeftEstLeMiroirDeConcaveUpLeft) {
    const auto left = core::ceilingSlopeHeight(core::TileType::ConcaveDownLeft, 0.0f);
    const auto center = core::ceilingSlopeHeight(core::TileType::ConcaveDownLeft, 0.5f);
    const auto right = core::ceilingSlopeHeight(core::TileType::ConcaveDownLeft, 1.0f);

    ASSERT_TRUE(left.has_value());
    ASSERT_TRUE(center.has_value());
    ASSERT_TRUE(right.has_value());
    EXPECT_NEAR(*left, 1.0f, 1e-6f);
    EXPECT_NEAR(*center, 1.0f - std::sqrt(0.75f), 1e-4f);
    EXPECT_NEAR(*right, 0.0f, 1e-6f);
}

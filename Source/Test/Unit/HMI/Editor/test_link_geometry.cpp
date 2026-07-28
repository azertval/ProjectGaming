#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileType.h"
#include "HMI/Editor/LinkGeometry.h"

namespace {

using core::GridPosition;
using core::TileType;

}  // namespace

// Switch/PressurePlate sont des declencheurs ; les autres types ne le sont pas.
TEST(LinkGeometry, IsTriggerTileReconnaitDeclencheurs) {
    EXPECT_TRUE(hmi::isTriggerTile(TileType::Switch));
    EXPECT_TRUE(hmi::isTriggerTile(TileType::PressurePlate));
    EXPECT_FALSE(hmi::isTriggerTile(TileType::Door));
    EXPECT_FALSE(hmi::isTriggerTile(TileType::DangerSwitched));
    EXPECT_FALSE(hmi::isTriggerTile(TileType::Solid));
}

// Door/DangerSwitched sont des cibles ; les autres types ne le sont pas.
TEST(LinkGeometry, IsLinkTargetTileReconnaitCibles) {
    EXPECT_TRUE(hmi::isLinkTargetTile(TileType::Door));
    EXPECT_TRUE(hmi::isLinkTargetTile(TileType::DangerSwitched));
    EXPECT_FALSE(hmi::isLinkTargetTile(TileType::Switch));
    EXPECT_FALSE(hmi::isLinkTargetTile(TileType::PressurePlate));
}

// linkSegment relie les centres de case (origine haut-gauche, +0.5 pour le centre).
TEST(LinkGeometry, LinkSegmentRelieLesCentresDeCase) {
    const hmi::LinkSegment segment = hmi::linkSegment(GridPosition{0, 0}, GridPosition{3, 0});
    EXPECT_FLOAT_EQ(segment.a.x, 0.5f);
    EXPECT_FLOAT_EQ(segment.a.y, 0.5f);
    EXPECT_FLOAT_EQ(segment.b.x, 3.5f);
    EXPECT_FLOAT_EQ(segment.b.y, 0.5f);
}

// Segment vertical : seule la coordonnee row varie.
TEST(LinkGeometry, LinkSegmentVertical) {
    const hmi::LinkSegment segment = hmi::linkSegment(GridPosition{2, 1}, GridPosition{2, 5});
    EXPECT_FLOAT_EQ(segment.a.x, segment.b.x);
    EXPECT_LT(segment.a.y, segment.b.y);
}

// Segment diagonal : les deux coordonnees varient, sans decalage (un seul lien -> fanCount 1).
TEST(LinkGeometry, LinkSegmentDiagonalSansDecalage) {
    const hmi::LinkSegment segment = hmi::linkSegment(GridPosition{0, 0}, GridPosition{2, 2});
    EXPECT_FLOAT_EQ(segment.a.x, 0.5f);
    EXPECT_FLOAT_EQ(segment.a.y, 0.5f);
    EXPECT_FLOAT_EQ(segment.b.x, 2.5f);
    EXPECT_FLOAT_EQ(segment.b.y, 2.5f);
}

// Plusieurs liens partageant un declencheur : base commune (extremite declencheur non decalee,
// effet d'eventail) ; seule l'extremite cible est decalee, de facon deterministe et symetrique.
TEST(LinkGeometry, LinkSegmentDecalageAntiSuperpositionBaseCommune) {
    constexpr GridPosition trigger{0, 0};
    constexpr GridPosition target{4, 0};
    const hmi::LinkSegment first = hmi::linkSegment(trigger, target, 0, 3);
    const hmi::LinkSegment second = hmi::linkSegment(trigger, target, 1, 3);
    const hmi::LinkSegment third = hmi::linkSegment(trigger, target, 2, 3);

    // Base commune (centre du declencheur) : identique quel que soit le decalage du frere.
    EXPECT_FLOAT_EQ(first.a.x, second.a.x);
    EXPECT_FLOAT_EQ(first.a.y, second.a.y);
    EXPECT_FLOAT_EQ(second.a.x, third.a.x);
    EXPECT_FLOAT_EQ(second.a.y, third.a.y);

    // Segment du milieu (index 1 sur 3) : cible non decalee (offset nul par symetrie).
    EXPECT_FLOAT_EQ(second.b.y, 0.5f);

    // Les deux extremes sont decales de part et d'autre de la cible, de la meme distance.
    EXPECT_NE(first.b.y, second.b.y);
    EXPECT_NE(third.b.y, second.b.y);
    EXPECT_FLOAT_EQ(second.b.y - first.b.y, third.b.y - second.b.y);

    // Meme appel deux fois -> meme resultat (deterministe).
    EXPECT_EQ(hmi::linkSegment(trigger, target, 0, 3).b.y, first.b.y);
}

// La pointe de fleche a ses deux ailes en retrait, symetriques par rapport a l'axe du segment.
TEST(LinkGeometry, ArrowHeadOrienteeVersLaCible) {
    const hmi::ArrowHead head =
        hmi::arrowHead(core::Vector2{0.0f, 0.0f}, core::Vector2{4.0f, 0.0f});
    // Les deux ailes reculent par rapport a la pointe (x < 4) et sont symetriques en y.
    EXPECT_LT(head.left.x, 4.0f);
    EXPECT_LT(head.right.x, 4.0f);
    EXPECT_FLOAT_EQ(head.left.y, -head.right.y);
    EXPECT_NE(head.left.y, 0.0f);
}

// Segment degenere (memes extremites) : pas de direction definie, la pointe reste sur la cible.
TEST(LinkGeometry, ArrowHeadSegmentDegenere) {
    const core::Vector2 point{1.0f, 1.0f};
    const hmi::ArrowHead head = hmi::arrowHead(point, point);
    EXPECT_FLOAT_EQ(head.left.x, point.x);
    EXPECT_FLOAT_EQ(head.left.y, point.y);
    EXPECT_FLOAT_EQ(head.right.x, point.x);
    EXPECT_FLOAT_EQ(head.right.y, point.y);
}

// buildLinkRows : ordre deterministe (mecanismes puis liens de danger), contenu fidele au
// brouillon.
TEST(LinkGeometry, BuildLinkRowsOrdreEtContenu) {
    core::LevelDraft draft = core::LevelDraft::empty("N", 6, 6);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(1, 0, TileType::Door);
    draft.paintTile(2, 0, TileType::PressurePlate);
    draft.paintTile(3, 0, TileType::DangerSwitched);
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{1, 0});
    draft.linkMechanism(GridPosition{2, 0}, GridPosition{3, 0});

    const std::vector<hmi::LinkRow> rows = hmi::buildLinkRows(draft);
    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0].kind, hmi::LinkKind::Mechanism);
    EXPECT_EQ(rows[0].trigger, (GridPosition{0, 0}));
    EXPECT_EQ(rows[0].target, (GridPosition{1, 0}));
    EXPECT_EQ(rows[1].kind, hmi::LinkKind::DangerLink);
    EXPECT_EQ(rows[1].trigger, (GridPosition{2, 0}));
    EXPECT_EQ(rows[1].target, (GridPosition{3, 0}));
}

// Brouillon sans liaison : liste vide.
TEST(LinkGeometry, BuildLinkRowsVideSansLiaison) {
    const core::LevelDraft draft = core::LevelDraft::empty("N", 4, 4);
    EXPECT_TRUE(hmi::buildLinkRows(draft).empty());
}

#include "HMI/Interface/IconGeometry.h"

#include <cmath>
#include <numbers>

namespace hmi {

namespace {

constexpr float PI = std::numbers::pi_v<float>;

[[nodiscard]] std::vector<IconPoint> circlePoints(float cx, float cy, float radius,
                                                  int segments = 16) {
    std::vector<IconPoint> points;
    points.reserve(static_cast<std::size_t>(segments));
    for (int i = 0; i < segments; ++i) {
        const float angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
        points.push_back(IconPoint{cx + radius * std::cos(angle), cy + radius * std::sin(angle)});
    }
    return points;
}

[[nodiscard]] IconStroke rectStroke(float x, float y, float width, float height, bool filled,
                                    IconColorRole color) {
    return IconStroke{{IconPoint{x, y}, IconPoint{x + width, y}, IconPoint{x + width, y + height},
                       IconPoint{x, y + height}},
                      /*closed=*/true, filled, color};
}

}  // namespace

IconGeometry iconGeometry(IconId id) {
    switch (id) {
        case IconId::ToolPaint:
            return IconGeometry{{
                IconStroke{{IconPoint{0.20f, 0.80f}, IconPoint{0.58f, 0.42f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{circlePoints(0.70f, 0.30f, 0.15f), true, true, IconColorRole::Accent},
            }};
        case IconId::ToolRectangle:
            return IconGeometry{{rectStroke(0.20f, 0.20f, 0.60f, 0.60f, false,
                                            IconColorRole::Foreground)}};
        case IconId::ToolSelection:
            return IconGeometry{{
                IconStroke{{IconPoint{0.20f, 0.35f}, IconPoint{0.20f, 0.20f}, IconPoint{0.35f, 0.20f}},
                          false, false, IconColorRole::Foreground},
                IconStroke{{IconPoint{0.65f, 0.20f}, IconPoint{0.80f, 0.20f}, IconPoint{0.80f, 0.35f}},
                          false, false, IconColorRole::Foreground},
                IconStroke{{IconPoint{0.80f, 0.65f}, IconPoint{0.80f, 0.80f}, IconPoint{0.65f, 0.80f}},
                          false, false, IconColorRole::Foreground},
                IconStroke{{IconPoint{0.35f, 0.80f}, IconPoint{0.20f, 0.80f}, IconPoint{0.20f, 0.65f}},
                          false, false, IconColorRole::Foreground},
            }};
        case IconId::ToolLink:
            return IconGeometry{{
                IconStroke{circlePoints(0.27f, 0.5f, 0.14f), true, false, IconColorRole::Foreground},
                IconStroke{{IconPoint{0.41f, 0.5f}, IconPoint{0.59f, 0.5f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{circlePoints(0.73f, 0.5f, 0.14f), true, true, IconColorRole::Accent},
            }};
        case IconId::ToolTextureAssign:
            return IconGeometry{{
                rectStroke(0.15f, 0.15f, 0.70f, 0.70f, false, IconColorRole::Foreground),
                rectStroke(0.35f, 0.35f, 0.30f, 0.30f, true, IconColorRole::Accent),
            }};
        case IconId::ToolDecor: {
            // Etoile a cinq branches : dix points alternant rayon exterieur/interieur.
            std::vector<IconPoint> star;
            star.reserve(10);
            constexpr float outerRadius = 0.40f;
            constexpr float innerRadius = 0.16f;
            for (int i = 0; i < 10; ++i) {
                const float radius = (i % 2 == 0) ? outerRadius : innerRadius;
                const float angle = -PI / 2.0f + PI * static_cast<float>(i) / 5.0f;
                star.push_back(IconPoint{0.5f + radius * std::cos(angle), 0.5f + radius * std::sin(angle)});
            }
            return IconGeometry{{IconStroke{star, true, true, IconColorRole::Accent}}};
        }
        case IconId::Save:
            return IconGeometry{{
                rectStroke(0.20f, 0.15f, 0.60f, 0.70f, false, IconColorRole::Foreground),
                rectStroke(0.35f, 0.15f, 0.30f, 0.20f, true, IconColorRole::Foreground),
                rectStroke(0.30f, 0.55f, 0.40f, 0.25f, false, IconColorRole::Foreground),
            }};
        case IconId::Playtest:
            return IconGeometry{{IconStroke{{IconPoint{0.30f, 0.20f}, IconPoint{0.30f, 0.80f},
                                            IconPoint{0.80f, 0.50f}},
                                            true, true, IconColorRole::Accent}}};
        case IconId::Undo:
            return IconGeometry{{
                IconStroke{{IconPoint{0.75f, 0.5f}, IconPoint{0.32f, 0.5f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{{IconPoint{0.32f, 0.5f}, IconPoint{0.47f, 0.35f}, IconPoint{0.47f, 0.65f}},
                          true, true, IconColorRole::Foreground},
            }};
        case IconId::Redo:
            return IconGeometry{{
                IconStroke{{IconPoint{0.25f, 0.5f}, IconPoint{0.68f, 0.5f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{{IconPoint{0.68f, 0.5f}, IconPoint{0.53f, 0.35f}, IconPoint{0.53f, 0.65f}},
                          true, true, IconColorRole::Foreground},
            }};
        case IconId::ToggleGrid:
            return IconGeometry{{
                IconStroke{{IconPoint{0.37f, 0.2f}, IconPoint{0.37f, 0.8f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{{IconPoint{0.63f, 0.2f}, IconPoint{0.63f, 0.8f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{{IconPoint{0.2f, 0.37f}, IconPoint{0.8f, 0.37f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{{IconPoint{0.2f, 0.63f}, IconPoint{0.8f, 0.63f}}, false, false,
                          IconColorRole::Foreground},
            }};
        case IconId::ResetCamera:
            return IconGeometry{{
                IconStroke{circlePoints(0.5f, 0.5f, 0.30f), true, false, IconColorRole::Foreground},
                IconStroke{circlePoints(0.5f, 0.5f, 0.07f), true, true, IconColorRole::Accent},
            }};
        case IconId::ToggleRenderMode:
            return IconGeometry{{
                rectStroke(0.15f, 0.2f, 0.35f, 0.6f, true, IconColorRole::Accent),
                rectStroke(0.50f, 0.2f, 0.35f, 0.6f, false, IconColorRole::Foreground),
            }};
        case IconId::Copy:
            return IconGeometry{{
                rectStroke(0.18f, 0.30f, 0.44f, 0.50f, false, IconColorRole::Foreground),
                rectStroke(0.38f, 0.18f, 0.44f, 0.50f, true, IconColorRole::Accent),
            }};
        case IconId::Paste:
            return IconGeometry{{
                rectStroke(0.22f, 0.26f, 0.56f, 0.58f, false, IconColorRole::Foreground),
                rectStroke(0.38f, 0.15f, 0.24f, 0.14f, true, IconColorRole::Accent),
            }};
        case IconId::Rename:
            return IconGeometry{{
                IconStroke{{IconPoint{0.20f, 0.80f}, IconPoint{0.60f, 0.40f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{{IconPoint{0.60f, 0.40f}, IconPoint{0.76f, 0.24f}, IconPoint{0.84f, 0.32f},
                           IconPoint{0.68f, 0.48f}},
                          true, true, IconColorRole::Accent},
                IconStroke{{IconPoint{0.18f, 0.82f}, IconPoint{0.30f, 0.82f}}, false, false,
                          IconColorRole::Foreground},
            }};
        case IconId::ShortcutsOverview:
            return IconGeometry{{
                rectStroke(0.16f, 0.34f, 0.16f, 0.16f, true, IconColorRole::Accent),
                rectStroke(0.42f, 0.34f, 0.16f, 0.16f, true, IconColorRole::Accent),
                rectStroke(0.68f, 0.34f, 0.16f, 0.16f, true, IconColorRole::Accent),
                rectStroke(0.16f, 0.56f, 0.68f, 0.14f, false, IconColorRole::Foreground),
            }};
    }
    return IconGeometry{};
}

}  // namespace hmi

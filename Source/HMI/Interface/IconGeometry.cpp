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
        case IconId::PixelBrush:
            // Manche vertical + touffe (cercle plein) : distinct de ToolPaint (manche diagonal,
            // cercle en haut a droite) pour rester reconnaissable meme cote a cote dans un menu.
            return IconGeometry{{
                IconStroke{{IconPoint{0.5f, 0.15f}, IconPoint{0.5f, 0.55f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{circlePoints(0.5f, 0.72f, 0.16f), true, true, IconColorRole::Accent},
            }};
        case IconId::PixelEraser: {
            const std::vector<IconPoint> body{IconPoint{0.20f, 0.55f}, IconPoint{0.45f, 0.25f},
                                              IconPoint{0.82f, 0.50f}, IconPoint{0.57f, 0.80f}};
            return IconGeometry{{
                IconStroke{body, true, true, IconColorRole::Accent},
                IconStroke{body, true, false, IconColorRole::Foreground},
            }};
        }
        case IconId::PixelFill:
            // Bec verseur (triangle) + goutte (cercle plein) : pot de peinture qui se renverse.
            return IconGeometry{{
                IconStroke{{IconPoint{0.20f, 0.30f}, IconPoint{0.60f, 0.30f}, IconPoint{0.40f, 0.65f}},
                          true, false, IconColorRole::Foreground},
                IconStroke{circlePoints(0.72f, 0.72f, 0.14f), true, true, IconColorRole::Accent},
            }};
        case IconId::PixelEyedropper:
            // Diagonale inverse de ToolPaint (cercle en bas a gauche plutot qu'en haut a droite) :
            // silhouette distincte tout en restant reconnaissable comme une pipette.
            return IconGeometry{{
                IconStroke{{IconPoint{0.75f, 0.25f}, IconPoint{0.35f, 0.65f}}, false, false,
                          IconColorRole::Foreground},
                IconStroke{circlePoints(0.25f, 0.75f, 0.12f), true, true, IconColorRole::Accent},
            }};
        case IconId::PixelOpen:
            // Dossier ouvert : base + rabat superieur.
            return IconGeometry{{
                rectStroke(0.15f, 0.35f, 0.70f, 0.45f, false, IconColorRole::Foreground),
                IconStroke{{IconPoint{0.15f, 0.35f}, IconPoint{0.25f, 0.20f}, IconPoint{0.55f, 0.20f},
                           IconPoint{0.62f, 0.35f}},
                          true, true, IconColorRole::Accent},
            }};
        case IconId::PixelCreate:
            // Feuille vierge + signe plus : "nouveau".
            return IconGeometry{{
                rectStroke(0.20f, 0.20f, 0.60f, 0.60f, false, IconColorRole::Foreground),
                IconStroke{{IconPoint{0.5f, 0.32f}, IconPoint{0.5f, 0.68f}}, false, false,
                          IconColorRole::Accent},
                IconStroke{{IconPoint{0.32f, 0.5f}, IconPoint{0.68f, 0.5f}}, false, false,
                          IconColorRole::Accent},
            }};
        case IconId::PixelSave:
            // Meme silhouette de disquette que Save (niveau) : le concept est identique, seule la
            // cible (fichier d'asset plutot que de niveau) differe -- deux barres d'outils
            // distinctes evitent toute confusion malgre l'icone partagee.
            return IconGeometry{{
                rectStroke(0.20f, 0.15f, 0.60f, 0.70f, false, IconColorRole::Foreground),
                rectStroke(0.35f, 0.15f, 0.30f, 0.20f, true, IconColorRole::Foreground),
                rectStroke(0.30f, 0.55f, 0.40f, 0.25f, false, IconColorRole::Accent),
            }};
        case IconId::PixelSaveAs:
            // Disquette reduite + pastille : "enregistrer sous" (une copie).
            return IconGeometry{{
                rectStroke(0.14f, 0.15f, 0.52f, 0.70f, false, IconColorRole::Foreground),
                rectStroke(0.24f, 0.15f, 0.26f, 0.18f, true, IconColorRole::Foreground),
                rectStroke(0.20f, 0.52f, 0.34f, 0.24f, false, IconColorRole::Foreground),
                IconStroke{circlePoints(0.78f, 0.72f, 0.15f), true, true, IconColorRole::Accent},
            }};
        case IconId::PixelSelectionTool:
            // Meme motif que ToolSelection (quatre coins en L) : le concept de selection est
            // identique, seule la cible (canevas plutot que niveau) differe.
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
        case IconId::PixelFlipHorizontal:
            // Axe vertical + deux blocs miroir (un plein, un vide) de part et d'autre.
            return IconGeometry{{
                IconStroke{{IconPoint{0.5f, 0.15f}, IconPoint{0.5f, 0.85f}}, false, false,
                          IconColorRole::Foreground},
                rectStroke(0.15f, 0.30f, 0.25f, 0.40f, true, IconColorRole::Accent),
                rectStroke(0.60f, 0.30f, 0.25f, 0.40f, false, IconColorRole::Foreground),
            }};
        case IconId::PixelFlipVertical:
            // Axe horizontal + deux blocs miroir de part et d'autre.
            return IconGeometry{{
                IconStroke{{IconPoint{0.15f, 0.5f}, IconPoint{0.85f, 0.5f}}, false, false,
                          IconColorRole::Foreground},
                rectStroke(0.30f, 0.15f, 0.40f, 0.25f, true, IconColorRole::Accent),
                rectStroke(0.30f, 0.60f, 0.40f, 0.25f, false, IconColorRole::Foreground),
            }};
        case IconId::PixelRotateClockwise:
            // Anneau ouvert (cercle a douze points, trace non ferme) + fleche a l'ouverture.
            return IconGeometry{{
                IconStroke{circlePoints(0.5f, 0.5f, 0.28f, 12), false, false, IconColorRole::Foreground},
                IconStroke{{IconPoint{0.76f, 0.30f}, IconPoint{0.90f, 0.40f}, IconPoint{0.74f, 0.46f}},
                          true, true, IconColorRole::Accent},
            }};
        case IconId::PixelRotateCounterClockwise:
            // Meme anneau, fleche du cote oppose : sens inverse.
            return IconGeometry{{
                IconStroke{circlePoints(0.5f, 0.5f, 0.28f, 12), false, false, IconColorRole::Foreground},
                IconStroke{{IconPoint{0.24f, 0.30f}, IconPoint{0.10f, 0.40f}, IconPoint{0.26f, 0.46f}},
                          true, true, IconColorRole::Accent},
            }};
    }
    return IconGeometry{};
}

}  // namespace hmi

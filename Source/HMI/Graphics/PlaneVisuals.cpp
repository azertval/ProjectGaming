// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/PlaneVisuals.h"

#include <algorithm>
#include <cstdint>

#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

// Projette la profondeur d'un plan sur le calque de rendu correspondant.
RenderLayer planeRenderLayer(core::PlaneDepth depth) noexcept {
    switch (depth) {
        case core::PlaneDepth::Behind:
            return RenderLayer::Plane;
        case core::PlaneDepth::Front:
            return RenderLayer::Foreground;
    }
    return RenderLayer::Plane;
}

// Compose les plans picturaux d'un niveau, dans l'ordre declare.
void composePlanes(ComposedScene& scene, const std::vector<core::Plane>& planes,
                   const std::vector<PlaneTexture>& textures, int levelWidth, int levelHeight,
                   RenderMode mode, const PlaneVisibility& visibility) {
    if (mode != RenderMode::Texture || levelWidth <= 0 || levelHeight <= 0) {
        return;  // mode Physique : lecture nue des collisions, aucun habillage (EX-REN-046).
    }

    const std::size_t count = (std::min)(planes.size(), textures.size());
    for (std::size_t rank = 0; rank < count; ++rank) {
        if (!visibility.visible(rank)) {
            continue;
        }
        const PlaneTexture& texture = textures[rank];
        if (texture.texture == nullptr) {
            continue;
        }
        const core::Plane& plane = planes[rank];

        SpriteQuad quad;
        quad.x = 0.0f;
        quad.y = 0.0f;
        quad.width = static_cast<float>(levelWidth);
        quad.height = static_cast<float>(levelHeight);
        // UV pleines, quelle que soit la densite : l'image EST le niveau a son echelle propre.
        quad.u0 = 0.0f;
        quad.v0 = 0.0f;
        quad.u1 = 1.0f;
        quad.v1 = 1.0f;
        // Opacite (EX-DEC-040) portee par la teinte : le pipeline multiplie deja texture x teinte,
        // aucun etat de melange supplementaire n'est necessaire.
        quad.a = std::clamp(plane.opacity, 0.0f, 1.0f);

        // `sortOrder` = rang du plan : depart le tri fin a l'interieur d'un calque quand deux plans
        // partagent la meme image, cas ou le rang de texture ne les departage pas.
        scene.addSprite(planeRenderLayer(plane.depth), texture.texture,
                        static_cast<std::int32_t>(rank), quad);
    }
}

}  // namespace hmi

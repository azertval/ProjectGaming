#include "HMI/Graphics/DecorVisuals.h"

#include "HMI/Graphics/TileAppearance.h"  // hmi::SceneTextures

namespace hmi {

// Projette une couche de decor (Core) vers son calque de rendu (HMI), EX-DEC-002 (voir en-tete).
RenderLayer decorRenderLayer(core::DecorLayer layer) noexcept {
    switch (layer) {
        case core::DecorLayer::Background:
        case core::DecorLayer::Decor:
            return RenderLayer::Decor;
        case core::DecorLayer::Foreground:
            return RenderLayer::Foreground;
    }
    return RenderLayer::Decor;
}

// Resout l'apparence d'un decor : asset designe si charge, damier de repli a sa taille sinon (voir
// en-tete -- contrairement au fond de niveau, un decor designe est toujours cense exister).
DecorAppearance resolveDecorAppearance(const DecorVisualTag& tag,
                                       const SceneTextures& textures) noexcept {
    const int index = textures.decorIndexOf(tag.assetName);
    if (index >= 0) {
        const SkinTexture& loaded = textures.decors[static_cast<std::size_t>(index)];
        return DecorAppearance{loaded.texture, loaded.width, loaded.height};
    }
    return DecorAppearance{textures.missing, textures.missingWidth, textures.missingHeight};
}

}  // namespace hmi

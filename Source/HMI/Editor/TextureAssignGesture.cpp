#include "HMI/Editor/TextureAssignGesture.h"

namespace hmi {

TextureAssignDecision resolveTextureAssignClick(
    core::GridPosition clickedCell, core::TileType clickedTileType,
    const std::optional<std::string>& existingOverride,
    const std::optional<std::string>& selectedAsset, bool rightClick) noexcept {
    if (clickedTileType == core::TileType::Empty) {
        return TextureAssignDecision{TextureAssignAction::Ignore, {}, {}};
    }

    if (rightClick) {
        if (!existingOverride) {
            return TextureAssignDecision{TextureAssignAction::Ignore, {}, {}};
        }
        return TextureAssignDecision{TextureAssignAction::Remove, clickedCell, {}};
    }

    if (!selectedAsset) {
        return TextureAssignDecision{TextureAssignAction::Ignore, {}, {}};
    }
    if (existingOverride && *existingOverride == *selectedAsset) {
        // Recliquer le meme asset deja assigne retire l'override (bascule) : evite d'exiger un
        // clic droit pour le cas le plus courant (defaire son propre geste).
        return TextureAssignDecision{TextureAssignAction::Remove, clickedCell, {}};
    }
    return TextureAssignDecision{TextureAssignAction::Assign, clickedCell, *selectedAsset};
}

}  // namespace hmi

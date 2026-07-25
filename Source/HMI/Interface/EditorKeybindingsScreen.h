#pragma once

#include <filesystem>

#include "HMI/Interface/EditorKeybindingsModel.h"
#include "HMI/Interface/IScreen.h"

namespace hmi {

class Localization;

/**
 * @file HMI/Interface/EditorKeybindingsScreen.h
 * @brief Écran du sous-menu « Touches de l'éditeur » (`EX-CTRL-012`, `LOT-29`).
 */

/**
 * @brief Écran de remappage des touches d'éditeur : dessine le sous-menu et applique ses actions.
 *
 * Même patron que `GameKeybindingsScreen` (voir sa doc) — habillage de `EditorKeybindingsModel`.
 */
class EditorKeybindingsScreen : public IScreen {
public:
    /**
     * @brief Construit l'écran.
     * @param localization Catalogue de traduction (référence conservée).
     * @param bindings     Bindings d'éditeur affichés/modifiés (référence conservée, mutable).
     * @param savePath     Chemin de persistance (`keybindings.json`).
     */
    EditorKeybindingsScreen(Localization& localization, EditorKeyBindings& bindings,
                           std::filesystem::path savePath);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    Localization& _localization;
    EditorKeybindingsModel _model;
};

}  // namespace hmi

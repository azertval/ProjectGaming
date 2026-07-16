#pragma once

#include "HMI/Interface/IScreen.h"

/**
 * @file HMI/Interface/EditorScreen.h
 * @brief Écran « Mode Edition » : placeholder « à venir » en attendant le vrai éditeur.
 */

namespace hmi {

/**
 * @brief Écran d'attente pour le Mode Edition (le vrai éditeur est un lot ultérieur).
 *
 * Affiche un texte « à venir » issu du **catalogue de traduction** (clé `editeur.a_venir`,
 * `EX-REN-032`, `EX-REN-033`) ; **Échap** revient au menu. Ne possède aucune ressource propre.
 */
class EditorScreen : public IScreen {
public:
    EditorScreen() = default;

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;
};

}  // namespace hmi

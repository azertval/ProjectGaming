#include "HMI/Interface/EditorScreen.h"

#include <string>

#include "Core/Ecs/Components/Sprite.h"  // core::Color
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/RenderContext.h"

namespace hmi {

namespace {
// Échelle du texte « à venir ».
constexpr float TEXT_SCALE = 5.0f;
// Teinte du texte « à venir ».
constexpr core::Color TEXT_COLOR{0.85f, 0.85f, 0.90f, 1.0f};
}  // namespace

// Construit l'écran éditeur (placeholder).
EditorScreen::EditorScreen() {
    HMI_LOG_TRACE("EditorScreen cree (placeholder Mode Edition)");
}

// Gère le retour au menu.
// « Basculer vers le menu » sur Échap, sinon « rester ».
ScreenTransition EditorScreen::update(const InputState& input, float /*fixedDelta*/) {
    if (input.keyPressed(Key::Escape)) {
        return ScreenTransition::switchTo(ScreenId::Menu);
    }
    return ScreenTransition::none();
}

// Dessine le texte « à venir » centré à l'écran.
void EditorScreen::render(RenderContext& context) {
    const std::string label = context.localization.text("editeur.a_venir");
    const float width = context.font.textWidth(label, TEXT_SCALE);
    const float height = context.font.lineHeight(TEXT_SCALE);
    const float x = (static_cast<float>(context.viewportWidth) - width) * 0.5f;
    const float y = (static_cast<float>(context.viewportHeight) - height) * 0.5f;

    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, context.font.textureView());
    context.font.drawText(context.spriteBatch, label, x, y, TEXT_SCALE, TEXT_COLOR);
    context.spriteBatch.end();
}

}  // namespace hmi

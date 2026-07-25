#include "HMI/Interface/GameKeybindingsScreen.h"

#include <string>
#include <utility>

#include "Core/Ecs/Components/Sprite.h"  // core::Color
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/MenuModel.h"
#include "HMI/Interface/RenderContext.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {
// Teintes reprises d'OptionsScreen (meme identite visuelle sur tous les ecrans a liste).
constexpr core::Color TITLE_COLOR{0.95f, 0.95f, 1.0f, 1.0f};
constexpr core::Color OPTION_COLOR{0.70f, 0.72f, 0.78f, 1.0f};
constexpr core::Color SELECTED_COLOR{1.0f, 0.82f, 0.20f, 1.0f};
constexpr core::Color CAPTURING_COLOR{0.95f, 0.45f, 0.35f, 1.0f};
}  // namespace

GameKeybindingsScreen::GameKeybindingsScreen(Localization& localization, GameKeyBindings& bindings,
                                            std::filesystem::path savePath)
    : _localization(localization),
      _model(localization, bindings, std::move(savePath)) {}

// Met a jour la logique du sous-menu et applique une eventuelle action.
ScreenTransition GameKeybindingsScreen::update(const InputState& input, float /*fixedDelta*/) {
    const std::optional<GameKeybindingsAction> action = _model.update(input);
    if (action) {
        switch (*action) {
            case GameKeybindingsAction::Rebound:
                HMI_LOG_INFO("Touche de jeu remappee.");
                break;
            case GameKeybindingsAction::Reset:
                HMI_LOG_INFO("Touches de jeu reinitialisees.");
                break;
            case GameKeybindingsAction::Back:
                return ScreenTransition::switchTo(ScreenId::Options);
        }
    }
    return ScreenTransition::none();
}

// Dessine le titre et les huit lignes (libelle + touche liee, celle selectionnee en evidence).
void GameKeybindingsScreen::render(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);

    context.spriteBatch.begin(projection, context.font.textureView());
    context.font.drawText(context.spriteBatch, _localization.text("keybindings.titre_jeu"),
                          MenuModel::MARGIN_X, GameKeybindingsModel::TITLE_Y,
                          GameKeybindingsModel::TITLE_SCALE, TITLE_COLOR);

    for (int index = 0; index < GameKeybindingsModel::ROW_COUNT; ++index) {
        const bool selected = index == _model.selectedIndex();
        const bool capturing = selected && _model.isCapturing();
        const core::Color color = capturing ? CAPTURING_COLOR
                                           : selected ? SELECTED_COLOR
                                                      : OPTION_COLOR;
        const float top = GameKeybindingsModel::rowTop(index);
        context.font.drawText(context.spriteBatch, _model.rowLabel(index), MenuModel::MARGIN_X,
                              top, GameKeybindingsModel::ROW_SCALE, color);
        const std::string value = _model.rowValue(index);
        if (!value.empty()) {
            context.font.drawText(context.spriteBatch, value,
                                  MenuModel::MARGIN_X + GameKeybindingsModel::VALUE_COLUMN_X, top,
                                  GameKeybindingsModel::ROW_SCALE, color);
        }
    }
    context.spriteBatch.end();
}

}  // namespace hmi

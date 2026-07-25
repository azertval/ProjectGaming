#include "HMI/Interface/OptionsScreen.h"

#include <algorithm>
#include <string>

#include "Core/Ecs/Components/Sprite.h"  // core::Color, core::AtlasRegion
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/GraphicsDevice.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/HmiLog.h"
#include "HMI/Input/InputState.h"
#include "HMI/Interface/MenuModel.h"
#include "HMI/Interface/RenderContext.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {
// Teintes reprises de MenuScreen (même identité visuelle sur tous les écrans à liste).
constexpr core::Color TITLE_COLOR{0.95f, 0.95f, 1.0f, 1.0f};
constexpr core::Color OPTION_COLOR{0.70f, 0.72f, 0.78f, 1.0f};
constexpr core::Color SELECTED_COLOR{1.0f, 0.82f, 0.20f, 1.0f};
constexpr core::Color STATUS_COLOR{0.55f, 0.58f, 0.65f, 1.0f};
}  // namespace

// Construit l'écran d'options : le modèle démarre sur l'état V-Sync réel du périphérique.
OptionsScreen::OptionsScreen(Localization& localization, GraphicsDevice& graphics)
    : _localization(localization),
      _graphics(graphics),
      _model(localization, graphics.vsyncEnabled()) {}

// Met à jour la logique des options, le bouton de langue, et applique une éventuelle action.
ScreenTransition OptionsScreen::update(const InputState& input, float /*fixedDelta*/) {
    _gamepadConnected = input.gamepadConnected();

    const std::optional<OptionsAction> action =
        _model.update(input, static_cast<float>(_viewportHeight));
    if (action) {
        switch (*action) {
            case OptionsAction::ToggleVSync: {
                const bool enabled = !_graphics.vsyncEnabled();
                _graphics.setVSyncEnabled(enabled);
                _model.setVSyncEnabled(enabled);
                HMI_LOG_INFO(std::string("V-Sync : ") + (enabled ? "active" : "desactive"));
                break;
            }
            case OptionsAction::OpenGameKeybindings:
                return ScreenTransition::switchTo(ScreenId::GameKeybindings);
            case OptionsAction::OpenEditorKeybindings:
                return ScreenTransition::switchTo(ScreenId::EditorKeybindings);
            case OptionsAction::OpenGamepadBindings:
                return ScreenTransition::switchTo(ScreenId::GamepadBindings);
            case OptionsAction::Back:
                return ScreenTransition::switchTo(ScreenId::Menu);
        }
    }

    // Bouton de langue (bas-droit), même geste que MenuScreen : recharge le catalogue.
    const LanguageSelector::Toggle toggle =
        _languageButton.update(input, _localization.activeLanguage(), _viewportWidth, _viewportHeight);
    if (toggle.requested) {
        const std::string previous = _localization.activeLanguage();
        if (_localization.loadLanguage(toggle.next)) {
            HMI_LOG_INFO("Langue changee : " + previous + " -> " + toggle.next);
        } else {
            HMI_LOG_WARNING("Echec du chargement de la langue : " + toggle.next);
        }
    }

    return ScreenTransition::none();
}

// Dessine le titre, les options (fenêtre visible + défilement si besoin), l'état de la manette et
// le bouton de langue.
void OptionsScreen::render(RenderContext& context) {
    _viewportWidth = context.viewportWidth;
    _viewportHeight = context.viewportHeight;

    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);

    const int visible = OptionsModel::visibleOptionCount(static_cast<float>(context.viewportHeight));
    const int scrollOffset = _model.scrollOffset();
    const int lastVisible = (std::min)(OptionsModel::OPTION_COUNT, scrollOffset + visible);

    // Barre de défilement (piste + curseur), uniquement si toutes les options ne tiennent pas
    // dans la fenêtre visible — même patron que EditorScreen::renderPicker (LevelPicker).
    if (OptionsModel::OPTION_COUNT > visible) {
        constexpr float SCROLLBAR_WIDTH = 10.0f;
        constexpr float SCROLLBAR_MARGIN_RIGHT = 24.0f;
        const float trackX =
            static_cast<float>(context.viewportWidth) - SCROLLBAR_MARGIN_RIGHT - SCROLLBAR_WIDTH;
        const float trackTop = MenuModel::OPTIONS_TOP;
        const float trackHeight = static_cast<float>(visible) * MenuModel::OPTION_SPACING;
        const float thumbHeight = (std::max)(
            trackHeight * static_cast<float>(visible) / static_cast<float>(OptionsModel::OPTION_COUNT),
            SCROLLBAR_WIDTH * 2.0f);
        const float maxThumbTravel = trackHeight - thumbHeight;
        const int maxOffset = OptionsModel::OPTION_COUNT - visible;
        const float thumbY =
            trackTop + (maxOffset > 0 ? maxThumbTravel * static_cast<float>(scrollOffset) /
                                            static_cast<float>(maxOffset)
                                      : 0.0f);

        const core::AtlasRegion pixel = context.atlas.tile(0, 0);
        const float atlasWidth = static_cast<float>(context.atlas.width());
        const float atlasHeight = static_cast<float>(context.atlas.height());
        auto scrollbarQuad = [&](float y, float height, core::Color tint) {
            SpriteQuad quad;
            quad.x = trackX;
            quad.y = y;
            quad.width = SCROLLBAR_WIDTH;
            quad.height = height;
            quad.u0 = static_cast<float>(pixel.x) / atlasWidth;
            quad.v0 = static_cast<float>(pixel.y) / atlasHeight;
            quad.u1 = static_cast<float>(pixel.x + pixel.width) / atlasWidth;
            quad.v1 = static_cast<float>(pixel.y + pixel.height) / atlasHeight;
            quad.r = tint.r;
            quad.g = tint.g;
            quad.b = tint.b;
            quad.a = tint.a;
            return quad;
        };

        context.spriteBatch.begin(projection, context.atlas.textureView());
        context.spriteBatch.draw(
            scrollbarQuad(trackTop, trackHeight, core::Color{1.0f, 1.0f, 1.0f, 0.12f}));
        context.spriteBatch.draw(
            scrollbarQuad(thumbY, thumbHeight, core::Color{1.0f, 1.0f, 1.0f, 0.45f}));
        context.spriteBatch.end();
    }

    context.spriteBatch.begin(projection, context.font.textureView());
    context.font.drawText(context.spriteBatch, _localization.text("options.titre"),
                          MenuModel::MARGIN_X, MenuModel::TITLE_Y, MenuModel::TITLE_SCALE,
                          TITLE_COLOR);
    for (int index = scrollOffset; index < lastVisible; ++index) {
        const core::Color color = index == _model.selectedIndex() ? SELECTED_COLOR : OPTION_COLOR;
        const float top = MenuModel::OPTIONS_TOP +
                         static_cast<float>(index - scrollOffset) * MenuModel::OPTION_SPACING;
        context.font.drawText(context.spriteBatch, _model.optionLabel(index), MenuModel::MARGIN_X,
                              top, MenuModel::OPTION_SCALE, color);
    }

    // Ligne d'état, sous la fenêtre visible des options : information seule, non navigable
    // (EX-CTRL-002) — position relative au nombre de lignes réellement affichées, jamais à
    // OPTION_COUNT (qui peut dépasser la fenêtre visible).
    const std::string gamepadStatus = _localization.text(
        _gamepadConnected ? "options.manette_connectee" : "options.manette_absente");
    const float statusY = MenuModel::OPTIONS_TOP +
                         static_cast<float>(lastVisible - scrollOffset) * MenuModel::OPTION_SPACING +
                         32.0f;
    context.font.drawText(context.spriteBatch, gamepadStatus, MenuModel::MARGIN_X, statusY, 2.5f,
                          STATUS_COLOR);
    context.spriteBatch.end();

    // Bouton de langue en bas à droite, même dessin que MenuScreen.
    const LanguageSelector::Rect button =
        LanguageSelector::rect(context.viewportWidth, context.viewportHeight);
    const core::AtlasRegion flag = context.flags.region(_localization.activeLanguage());
    const float textureWidth = static_cast<float>(context.flags.width());
    const float textureHeight = static_cast<float>(context.flags.height());

    SpriteQuad quad;
    quad.x = button.x;
    quad.y = button.y;
    quad.width = button.width;
    quad.height = button.height;
    quad.u0 = static_cast<float>(flag.x) / textureWidth;
    quad.v0 = static_cast<float>(flag.y) / textureHeight;
    quad.u1 = static_cast<float>(flag.x + flag.width) / textureWidth;
    quad.v1 = static_cast<float>(flag.y + flag.height) / textureHeight;

    context.spriteBatch.begin(projection, context.flags.textureView());
    context.spriteBatch.draw(quad);
    context.spriteBatch.end();
}

}  // namespace hmi

#include "HMI/Interface/MenuScreen.h"

#include "Core/Ecs/Components/Sprite.h"  // core::Color
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Interface/RenderContext.h"

namespace hmi {

namespace {
/// Teinte du titre.
constexpr core::Color TITLE_COLOR{0.95f, 0.95f, 1.0f, 1.0f};
/// Teinte d'une option non sélectionnée.
constexpr core::Color OPTION_COLOR{0.70f, 0.72f, 0.78f, 1.0f};
/// Teinte de l'option sélectionnée (mise en évidence).
constexpr core::Color SELECTED_COLOR{1.0f, 0.82f, 0.20f, 1.0f};
}  // namespace

/**
 * @brief Construit l'écran de menu.
 * @param localization Catalogue de traduction résolvant les libellés.
 */
MenuScreen::MenuScreen(const Localization& localization) : _model(localization) {}

/**
 * @brief Met à jour la logique du menu (déléguée au modèle).
 * @param input État des entrées de la frame.
 * @return La transition demandée par le modèle.
 */
ScreenTransition MenuScreen::update(const InputState& input, float /*fixedDelta*/) {
    return _model.update(input);
}

/**
 * @brief Dessine le titre et les options, l'option sélectionnée mise en évidence.
 * @param context Ressources de rendu partagées.
 *
 * Le texte est rendu en espace écran (pixels) : on démarre un lot avec la projection écran et
 * la texture de police, puis on dessine le titre et chaque option à la position de mise en
 * page du modèle, avec la teinte de sélection pour l'option courante.
 */
void MenuScreen::render(RenderContext& context) {
    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);
    context.spriteBatch.begin(projection, context.font.textureView());

    context.font.drawText(context.spriteBatch, _model.title(), MenuModel::MARGIN_X,
                          MenuModel::TITLE_Y, MenuModel::TITLE_SCALE, TITLE_COLOR);

    for (int index = 0; index < MenuModel::OPTION_COUNT; ++index) {
        const core::Color color = index == _model.selectedIndex() ? SELECTED_COLOR : OPTION_COLOR;
        context.font.drawText(context.spriteBatch, _model.optionLabel(index), MenuModel::MARGIN_X,
                              MenuModel::optionTop(index), MenuModel::OPTION_SCALE, color);
    }

    context.spriteBatch.end();
}

}  // namespace hmi

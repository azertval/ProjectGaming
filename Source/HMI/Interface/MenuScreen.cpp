#include "HMI/Interface/MenuScreen.h"

#include <string>

#include "Core/Ecs/Components/Sprite.h"  // core::Color, core::AtlasRegion
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/FlagIcons.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/RenderContext.h"
#include "HMI/Localization/Localization.h"

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
 * @param localization Catalogue de traduction (mutable pour la bascule de langue).
 */
MenuScreen::MenuScreen(Localization& localization)
    : _localization(localization), _model(localization) {}

/**
 * @brief Met à jour la logique du menu et le bouton de langue.
 * @param input État des entrées de la frame.
 * @return La transition demandée par le modèle de menu.
 *
 * Le clic sur le bouton de langue (bas-droit) est traité en plus de la navigation du menu :
 * il recharge le catalogue dans l'autre langue, ce qui met à jour les libellés dès la frame
 * suivante. Le bouton étant à l'opposé des options, un clic dessus ne déclenche aucune action
 * de menu.
 */
ScreenTransition MenuScreen::update(const InputState& input, float /*fixedDelta*/) {
    const ScreenTransition transition = _model.update(input);

    const LanguageSelector::Toggle toggle =
        _languageButton.update(input, _localization.activeLanguage(), _viewportWidth, _viewportHeight);
    if (toggle.requested) {
        // Bascule récupérable : si le catalogue cible est absent, la langue courante est conservée.
        const std::string previous = _localization.activeLanguage();
        if (_localization.loadLanguage(toggle.next)) {
            HMI_LOG_INFO("Langue changee : " + previous + " -> " + toggle.next);
        } else {
            HMI_LOG_WARNING("Echec du chargement de la langue : " + toggle.next);
        }
    }

    return transition;
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
    // Mémorise la surface pour le survol/clic du bouton langue à la frame suivante.
    _viewportWidth = context.viewportWidth;
    _viewportHeight = context.viewportHeight;

    const DirectX::XMFLOAT4X4 projection =
        BitmapFont::screenProjection(context.viewportWidth, context.viewportHeight);

    // Passe texte : titre et options (texture de police).
    context.spriteBatch.begin(projection, context.font.textureView());
    context.font.drawText(context.spriteBatch, _model.title(), MenuModel::MARGIN_X,
                          MenuModel::TITLE_Y, MenuModel::TITLE_SCALE, TITLE_COLOR);
    for (int index = 0; index < MenuModel::OPTION_COUNT; ++index) {
        const core::Color color = index == _model.selectedIndex() ? SELECTED_COLOR : OPTION_COLOR;
        context.font.drawText(context.spriteBatch, _model.optionLabel(index), MenuModel::MARGIN_X,
                              MenuModel::optionTop(index), MenuModel::OPTION_SCALE, color);
    }
    context.spriteBatch.end();

    // Passe drapeau : icône de la langue courante, en bas à droite (texture des drapeaux).
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

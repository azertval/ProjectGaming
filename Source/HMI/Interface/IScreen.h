#pragma once

#include "HMI/Input/InputState.h"

/**
 * @file HMI/Interface/IScreen.h
 * @brief Interface d'un écran d'application et modèle de transition entre écrans.
 */

namespace hmi {

struct RenderContext;

/**
 * @brief Écran cible de l'application.
 *
 * `Menu` est l'écran d'accueil ; `Game` lance la scène de jeu ; `Editor` l'écran d'édition ;
 * `Options` le menu de réglages (V-Sync, langue, LOT-20) ; `GameKeybindings`/`EditorKeybindings`
 * ses deux sous-menus de remappage des touches (`EX-CTRL-012`, `LOT-29`). La fabrication de
 * l'écran concret associé est déléguée (cf. `ScreenManager`), pour garder la logique de
 * navigation indépendante des écrans eux-mêmes.
 */
enum class ScreenId { Menu, Game, Editor, Options, GameKeybindings, EditorKeybindings };

/**
 * @brief Intention de transition renvoyée par un écran à l'issue de sa mise à jour.
 *
 * Un écran ne change jamais lui-même l'écran actif : il exprime une **intention** (rester,
 * basculer vers un écran, ou quitter l'application), que le `ScreenManager` applique. Ce
 * découplage rend la navigation testable et évite qu'un écran connaisse les autres.
 */
struct ScreenTransition {
    /// Nature de la transition demandée.
    enum class Kind { None, Switch, Quit };

    Kind kind = Kind::None;
    ScreenId target = ScreenId::Menu;  ///< Écran visé si `kind == Switch`.

    /// @return Une transition « rester sur l'écran courant ».
    [[nodiscard]] static ScreenTransition none() noexcept {
        return {};
    }

    /// @return Une transition « basculer vers @p id ».
    [[nodiscard]] static ScreenTransition switchTo(ScreenId id) noexcept {
        return {Kind::Switch, id};
    }

    /// @return Une transition « quitter l'application ».
    [[nodiscard]] static ScreenTransition quit() noexcept {
        return {Kind::Quit, ScreenId::Menu};
    }
};

/**
 * @brief Écran d'application : met à jour sa logique et se dessine.
 *
 * Chaque écran (menu, jeu, éditeur) implémente cette interface. `update` reçoit les entrées
 * de la frame et le pas de temps fixe, et renvoie son intention de transition. `render`
 * dessine l'écran à partir des ressources partagées du `RenderContext`. Les écrans sont
 * possédés par le `ScreenManager` (RAII) et s'ignorent mutuellement.
 */
class IScreen {
public:
    virtual ~IScreen() = default;

    /**
     * @brief Met à jour la logique de l'écran pour une frame.
     * @param input      État des entrées de la frame.
     * @param fixedDelta Pas de temps fixe de simulation, en secondes.
     * @return L'intention de transition (rester, basculer, quitter).
     */
    [[nodiscard]] virtual ScreenTransition update(const InputState& input, float fixedDelta) = 0;

    /**
     * @brief Dessine l'écran.
     * @param context Ressources de rendu partagées (lot de sprites, police, atlas, i18n…).
     */
    virtual void render(RenderContext& context) = 0;
};

}  // namespace hmi

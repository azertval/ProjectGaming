#pragma once

#include <functional>
#include <memory>

#include "HMI/Interface/IScreen.h"

/**
 * @file HMI/Interface/ScreenManager.h
 * @brief Gestionnaire d'écrans : détient l'écran courant et applique les transitions.
 */

namespace hmi {

struct RenderContext;

/**
 * @brief Détient l'**écran courant** et applique les **transitions** demandées.
 *
 * Le gestionnaire exécute l'écran actif (mise à jour puis dessin) et, selon l'intention
 * renvoyée par `IScreen::update`, conserve l'écran, le remplace par un autre (fabriqué à la
 * demande) ou signale la fermeture de l'application. La **fabrique** d'écrans est injectée :
 * le gestionnaire ne connaît donc pas les écrans concrets ni leurs dépendances de rendu, ce
 * qui le rend testable sans fenêtre ni GPU (`EX-NFR-010`). Les écrans sont possédés en RAII.
 */
class ScreenManager {
public:
    /// Fabrique un écran concret pour un `ScreenId` donné.
    using Factory = std::function<std::unique_ptr<IScreen>(ScreenId)>;

    /**
     * @brief Construit le gestionnaire et son écran de départ.
     * @param factory Fabrique d'écrans (fournie par l'assemblage de l'application).
     * @param initial Écran affiché au démarrage.
     */
    ScreenManager(Factory factory, ScreenId initial);

    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

    /**
     * @brief Met à jour l'écran courant et applique sa transition.
     * @param input      État des entrées de la frame.
     * @param fixedDelta Pas de temps fixe de simulation, en secondes.
     * @return true si l'application doit **se fermer** (transition « quitter »).
     */
    [[nodiscard]] bool update(const InputState& input, float fixedDelta);

    /**
     * @brief Dessine l'écran courant.
     * @param context Ressources de rendu partagées.
     */
    void render(RenderContext& context);

    /// @return L'écran courant (pour inspection/tests), ou nullptr après une demande de fermeture.
    [[nodiscard]] IScreen* currentScreen() const noexcept {
        return _current.get();
    }

    /// @return true si une transition « quitter » a été appliquée.
    [[nodiscard]] bool shouldQuit() const noexcept {
        return _quit;
    }

private:
    Factory _factory;
    std::unique_ptr<IScreen> _current;
    ScreenId _currentId;  ///< Identifiant de l'écran courant (pour la journalisation).
    bool _quit = false;
};

}  // namespace hmi

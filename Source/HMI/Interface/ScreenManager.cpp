#include "HMI/Interface/ScreenManager.h"

#include <utility>

namespace hmi {

/**
 * @brief Construit le gestionnaire et son écran de départ.
 * @param factory Fabrique d'écrans.
 * @param initial Écran affiché au démarrage.
 */
ScreenManager::ScreenManager(Factory factory, ScreenId initial)
    : _factory(std::move(factory)), _current(_factory(initial)) {}

/**
 * @brief Met à jour l'écran courant et applique sa transition.
 * @param input      État des entrées de la frame.
 * @param fixedDelta Pas de temps fixe de simulation, en secondes.
 * @return true si l'application doit se fermer.
 *
 * La transition est appliquée immédiatement : un basculement remplace l'écran actif (l'ancien
 * est détruit, ses ressources libérées), si bien que le dessin de la même frame concerne déjà
 * le nouvel écran. Une demande de fermeture libère l'écran courant et se propage à la boucle.
 */
bool ScreenManager::update(const InputState& input, float fixedDelta) {
    if (_quit || _current == nullptr) {
        return true;
    }

    const ScreenTransition transition = _current->update(input, fixedDelta);
    switch (transition.kind) {
        case ScreenTransition::Kind::None:
            break;
        case ScreenTransition::Kind::Switch:
            _current = _factory(transition.target);
            break;
        case ScreenTransition::Kind::Quit:
            _current.reset();
            _quit = true;
            break;
    }
    return _quit;
}

/**
 * @brief Dessine l'écran courant.
 * @param context Ressources de rendu partagées.
 */
void ScreenManager::render(RenderContext& context) {
    if (_current != nullptr) {
        _current->render(context);
    }
}

}  // namespace hmi

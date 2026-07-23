#include "HMI/Interface/ScreenManager.h"

#include <string>
#include <utility>

#include "HMI/HmiLog.h"

namespace hmi {

namespace {
// Un nom lisible pour la journalisation d'un écran.
[[nodiscard]] const char* screenName(ScreenId id) noexcept {
    switch (id) {
        case ScreenId::Menu:
            return "Menu";
        case ScreenId::Game:
            return "Jeu";
        case ScreenId::Editor:
            return "Editeur";
        case ScreenId::Options:
            return "Options";
    }
    return "Inconnu";
}
}  // namespace

// Construit le gestionnaire et son écran de départ.
ScreenManager::ScreenManager(Factory factory, ScreenId initial)
    : _factory(std::move(factory)), _current(_factory(initial)), _currentId(initial) {
    HMI_LOG_INFO(std::string("Ecran initial : ") + screenName(initial));
}

// Met à jour l'écran courant et applique sa transition.
//
// La transition est appliquée immédiatement : un basculement remplace l'écran actif (l'ancien
// est détruit, ses ressources libérées), si bien que le dessin de la même frame concerne déjà
// le nouvel écran. Une demande de fermeture libère l'écran courant et se propage à la boucle.
bool ScreenManager::update(const InputState& input, float fixedDelta) {
    if (_quit || _current == nullptr) {
        return true;
    }

    const ScreenTransition transition = _current->update(input, fixedDelta);
    switch (transition.kind) {
        case ScreenTransition::Kind::None:
            break;
        case ScreenTransition::Kind::Switch:
            HMI_LOG_INFO(std::string("Transition d'ecran : ") + screenName(_currentId) + " -> " +
                         screenName(transition.target));
            _current = _factory(transition.target);
            _currentId = transition.target;
            break;
        case ScreenTransition::Kind::Quit:
            HMI_LOG_INFO(std::string("Fermeture demandee depuis l'ecran ") + screenName(_currentId));
            _current.reset();
            _quit = true;
            break;
    }
    return _quit;
}

// Dessine l'écran courant.
void ScreenManager::render(RenderContext& context) {
    if (_current != nullptr) {
        _current->render(context);
    }
}

}  // namespace hmi

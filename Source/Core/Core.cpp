#include "Core/Core.h"

namespace core {

// Retourne la version du moteur.
// Chaîne de version sémantique (par exemple "0.1.0").
std::string Engine::version() const {
    // Version fixée en dur pour l'amorçage. Elle sera dérivée du numéro de
    // version du projet CMake (PROJECT_VERSION) lors de la mise en place du
    // versionnage, afin d'avoir une source unique de vérité.
    return "0.1.0";
}

}  // namespace core

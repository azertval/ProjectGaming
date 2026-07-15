#pragma once

#include <string>

/**
 * @file Core.h
 * @brief Interface publique de la bibliothèque Core (moteur / logique de jeu).
 */

/// Espace de noms du moteur et de la logique de jeu.
namespace core {

/**
 * @brief Fournit les informations globales du moteur.
 *
 * Classe d'amorçage servant d'exemple de convention (RAII, membres privés).
 * Elle sera remplacée par la véritable logique de jeu lors du premier lot.
 */
class Engine {
public:
    /**
     * @brief Retourne la version du moteur.
     * @return Chaîne de version sémantique (par exemple "0.1.0").
     */
    std::string version() const;
};

}  // namespace core

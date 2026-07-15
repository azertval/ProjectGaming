/**
 * @file main.cpp
 * @brief Point d'entrée de l'application (amorçage).
 *
 * Sera remplacé par l'initialisation de la fenêtre et du rendu DirectX
 * lors du lot correspondant.
 */

#include <iostream>

#include "Core/Core.h"

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main() {
    // Instancie le moteur puis affiche sa version : simple vérification que la
    // liaison entre l'exécutable (HMI) et la bibliothèque Core fonctionne.
    const core::Engine engine;
    std::cout << "ProjectGaming " << engine.version() << '\n';
    return 0;
}

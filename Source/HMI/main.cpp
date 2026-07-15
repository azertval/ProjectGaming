/**
 * @file main.cpp
 * @brief Point d'entrée : assemble fenêtre, device Direct3D 11 et boucle à pas de temps fixe.
 */

#include <chrono>
#include <cstdio>
#include <exception>

#include "Core/FixedTimestep.h"
#include "HMI/GraphicsDevice.h"
#include "HMI/Window.h"

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main() {
    try {
        hmi::Window window(L"ProjectGaming", 1280, 720);
        hmi::GraphicsDevice graphics(window.handle(), window.clientWidth(), window.clientHeight());
        core::FixedTimestep timestep;

        using Clock = std::chrono::steady_clock;
        Clock::time_point previous = Clock::now();

        while (!window.shouldClose()) {
            window.pumpMessages();

            // Répercute un éventuel redimensionnement de la fenêtre sur la swap chain.
            int resizedWidth = 0;
            int resizedHeight = 0;
            if (window.consumeResize(resizedWidth, resizedHeight)) {
                graphics.resize(resizedWidth, resizedHeight);
            }

            // Mesure du temps réel écoulé, découpé en pas fixes déterministes.
            const Clock::time_point now = Clock::now();
            const float elapsedSeconds = std::chrono::duration<float>(now - previous).count();
            previous = now;

            const int steps = timestep.advance(elapsedSeconds);
            for (int step = 0; step < steps; ++step) {
                // Mise à jour de la simulation : encore vide (arrive au LOT-02).
            }

            graphics.clear(0.10f, 0.12f, 0.16f, 1.0f);
            graphics.present();
        }
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Erreur fatale : %s\n", error.what());
        return 1;
    }
}

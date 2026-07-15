/**
 * @file main.cpp
 * @brief Point d'entrée : assemble fenêtre, device Direct3D 11 et boucle à pas de temps fixe.
 */

#include <chrono>
#include <exception>
#include <memory>
#include <string>

#include "Core/Diagnostics/ConsoleLogSink.h"
#include "Core/Time/FixedTimestep.h"
#include "HMI/Graphics/GraphicsDevice.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/Window.h"

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main() {
    // Configure le journaliseur global avec une sortie console dès le démarrage.
    core::defaultLogger().addSink(std::make_unique<core::ConsoleLogSink>());

    try {
        HMI_LOG_INFO("Demarrage de ProjectGaming");

        hmi::Window window(L"ProjectGaming", 1280, 720);
        HMI_LOG_INFO("Fenetre creee");

        hmi::GraphicsDevice graphics(window.handle(), window.clientWidth(), window.clientHeight());
        HMI_LOG_INFO("Direct3D 11 initialise");

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
                // Mise à jour de la simulation : encore vide (arrive au LOT-03).
            }

            graphics.clear(0.10f, 0.12f, 0.16f, 1.0f);
            graphics.present();
        }

        HMI_LOG_INFO("Arret propre");
        return 0;
    } catch (const std::exception& error) {
        HMI_LOG_ERROR(std::string("Erreur fatale : ") + error.what());
        return 1;
    }
}

/**
 * @file main.cpp
 * @brief Point d'entrée : assemble fenêtre, Direct3D 11, ressources partagées et écrans.
 */

#include <chrono>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>

#include "Core/Diagnostics/ConsoleLogSink.h"
#include "Core/Time/FixedTimestep.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/FlagIcons.h"
#include "HMI/Graphics/GraphicsDevice.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/EditorScreen.h"
#include "HMI/Interface/GameScreen.h"
#include "HMI/Interface/IScreen.h"
#include "HMI/Interface/MenuScreen.h"
#include "HMI/Interface/RenderContext.h"
#include "HMI/Interface/ScreenManager.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/Window.h"

namespace {

/// @return Le dossier contenant l'exécutable (pour localiser les ressources copiées à côté).
[[nodiscard]] std::filesystem::path executableDirectory() {
    wchar_t buffer[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(std::wstring(buffer, length)).parent_path();
}

}  // namespace

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
        HMI_LOG_INFO("Fenetre creee (" + std::to_string(window.clientWidth()) + "x" +
                     std::to_string(window.clientHeight()) + ")");

        hmi::GraphicsDevice graphics(window.handle(), window.clientWidth(), window.clientHeight());
        HMI_LOG_INFO("Direct3D 11 initialise");

        // Ressources de rendu partagées, communes à tous les écrans.
        hmi::SpriteBatch spriteBatch(graphics.device(), graphics.context());
        hmi::TextureAtlas atlas(graphics.device());
        hmi::BitmapFont font(graphics.device());
        hmi::FlagIcons flags(graphics.device());
        HMI_LOG_INFO("Ressources de rendu pretes (atlas, police, drapeaux)");

        // Catalogue de traduction : chargé depuis les .lang copiés à côté de l'exécutable.
        hmi::Localization localization(executableDirectory() / "Localization");
        if (localization.loadDefaultLanguage("fr")) {
            HMI_LOG_INFO(std::string("Langue chargee : ") + localization.activeLanguage());
        } else {
            // Erreur récupérable : à défaut de traductions, les clés s'afficheront telles quelles.
            HMI_LOG_WARNING("Catalogue de traduction 'fr' introuvable : affichage des cles");
        }

        // Fabrique d'écrans : construit l'écran concret associé à un identifiant.
        hmi::ScreenManager::Factory factory =
            [&](hmi::ScreenId id) -> std::unique_ptr<hmi::IScreen> {
            switch (id) {
                case hmi::ScreenId::Menu:
                    return std::make_unique<hmi::MenuScreen>(localization);
                case hmi::ScreenId::Game:
                    return std::make_unique<hmi::GameScreen>(spriteBatch, atlas, window.clientWidth(),
                                                             window.clientHeight());
                case hmi::ScreenId::Editor:
                    return std::make_unique<hmi::EditorScreen>();
            }
            return nullptr;
        };

        hmi::ScreenManager screens(std::move(factory), hmi::ScreenId::Menu);
        HMI_LOG_INFO("Demarrage de la boucle principale");

        const core::FixedTimestep timestep;
        const float fixedDelta = timestep.fixedDeltaSeconds();

        while (!window.shouldClose()) {
            // 1. Entrées : échantillonnées une fois par frame (nouvelle frame + messages).
            window.pumpMessages();

            // 2. Répercute un éventuel redimensionnement sur la swap chain (les écrans ajustent
            //    leur caméra au rendu à partir des dimensions du contexte).
            int resizedWidth = 0;
            int resizedHeight = 0;
            if (window.consumeResize(resizedWidth, resizedHeight)) {
                graphics.resize(resizedWidth, resizedHeight);
                HMI_LOG_TRACE("Redimensionnement : " + std::to_string(resizedWidth) + "x" +
                              std::to_string(resizedHeight));
            }

            // 3. Met à jour l'écran courant ; une demande de fermeture arrête la boucle.
            if (screens.update(window.input(), fixedDelta)) {
                break;
            }

            // 4. Rendu : efface, dessine l'écran courant, présente.
            graphics.clear(0.10f, 0.12f, 0.16f, 1.0f);
            hmi::RenderContext context{spriteBatch,  atlas,
                                       font,          localization,
                                       flags,         window.clientWidth(),
                                       window.clientHeight()};
            screens.render(context);
            graphics.present();
        }

        HMI_LOG_INFO("Arret propre");
        return 0;
    } catch (const std::exception& error) {
        HMI_LOG_ERROR(std::string("Erreur fatale : ") + error.what());
        return 1;
    }
}

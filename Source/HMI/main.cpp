/**
 * @file main.cpp
 * @brief Point d'entrée : assemble fenêtre, Direct3D 11, ressources partagées et écrans.
 */

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "Core/Diagnostics/ConsoleLogSink.h"
#include "Core/Diagnostics/LogLevelParse.h"
#include "Core/Diagnostics/Logger.h"
#include "Core/Diagnostics/MemoryLogSink.h"
#include "Core/Time/FixedTimestep.h"
#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/FlagIcons.h"
#include "HMI/Graphics/GraphicsDevice.h"
#include "HMI/Graphics/SaveIcon.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/EditorScreen.h"
#include "HMI/Interface/GameScreen.h"
#include "HMI/Interface/IScreen.h"
#include "HMI/Interface/MenuScreen.h"
#include "HMI/Interface/RenderContext.h"
#include "HMI/Interface/ScreenManager.h"
#include "HMI/Interface/SessionLog.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/Window.h"

namespace {

/// @return Le dossier contenant l'exécutable (pour localiser les ressources copiées à côté).
[[nodiscard]] std::filesystem::path executableDirectory() {
    wchar_t buffer[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(std::wstring(buffer, length)).parent_path();
}

/// @return La valeur d'une variable d'environnement, ou `std::nullopt` si absente.
[[nodiscard]] std::optional<std::string> environmentVariable(const char* name) {
    std::size_t length = 0;
    if (getenv_s(&length, nullptr, 0, name) != 0 || length == 0) {
        return std::nullopt;
    }
    std::string value(length, '\0');
    if (getenv_s(&length, value.data(), length, name) != 0) {
        return std::nullopt;
    }
    if (!value.empty() && value.back() == '\0') {
        value.pop_back();  // getenv_s inclut le terminateur nul dans la longueur
    }
    return value;
}

/// @return La valeur de l'argument `--log-level=…` s'il est présent sur la ligne de commande.
[[nodiscard]] std::optional<std::string> commandLineLogLevel(int argc, char** argv) {
    constexpr std::string_view prefix = "--log-level=";
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument = argv[index];
        if (argument.substr(0, prefix.size()) == prefix) {
            return std::string(argument.substr(prefix.size()));
        }
    }
    return std::nullopt;
}

/**
 * @brief Détermine le niveau de log minimum au lancement.
 *
 * Par défaut `Trace`. La variable d'environnement `PROJECTGAMING_LOG_LEVEL` puis, avec priorité,
 * l'argument `--log-level=<trace|info|warning|error>` peuvent l'ajuster. Une valeur non reconnue
 * est ignorée (signalée via @p invalidValueGiven).
 */
[[nodiscard]] core::LogLevel resolveMinimumLogLevel(int argc, char** argv,
                                                    bool& invalidValueGiven) {
    core::LogLevel level = core::LogLevel::Trace;
    invalidValueGiven = false;

    if (const std::optional<std::string> fromEnvironment =
            environmentVariable("PROJECTGAMING_LOG_LEVEL")) {
        if (const std::optional<core::LogLevel> parsed = core::parseLogLevel(*fromEnvironment)) {
            level = *parsed;
        } else {
            invalidValueGiven = true;
        }
    }
    if (const std::optional<std::string> fromCommandLine = commandLineLogLevel(argc, argv)) {
        if (const std::optional<core::LogLevel> parsed = core::parseLogLevel(*fromCommandLine)) {
            level = *parsed;
        } else {
            invalidValueGiven = true;
        }
    }
    return level;
}

/// @return Un horodatage `AAAAMMJJ_HHMMSS` pour nommer un fichier de session.
[[nodiscard]] std::string timestampForFilename() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &local);
    return buffer;
}

}  // namespace

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    // Journaliseur global : sortie console + capture en mémoire (pour enregistrer les logs).
    auto memoryLogSink = std::make_unique<core::MemoryLogSink>();
    core::MemoryLogSink* sessionLog = memoryLogSink.get();
    core::defaultLogger().addSink(std::make_unique<core::ConsoleLogSink>());
    core::defaultLogger().addSink(std::move(memoryLogSink));

    // Niveau de log configurable au lancement (env PROJECTGAMING_LOG_LEVEL ou --log-level=).
    bool invalidLogLevel = false;
    const core::LogLevel logLevel = resolveMinimumLogLevel(argc, argv, invalidLogLevel);
    core::defaultLogger().setMinimumLevel(logLevel);

    try {
        HMI_LOG_INFO(std::string("Demarrage de ProjectGaming (niveau de log : ") +
                     core::toString(logLevel) + ")");
        if (invalidLogLevel) {
            HMI_LOG_WARNING("Niveau de log fourni invalide : valeur ignoree");
        }

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
        hmi::SaveIcon saveIcon(graphics.device());
        HMI_LOG_INFO("Ressources de rendu pretes (atlas, police, drapeaux, icones)");

        // Catalogue de traduction : chargé depuis les .lang copiés à côté de l'exécutable.
        hmi::Localization localization(executableDirectory() / "Localization");
        if (localization.loadDefaultLanguage("fr")) {
            HMI_LOG_INFO(std::string("Langue chargee : ") + localization.activeLanguage());
        } else {
            // Erreur récupérable : à défaut de traductions, les clés s'afficheront telles quelles.
            HMI_LOG_WARNING("Catalogue de traduction 'fr' introuvable : affichage des cles");
        }

        // Action d'enregistrement des logs de la session (déclenchée par le bouton du menu).
        const std::filesystem::path logDirectory = executableDirectory() / "logs";
        const hmi::MenuScreen::SaveLogAction saveLogAction = [sessionLog, logDirectory] {
            const std::filesystem::path path =
                logDirectory / ("session_" + timestampForFilename() + ".log");
            if (hmi::saveSessionLog(sessionLog->entries(), path)) {
                HMI_LOG_INFO("Logs de session enregistres : " + path.string());
            } else {
                HMI_LOG_WARNING("Echec de l'enregistrement des logs : " + path.string());
            }
        };

        // Fabrique d'écrans : construit l'écran concret associé à un identifiant.
        hmi::ScreenManager::Factory factory =
            [&](hmi::ScreenId id) -> std::unique_ptr<hmi::IScreen> {
            switch (id) {
                case hmi::ScreenId::Menu:
                    return std::make_unique<hmi::MenuScreen>(localization, saveLogAction);
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
            hmi::RenderContext context{spriteBatch, atlas,
                                       font,        localization,
                                       flags,       saveIcon,
                                       window.clientWidth(), window.clientHeight()};
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

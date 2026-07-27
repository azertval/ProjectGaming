/**
 * @file HMI/main.cpp
 * @brief Point d'entrée de l'application Qt (`ProjectGaming`).
 *
 * Configure la journalisation (console + mémoire en développement, niveau réglable), applique le
 * thème de l'IHM, puis ouvre la fenêtre principale (`hmi::MainWindow`) dont le widget central est le
 * viewport Direct3D 11 (`hmi::GameViewport`).
 */

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QString>

#include "Core/BuildConfig.h"
#include "Core/Diagnostics/ConsoleLogSink.h"
#include "Core/Diagnostics/LogLevel.h"
#include "Core/Diagnostics/LogLevelParse.h"
#include "Core/Diagnostics/Logger.h"
#include "Core/Diagnostics/MemoryLogSink.h"
#include "HMI/HmiLog.h"
#include "HMI/Interface/MainWindow.h"

namespace {

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

}  // namespace

/**
 * @brief Point d'entrée du programme.
 * @return Code de sortie du processus (0 en cas de succès).
 */
int main(int argc, char** argv) {
    // Journaliseur. En développement : sortie console et capture en mémoire (`MemoryLogSink`,
    // collectée par `Core` — cf. le guide de journalisation). En Release : aucun sink (l'exécutable
    // n'a pas de console et une croissance mémoire serait inutile).
    core::MemoryLogSink* sessionLog = nullptr;
    if constexpr (core::kDeveloperBuild) {
        core::defaultLogger().addSink(std::make_unique<core::ConsoleLogSink>());
        auto memorySink = std::make_unique<core::MemoryLogSink>();
        sessionLog = memorySink.get();
        core::defaultLogger().addSink(std::move(memorySink));
    }

    // Niveau de log configurable au lancement (env PROJECTGAMING_LOG_LEVEL ou --log-level=).
    bool invalidLogLevel = false;
    const core::LogLevel logLevel = resolveMinimumLogLevel(argc, argv, invalidLogLevel);
    core::defaultLogger().setMinimumLevel(logLevel);
    HMI_LOG_INFO(std::string("Demarrage de ProjectGaming (niveau de log : ")
                 + core::toString(logLevel) + ").");
    if (invalidLogLevel) {
        HMI_LOG_WARNING("Niveau de log fourni invalide : valeur ignoree.");
    }

    QApplication application(argc, argv);
    // Identité de l'application : sert de portée aux réglages persistés (QSettings — disposition
    // des panneaux de l'éditeur, EX-IHM-011).
    QCoreApplication::setOrganizationName(QStringLiteral("ProjectGaming"));
    QCoreApplication::setApplicationName(QStringLiteral("Editor"));

    // Thème de l'IHM (menu/options), embarqué en ressource (resources.qrc -> theme.qss). Portée par
    // objectName : l'éditeur (docks) conserve le thème Qt par défaut.
    if (QFile themeFile(QStringLiteral(":/resources/theme.qss"));
        themeFile.open(QFile::ReadOnly | QFile::Text)) {
        application.setStyleSheet(QString::fromUtf8(themeFile.readAll()));
    } else {
        HMI_LOG_WARNING("Theme d'interface introuvable (:/resources/theme.qss) : style par defaut.");
    }

    hmi::MainWindow window(sessionLog);
    window.show();

    const int code = application.exec();
    HMI_LOG_INFO("Arret de ProjectGaming (code " + std::to_string(code) + ").");
    return code;
}

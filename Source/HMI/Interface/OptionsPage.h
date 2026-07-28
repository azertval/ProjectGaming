#pragma once

#include <filesystem>
#include <memory>

#include <QWidget>

/**
 * @file HMI/Interface/OptionsPage.h
 * @brief Écran Options à onglets, intégré à la fenêtre (LOT-38). Layout dans `OptionsPage.ui`.
 */

namespace Ui {
class OptionsPage;
}

namespace hmi {

class GameViewport;
class Localization;
class KeybindingsWidget;
class GamepadBindingsWidget;

/**
 * @brief Page Options **unique**, affichée dans la fenêtre principale (pas en pop-up), au style du
 *        menu principal (thème `theme.qss`).
 *
 * Mise en page dans `OptionsPage.ui` (éditable dans Qt Designer) : titre + `QTabWidget` avec les
 * onglets **Vidéo** et **Audio** (contenu statique). Les onglets **Commande clavier** et **Commande
 * manette** (contenu généré) sont ajoutés en code (`EX-IHM-040`). N'émet que des intentions non
 * triviales (retour, plein écran) ; les réglages simples agissent directement sur le viewport.
 */
class OptionsPage : public QWidget {
    Q_OBJECT

public:
    OptionsPage(GameViewport* viewport, std::filesystem::path keybindingsPath,
                QWidget* parent = nullptr);
    ~OptionsPage() override;

    /// Applique la langue active à tous les libellés (onglets, champs, boutons).
    void retranslateUi(const Localization& loc);

signals:
    void backRequested();
    void fullscreenRequested(bool enabled);
    /// Émis quand l'utilisateur choisit une langue (« fr »/« en »).
    void languageChanged(const QString& code);
    /// Émis quand l'utilisateur demande l'enregistrement des journaux de session.
    void saveLogsRequested();

private:
    std::unique_ptr<Ui::OptionsPage> _ui;
    KeybindingsWidget* _keyboard = nullptr;  ///< Onglet remappage clavier (contenu dynamique).
    GamepadBindingsWidget* _gamepad = nullptr;  ///< Onglet remappage manette (contenu dynamique).
    int _keyboardTabIndex = -1;
    int _gamepadTabIndex = -1;
};

}  // namespace hmi

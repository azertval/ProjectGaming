#pragma once

#include <filesystem>
#include <memory>

#include <QWidget>

/**
 * @file Editor/OptionsPage.h
 * @brief Écran Options à onglets, intégré à la fenêtre (LOT-38). Layout dans `OptionsPage.ui`.
 */

namespace Ui {
class OptionsPage;
}

namespace editor {

class GameViewport;

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

signals:
    void backRequested();
    void fullscreenRequested(bool enabled);

private:
    std::unique_ptr<Ui::OptionsPage> _ui;
};

}  // namespace editor

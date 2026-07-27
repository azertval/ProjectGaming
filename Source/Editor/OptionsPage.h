#pragma once

#include <filesystem>

#include <QWidget>

/**
 * @file Editor/OptionsPage.h
 * @brief Écran Options à onglets (Vidéo/Audio/Clavier/Manette), intégré à la fenêtre (LOT-38).
 */

namespace editor {

class GameViewport;

/**
 * @brief Page Options **unique**, affichée dans la fenêtre principale (pas en pop-up), au style du
 *        menu principal.
 *
 * Regroupe les réglages en **onglets** (`EX-IHM-040`) : **Vidéo** (V-Sync, plein écran…), **Audio**
 * (à venir), **Commande clavier** (remappage des touches de jeu, `EX-CTRL-012`) et **Commande
 * manette** (à venir). N'émet que des intentions non triviales (retour, plein écran) ; les réglages
 * simples agissent directement sur le viewport.
 */
class OptionsPage : public QWidget {
    Q_OBJECT

public:
    OptionsPage(GameViewport* viewport, std::filesystem::path keybindingsPath,
                QWidget* parent = nullptr);

signals:
    void backRequested();
    void fullscreenRequested(bool enabled);
};

}  // namespace editor

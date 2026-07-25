#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Interface/MenuModel.h"

/**
 * @file HMI/Interface/GameKeybindingsModel.h
 * @brief Logique (testable) du sous-menu « Touches de jeu » : liste, capture, réinitialisation.
 */

namespace hmi {

class Localization;
class InputState;

/// Action produite par la confirmation d'une ligne (hors capture elle-même) — voir `OptionsAction`
/// pour le même principe : le modèle ne connaît que le stockage (`GameKeyBindings`), pas l'écran.
enum class GameKeybindingsAction {
    Rebound,  ///< Une action vient d'être liée à une nouvelle touche (capture terminée).
    Reset,    ///< « Réinitialiser » confirmé : les six actions sont revenues à leurs défauts.
    Back,     ///< « Retour » confirmé : transition vers Options.
};

/**
 * @brief Logique du sous-menu « Touches de jeu », indépendante du dessin.
 *
 * Huit lignes navigables (six actions de jeu + « Réinitialiser » + « Retour »), même patron que
 * `OptionsModel`/`MenuModel` (flèches + Entrée, survol + clic), réutilisant `MenuModel::MARGIN_X`
 * pour l'alignement. Sélectionner puis confirmer une ligne d'action entre en **capture** : la
 * frame suivante qui voit une touche assignable pressée (`capturedKey`, `Échap`/`Entrée` exclus)
 * la lie via `GameKeyBindings::setKey` (échange automatique en cas de conflit) et sauvegarde
 * immédiatement ; `Échap` pendant la capture l'annule sans effet (même convention que
 * `TextInputField`).
 */
class GameKeybindingsModel {
public:
    /// Nombre de lignes : six actions de jeu + « Réinitialiser » + « Retour ».
    static constexpr int ROW_COUNT = GAME_ACTION_COUNT + 2;
    static constexpr int RESET_ROW = GAME_ACTION_COUNT;
    static constexpr int BACK_ROW = GAME_ACTION_COUNT + 1;

    // Mise en page en pixels, plus compacte que MenuModel (davantage de lignes) : partagée avec
    // EditorKeybindingsModel (11 lignes), qui les réutilise telles quelles.
    static constexpr float TITLE_Y = 40.0f;
    static constexpr float TITLE_SCALE = 4.0f;
    static constexpr float ROWS_TOP = 110.0f;
    static constexpr float ROW_SPACING = 40.0f;
    static constexpr float ROW_SCALE = 2.4f;
    static constexpr float VALUE_COLUMN_X = 420.0f;  ///< Abscisse de la touche, à droite du libellé.
    static constexpr float ROW_CLICK_WIDTH = 600.0f;  ///< Largeur cliquable d'une ligne (label+valeur).

    /// @return L'ordonnée (haut) de la ligne @p index, en pixels.
    [[nodiscard]] static constexpr float rowTop(int index) noexcept {
        return ROWS_TOP + static_cast<float>(index) * ROW_SPACING;
    }

    /// @return La hauteur d'une ligne, en pixels.
    [[nodiscard]] static constexpr float rowHeight() noexcept {
        return static_cast<float>(BitmapFont::CELL_HEIGHT) * ROW_SCALE;
    }

    /**
     * @brief Construit le modèle.
     * @param localization Catalogue de traduction résolvant les libellés (référence conservée).
     * @param bindings     Bindings à afficher/modifier (référence conservée, mutable).
     * @param savePath     Chemin du fichier de persistance (`keybindings.json`), utilisé à chaque
     *                     remap/réinitialisation.
     */
    GameKeybindingsModel(const Localization& localization, GameKeyBindings& bindings,
                        std::filesystem::path savePath);

    /**
     * @brief Met à jour la sélection/capture selon les entrées.
     * @param input État des entrées de la frame.
     * @return L'action produite (liaison terminée, réinitialisation, ou retour), sinon vide.
     */
    [[nodiscard]] std::optional<GameKeybindingsAction> update(const InputState& input);

    /// @return L'indice de la ligne sélectionnée (0 à ROW_COUNT-1).
    [[nodiscard]] int selectedIndex() const noexcept {
        return _selected;
    }

    /// @return `true` si une capture de touche est en cours (ligne @ref selectedIndex).
    [[nodiscard]] bool isCapturing() const noexcept {
        return _capturing;
    }

    /// @return Le libellé de la ligne @p index (résolu par le catalogue).
    [[nodiscard]] std::string rowLabel(int index) const;

    /// @return La touche liée à la ligne @p index (nom affichable), vide pour Réinitialiser/Retour,
    ///         ou l'invite de capture si @p index est en cours de capture.
    [[nodiscard]] std::string rowValue(int index) const;

private:
    /// @return L'indice de la ligne dont le rectangle contient (@p x, @p y), ou -1.
    [[nodiscard]] int rowAtPoint(int x, int y) const;

    /// @return L'action de jeu associée à la ligne @p index (0 à GAME_ACTION_COUNT-1 uniquement).
    [[nodiscard]] static GameAction actionForRow(int index) noexcept;

    const Localization& _localization;
    GameKeyBindings& _bindings;
    std::filesystem::path _savePath;
    int _selected = 0;
    bool _capturing = false;
};

}  // namespace hmi

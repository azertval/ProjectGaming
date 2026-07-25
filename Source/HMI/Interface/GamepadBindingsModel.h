#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "HMI/Input/GamepadBindings.h"
#include "HMI/Interface/GameKeybindingsModel.h"

/**
 * @file HMI/Interface/GamepadBindingsModel.h
 * @brief Logique (testable) du sous-menu « Touches de la manette » : liste, capture,
 *        réinitialisation.
 */

namespace hmi {

class Localization;
class InputState;

/// Même principe que `GameKeybindingsAction` (voir sa doc), pour le sous-menu manette.
enum class GamepadBindingsAction {
    Rebound,
    Reset,
    Back,
};

/**
 * @brief Logique du sous-menu « Touches de la manette », indépendante du dessin.
 *
 * Huit lignes navigables (six actions de jeu + « Réinitialiser » + « Retour »), même mécanique
 * que `GameKeybindingsModel` (voir sa doc pour le protocole de capture), sur `GamepadBindings` au
 * lieu de `GameKeyBindings` — `capturedGamepadButton` remplace `capturedKey`. Réutilise telles
 * quelles les constantes de mise en page de `GameKeybindingsModel`. Confirmer une ligne d'action
 * n'entre en capture que si une manette est connectée (`InputState::gamepadConnected()`) — sinon
 * l'écran affiche une invite dédiée plutôt que d'attendre indéfiniment une capture impossible.
 */
class GamepadBindingsModel {
public:
    /// Nombre de lignes : six actions de jeu + « Réinitialiser » + « Retour ».
    static constexpr int ROW_COUNT = GAME_ACTION_COUNT + 2;
    static constexpr int RESET_ROW = GAME_ACTION_COUNT;
    static constexpr int BACK_ROW = GAME_ACTION_COUNT + 1;

    /**
     * @brief Construit le modèle.
     * @param localization Catalogue de traduction résolvant les libellés (référence conservée).
     * @param bindings     Bindings à afficher/modifier (référence conservée, mutable).
     * @param savePath     Chemin du fichier de persistance (`keybindings.json`).
     */
    GamepadBindingsModel(const Localization& localization, GamepadBindings& bindings,
                        std::filesystem::path savePath);

    /**
     * @brief Met à jour la sélection/capture selon les entrées.
     * @param input État des entrées de la frame.
     * @return L'action produite (liaison terminée, réinitialisation, ou retour), sinon vide.
     */
    [[nodiscard]] std::optional<GamepadBindingsAction> update(const InputState& input);

    /// @return L'indice de la ligne sélectionnée (0 à ROW_COUNT-1).
    [[nodiscard]] int selectedIndex() const noexcept {
        return _selected;
    }

    /// @return `true` si une capture de bouton est en cours (ligne @ref selectedIndex).
    [[nodiscard]] bool isCapturing() const noexcept {
        return _capturing;
    }

    /// @return Le libellé de la ligne @p index (résolu par le catalogue).
    [[nodiscard]] std::string rowLabel(int index) const;

    /// @return Le bouton lié à la ligne @p index (nom affichable), vide pour Réinitialiser/Retour,
    ///         ou l'invite de capture si @p index est en cours de capture.
    [[nodiscard]] std::string rowValue(int index) const;

private:
    /// @return L'indice de la ligne dont le rectangle contient (@p x, @p y), ou -1.
    [[nodiscard]] int rowAtPoint(int x, int y) const;

    /// @return L'action de jeu associée à la ligne @p index (0 à GAME_ACTION_COUNT-1 uniquement).
    [[nodiscard]] static GameAction actionForRow(int index) noexcept;

    const Localization& _localization;
    GamepadBindings& _bindings;
    std::filesystem::path _savePath;
    int _selected = 0;
    bool _capturing = false;
};

}  // namespace hmi

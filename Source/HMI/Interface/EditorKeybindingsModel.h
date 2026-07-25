#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "HMI/Input/EditorKeyBindings.h"
#include "HMI/Interface/GameKeybindingsModel.h"

/**
 * @file HMI/Interface/EditorKeybindingsModel.h
 * @brief Logique (testable) du sous-menu « Touches de l'éditeur » : liste, capture,
 *        réinitialisation.
 */

namespace hmi {

class Localization;
class InputState;

/// Même principe que `GameKeybindingsAction` (voir sa doc), pour le sous-menu éditeur.
enum class EditorKeybindingsAction {
    Rebound,
    Reset,
    Back,
};

/**
 * @brief Logique du sous-menu « Touches de l'éditeur », indépendante du dessin.
 *
 * Onze lignes navigables (neuf actions d'éditeur + « Réinitialiser » + « Retour »), même
 * mécanique que `GameKeybindingsModel` (voir sa doc pour le protocole de capture) — classe
 * séparée plutôt qu'une abstraction commune (décision de cadrage `LOT-29` : deux cas concrets
 * connus). La mise en page réutilise telles quelles les constantes de `GameKeybindingsModel`
 * (`TITLE_Y`, `ROW_SPACING`, `ROW_SCALE`, …), sur le même principe qu'`OptionsModel` réutilisant
 * `MenuModel`.
 */
class EditorKeybindingsModel {
public:
    /// Nombre de lignes : neuf actions d'éditeur + « Réinitialiser » + « Retour ».
    static constexpr int ROW_COUNT = EDITOR_ACTION_COUNT + 2;
    static constexpr int RESET_ROW = EDITOR_ACTION_COUNT;
    static constexpr int BACK_ROW = EDITOR_ACTION_COUNT + 1;

    /**
     * @brief Construit le modèle.
     * @param localization Catalogue de traduction résolvant les libellés (référence conservée).
     * @param bindings     Bindings à afficher/modifier (référence conservée, mutable).
     * @param savePath     Chemin du fichier de persistance (`keybindings.json`).
     */
    EditorKeybindingsModel(const Localization& localization, EditorKeyBindings& bindings,
                          std::filesystem::path savePath);

    /**
     * @brief Met à jour la sélection/capture selon les entrées.
     * @param input État des entrées de la frame.
     * @return L'action produite (liaison terminée, réinitialisation, ou retour), sinon vide.
     */
    [[nodiscard]] std::optional<EditorKeybindingsAction> update(const InputState& input);

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

    /// @return L'action d'éditeur associée à la ligne @p index (0 à EDITOR_ACTION_COUNT-1).
    [[nodiscard]] static EditorAction actionForRow(int index) noexcept;

    const Localization& _localization;
    EditorKeyBindings& _bindings;
    std::filesystem::path _savePath;
    int _selected = 0;
    bool _capturing = false;
};

}  // namespace hmi

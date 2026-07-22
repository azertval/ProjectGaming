#pragma once

#include <functional>
#include <string>

/**
 * @file HMI/Editor/TextInputField.h
 * @brief Champ de saisie de texte minimal, pour le nommage de niveau dans l'éditeur (LOT-15).
 */

namespace hmi {

class InputState;

/**
 * @brief Champ de saisie de texte **pur** (aucune dépendance rendu), pilotable par `InputState`.
 *
 * Consomme `InputState::typedCharacters()` (LOT-15 TACHE-01, `WM_CHAR`) et les touches
 * `Retour arrière`/`Entrée`/`Échap`. Le dessin (cadre, curseur) est délégué à l'appelant
 * (`EditorScreen`), à l'identique du principe déjà suivi par `TilePalette`/`LevelPicker` —
 * testable sans fenêtre ni GPU (`EX-NFR-010`).
 *
 * Un **validateur** optionnel conditionne la confirmation (`Entrée`) : si le texte courant ne le
 * satisfait pas, `confirmed()` reste faux et `rejected()` devient vrai (jusqu'à la prochaine
 * modification du texte) au lieu de fermer le champ — l'appelant peut alors afficher le motif du
 * refus sans perdre la saisie en cours.
 */
class TextInputField {
public:
    /// Renvoie `true` si le texte passé peut être confirmé.
    using Validator = std::function<bool(const std::string&)>;

    /**
     * @brief Construit un champ pré-rempli, avec un validateur optionnel.
     * @param initialText Texte initial (ex. le nom courant, pour un renommage).
     * @param validator   Validateur de confirmation ; absent (`nullptr`) accepte tout texte.
     */
    explicit TextInputField(std::string initialText = {}, Validator validator = nullptr);

    /**
     * @brief Consomme les entrées de la frame courante (texte tapé, retour arrière, Entrée,
     *        Échap). Sans effet une fois `confirmed()`/`cancelled()` (l'appelant doit alors
     *        cesser d'appeler `update()` et relire le résultat).
     * @param input État des entrées de la frame.
     */
    void update(const InputState& input);

    /// @return Le texte courant, tel que saisi (encodage UTF-8).
    [[nodiscard]] const std::string& text() const noexcept {
        return _text;
    }

    /// @return `true` si `Entrée` a confirmé une saisie acceptée par le validateur.
    [[nodiscard]] bool confirmed() const noexcept {
        return _confirmed;
    }

    /// @return `true` si `Échap` a annulé la saisie.
    [[nodiscard]] bool cancelled() const noexcept {
        return _cancelled;
    }

    /// @return `true` si la dernière confirmation tentée a été refusée par le validateur, et
    ///         qu'aucune modification n'a eu lieu depuis (message d'erreur toujours pertinent).
    [[nodiscard]] bool rejected() const noexcept {
        return _rejected;
    }

private:
    std::string _text;
    Validator _validator;
    bool _confirmed = false;
    bool _cancelled = false;
    bool _rejected = false;
};

}  // namespace hmi

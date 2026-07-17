#pragma once

#include <cstddef>
#include <filesystem>
#include <utility>
#include <vector>

/**
 * @file HMI/Interface/LevelSequence.h
 * @brief Séquence ordonnée de niveaux et progression courante (logique pure).
 */

namespace hmi {

/**
 * @brief Liste **ordonnée** de niveaux et position courante dans la progression (`EX-LVL-010`).
 *
 * Logique **pure** (chemins de fichiers, indice), sans rendu ni fenêtre : le `GameScreen` s'en sert
 * pour savoir quel niveau charger et, à la réussite, s'il faut **enchaîner** le suivant ou revenir
 * au titre (`EX-LVL-011`). Testable en isolation (`EX-NFR-010`). La **source** de l'ordre est
 * décidée à la construction (ici : liste explicite, ordre de difficulté maîtrisé).
 */
class LevelSequence {
public:
    /// Construit la séquence à partir d'une liste ordonnée de chemins de niveaux.
    explicit LevelSequence(std::vector<std::filesystem::path> levels)
        : _levels(std::move(levels)) {}

    /// @return true si la séquence ne contient aucun niveau.
    [[nodiscard]] bool empty() const noexcept {
        return _levels.empty();
    }

    /// @return Le nombre de niveaux de la séquence.
    [[nodiscard]] std::size_t size() const noexcept {
        return _levels.size();
    }

    /// @return L'indice (base 0) du niveau courant.
    [[nodiscard]] std::size_t index() const noexcept {
        return _index;
    }

    /**
     * @brief Chemin du niveau **courant**.
     * @pre La séquence n'est pas vide et n'est pas terminée (`!empty() && index() < size()`).
     */
    [[nodiscard]] const std::filesystem::path& current() const {
        return _levels[_index];
    }

    /// @return true s'il existe un niveau **après** le courant (enchaînement possible).
    [[nodiscard]] bool hasNext() const noexcept {
        return _index + 1 < _levels.size();
    }

    /// Passe au niveau suivant. Sans effet s'il n'y en a pas (cf. hasNext()).
    void advance() noexcept {
        if (hasNext()) {
            ++_index;
        }
    }

private:
    std::vector<std::filesystem::path> _levels;
    std::size_t _index = 0;
};

}  // namespace hmi

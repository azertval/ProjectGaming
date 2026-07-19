#pragma once

#include <vector>

#include "Core/Levels/TileType.h"

/**
 * @file HMI/Editor/TilePalette.h
 * @brief Sélection du type de tuile actif pour la peinture dans l'éditeur (EX-EDIT-002).
 */

namespace hmi {

/**
 * @brief Bande de tuiles cliquables (palette) sélectionnant le type de tuile à peindre.
 *
 * Logique **pure** : ne connaît que la géométrie écran (rectangles en pixels) et l'état de
 * sélection, sans dépendance de rendu — testable sans GPU (`EX-NFR-010`). Le dessin des entrées
 * (couleurs d'atlas, surbrillance de la sélection) est délégué à `EditorScreen`, qui lit
 * `entries()`/`selected()`.
 *
 * Types proposés : ceux que `Core` sait réellement gérer aujourd'hui (`Empty`, `Solid`, `Danger`,
 * `Entry`, `Exit`, `Switch`, `Door`) — ni bloc poussable ni clé/porte verrouillée, non implémentés
 * côté gameplay (hors périmètre du LOT-14, voir `epic.md`).
 */
class TilePalette {
public:
    /// Une entrée cliquable de la palette : son rectangle écran et le type qu'elle sélectionne.
    struct Entry {
        core::TileType type;
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    /// Construit la palette, disposée en bande horizontale dans le coin haut-gauche de l'écran.
    TilePalette();

    /// @return Les entrées de la palette, dans leur ordre d'affichage.
    [[nodiscard]] const std::vector<Entry>& entries() const noexcept {
        return _entries;
    }

    /// @return Le type de tuile actuellement sélectionné (`Solid` par défaut).
    [[nodiscard]] core::TileType selected() const noexcept {
        return _selected;
    }

    /**
     * @brief Traite un clic en position écran : sélectionne l'entrée couverte, s'il y en a une.
     * @param x Abscisse écran du clic, en pixels.
     * @param y Ordonnée écran du clic, en pixels.
     * @return `true` si le clic a touché la palette (sélection changée ou confirmée) ; `false`
     *         s'il est en dehors — l'appelant doit alors le traiter comme un clic sur la grille.
     */
    bool handleClick(float x, float y);

private:
    std::vector<Entry> _entries;
    core::TileType _selected = core::TileType::Solid;
};

}  // namespace hmi

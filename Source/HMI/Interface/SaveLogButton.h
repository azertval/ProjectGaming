#pragma once

/**
 * @file HMI/Interface/SaveLogButton.h
 * @brief Logique (testable) du bouton d'enregistrement des logs, à gauche du bouton de langue.
 */

namespace hmi {

class InputState;

/**
 * @brief Logique du bouton « enregistrer les logs » : un carré cliquable en bas à droite.
 *
 * Le bouton est ancré à **gauche du bouton de langue**. Un clic gauche à l'intérieur signale
 * une demande d'enregistrement des logs de la session. Logique pure (aucune ressource GPU),
 * testable en isolation (`EX-NFR-010`) ; le dessin de l'icône est assuré par l'écran.
 */
class SaveLogButton {
public:
    /// Côté du bouton, en pixels d'écran.
    static constexpr float SIZE = 48.0f;
    /// Écart horizontal entre ce bouton et le bouton de langue, en pixels.
    static constexpr float GAP = 20.0f;

    /// Rectangle du bouton, en pixels d'écran.
    struct Rect {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    /**
     * @brief Rectangle du bouton pour une surface de rendu donnée.
     * @param viewportWidth  Largeur de la surface, en pixels.
     * @param viewportHeight Hauteur de la surface, en pixels.
     */
    [[nodiscard]] static Rect rect(int viewportWidth, int viewportHeight) noexcept;

    /**
     * @brief Indique si le bouton a été cliqué cette frame.
     * @param input          État des entrées de la frame.
     * @param viewportWidth  Largeur de la surface de rendu, en pixels.
     * @param viewportHeight Hauteur de la surface de rendu, en pixels.
     * @return true si un clic gauche est survenu dans le rectangle du bouton.
     */
    [[nodiscard]] bool clicked(const InputState& input, int viewportWidth,
                               int viewportHeight) const;
};

}  // namespace hmi

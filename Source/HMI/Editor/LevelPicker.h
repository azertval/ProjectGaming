#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

/**
 * @file HMI/Editor/LevelPicker.h
 * @brief Sélection du niveau à éditer : nouveau brouillon ou fichier existant (LOT-14 TACHE-06).
 */

namespace hmi {

class InputState;

/**
 * @brief Liste « Nouveau niveau » + les fichiers `.json` d'un dossier, navigable au clavier et à
 *        la souris.
 *
 * Affiché avant l'entrée en édition (`EX-EDIT-001`) : un non-codeur choisit de partir d'un
 * brouillon vierge ou de reprendre un niveau déjà enregistré, sans ligne de commande. Navigation
 * clavier (`↑`/`↓` + `Entrée`) et souris (survol + clic gauche), à l'identique de `MenuModel`.
 * Le constructeur public est **pur**
 * (liste de choix déjà résolue, testable sans système de fichiers, `EX-NFR-010`) ; `forDirectory`
 * fait le pont avec le disque (usage réel, non testé unitairement — comme le reste des accès
 * fichier de `HMI`).
 */
class LevelPicker {
public:
    /// Un choix présentable : `path` absent signifie « nouveau niveau vierge ».
    struct Choice {
        std::string label;
        std::optional<std::filesystem::path> path;
    };

    // Mise en page en pixels (espace écran), partagée logique/dessin — même esprit que MenuModel.
    static constexpr float MARGIN_X = 80.0f;
    static constexpr float TITLE_Y = 90.0f;
    static constexpr float OPTIONS_TOP = 260.0f;
    static constexpr float OPTION_SPACING = 64.0f;
    static constexpr float OPTION_SCALE = 4.0f;

    /// Construit le sélecteur à partir d'une liste de choix déjà résolue (testable).
    explicit LevelPicker(std::vector<Choice> choices);

    /**
     * @brief Scanne @p levelsDirectory pour ses fichiers `.json` et construit la liste
     *        correspondante, précédée de « Nouveau niveau ».
     * @param levelsDirectory Dossier à scanner ; un dossier absent produit une liste réduite au
     *                        seul « Nouveau niveau » (pas d'erreur : cas normal au premier lancement).
     */
    [[nodiscard]] static LevelPicker forDirectory(const std::filesystem::path& levelsDirectory);

    /// @return Les choix proposés, dans leur ordre d'affichage.
    [[nodiscard]] const std::vector<Choice>& choices() const noexcept {
        return _choices;
    }

    /// @return L'indice du choix actuellement sélectionné.
    [[nodiscard]] int selected() const noexcept {
        return _selected;
    }

    /// @return L'indice du premier choix affiché (défilement, `EX-EDIT-001`) — les choix
    ///         d'indice inférieur sont défilés au-dessus du haut de la liste, invisibles.
    [[nodiscard]] int scrollOffset() const noexcept {
        return _scrollOffset;
    }

    /**
     * @brief Nombre de choix affichables simultanément sans défilement, pour une hauteur de
     *        fenêtre donnée — partagé entre la logique (défilement) et le rendu (fenêtre visible).
     * @param viewportHeight Hauteur de la surface de rendu, en pixels.
     * @return Au moins 1 (une liste trop petite pour afficher une seule ligne reste
     *         franchissable, jamais un fenêtrage vide).
     */
    [[nodiscard]] static int visibleCount(float viewportHeight);

    /**
     * @brief Met à jour la sélection (`↑`/`↓`, survol souris, molette) et détecte la confirmation
     *        (`Entrée`, ou clic gauche sur un choix survolé).
     * @param input          État des entrées de la frame.
     * @param viewportHeight Hauteur de la surface de rendu, en pixels (fenêtrage du défilement).
     * @return L'indice confirmé ce pas-ci, ou `std::nullopt` si rien n'a été validé.
     */
    [[nodiscard]] std::optional<int> update(const InputState& input, float viewportHeight);

private:
    /// @return L'indice du choix dont le rectangle **affiché** (donc dans la fenêtre visible du
    ///         défilement courant) contient (@p x, @p y), ou -1.
    [[nodiscard]] int optionAtPoint(int x, int y, int visibleCount) const;

    /// Borne `_scrollOffset` à l'intervalle valide `[0, max(0, compte - visibleCount)]`, sans
    /// autre effet — utilisé après un défilement à la molette, qui ne doit **pas** forcer la
    /// fenêtre à revenir sur la sélection courante (l'utilisateur parcourt la liste sans changer
    /// son choix).
    void clampScrollRange(int visibleCount) noexcept;

    /// Recale `_scrollOffset` pour que `_selected` reste dans la fenêtre visible (suit la
    /// sélection) — appelé seulement quand le **clavier** vient de déplacer la sélection ; un
    /// défilement à la molette seul ne doit jamais être annulé par ce recalage.
    void followSelection(int visibleCount) noexcept;

    std::vector<Choice> _choices;
    int _selected = 0;
    int _scrollOffset = 0;
};

}  // namespace hmi

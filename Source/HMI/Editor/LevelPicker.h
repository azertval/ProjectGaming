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

    /**
     * @brief Met à jour la sélection (`↑`/`↓`, survol souris) et détecte la confirmation
     *        (`Entrée`, ou clic gauche sur un choix survolé).
     * @param input État des entrées de la frame.
     * @return L'indice confirmé ce pas-ci, ou `std::nullopt` si rien n'a été validé.
     */
    [[nodiscard]] std::optional<int> update(const InputState& input);

private:
    /// @return L'indice du choix dont le rectangle contient (@p x, @p y), ou -1.
    [[nodiscard]] int optionAtPoint(int x, int y) const;

    std::vector<Choice> _choices;
    int _selected = 0;
};

}  // namespace hmi

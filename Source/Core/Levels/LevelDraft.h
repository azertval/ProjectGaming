#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

/**
 * @file Core/Levels/LevelDraft.h
 * @brief Représentation mutable d'un niveau en cours d'édition.
 */

namespace core {

/**
 * @brief Niveau **mutable** en cours d'édition, distinct de `Level` (immuable, LOT-07).
 *
 * `LevelDraft` porte toute la mutabilité nécessaire à l'éditeur (`EX-EDIT-002` à `EX-EDIT-005`) :
 * peindre une tuile, déplacer l'entrée/la sortie, lier/délier un mécanisme, redimensionner la
 * grille. Il ne duplique **aucune règle de validation** : `toLevel()` reconstruit le niveau en
 * repassant par `LevelLoader::loadFromString` (même chemin que le chargement d'un fichier),
 * garantissant que le niveau produit satisfait exactement les mêmes règles (`EX-LVL-004`,
 * `EX-EDIT-010`).
 *
 * Invariant maintenu par tous les mutateurs : la grille de tuiles reste la **source de vérité**
 * des positions d'entrée/sortie/interrupteurs/portes (comme pour `Level`) ; `entry()`/`exit()`
 * ne sont que des accès en cache, toujours synchronisés avec le contenu de `tileMap()`.
 *
 * Logique **pure**, sans dépendance rendu ni fenêtre — testable sans GPU (`EX-NFR-010`).
 */
class LevelDraft {
public:
    /**
     * @brief Crée un brouillon vierge (grille entièrement `Empty`), sans entrée ni sortie.
     * @param name   Nom du niveau.
     * @param width  Largeur de la grille, en cases (> 0).
     * @param height Hauteur de la grille, en cases (> 0).
     */
    [[nodiscard]] static LevelDraft empty(std::string name, int width, int height);

    /**
     * @brief Crée un brouillon à partir d'un niveau déjà chargé (édition d'un fichier existant).
     * @param level Niveau source.
     */
    [[nodiscard]] static LevelDraft fromLevel(const Level& level);

    /**
     * @brief Peint le type de tuile @p type en (column, row).
     *
     * Cas particuliers : peindre `Entry`/`Exit` délègue à `setEntry`/`setExit` (unicité). Peindre
     * un autre type sur une case qui portait l'entrée, la sortie, un interrupteur ou une porte
     * **efface** l'état associé (entrée/sortie invalidée, liaisons de mécanismes retirées) pour
     * ne jamais laisser d'incohérence entre la grille et ces caches.
     * @param column Colonne visée (doit être dans les bornes).
     * @param row    Ligne visée (doit être dans les bornes).
     * @param type   Type de tuile à poser.
     */
    void paintTile(int column, int row, TileType type);

    /**
     * @brief Place l'entrée en (column, row) ; déplace l'occurrence existante s'il y en avait
     *        une (unicité, `EX-EDIT-004`).
     */
    void setEntry(int column, int row);

    /**
     * @brief Place la sortie en (column, row) ; déplace l'occurrence existante s'il y en avait
     *        une (unicité, `EX-EDIT-004`).
     */
    void setExit(int column, int row);

    /**
     * @brief Lie un interrupteur à une porte (`EX-EDIT-003`).
     *
     * Remplace toute liaison existante pour @p doorPosition (une porte n'a qu'un seul
     * interrupteur associé) ; plusieurs portes peuvent en revanche partager le même interrupteur.
     * @pre La case @p switchPosition porte un `Switch`, la case @p doorPosition porte une `Door`.
     */
    void linkMechanism(GridPosition switchPosition, GridPosition doorPosition);

    /// Retire la liaison de la porte à @p doorPosition, si elle en a une. Sans effet sinon.
    void unlinkMechanism(GridPosition doorPosition);

    /**
     * @brief Redimensionne la grille (`EX-EDIT-005`).
     *
     * Agrandir complète les nouvelles cases en `Empty` ; réduire **tronque** silencieusement le
     * contenu hors des nouvelles bornes (entrée, sortie ou liaisons de mécanismes perdues sont
     * invalidées en conséquence). L'avertissement de perte revient à l'appelant `HMI`, avant
     * d'invoquer `resize`.
     * @param width  Nouvelle largeur, en cases (> 0).
     * @param height Nouvelle hauteur, en cases (> 0).
     */
    void resize(int width, int height);

    /**
     * @brief Indique si redimensionner à (@p width, @p height) supprimerait du contenu déjà posé.
     *
     * Requête **pure** (n'altère rien) : vraie si les nouvelles bornes excluraient l'entrée, la
     * sortie, ou une des deux extrémités d'une liaison de mécanisme actuellement posées. Permet à
     * `HMI` d'avertir avant d'appeler `resize` (`EX-EDIT-012`), sans dupliquer la logique de
     * troncature déjà portée par `resize`.
     * @param width  Largeur envisagée, en cases (> 0).
     * @param height Hauteur envisagée, en cases (> 0).
     * @return `true` si l'entrée, la sortie ou une liaison serait perdue.
     */
    [[nodiscard]] bool wouldResizeDropContent(int width, int height) const noexcept;

    /**
     * @brief Annule la dernière mutation (`EX-EDIT-005`).
     * @return `true` si une mutation a été annulée, `false` si l'historique était vide.
     */
    bool undo();

    /**
     * @brief Refait la dernière mutation annulée.
     *
     * Toute nouvelle mutation après un `undo()` invalide la branche de refaire (historique
     * linéaire classique) : `redo()` redevient sans effet tant qu'aucun nouvel `undo()` n'a eu
     * lieu depuis.
     * @return `true` si une mutation a été refaite, `false` si rien n'était à refaire.
     */
    bool redo();

    /// @return `true` si `undo()` aurait un effet.
    [[nodiscard]] bool canUndo() const noexcept {
        return !_undoHistory.empty();
    }

    /// @return `true` si `redo()` aurait un effet.
    [[nodiscard]] bool canRedo() const noexcept {
        return !_redoHistory.empty();
    }

    /// Définit le budget de sauts (`-1` = illimité).
    void setJumpBudget(int jumpBudget) noexcept {
        _jumpBudget = jumpBudget;
    }

    /// Définit le budget de dashs (`-1` = illimité).
    void setDashBudget(int dashBudget) noexcept {
        _dashBudget = dashBudget;
    }

    /// Renomme le niveau.
    void setName(std::string name) {
        _name = std::move(name);
    }

    /// @return Le nom courant du niveau.
    [[nodiscard]] const std::string& name() const noexcept {
        return _name;
    }

    /// @return La grille de tuiles courante.
    [[nodiscard]] const TileMap& tileMap() const noexcept {
        return _tileMap;
    }

    /// @return La position d'entrée, si elle est posée.
    [[nodiscard]] std::optional<GridPosition> entry() const noexcept {
        return _entry;
    }

    /// @return La position de sortie, si elle est posée.
    [[nodiscard]] std::optional<GridPosition> exit() const noexcept {
        return _exit;
    }

    /// @return Les liaisons de mécanismes courantes.
    [[nodiscard]] const std::vector<Mechanism>& mechanisms() const noexcept {
        return _mechanisms;
    }

    /// @return Le budget de sauts courant (`-1` = illimité).
    [[nodiscard]] int jumpBudget() const noexcept {
        return _jumpBudget;
    }

    /// @return Le budget de dashs courant (`-1` = illimité).
    [[nodiscard]] int dashBudget() const noexcept {
        return _dashBudget;
    }

    /**
     * @brief Convertit le brouillon en `Level` **validé** (`EX-EDIT-007`), en repassant par la
     *        même validation que `LevelLoader` (sérialise puis recharge : aucune règle
     *        dupliquée, `EX-EDIT-010`).
     * @return Un résultat récupérable : niveau valide, ou message d'erreur exploitable si le
     *         brouillon est incomplet (pas d'entrée/de sortie, liaison non résolue, …).
     */
    [[nodiscard]] LevelLoadResult toLevel() const;

private:
    LevelDraft(std::string name, TileMap tileMap);

    /// Retire toute liaison de mécanisme référençant @p position (comme interrupteur ou porte).
    void removeMechanismsAt(GridPosition position);

    /// État complet du brouillon, hors historique (utilisé pour les snapshots undo/redo).
    struct State {
        std::string name;
        TileMap tileMap;
        std::optional<GridPosition> entry;
        std::optional<GridPosition> exit;
        std::vector<Mechanism> mechanisms;
        int jumpBudget;
        int dashBudget;
    };

    /// Capture l'état courant (pour empiler dans l'historique undo/redo).
    [[nodiscard]] State snapshot() const;

    /// Restitue un état capturé précédemment.
    void restore(State state);

    /// Empile l'état courant sur la pile d'annulation ; à appeler avant toute mutation
    /// undoable. Une nouvelle mutation invalide toujours la branche de refaire.
    void pushUndo();

    std::string _name;
    TileMap _tileMap;
    std::optional<GridPosition> _entry;
    std::optional<GridPosition> _exit;
    std::vector<Mechanism> _mechanisms;
    int _jumpBudget = -1;
    int _dashBudget = -1;
    std::vector<State> _undoHistory;
    std::vector<State> _redoHistory;
};

}  // namespace core

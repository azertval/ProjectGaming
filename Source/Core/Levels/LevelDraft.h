#pragma once

#include <cstddef>
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
     * @brief Applique un bloc rectangulaire de types de tuiles à partir de (@p originColumn,
     *        @p originRow).
     *
     * Repasse par la même sémantique cellule-par-cellule que `paintTile` (déplacement
     * entrée/sortie, nettoyage des liaisons) pour chaque case du bloc, mais ne pousse **qu'un
     * seul** snapshot undo pour toute l'opération — sert au remplissage rectangulaire et au
     * collage (`EX-EDIT-014`), sans dupliquer de règle de niveau (`EX-EDIT-010`). Les cases du
     * bloc hors des bornes de la grille sont silencieusement ignorées (découpe aux bords, même
     * principe que `resize`).
     * @param originColumn Colonne de la case (0,0) du bloc.
     * @param originRow    Ligne de la case (0,0) du bloc.
     * @param block        Bloc de types, indexé `[ligne][colonne]` ; sans effet si vide.
     */
    void paintRegion(int originColumn, int originRow,
                     const std::vector<std::vector<TileType>>& block);

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
     * @brief Lie un interrupteur/plaque de pression à une **porte** ou à un **danger commuté**
     *        (`EX-EDIT-003`, `EX-GP-052`), selon le type de tuile posé en @p targetPosition.
     *
     * Remplace toute liaison existante pour @p targetPosition (une cible n'a qu'un seul
     * déclencheur associé) ; plusieurs cibles peuvent en revanche partager le même déclencheur.
     * Même geste éditeur pour les deux cibles (clic déclencheur, clic cible) — la liaison
     * résultante est rangée dans `mechanisms()` (cible `Door`) ou `dangerLinks()` (cible
     * `DangerSwitched`) selon ce que porte réellement @p targetPosition.
     * @pre La case @p switchPosition porte un `Switch`/`PressurePlate`/`Key`, la case
     *      @p targetPosition porte une `Door`/`LockedDoor` **ou** un `DangerSwitched`.
     */
    void linkMechanism(GridPosition switchPosition, GridPosition targetPosition);

    /// Retire la liaison (porte ou danger commuté) à @p targetPosition, si elle en a une. Sans
    /// effet sinon.
    void unlinkMechanism(GridPosition targetPosition);

    /**
     * @brief Définit l'axe et la portée d'un danger mobile (`EX-GP-051`).
     *
     * Remplace toute configuration existante pour @p position. Sans appel, un `DangerMover` garde
     * les valeurs de conception par défaut de `DangerMoverConfig` (appliquées par `LevelLoader` au
     * rechargement, ce brouillon n'a pas besoin de les dupliquer tant qu'elles ne sont pas
     * personnalisées).
     * @pre La case @p position porte un `DangerMover`.
     */
    void setMoverConfig(GridPosition position, DangerMoverAxis axis, int range);

    /**
     * @brief Définit la période, le déphasage et la durée active d'un danger temporisé
     *        (`EX-GP-053`).
     *
     * Mêmes remarques que `setMoverConfig` : sans appel, valeurs de conception par défaut.
     * @pre La case @p position porte un `DangerBlink`.
     */
    void setBlinkConfig(GridPosition position, int period, int phase, int activeDuration);

    /**
     * @brief Définit le second point de parcours, la vitesse et le déphasage d'une plateforme
     *        mobile (`EX-GP-026`).
     *
     * Remplace toute configuration existante pour @p position. Mêmes remarques que
     * `setMoverConfig`/`setBlinkConfig` : sans appel, une `MovingPlatform` garde les valeurs de
     * conception par défaut (`MovingPlatformConfig`), appliquées par `LevelLoader` au rechargement.
     * @pre La case @p position porte une `MovingPlatform`.
     */
    void setPlatformConfig(GridPosition position, GridPosition endPosition, float speed, int phase);

    /**
     * @brief Assigne (ou remplace) la texture affichée pour **une case précise** (`EX-EDIT-043`),
     *        prioritaire sur le skin de son type (LOT-42).
     *
     * Repeindre un **autre** type de tuile sur @p position retire l'override (funnel
     * `removeLinkedDataAt`) ; repeindre le **même** type le conserve. Ne voyage pas avec
     * `paintRegion` (collage) : un override reste attaché à sa case d'origine.
     * @pre La case @p position porte un type de tuile non `Empty`.
     */
    void setTextureOverride(GridPosition position, std::string assetName);

    /// Retire l'override de texture de @p position, s'il y en a un. Sans effet sinon.
    void removeTextureOverride(GridPosition position);

    /**
     * @brief Ajoute un décor libre en fin de vecteur (`EX-DEC-001`, `EX-EDIT-010`).
     *
     * L'ordre d'ajout fixe le rang de superposition intra-couche (TACHE-02) : un décor ajouté
     * après un autre de la même couche se dessine par-dessus.
     * @param decor Décor à ajouter (position libre, hors grille).
     */
    void addDecor(Decor decor);

    /**
     * @brief Retire le décor au rang @p index (`EX-DEC-010`).
     * @param index Rang dans `decors()` ; sans effet si hors bornes.
     */
    void removeDecor(std::size_t index);

    /**
     * @brief Déplace le décor au rang @p index à une nouvelle position libre (`EX-DEC-010`).
     *
     * Ne change ni la couche ni le rang : seule `Decor::position` est modifiée.
     * @param index    Rang dans `decors()`.
     * @param position Nouvelle position, en unités monde (aucune contrainte de grille).
     * @return `true` si appliqué ; `false` si @p index est hors bornes (sans effet, `EX-NFR-040`).
     */
    bool moveDecor(std::size_t index, Vector2 position);

    /**
     * @brief Redimensionne le décor au rang @p index (`EX-DEC-010`), en repositionnant son coin
     *        éventuellement du même geste.
     *
     * @p position et @p scale sont appliquées **atomiquement**, en une seule mutation annulable :
     * redimensionner depuis un coin (poignée, `LOT-50` TACHE-02) déplace le coin **opposé au
     * coin ancré**, sauf depuis le coin bas-droit — les deux changent donc ensemble le plus
     * souvent, et scinder l'opération en deux appels (`moveDecor` + `resizeDecor`) empilerait
     * deux entrées d'historique pour un seul geste utilisateur. Un appelant qui ne change que
     * l'échelle (ex. futur champ numérique, `LOT-50` TACHE-04) passe simplement la position
     * courante du décor, inchangée.
     *
     * Une échelle non strictement positive sur un axe est **rejetée** (aucune mutation, position
     * comprise) : un décor invisible (échelle nulle) ou retourné par accident (échelle négative)
     * serait un piège d'usage plutôt qu'une fonctionnalité.
     * @param index    Rang dans `decors()`.
     * @param position Nouvelle position (coin haut-gauche), en unités monde.
     * @param scale    Nouvelle échelle ; chaque composante doit être strictement positive.
     * @return `true` si appliqué ; `false` si @p index est hors bornes ou si @p scale n'est pas
     *         strictement positive sur les deux axes (sans effet, `EX-NFR-040`).
     */
    bool resizeDecor(std::size_t index, Vector2 position, Vector2 scale);

    /**
     * @brief Pivote le décor au rang @p index (`EX-DEC-010`).
     *
     * La rotation est **normalisée** dans `[0, 2π[` avant d'être stockée : contrairement à
     * l'échelle, aucune valeur n'est rejetée, une rotation n'a pas de valeur invalide.
     * @param index    Rang dans `decors()`.
     * @param rotation Nouvelle rotation, en radians (n'importe quelle valeur, y compris hors
     *                 `[0, 2π[` ou négative).
     * @return `true` si appliqué ; `false` si @p index est hors bornes (sans effet).
     */
    bool rotateDecor(std::size_t index, float rotation);

    /**
     * @brief Change la couche du décor au rang @p index (`EX-DEC-010`, `EX-DEC-002`).
     *
     * Comportement défini explicitement (`EX-EDIT-010`, pour ne pas le laisser émerger) : le
     * décor rejoint la **fin** du vecteur, ce qui en fait le rang le plus élevé (« dessus ») de sa
     * nouvelle couche — même convention que `addDecor`, qui place toujours un nouveau décor en
     * fin de vecteur. Sans effet (mais succès) si @p layer est déjà la couche courante du décor.
     * @param index Rang dans `decors()`.
     * @param layer Nouvelle couche.
     * @return Le nouveau rang du décor, ou `std::nullopt` si @p index est hors bornes.
     */
    std::optional<std::size_t> setDecorLayer(std::size_t index, DecorLayer layer);

    /**
     * @brief Avance le décor au rang @p index d'un cran dans l'ordre de superposition **de sa
     *        couche** (`EX-DEC-010`) : échange avec le décor de même couche immédiatement suivant.
     *
     * Ne change jamais la couche du décor (`EX-DEC-002`) : seul l'ordre **à l'intérieur** de sa
     * couche est affecté. Sans effet (mais succès, rang inchangé) si @p index désigne déjà le
     * décor le plus en avant de sa couche.
     * @param index Rang dans `decors()`.
     * @return Le nouveau rang du décor, ou `std::nullopt` si @p index est hors bornes.
     */
    std::optional<std::size_t> bringDecorForward(std::size_t index);

    /// Recule le décor au rang @p index d'un cran dans l'ordre de sa couche (symétrique de
    /// `bringDecorForward`) : échange avec le décor de même couche immédiatement précédent.
    std::optional<std::size_t> sendDecorBackward(std::size_t index);

    /// Amène le décor au rang @p index au premier rang (le plus en avant) de sa couche.
    std::optional<std::size_t> bringDecorToFront(std::size_t index);

    /// Envoie le décor au rang @p index au dernier rang (le plus en arrière) de sa couche.
    std::optional<std::size_t> sendDecorToBack(std::size_t index);

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

    /**
     * @brief Assigne l'asset de fond du niveau (`EX-REN-044`), annulable.
     * @param background Nom de l'asset (ex. `"forest.png"`), ou vide pour retirer le fond posé.
     *                    Une chaîne, jamais un handle : `Core` ignore tout du dossier d'assets.
     */
    void setBackground(std::optional<std::string> background);

    /**
     * @brief Assigne le jeu de skins du niveau (`EX-EDIT-024`), annulable.
     * @param skinSet Nom du jeu (`skins.json`), ou vide pour le jeu par défaut.
     */
    void setSkinSet(std::optional<std::string> skinSet);

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

    /// @return Les liaisons de mécanismes courantes (déclencheur ↔ porte).
    [[nodiscard]] const std::vector<Mechanism>& mechanisms() const noexcept {
        return _mechanisms;
    }

    /// @return Les liaisons de danger commuté courantes (déclencheur ↔ danger, `EX-GP-052`).
    [[nodiscard]] const std::vector<DangerLink>& dangerLinks() const noexcept {
        return _dangerLinks;
    }

    /// @return Les configurations de danger mobile posées explicitement (`EX-GP-051`).
    [[nodiscard]] const std::vector<DangerMoverConfig>& moverConfigs() const noexcept {
        return _moverConfigs;
    }

    /// @return Les configurations de danger temporisé posées explicitement (`EX-GP-053`).
    [[nodiscard]] const std::vector<DangerBlinkConfig>& blinkConfigs() const noexcept {
        return _blinkConfigs;
    }

    /// @return Les configurations de plateforme mobile posées explicitement (`EX-GP-026`).
    [[nodiscard]] const std::vector<MovingPlatformConfig>& platformConfigs() const noexcept {
        return _platformConfigs;
    }

    /// @return Les textures assignées par instance du niveau (`EX-EDIT-043`).
    [[nodiscard]] const std::vector<TileTextureOverride>& textureOverrides() const noexcept {
        return _textureOverrides;
    }

    /// @return Les décors libres courants, dans leur ordre de superposition intra-couche.
    [[nodiscard]] const std::vector<Decor>& decors() const noexcept {
        return _decors;
    }

    /// @return Le budget de sauts courant (`-1` = illimité).
    [[nodiscard]] int jumpBudget() const noexcept {
        return _jumpBudget;
    }

    /// @return Le budget de dashs courant (`-1` = illimité).
    [[nodiscard]] int dashBudget() const noexcept {
        return _dashBudget;
    }

    /// @return L'asset de fond courant (`EX-REN-044`), absent si aucun n'est posé.
    [[nodiscard]] const std::optional<std::string>& background() const noexcept {
        return _background;
    }

    /// @return Le jeu de skins courant du niveau (`EX-EDIT-024`), absent pour le jeu par défaut.
    [[nodiscard]] const std::optional<std::string>& skinSet() const noexcept {
        return _skinSet;
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

    /// Logique de `paintTile`, sans `pushUndo()` : réutilisée cellule par cellule par
    /// `paintRegion` pour n'empiler qu'un seul snapshot par opération de bloc.
    void paintTileInternal(int column, int row, TileType type);

    /// Logique de `setEntry`, sans `pushUndo()`.
    void setEntryInternal(int column, int row);

    /// Logique de `setExit`, sans `pushUndo()`.
    void setExitInternal(int column, int row);

    /// Retire toute liaison/configuration référençant @p position (déclencheur, porte, danger
    /// commuté, configuration de danger mobile/temporisé, ou override de texture) — appelé avant
    /// de reposer un autre type sur une case qui en portait une, pour ne jamais laisser d'entrée
    /// orpheline. @p keepTextureOverride préserve l'override de texture (repeindre le **même**
    /// type ne doit pas effacer un habillage, `EX-EDIT-043`) ; les autres données annexes sont
    /// toujours retirées, comme avant ce paramètre.
    void removeLinkedDataAt(GridPosition position, bool keepTextureOverride = false);

    /// État complet du brouillon, hors historique (utilisé pour les snapshots undo/redo).
    struct State {
        std::string name;
        TileMap tileMap;
        std::optional<GridPosition> entry;
        std::optional<GridPosition> exit;
        std::vector<Mechanism> mechanisms;
        int jumpBudget;
        int dashBudget;
        std::vector<DangerLink> dangerLinks;
        std::vector<DangerMoverConfig> moverConfigs;
        std::vector<DangerBlinkConfig> blinkConfigs;
        std::optional<std::string> background;
        std::optional<std::string> skinSet;
        std::vector<TileTextureOverride> textureOverrides;
        std::vector<Decor> decors;
        std::vector<MovingPlatformConfig> platformConfigs;
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
    std::vector<DangerLink> _dangerLinks;
    std::vector<DangerMoverConfig> _moverConfigs;
    std::vector<DangerBlinkConfig> _blinkConfigs;
    std::optional<std::string> _background;
    std::optional<std::string> _skinSet;
    std::vector<TileTextureOverride> _textureOverrides;
    std::vector<Decor> _decors;
    std::vector<MovingPlatformConfig> _platformConfigs;
    std::vector<State> _undoHistory;
    std::vector<State> _redoHistory;
};

}  // namespace core

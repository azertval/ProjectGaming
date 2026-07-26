#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/Level.h"
#include "HMI/Game/GameSession.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"
#include "HMI/Interface/IScreen.h"
#include "HMI/Interface/LevelSequence.h"

/**
 * @file HMI/Interface/GameScreen.h
 * @brief Écran de jeu : joue une séquence de niveaux, avec enchaînement à la réussite.
 */

namespace hmi {

class SpriteBatch;
class TextureAtlas;

/**
 * @brief Écran de jeu jouant une **séquence de niveaux** chargés depuis des fichiers.
 *
 * Depuis `LOT-34`, toute la **logique de jeu** (chargement de la scène, physique, mécanismes, blocs,
 * dangers, caméra par salle, animation, interpolation) est déléguée à `hmi::GameSession` — la même
 * session réutilisée par l'éditeur Qt, sans duplication. Cet écran n'ajoute que les **concerns
 * d'écran** : la séquence de niveaux (`LevelSequence`) et son enchaînement à la réussite
 * (`EX-LVL-010`/`011`), le retour au menu (`Échap` ou après le dernier niveau), et l'affichage d'un
 * état d'erreur si un fichier de niveau est invalide (`EX-NFR-040`). Le comportement de jeu est
 * **inchangé** : `GameSession` recharge le niveau courant sur un échec (danger/chute), l'écran
 * enchaîne ou revient au menu sur une réussite.
 */
class GameScreen : public IScreen {
public:
    /**
     * @brief Construit l'écran et charge le premier niveau de la séquence.
     * @param batch           Lot de sprites partagé (rendu).
     * @param atlas           Atlas de tuiles fournissant les régions de sprites.
     * @param viewportWidth   Largeur initiale de la surface de rendu, en pixels.
     * @param viewportHeight  Hauteur initiale de la surface de rendu, en pixels.
     * @param levels          Liste **ordonnée** des chemins de niveaux à enchaîner.
     * @param gameBindings    Touches clavier de jeu (référence conservée, doit survivre à l'écran).
     * @param gamepadBindings Boutons manette de jeu (référence conservée, doit survivre à l'écran).
     */
    GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth, int viewportHeight,
               std::vector<std::filesystem::path> levels, const GameKeyBindings& gameBindings,
               const GamepadBindings& gamepadBindings);

    /**
     * @brief Construit l'écran pour un **niveau unique déjà en mémoire**, sans fichier ni séquence
     *        (essai immédiat de l'éditeur, `EX-EDIT-008`).
     *
     * Atteindre la sortie termine l'essai (retour au menu, pas d'enchaînement) ; un échec redémarre
     * ce même niveau, à l'identique du mode séquence.
     */
    GameScreen(SpriteBatch& batch, const TextureAtlas& atlas, int viewportWidth, int viewportHeight,
               core::Level level, const GameKeyBindings& gameBindings,
               const GamepadBindings& gamepadBindings);

    [[nodiscard]] ScreenTransition update(const InputState& input, float fixedDelta) override;

    void render(RenderContext& context) override;

private:
    /// Charge le niveau courant de la séquence depuis un fichier et (re)crée la session. Échec
    /// récupérable (`EX-NFR-040`) : `_session` reste vide, `_loadError` renseigné.
    void loadCurrentSequenceLevel();

    SpriteBatch& _batch;
    const TextureAtlas& _atlas;
    const GameKeyBindings& _gameBindings;
    const GamepadBindings& _gamepadBindings;
    int _viewportWidth;
    int _viewportHeight;
    /// Progression : niveaux ordonnés + indice courant ; absente en mode niveau unique en mémoire
    /// (essai immédiat), auquel cas atteindre la sortie termine sans enchaîner.
    std::optional<LevelSequence> _sequence;
    std::optional<GameSession> _session;  ///< Session de jeu du niveau courant (nul si échec).
    std::string _loadError;  ///< Vide si un niveau est chargé ; message d'erreur sinon.
};

}  // namespace hmi

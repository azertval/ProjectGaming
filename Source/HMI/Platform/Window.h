#pragma once

#include <Windows.h>

#include "HMI/Input/InputState.h"

/**
 * @file HMI/Platform/Window.h
 * @brief Fenêtre native Win32, sa pompe de messages et la capture des entrées.
 */

namespace hmi {

/**
 * @brief Encapsule une fenêtre Win32 (création, événements, entrées, fermeture) en RAII.
 *
 * La fenêtre est créée à la construction et détruite au destructeur. Elle expose les
 * événements utiles à la boucle de jeu (demande de fermeture, redimensionnement) et **capture
 * les entrées** clavier/souris/manette dans un `InputState` échantillonné une fois par frame
 * (`EX-CTRL-021`). La manette (XInput, `EX-CTRL-002`) est **sondée** (pas événementielle comme le
 * clavier/la souris) : `pollGamepad`, appelée depuis `pumpMessages`, alimente deux pistes
 * indépendantes du même `InputState` — la fusion clavier/manette sur `Key`
 * (`onGamepadKeyDown`/`onGamepadKeyUp`, fixe, utilisée par la navigation de menu, aucune touche
 * clavier n'est jamais écrasée) et l'état brut par `GamepadButton`
 * (`onGamepadButtonDown`/`onGamepadButtonUp`, remappable via `GamepadBindings`). La traduction de
 * l'`InputState` en actions de gameplay est un
 * module dédié et indépendant de toute fenêtre (`HMI/Input/PlayerInputMapper`,
 * `hmi::toPlayerInput`).
 */
class Window {
public:
    /**
     * @brief Crée et affiche la fenêtre.
     * @param title  Titre affiché dans la barre de la fenêtre.
     * @param width  Largeur souhaitée de la zone client, en pixels.
     * @param height Hauteur souhaitée de la zone client, en pixels.
     */
    Window(const wchar_t* title, int width, int height);

    /// Détruit la fenêtre Win32.
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /**
     * @brief Sonde la manette puis traite les messages Win32 en attente.
     *
     * Non bloquant. Met à jour l'**état courant** des entrées à partir de la manette
     * (`pollGamepad`) et des messages clavier/souris de la frame réelle. N'avance **pas** les
     * fronts (pressée/relâchée) : cette responsabilité revient à `beginInputFrame`, appelée par la
     * boucle de simulation **une fois par pas fixe consommé**, jamais par frame de rendu (voir
     * `beginInputFrame`).
     */
    void pumpMessages();

    /**
     * @brief Ouvre une nouvelle frame d'entrées : recopie l'état courant vers l'état précédent.
     *
     * À appeler par la boucle de simulation **après chaque pas fixe** qui a lu les entrées, jamais
     * une fois par frame de rendu. Découpler l'avancée des fronts du rendu corrige la perte
     * d'entrées lorsque le rendu tourne plus vite que le pas fixe (`FixedTimestep`) : sur les
     * frames réelles sans pas de simulation (`steps == 0`, écran > 60 Hz), l'état courant — donc un
     * appui capturé entre-temps — **survit** jusqu'à ce qu'un pas le lise, au lieu d'être écrasé
     * par une recopie prématurée. Vide aussi la molette et les caractères tapés accumulés
     * (`EX-CTRL-021`).
     */
    void beginInputFrame();

    /**
     * @brief Indique si la fenêtre demande à se fermer (croix ou `requestClose`).
     * @return true si la boucle de jeu doit s'arrêter.
     * @note La touche Échap **ne ferme plus** la fenêtre : c'est une touche normale, lue via
     *       `input()` (elle sert à revenir au menu depuis les écrans).
     */
    [[nodiscard]] bool shouldClose() const;

    /**
     * @brief État des entrées clavier/souris de la frame courante.
     * @return L'`InputState` capturé, en lecture seule.
     */
    [[nodiscard]] const InputState& input() const;

    /// Demande la fermeture programmée de la fenêtre (action « Quitter » du menu).
    void requestClose();

    /**
     * @brief Handle natif de la fenêtre, pour l'initialisation Direct3D.
     * @return Le HWND de la fenêtre.
     */
    [[nodiscard]] HWND handle() const;

    /// @return Largeur courante de la zone client, en pixels.
    [[nodiscard]] int clientWidth() const;

    /// @return Hauteur courante de la zone client, en pixels.
    [[nodiscard]] int clientHeight() const;

    /**
     * @brief Récupère et consomme un éventuel redimensionnement survenu depuis le dernier appel.
     * @param outWidth  Reçoit la nouvelle largeur client si un redimensionnement a eu lieu.
     * @param outHeight Reçoit la nouvelle hauteur client si un redimensionnement a eu lieu.
     * @return true si un redimensionnement était en attente (les sorties sont alors valides).
     */
    [[nodiscard]] bool consumeResize(int& outWidth, int& outHeight);

private:
    /// Procédure de fenêtre statique : route les messages vers l'instance.
    static LRESULT CALLBACK windowProcedure(HWND handle, UINT message, WPARAM wParam,
                                            LPARAM lParam);

    /// Traite un message pour cette instance.
    LRESULT handleMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

    /// Sonde la manette (XInput, joueur 0) et fusionne son état dans `_input` (`EX-CTRL-002`).
    void pollGamepad();

    /// Nombre de frames entre deux sondages XInput d'un slot resté déconnecté (anti-saccade).
    static constexpr int GAMEPAD_DISCONNECTED_POLL_INTERVAL = 120;

    HWND _handle;
    bool _shouldClose;
    bool _resized;
    int _clientWidth;
    int _clientHeight;
    InputState _input;
    bool _gamepadWasConnected = false;  ///< Pour ne journaliser qu'un changement de connexion.
    int _gamepadPollCountdown = 0;  ///< Frames restantes avant de re-sonder une manette absente.
};

}  // namespace hmi

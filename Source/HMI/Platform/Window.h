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
 * clavier/la souris) : `pollGamepad`, appelée depuis `pumpMessages`, fusionne son état dans le
 * même `InputState` via `onGamepadKeyDown`/`onGamepadKeyUp` (voir `InputState`, aucune touche
 * clavier n'est jamais écrasée). La traduction des entrées en actions de gameplay relèvera d'un
 * module dédié.
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
     * @brief Ouvre une nouvelle frame d'entrées puis traite les messages Win32 en attente.
     *
     * Non bloquant. Recopie l'état d'entrées courant vers l'état précédent (via
     * `InputState::beginFrame`) puis met à jour l'état courant à partir des messages de la
     * frame : l'`InputState` est ainsi échantillonné **une fois par frame** (`EX-CTRL-021`).
     */
    void pumpMessages();

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
    static LRESULT CALLBACK windowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

    /// Traite un message pour cette instance.
    LRESULT handleMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

    /// Sonde la manette (XInput, joueur 0) et fusionne son état dans `_input` (`EX-CTRL-002`).
    void pollGamepad();

    HWND _handle;
    bool _shouldClose;
    bool _resized;
    int _clientWidth;
    int _clientHeight;
    InputState _input;
};

}  // namespace hmi

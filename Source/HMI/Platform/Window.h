#pragma once

#include <Windows.h>

/**
 * @file HMI/Platform/Window.h
 * @brief Fenêtre native Win32 et sa pompe de messages.
 */

namespace hmi {

/**
 * @brief Encapsule une fenêtre Win32 (création, événements, fermeture) en RAII.
 *
 * La fenêtre est créée à la construction et détruite au destructeur. Elle expose
 * les événements utiles à la boucle de jeu (demande de fermeture, redimensionnement)
 * sans traiter les entrées de gameplay, qui relèveront d'un module dédié.
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

    /// Traite tous les messages Win32 en attente (non bloquant).
    void pumpMessages();

    /**
     * @brief Indique si la fenêtre demande à se fermer (croix ou touche Échap).
     * @return true si la boucle de jeu doit s'arrêter.
     */
    [[nodiscard]] bool shouldClose() const;

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

    HWND _handle;
    bool _shouldClose;
    bool _resized;
    int _clientWidth;
    int _clientHeight;
};

}  // namespace hmi

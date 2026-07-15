#include "HMI/Window.h"

#include <stdexcept>

namespace hmi {

namespace {
/// Nom de la classe de fenêtre Win32, partagé par toutes les instances.
constexpr const wchar_t* WINDOW_CLASS_NAME = L"ProjectGamingWindowClass";
}  // namespace

/**
 * @brief Crée et affiche la fenêtre.
 * @param title  Titre affiché dans la barre de la fenêtre.
 * @param width  Largeur souhaitée de la zone client, en pixels.
 * @param height Hauteur souhaitée de la zone client, en pixels.
 */
Window::Window(const wchar_t* title, int width, int height)
    : _handle(nullptr),
      _shouldClose(false),
      _resized(false),
      _clientWidth(width),
      _clientHeight(height) {
    const HINSTANCE instance = GetModuleHandleW(nullptr);

    // Enregistrement de la classe de fenêtre. Un éventuel « déjà enregistré »
    // (plusieurs fenêtres au cours de la session) n'est pas une erreur.
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &Window::windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;
    RegisterClassExW(&windowClass);

    // La taille demandée concerne la zone client : on agrandit le rectangle
    // fenêtre pour tenir compte des bordures et de la barre de titre.
    RECT rectangle{0, 0, width, height};
    AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE);
    const int windowWidth = rectangle.right - rectangle.left;
    const int windowHeight = rectangle.bottom - rectangle.top;

    // `this` est transmis en paramètre de création pour être relié au HWND
    // dès WM_NCCREATE (voir windowProcedure).
    _handle = CreateWindowExW(0, WINDOW_CLASS_NAME, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                              CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, instance,
                              this);
    if (_handle == nullptr) {
        throw std::runtime_error("Echec de la creation de la fenetre Win32");
    }

    ShowWindow(_handle, SW_SHOW);
}

/// Détruit la fenêtre Win32.
Window::~Window() {
    if (_handle != nullptr) {
        DestroyWindow(_handle);
    }
}

/**
 * @brief Procédure de fenêtre statique : route les messages vers l'instance.
 *
 * Au tout premier message (WM_NCCREATE), le pointeur d'instance transmis à la
 * création est stocké dans les données utilisateur du HWND, puis récupéré à
 * chaque message suivant.
 */
LRESULT CALLBACK Window::windowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        auto* createStructure = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* self = static_cast<Window*>(createStructure->lpCreateParams);
        SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->_handle = handle;
        return self->handleMessage(handle, message, wParam, lParam);
    }

    auto* self = reinterpret_cast<Window*>(GetWindowLongPtrW(handle, GWLP_USERDATA));
    if (self != nullptr) {
        return self->handleMessage(handle, message, wParam, lParam);
    }
    return DefWindowProcW(handle, message, wParam, lParam);
}

/// Traite un message pour cette instance.
LRESULT Window::handleMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SIZE:
            // On ignore la minimisation (taille client nulle) : le rendu et le
            // swap chain ne doivent pas être redimensionnés à zéro.
            if (wParam != static_cast<WPARAM>(SIZE_MINIMIZED)) {
                _clientWidth = static_cast<int>(LOWORD(lParam));
                _clientHeight = static_cast<int>(HIWORD(lParam));
                _resized = true;
            }
            return 0;
        case WM_KEYDOWN:
            if (wParam == static_cast<WPARAM>(VK_ESCAPE)) {
                _shouldClose = true;
            }
            return 0;
        case WM_CLOSE:
            _shouldClose = true;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(handle, message, wParam, lParam);
    }
}

/// Traite tous les messages Win32 en attente (non bloquant).
void Window::pumpMessages() {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
        if (message.message == WM_QUIT) {
            _shouldClose = true;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

bool Window::shouldClose() const {
    return _shouldClose;
}

HWND Window::handle() const {
    return _handle;
}

int Window::clientWidth() const {
    return _clientWidth;
}

int Window::clientHeight() const {
    return _clientHeight;
}

/**
 * @brief Récupère et consomme un éventuel redimensionnement survenu depuis le dernier appel.
 * @param outWidth  Reçoit la nouvelle largeur client si un redimensionnement a eu lieu.
 * @param outHeight Reçoit la nouvelle hauteur client si un redimensionnement a eu lieu.
 * @return true si un redimensionnement était en attente (les sorties sont alors valides).
 */
bool Window::consumeResize(int& outWidth, int& outHeight) {
    if (!_resized) {
        return false;
    }
    _resized = false;
    outWidth = _clientWidth;
    outHeight = _clientHeight;
    return true;
}

}  // namespace hmi

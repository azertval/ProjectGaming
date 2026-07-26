#include "HMI/Platform/Window.h"

#include <cstdint>
#include <stdexcept>

#include <windowsx.h>

#include "HMI/Platform/PlatformLog.h"

namespace hmi {

namespace {
// Nom de la classe de fenêtre Win32, partagé par toutes les instances.
constexpr const wchar_t* WINDOW_CLASS_NAME = L"ProjectGamingWindowClass";
}  // namespace

// Crée et affiche la fenêtre.
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
    _handle =
        CreateWindowExW(0, WINDOW_CLASS_NAME, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                        CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, instance, this);
    if (_handle == nullptr) {
        throw std::runtime_error("Echec de la creation de la fenetre Win32");
    }

    ShowWindow(_handle, SW_SHOW);
    PLATFORM_LOG_TRACE("Fenetre Win32 creee et affichee");
}

// Détruit la fenêtre Win32.
Window::~Window() {
    if (_handle != nullptr) {
        DestroyWindow(_handle);
        PLATFORM_LOG_TRACE("Fenetre Win32 detruite");
    }
}

// Procédure de fenêtre statique : route les messages vers l'instance.
//
// Au tout premier message (WM_NCCREATE), le pointeur d'instance transmis à la
// création est stocké dans les données utilisateur du HWND, puis récupéré à
// chaque message suivant.
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

// Traite un message pour cette instance.
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
            // Échap n'est plus un raccourci de fermeture : toutes les touches sont de
            // simples entrées, converties en `Key` par leur code virtuel (valeurs alignées).
            _input.onKeyDown(static_cast<Key>(static_cast<std::uint16_t>(wParam)));
            return 0;
        case WM_KEYUP:
            _input.onKeyUp(static_cast<Key>(static_cast<std::uint16_t>(wParam)));
            return 0;
        case WM_SYSKEYDOWN:
            // F10 (et les touches Alt) empruntent WM_SYSKEYDOWN par convention Win32 historique
            // (activation du menu), jamais WM_KEYDOWN — sans cette prise en charge, l'éditeur ne
            // recevrait jamais F10 (LOT-15, bascule de la grille de repère). Cette fenêtre n'a pas
            // de menu : on absorbe uniquement F10 (évite l'activation visuelle du système de menu
            // au relâchement) et laisse le traitement système par défaut pour le reste (Alt+F4,
            // Alt+Tab...).
            _input.onKeyDown(static_cast<Key>(static_cast<std::uint16_t>(wParam)));
            if (static_cast<std::uint16_t>(wParam) == static_cast<std::uint16_t>(Key::F10)) {
                return 0;
            }
            return DefWindowProcW(handle, message, wParam, lParam);
        case WM_SYSKEYUP:
            _input.onKeyUp(static_cast<Key>(static_cast<std::uint16_t>(wParam)));
            if (static_cast<std::uint16_t>(wParam) == static_cast<std::uint16_t>(Key::F10)) {
                return 0;
            }
            return DefWindowProcW(handle, message, wParam, lParam);
        case WM_CHAR:
            // Caractère déjà traduit selon la disposition clavier active par TranslateMessage
            // (appelé en amont dans pumpMessages) : distinct des codes virtuels de WM_KEYDOWN,
            // nécessaire à la saisie de texte de l'éditeur (LOT-15).
            _input.onCharTyped(static_cast<wchar_t>(wParam));
            return 0;
        case WM_MOUSEWHEEL:
            // GET_WHEEL_DELTA_WPARAM extrait l'incrément signé (multiple de WHEEL_DELTA = 120).
            _input.onMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;
        case WM_MOUSEMOVE:
            // GET_X/Y_LPARAM extraient des coordonnées **signées** (souris hors zone client
            // lors d'un capture), en pixels de la zone client.
            _input.onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_LBUTTONDOWN:
            _input.onMouseButtonDown(MouseButton::Left);
            return 0;
        case WM_LBUTTONUP:
            _input.onMouseButtonUp(MouseButton::Left);
            return 0;
        case WM_RBUTTONDOWN:
            _input.onMouseButtonDown(MouseButton::Right);
            return 0;
        case WM_RBUTTONUP:
            _input.onMouseButtonUp(MouseButton::Right);
            return 0;
        case WM_MBUTTONDOWN:
            _input.onMouseButtonDown(MouseButton::Middle);
            return 0;
        case WM_MBUTTONUP:
            _input.onMouseButtonUp(MouseButton::Middle);
            return 0;
        case WM_KILLFOCUS:
            // Perte de focus (Alt+Tab, clic ailleurs...) : Windows n'enverra pas de WM_KEYUP pour
            // les touches maintenues au moment du basculement. Sans ce relâchement global, elles
            // resteraient « collées » et le personnage continuerait d'avancer au retour.
            _input.releaseAll();
            return 0;
        case WM_CLOSE:
            PLATFORM_LOG_TRACE("Fermeture de la fenetre demandee (croix)");
            _shouldClose = true;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(handle, message, wParam, lParam);
    }
}

// Sonde la manette puis draine les messages Win32 dans l'état d'entrées courant.
//
// N'appelle PAS `beginFrame` : les fronts (pressée/relâchée) ne sont figés qu'au moment où un pas
// de simulation les consomme (voir `beginInputFrame`), pas à chaque frame de rendu. La manette est
// sondée d'abord, puis le clavier/la souris sont drainés : tous atterrissent dans le même état
// courant, qui persiste tel quel tant qu'aucun pas fixe ne l'a lu.
void Window::pumpMessages() {
    _gamepad.poll(_input);

    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
        if (message.message == WM_QUIT) {
            _shouldClose = true;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

// Ouvre une nouvelle frame d'entrées : recopie l'état courant vers l'état précédent (voir en-tête).
// Appelée par la boucle de simulation après chaque pas fixe consommé, pour que chaque front soit
// consommé par exactement un pas, au rythme de la simulation et non du framerate de rendu.
void Window::beginInputFrame() {
    _input.beginFrame();
}

bool Window::shouldClose() const {
    return _shouldClose;
}

// État des entrées clavier/souris de la frame courante.
// L'`InputState` capturé, en lecture seule.
const InputState& Window::input() const {
    return _input;
}

// Demande la fermeture programmée de la fenêtre (action « Quitter » du menu).
void Window::requestClose() {
    PLATFORM_LOG_TRACE("Fermeture programmee de la fenetre demandee");
    _shouldClose = true;
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

// Récupère et consomme un éventuel redimensionnement survenu depuis le dernier appel.
// true si un redimensionnement était en attente (les sorties sont alors valides).
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

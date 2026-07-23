#include "HMI/Platform/Window.h"

#include <Xinput.h>
#include <windowsx.h>

#include <cstdint>
#include <stdexcept>

#include "HMI/Platform/PlatformLog.h"

namespace hmi {

namespace {
// Nom de la classe de fenêtre Win32, partagé par toutes les instances.
constexpr const wchar_t* WINDOW_CLASS_NAME = L"ProjectGamingWindowClass";

// Sens du stick gauche sur un axe, au-dela de la zone morte XInput ; 0 si dans la zone morte
// (evite un deplacement fantome au repos, jitter materiel).
[[nodiscard]] int stickDirection(SHORT axis) noexcept {
    if (axis > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
        return 1;
    }
    if (axis < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
        return -1;
    }
    return 0;
}
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
    _handle = CreateWindowExW(0, WINDOW_CLASS_NAME, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                              CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, instance,
                              this);
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

// Sonde la manette (XInput, joueur 0) et fusionne son état dans _input (EX-CTRL-002).
//
// XInput est interroge (pas evenementiel comme le clavier/la souris) : appelee une fois par
// frame, elle doit explicitement liberer (onGamepadKeyUp) chaque touche synthetique dont le
// bouton/la direction correspondante n'est plus enfonce ce pas-ci — y compris quand la manette
// est absente/debranchee (ERROR_DEVICE_NOT_CONNECTED), auquel cas pad reste a zero (state a ete
// initialise a zero avant l'appel) et tout est relache, sans qu'aucune touche ne reste "collee".
// Chaque bouton n'est JAMAIS ecrit via onKeyDown/onKeyUp (clavier) : voir InputState pour la
// fusion en lecture qui evite d'effacer une touche clavier reellement maintenue.
void Window::pollGamepad() {
    XINPUT_STATE state{};
    const bool connected = XInputGetState(0, &state) == ERROR_SUCCESS;
    _input.setGamepadConnected(connected);
    if (connected != _gamepadWasConnected) {
        PLATFORM_LOG_INFO(connected ? "Manette connectee" : "Manette deconnectee");
        _gamepadWasConnected = connected;
    }

    const XINPUT_GAMEPAD& pad = state.Gamepad;
    const int stickX = stickDirection(pad.sThumbLX);
    const int stickY = stickDirection(pad.sThumbLY);  // XInput : Y positif = vers le haut

    auto setKey = [this](Key key, bool pressed) {
        if (pressed) {
            _input.onGamepadKeyDown(key);
        } else {
            _input.onGamepadKeyUp(key);
        }
    };

    // D-pad ou stick gauche : les deux pilotent les memes directions (EX-CTRL-002).
    setKey(Key::Left, (pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0 || stickX < 0);
    setKey(Key::Right, (pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0 || stickX > 0);
    setKey(Key::Up, (pad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0 || stickY > 0);
    setKey(Key::Down, (pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0 || stickY < 0);
    // A valide (menu) ET saute (jeu) : meme bouton physique pour les deux usages, comme au
    // clavier (Entree/Espace sont deja deux touches distinctes pour la meme intention).
    const bool aHeld = (pad.wButtons & XINPUT_GAMEPAD_A) != 0;
    setKey(Key::Enter, aHeld);
    setKey(Key::Space, aHeld);
    // B et Start reproduisent tous deux Echap (retour/pause) : pas d'ecran de pause dedie.
    setKey(Key::Escape,
          (pad.wButtons & XINPUT_GAMEPAD_B) != 0 || (pad.wButtons & XINPUT_GAMEPAD_START) != 0);
    setKey(Key::Shift, (pad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);  // dash, EX-CTRL-013
}

// Ouvre une nouvelle frame d'entrées puis traite les messages Win32 en attente.
//
// `beginFrame` fige l'état d'entrées de la frame précédente **avant** de drainer les messages,
// afin que les fronts (pressée/relâchée) se calculent sur la seule frame courante. La manette
// est sondée juste après : ses événements atterrissent dans le nouvel état courant, comme ceux
// du clavier/de la souris drainés par la boucle de messages qui suit.
void Window::pumpMessages() {
    _input.beginFrame();
    pollGamepad();

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

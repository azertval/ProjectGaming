#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

/**
 * @file HMI/Input/InputState.h
 * @brief État des entrées clavier/souris échantillonné une fois par frame.
 */

namespace hmi {

/**
 * @brief Touche du clavier, identifiée par son **code virtuel Win32**.
 *
 * Les valeurs coïncident volontairement avec les codes `VK_*` de Win32, ce qui permet à la
 * couche de capture (`Window`) de convertir directement un `WPARAM` en `Key` par un simple
 * `static_cast`, sans table de correspondance. Seules les touches utiles au menu et à la
 * navigation sont nommées ici ; l'`InputState` stocke néanmoins n'importe quel code, si bien
 * qu'ajouter une touche revient à ajouter un énumérateur (aucune autre modification).
 */
enum class Key : std::uint16_t {
    Backspace = 0x08,
    Tab = 0x09,
    Enter = 0x0D,
    Shift = 0x10,  // Maj : action de dash (`EX-CTRL-013`)
    Escape = 0x1B,
    Space = 0x20,
    Left = 0x25,
    Up = 0x26,
    Right = 0x27,
    Down = 0x28,
    A = 0x41,  // touches lettres (codes VK_*) pour les schémas ZQSD / WASD
    D = 0x44,
    Q = 0x51,
    W = 0x57,
};

/**
 * @brief Bouton de la souris suivi par l'`InputState`.
 *
 * `Count` n'est pas un bouton : c'est la sentinelle donnant le nombre de boutons suivis
 * (dimension des tableaux d'état).
 */
enum class MouseButton : std::uint8_t {
    Left = 0,
    Right = 1,
    Middle = 2,
    Count = 3,
};

/**
 * @brief État des entrées d'une frame, avec détection des fronts (pressée / relâchée).
 *
 * L'objet distingue, pour chaque touche et chaque bouton, l'état **courant** (enfoncé ou non)
 * et l'état de la **frame précédente**, ce qui permet de déduire les fronts :
 * *pressée* (« vient d'être enfoncée », `EX-CTRL-011`) et *relâchée*. Le cycle d'une frame est :
 * 1. `beginFrame()` recopie l'état courant vers l'état précédent ;
 * 2. la couche de capture (`Window`) applique les événements via `onKeyDown`/`onKeyUp`/… ;
 * 3. la logique lit `keyPressed`/`keyReleased`/`keyDown` et les équivalents souris.
 *
 * L'`InputState` est **indépendant de toute fenêtre** (aucune dépendance `<Windows.h>`) : les
 * événements peuvent être injectés directement, ce qui le rend testable en isolation
 * (`EX-NFR-010`). L'échantillonnage une fois par frame, en amont de la logique, satisfait
 * `EX-CTRL-021`.
 */
class InputState {
public:
    /// Construit un état vide (aucune touche ni bouton enfoncé, souris à l'origine).
    InputState() = default;

    /**
     * @brief Ouvre une nouvelle frame : recopie l'état courant vers l'état précédent.
     *
     * À appeler une fois par frame, avant d'appliquer les événements de la frame. Les fronts
     * (`keyPressed`, `keyReleased`, …) se calculent ensuite par comparaison courant/précédent.
     */
    void beginFrame() noexcept;

    /// Marque @p key comme enfoncée dans l'état courant.
    void onKeyDown(Key key) noexcept;

    /// Marque @p key comme relâchée dans l'état courant.
    void onKeyUp(Key key) noexcept;

    /**
     * @brief Met à jour la position de la souris.
     * @param x Abscisse en pixels de la zone client.
     * @param y Ordonnée en pixels de la zone client.
     */
    void onMouseMove(int x, int y) noexcept;

    /// Marque @p button comme enfoncé dans l'état courant.
    void onMouseButtonDown(MouseButton button) noexcept;

    /// Marque @p button comme relâché dans l'état courant.
    void onMouseButtonUp(MouseButton button) noexcept;

    /// @return true si @p key est enfoncée à cette frame (maintenue ou vient d'être pressée).
    [[nodiscard]] bool keyDown(Key key) const noexcept;

    /// @return true si @p key **vient d'être pressée** cette frame (front montant).
    [[nodiscard]] bool keyPressed(Key key) const noexcept;

    /// @return true si @p key **vient d'être relâchée** cette frame (front descendant).
    [[nodiscard]] bool keyReleased(Key key) const noexcept;

    /// @return Abscisse de la souris, en pixels de la zone client.
    [[nodiscard]] int mouseX() const noexcept;

    /// @return Ordonnée de la souris, en pixels de la zone client.
    [[nodiscard]] int mouseY() const noexcept;

    /// @return true si @p button est enfoncé à cette frame.
    [[nodiscard]] bool mouseButtonDown(MouseButton button) const noexcept;

    /// @return true si @p button **vient d'être cliqué** cette frame (front montant).
    [[nodiscard]] bool mouseButtonPressed(MouseButton button) const noexcept;

    /// @return true si @p button **vient d'être relâché** cette frame (front descendant).
    [[nodiscard]] bool mouseButtonReleased(MouseButton button) const noexcept;

private:
    /// Nombre de codes de touches suivis (couvre l'ensemble des codes virtuels Win32).
    static constexpr std::size_t KEY_COUNT = 256;

    /// Nombre de boutons de souris suivis.
    static constexpr std::size_t BUTTON_COUNT = static_cast<std::size_t>(MouseButton::Count);

    std::array<bool, KEY_COUNT> _keysCurrent{};
    std::array<bool, KEY_COUNT> _keysPrevious{};
    std::array<bool, BUTTON_COUNT> _buttonsCurrent{};
    std::array<bool, BUTTON_COUNT> _buttonsPrevious{};
    int _mouseX = 0;
    int _mouseY = 0;
};

}  // namespace hmi

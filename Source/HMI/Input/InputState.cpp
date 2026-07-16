#include "HMI/Input/InputState.h"

namespace hmi {

namespace {
/// Indice de tableau associé à une touche (son code virtuel Win32).
[[nodiscard]] std::size_t keyIndex(Key key) noexcept {
    return static_cast<std::size_t>(key);
}

/// Indice de tableau associé à un bouton de souris.
[[nodiscard]] std::size_t buttonIndex(MouseButton button) noexcept {
    return static_cast<std::size_t>(button);
}
}  // namespace

/**
 * @brief Ouvre une nouvelle frame : recopie l'état courant vers l'état précédent.
 *
 * Seul l'état *précédent* est réécrit : l'état courant (touches encore enfoncées, position
 * souris) persiste d'une frame à l'autre jusqu'au prochain événement de relâchement.
 */
void InputState::beginFrame() noexcept {
    _keysPrevious = _keysCurrent;
    _buttonsPrevious = _buttonsCurrent;
}

/**
 * @brief Marque @p key comme enfoncée dans l'état courant.
 *
 * Le code hors de la plage suivie est ignoré (garde-fou : les événements proviennent du
 * système et pourraient porter un code inattendu).
 */
void InputState::onKeyDown(Key key) noexcept {
    const std::size_t index = keyIndex(key);
    if (index < KEY_COUNT) {
        _keysCurrent[index] = true;
    }
}

/// @copydoc InputState::onKeyDown
void InputState::onKeyUp(Key key) noexcept {
    const std::size_t index = keyIndex(key);
    if (index < KEY_COUNT) {
        _keysCurrent[index] = false;
    }
}

/**
 * @brief Met à jour la position de la souris.
 * @param x Abscisse en pixels de la zone client.
 * @param y Ordonnée en pixels de la zone client.
 */
void InputState::onMouseMove(int x, int y) noexcept {
    _mouseX = x;
    _mouseY = y;
}

/// Marque @p button comme enfoncé dans l'état courant.
void InputState::onMouseButtonDown(MouseButton button) noexcept {
    const std::size_t index = buttonIndex(button);
    if (index < BUTTON_COUNT) {
        _buttonsCurrent[index] = true;
    }
}

/// Marque @p button comme relâché dans l'état courant.
void InputState::onMouseButtonUp(MouseButton button) noexcept {
    const std::size_t index = buttonIndex(button);
    if (index < BUTTON_COUNT) {
        _buttonsCurrent[index] = false;
    }
}

/// @return true si @p key est enfoncée à cette frame (maintenue ou vient d'être pressée).
bool InputState::keyDown(Key key) const noexcept {
    const std::size_t index = keyIndex(key);
    return index < KEY_COUNT && _keysCurrent[index];
}

/**
 * @brief Indique si @p key vient d'être pressée cette frame (front montant).
 * @return true si la touche est enfoncée maintenant mais ne l'était pas à la frame précédente.
 */
bool InputState::keyPressed(Key key) const noexcept {
    const std::size_t index = keyIndex(key);
    return index < KEY_COUNT && _keysCurrent[index] && !_keysPrevious[index];
}

/**
 * @brief Indique si @p key vient d'être relâchée cette frame (front descendant).
 * @return true si la touche n'est plus enfoncée mais l'était à la frame précédente.
 */
bool InputState::keyReleased(Key key) const noexcept {
    const std::size_t index = keyIndex(key);
    return index < KEY_COUNT && !_keysCurrent[index] && _keysPrevious[index];
}

/// @return Abscisse de la souris, en pixels de la zone client.
int InputState::mouseX() const noexcept {
    return _mouseX;
}

/// @return Ordonnée de la souris, en pixels de la zone client.
int InputState::mouseY() const noexcept {
    return _mouseY;
}

/// @return true si @p button est enfoncé à cette frame.
bool InputState::mouseButtonDown(MouseButton button) const noexcept {
    const std::size_t index = buttonIndex(button);
    return index < BUTTON_COUNT && _buttonsCurrent[index];
}

/**
 * @brief Indique si @p button vient d'être cliqué cette frame (front montant).
 * @return true si le bouton est enfoncé maintenant mais ne l'était pas à la frame précédente.
 */
bool InputState::mouseButtonPressed(MouseButton button) const noexcept {
    const std::size_t index = buttonIndex(button);
    return index < BUTTON_COUNT && _buttonsCurrent[index] && !_buttonsPrevious[index];
}

/**
 * @brief Indique si @p button vient d'être relâché cette frame (front descendant).
 * @return true si le bouton n'est plus enfoncé mais l'était à la frame précédente.
 */
bool InputState::mouseButtonReleased(MouseButton button) const noexcept {
    const std::size_t index = buttonIndex(button);
    return index < BUTTON_COUNT && !_buttonsCurrent[index] && _buttonsPrevious[index];
}

}  // namespace hmi

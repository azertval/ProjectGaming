#include "HMI/Editor/TextInputField.h"

#include "HMI/Input/InputState.h"

namespace hmi {

namespace {

// Encode un caractère (point de code Unicode issu d'une unité UTF-16 hors plage des substituts,
// suffisant pour un nom de niveau en français) en UTF-8, ajouté à `text`.
void appendUtf8(std::string& text, wchar_t character) {
    const unsigned int codePoint = static_cast<unsigned int>(character);
    if (codePoint < 0x80) {
        text.push_back(static_cast<char>(codePoint));
    } else if (codePoint < 0x800) {
        text.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
        text.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else {
        text.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
        text.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        text.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    }
}

// Retire le dernier caractère UTF-8 de `text` (potentiellement plusieurs octets, ex. un accent
// composé) : recule tant qu'on reste sur un octet de continuation (10xxxxxx), sans effet si vide.
void removeLastUtf8Character(std::string& text) {
    if (text.empty()) {
        return;
    }
    std::size_t index = text.size() - 1;
    while (index > 0 && (static_cast<unsigned char>(text[index]) & 0xC0) == 0x80) {
        --index;
    }
    text.erase(index);
}

}  // namespace

TextInputField::TextInputField(std::string initialText, Validator validator)
    : _text(std::move(initialText)), _validator(std::move(validator)) {}

void TextInputField::update(const InputState& input) {
    if (_confirmed || _cancelled) {
        return;
    }

    bool edited = false;
    for (const wchar_t character : input.typedCharacters()) {
        // Ignore les caractères de contrôle (Entrée/Tab/Échap arrivent aussi via WM_CHAR selon le
        // clavier ; ils sont traités explicitement ci-dessous, pas comme du texte).
        if (character >= 0x20 && character != 0x7F) {
            appendUtf8(_text, character);
            edited = true;
        }
    }
    if (input.keyPressed(Key::Backspace) && !_text.empty()) {
        removeLastUtf8Character(_text);
        edited = true;
    }
    if (edited) {
        _rejected = false;  // une nouvelle saisie rend un refus precedent obsolete
    }

    if (input.keyPressed(Key::Enter)) {
        if (!_validator || _validator(_text)) {
            _confirmed = true;
        } else {
            _rejected = true;
        }
    } else if (input.keyPressed(Key::Escape)) {
        _cancelled = true;
    }
}

}  // namespace hmi

#include "HMI/Graphics/BitmapFont.h"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/SpriteBatch.h"

namespace hmi {

namespace {

/// Nombre de colonnes et de lignes de pixels d'un glyphe de base (le corps de la lettre).
constexpr int GLYPH_COLUMNS = 5;
constexpr int GLYPH_ROWS = 7;

/// Ligne de la cellule où commence le corps du glyphe (au-dessus : zone d'accents).
constexpr int BODY_TOP = 2;

/// Nombre de cellules par ligne dans la texture de police.
constexpr int COLUMNS = 16;

/// Premier et dernier code ASCII imprimable couverts par la table de base.
constexpr char32_t ASCII_FIRST = 0x20;  // espace
constexpr char32_t ASCII_LAST = 0x7E;   // ~

/// Motif d'un glyphe : 7 lignes de 5 pixels (bit 4 = colonne de gauche).
using Pattern = std::array<std::uint8_t, GLYPH_ROWS>;

/**
 * @brief Table des glyphes ASCII imprimables (0x20 à 0x7E), un motif 5×7 par caractère.
 *
 * Chaque octet code une ligne : les 5 bits de poids faible sont les colonnes, le bit 4
 * (valeur 16) étant la colonne de gauche. Les formes sont volontairement simples (police
 * pixel art « from scratch ») ; elles restent lisibles à petite taille.
 */
constexpr std::array<Pattern, (ASCII_LAST - ASCII_FIRST) + 1> ASCII_FONT = {{
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},  // ' '
    {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100},  // '!'
    {0b01010, 0b01010, 0b01010, 0b00000, 0b00000, 0b00000, 0b00000},  // '"'
    {0b01010, 0b11111, 0b01010, 0b01010, 0b11111, 0b01010, 0b00000},  // '#'
    {0b00100, 0b01111, 0b10100, 0b01110, 0b00101, 0b11110, 0b00100},  // '$'
    {0b11001, 0b11010, 0b00010, 0b00100, 0b01000, 0b01011, 0b10011},  // '%'
    {0b01100, 0b10010, 0b10100, 0b01000, 0b10101, 0b10010, 0b01101},  // '&'
    {0b00100, 0b00100, 0b01000, 0b00000, 0b00000, 0b00000, 0b00000},  // '\''
    {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010},  // '('
    {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000},  // ')'
    {0b00000, 0b00100, 0b10101, 0b01110, 0b10101, 0b00100, 0b00000},  // '*'
    {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000},  // '+'
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00100, 0b00100, 0b01000},  // ','
    {0b00000, 0b00000, 0b00000, 0b01110, 0b00000, 0b00000, 0b00000},  // '-'
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00110, 0b00110},  // '.'
    {0b00001, 0b00010, 0b00010, 0b00100, 0b01000, 0b01000, 0b10000},  // '/'
    {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110},  // '0'
    {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},  // '1'
    {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111},  // '2'
    {0b11111, 0b00010, 0b00100, 0b00010, 0b00001, 0b10001, 0b01110},  // '3'
    {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010},  // '4'
    {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110},  // '5'
    {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110},  // '6'
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000},  // '7'
    {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110},  // '8'
    {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100},  // '9'
    {0b00000, 0b00110, 0b00110, 0b00000, 0b00110, 0b00110, 0b00000},  // ':'
    {0b00000, 0b00110, 0b00110, 0b00000, 0b00110, 0b00100, 0b01000},  // ';'
    {0b00010, 0b00100, 0b01000, 0b10000, 0b01000, 0b00100, 0b00010},  // '<'
    {0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000},  // '='
    {0b01000, 0b00100, 0b00010, 0b00001, 0b00010, 0b00100, 0b01000},  // '>'
    {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100},  // '?'
    {0b01110, 0b10001, 0b10111, 0b10101, 0b10111, 0b10000, 0b01110},  // '@'
    {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},  // 'A'
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110},  // 'B'
    {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110},  // 'C'
    {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110},  // 'D'
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111},  // 'E'
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000},  // 'F'
    {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111},  // 'G'
    {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},  // 'H'
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111},  // 'I'
    {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100},  // 'J'
    {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001},  // 'K'
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111},  // 'L'
    {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001},  // 'M'
    {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001},  // 'N'
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},  // 'O'
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000},  // 'P'
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101},  // 'Q'
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001},  // 'R'
    {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110},  // 'S'
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},  // 'T'
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},  // 'U'
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100},  // 'V'
    {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001},  // 'W'
    {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001},  // 'X'
    {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100},  // 'Y'
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111},  // 'Z'
    {0b01110, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01110},  // '['
    {0b10000, 0b01000, 0b01000, 0b00100, 0b00010, 0b00010, 0b00001},  // '\\'
    {0b01110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01110},  // ']'
    {0b00100, 0b01010, 0b10001, 0b00000, 0b00000, 0b00000, 0b00000},  // '^'
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111},  // '_'
    {0b01000, 0b00100, 0b00010, 0b00000, 0b00000, 0b00000, 0b00000},  // '`'
    {0b00000, 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111},  // 'a'
    {0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b11110},  // 'b'
    {0b00000, 0b00000, 0b01110, 0b10001, 0b10000, 0b10001, 0b01110},  // 'c'
    {0b00001, 0b00001, 0b01101, 0b10011, 0b10001, 0b10001, 0b01111},  // 'd'
    {0b00000, 0b00000, 0b01110, 0b10001, 0b11111, 0b10000, 0b01110},  // 'e'
    {0b00110, 0b01001, 0b01000, 0b11100, 0b01000, 0b01000, 0b01000},  // 'f'
    {0b00000, 0b00000, 0b01111, 0b10001, 0b01111, 0b00001, 0b01110},  // 'g'
    {0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001},  // 'h'
    {0b00100, 0b00000, 0b01100, 0b00100, 0b00100, 0b00100, 0b01110},  // 'i'
    {0b00010, 0b00000, 0b00110, 0b00010, 0b00010, 0b10010, 0b01100},  // 'j'
    {0b10000, 0b10000, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010},  // 'k'
    {0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},  // 'l'
    {0b00000, 0b00000, 0b11010, 0b10101, 0b10101, 0b10101, 0b10101},  // 'm'
    {0b00000, 0b00000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001},  // 'n'
    {0b00000, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110},  // 'o'
    {0b00000, 0b00000, 0b11110, 0b10001, 0b11110, 0b10000, 0b10000},  // 'p'
    {0b00000, 0b00000, 0b01111, 0b10001, 0b01111, 0b00001, 0b00001},  // 'q'
    {0b00000, 0b00000, 0b10110, 0b11001, 0b10000, 0b10000, 0b10000},  // 'r'
    {0b00000, 0b00000, 0b01111, 0b10000, 0b01110, 0b00001, 0b11110},  // 's'
    {0b01000, 0b01000, 0b11100, 0b01000, 0b01000, 0b01001, 0b00110},  // 't'
    {0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b10011, 0b01101},  // 'u'
    {0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100},  // 'v'
    {0b00000, 0b00000, 0b10001, 0b10001, 0b10101, 0b10101, 0b01010},  // 'w'
    {0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001},  // 'x'
    {0b00000, 0b00000, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110},  // 'y'
    {0b00000, 0b00000, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111},  // 'z'
    {0b00110, 0b00100, 0b00100, 0b01000, 0b00100, 0b00100, 0b00110},  // '{'
    {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},  // '|'
    {0b01100, 0b00100, 0b00100, 0b00010, 0b00100, 0b00100, 0b01100},  // '}'
    {0b00000, 0b00000, 0b01000, 0b10101, 0b00010, 0b00000, 0b00000},  // '~'
}};

/// Signe diacritique appliqué à une lettre de base pour former une lettre accentuée.
enum class Accent { Acute, Grave, Circumflex, Diaeresis, Cedilla };

/// Motif d'un accent supérieur (2 lignes de 5 pixels), placé au-dessus du corps.
using TopAccent = std::array<std::uint8_t, 2>;

/// @return Le motif (2 lignes) de l'accent supérieur @p accent.
[[nodiscard]] TopAccent topAccentPattern(Accent accent) noexcept {
    switch (accent) {
        case Accent::Acute:
            return {0b00010, 0b00100};
        case Accent::Grave:
            return {0b01000, 0b00100};
        case Accent::Circumflex:
            return {0b00100, 0b01010};
        case Accent::Diaeresis:
            return {0b01010, 0b00000};
        case Accent::Cedilla:
            break;
    }
    return {0b00000, 0b00000};
}

/// Une lettre accentuée : son code point, la lettre ASCII de base et le diacritique.
struct AccentedGlyph {
    char32_t codePoint;
    char base;
    Accent accent;
};

/// Lettres accentuées françaises couvertes, composées à partir des lettres de base.
///
/// Les code points sont écrits en valeur numérique (et non en littéral `U'é'`) pour être
/// indépendants de l'encodage du fichier source : le compilateur lit ce fichier en page de
/// code système, ce qui rendrait un littéral accentué ambigu.
constexpr std::array<AccentedGlyph, 21> ACCENTED_GLYPHS = {{
    {0x00E0, 'a', Accent::Grave},       // à
    {0x00E2, 'a', Accent::Circumflex},  // â
    {0x00E4, 'a', Accent::Diaeresis},   // ä
    {0x00E7, 'c', Accent::Cedilla},     // ç
    {0x00E9, 'e', Accent::Acute},       // é
    {0x00E8, 'e', Accent::Grave},       // è
    {0x00EA, 'e', Accent::Circumflex},  // ê
    {0x00EB, 'e', Accent::Diaeresis},   // ë
    {0x00EE, 'i', Accent::Circumflex},  // î
    {0x00EF, 'i', Accent::Diaeresis},   // ï
    {0x00F4, 'o', Accent::Circumflex},  // ô
    {0x00F6, 'o', Accent::Diaeresis},   // ö
    {0x00F9, 'u', Accent::Grave},       // ù
    {0x00FB, 'u', Accent::Circumflex},  // û
    {0x00FC, 'u', Accent::Diaeresis},   // ü
    {0x00C0, 'A', Accent::Grave},       // À
    {0x00C2, 'A', Accent::Circumflex},  // Â
    {0x00C7, 'C', Accent::Cedilla},     // Ç
    {0x00C9, 'E', Accent::Acute},       // É
    {0x00C8, 'E', Accent::Grave},       // È
    {0x00CA, 'E', Accent::Circumflex},  // Ê
}};

/// Assemble une couleur RVBA (octets) en pixel `R8G8B8A8_UNORM` (ordre mémoire R,G,B,A).
[[nodiscard]] std::uint32_t pack(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
                                 std::uint8_t alpha) noexcept {
    return static_cast<std::uint32_t>(red) | (static_cast<std::uint32_t>(green) << 8) |
           (static_cast<std::uint32_t>(blue) << 16) | (static_cast<std::uint32_t>(alpha) << 24);
}

/// Pixel blanc opaque : couleur d'un pixel « allumé » d'un glyphe (colorable par teinte).
constexpr std::uint32_t GLYPH_PIXEL = 0xFFFFFFFFu;

/// Décode le prochain code point UTF-8 de @p text à partir de @p index (avancé en sortie).
[[nodiscard]] char32_t nextCodePoint(std::string_view text, std::size_t& index) noexcept {
    const auto lead = static_cast<unsigned char>(text[index]);
    if (lead < 0x80) {
        index += 1;
        return lead;
    }

    int continuationBytes = 0;
    char32_t codePoint = 0;
    if ((lead & 0xE0) == 0xC0) {
        continuationBytes = 1;
        codePoint = lead & 0x1Fu;
    } else if ((lead & 0xF0) == 0xE0) {
        continuationBytes = 2;
        codePoint = lead & 0x0Fu;
    } else if ((lead & 0xF8) == 0xF0) {
        continuationBytes = 3;
        codePoint = lead & 0x07u;
    } else {
        index += 1;  // octet de tête invalide : on avance d'un octet
        return 0xFFFD;
    }

    for (int byte = 1; byte <= continuationBytes; ++byte) {
        if (index + static_cast<std::size_t>(byte) >= text.size()) {
            index = text.size();
            return 0xFFFD;
        }
        const auto continuation = static_cast<unsigned char>(text[index + byte]);
        if ((continuation & 0xC0) != 0x80) {
            index += 1;  // séquence tronquée : on avance d'un octet et on signale
            return 0xFFFD;
        }
        codePoint = (codePoint << 6) | (continuation & 0x3Fu);
    }
    index += static_cast<std::size_t>(continuationBytes) + 1;
    return codePoint;
}

}  // namespace

/**
 * @brief Génère la texture de police et crée la ressource Direct3D associée.
 *
 * La texture est une grille de cellules : d'abord les caractères ASCII imprimables, puis les
 * lettres accentuées composées (corps de la lettre de base + diacritique). Chaque pixel
 * « allumé » est blanc opaque ; le reste est transparent, pour que la teinte de `drawText`
 * colore le texte par simple multiplication dans le nuanceur.
 */
BitmapFont::BitmapFont(ID3D11Device* device) {
    const int asciiCount = static_cast<int>(ASCII_FONT.size());
    const int glyphCount = asciiCount + static_cast<int>(ACCENTED_GLYPHS.size());

    _columns = COLUMNS;
    const int rows = (glyphCount + _columns - 1) / _columns;
    _textureWidth = _columns * CELL_WIDTH;
    _textureHeight = rows * CELL_HEIGHT;

    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(_textureWidth) *
                                      static_cast<std::size_t>(_textureHeight));

    // Dessine le corps 5×7 d'une lettre dans la cellule d'indice donné.
    const auto blitBody = [&](int cellIndex, const Pattern& pattern) {
        const int originX = (cellIndex % _columns) * CELL_WIDTH;
        const int originY = (cellIndex / _columns) * CELL_HEIGHT;
        for (int row = 0; row < GLYPH_ROWS; ++row) {
            for (int column = 0; column < GLYPH_COLUMNS; ++column) {
                const bool lit = (pattern[static_cast<std::size_t>(row)] &
                                  (1u << (GLYPH_COLUMNS - 1 - column))) != 0;
                if (lit) {
                    const int x = originX + column;
                    const int y = originY + BODY_TOP + row;
                    pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(_textureWidth) +
                           static_cast<std::size_t>(x)] = GLYPH_PIXEL;
                }
            }
        }
    };

    // Ajoute un accent supérieur (2 lignes) au-dessus du corps, dans la même cellule.
    const auto blitTopAccent = [&](int cellIndex, const TopAccent& accent) {
        const int originX = (cellIndex % _columns) * CELL_WIDTH;
        const int originY = (cellIndex / _columns) * CELL_HEIGHT;
        for (int row = 0; row < static_cast<int>(accent.size()); ++row) {
            for (int column = 0; column < GLYPH_COLUMNS; ++column) {
                const bool lit = (accent[static_cast<std::size_t>(row)] &
                                  (1u << (GLYPH_COLUMNS - 1 - column))) != 0;
                if (lit) {
                    const int x = originX + column;
                    const int y = originY + row;
                    pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(_textureWidth) +
                           static_cast<std::size_t>(x)] = GLYPH_PIXEL;
                }
            }
        }
    };

    // Ajoute une cédille sous le corps (dernière ligne de la cellule).
    const auto blitCedilla = [&](int cellIndex) {
        const int originX = (cellIndex % _columns) * CELL_WIDTH;
        const int originY = (cellIndex / _columns) * CELL_HEIGHT;
        constexpr std::uint8_t cedilla = 0b01100;  // petit crochet sous la lettre
        for (int column = 0; column < GLYPH_COLUMNS; ++column) {
            const bool lit = (cedilla & (1u << (GLYPH_COLUMNS - 1 - column))) != 0;
            if (lit) {
                const int x = originX + column;
                const int y = originY + CELL_HEIGHT - 1;
                pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(_textureWidth) +
                       static_cast<std::size_t>(x)] = GLYPH_PIXEL;
            }
        }
    };

    // Cellules ASCII imprimables.
    for (int glyph = 0; glyph < asciiCount; ++glyph) {
        blitBody(glyph, ASCII_FONT[static_cast<std::size_t>(glyph)]);
        _cellIndex[ASCII_FIRST + static_cast<char32_t>(glyph)] = glyph;
    }

    // Cellules des lettres accentuées, composées à partir de la lettre de base.
    for (int accented = 0; accented < static_cast<int>(ACCENTED_GLYPHS.size()); ++accented) {
        const AccentedGlyph& entry = ACCENTED_GLYPHS[static_cast<std::size_t>(accented)];
        const int cellIndex = asciiCount + accented;
        blitBody(cellIndex, ASCII_FONT[static_cast<std::size_t>(entry.base - ASCII_FIRST)]);
        if (entry.accent == Accent::Cedilla) {
            blitCedilla(cellIndex);
        } else {
            blitTopAccent(cellIndex, topAccentPattern(entry.accent));
        }
        _cellIndex[entry.codePoint] = cellIndex;
    }

    // Alias : espaces et signes typographiques ramenes a un glyphe couvert (pas de plantage).
    // Code points numeriques pour rester independant de l'encodage du fichier source.
    _cellIndex[0x00A0] = _cellIndex[U' '];   // espace insecable -> espace
    _cellIndex[0x2013] = _cellIndex[U'-'];   // tiret demi-cadratin -> trait d'union
    _cellIndex[0x2014] = _cellIndex[U'-'];   // tiret cadratin -> trait d'union
    _cellIndex[0x2018] = _cellIndex[U'\''];  // guillemet simple ouvrant -> apostrophe
    _cellIndex[0x2019] = _cellIndex[U'\''];  // guillemet simple fermant -> apostrophe

    D3D11_TEXTURE2D_DESC description{};
    description.Width = static_cast<UINT>(_textureWidth);
    description.Height = static_cast<UINT>(_textureHeight);
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = pixels.data();
    data.SysMemPitch = static_cast<UINT>(_textureWidth) * sizeof(std::uint32_t);

    if (FAILED(device->CreateTexture2D(&description, &data, &_texture))) {
        throw std::runtime_error("Echec de creation de la texture de police");
    }
    if (FAILED(device->CreateShaderResourceView(_texture.Get(), nullptr, &_view))) {
        throw std::runtime_error("Echec de creation de la vue de la texture de police");
    }
    GRAPHICS_LOG_TRACE("BitmapFont : police generee (" + std::to_string(glyphCount) + " glyphes)");
}

/**
 * @brief Indice de cellule couvrant @p codePoint, ou -1 si non couvert.
 */
int BitmapFont::cellForCodePoint(char32_t codePoint) const {
    const auto found = _cellIndex.find(codePoint);
    return found == _cellIndex.end() ? -1 : found->second;
}

/// @brief Dessine une chaîne UTF-8 dans le lot courant, en espace écran (pixels).
void BitmapFont::drawText(SpriteBatch& batch, std::string_view text, float x, float y, float scale,
                          const core::Color& color) const {
    const float advance = static_cast<float>(CELL_WIDTH) * scale;
    float penX = x;
    float penY = y;

    std::size_t index = 0;
    while (index < text.size()) {
        const char32_t codePoint = nextCodePoint(text, index);

        // Le saut de ligne repart à l'abscisse de départ, une ligne plus bas.
        if (codePoint == U'\n') {
            penX = x;
            penY += static_cast<float>(CELL_HEIGHT) * scale;
            continue;
        }

        const int cell = cellForCodePoint(codePoint);
        if (cell >= 0) {
            const int cellColumn = cell % _columns;
            const int cellRow = cell / _columns;

            SpriteQuad quad;
            quad.x = penX;
            quad.y = penY;
            quad.width = static_cast<float>(CELL_WIDTH) * scale;
            quad.height = static_cast<float>(CELL_HEIGHT) * scale;
            quad.u0 = static_cast<float>(cellColumn * CELL_WIDTH) /
                      static_cast<float>(_textureWidth);
            quad.v0 = static_cast<float>(cellRow * CELL_HEIGHT) /
                      static_cast<float>(_textureHeight);
            quad.u1 = static_cast<float>((cellColumn + 1) * CELL_WIDTH) /
                      static_cast<float>(_textureWidth);
            quad.v1 = static_cast<float>((cellRow + 1) * CELL_HEIGHT) /
                      static_cast<float>(_textureHeight);
            quad.r = color.r;
            quad.g = color.g;
            quad.b = color.b;
            quad.a = color.a;
            batch.draw(quad);
        }

        // Chasse fixe : un caractère non couvert avance comme une espace (sans dessin).
        penX += advance;
    }
}

/**
 * @brief Largeur en pixels qu'occuperait @p text à l'échelle @p scale (chasse fixe).
 */
float BitmapFont::textWidth(std::string_view text, float scale) const {
    int glyphs = 0;
    std::size_t index = 0;
    while (index < text.size()) {
        const char32_t codePoint = nextCodePoint(text, index);
        if (codePoint != U'\n') {
            ++glyphs;
        }
    }
    return static_cast<float>(glyphs * CELL_WIDTH) * scale;
}

/// @return Hauteur d'une ligne de texte à l'échelle @p scale, en pixels.
float BitmapFont::lineHeight(float scale) const noexcept {
    return static_cast<float>(CELL_HEIGHT) * scale;
}

/**
 * @brief Construit une projection orthographique espace écran → clip.
 *
 * Le nuanceur applique `mul(float4(position, 0, 1), projection)` (vecteur-ligne) : la matrice
 * transforme une position en pixels en coordonnées de clip `[-1, 1]`, l'axe Y étant inversé
 * pour que l'ordonnée croisse vers le bas de l'écran.
 */
DirectX::XMFLOAT4X4 BitmapFont::screenProjection(int viewportWidth, int viewportHeight) noexcept {
    const float width = viewportWidth > 0 ? static_cast<float>(viewportWidth) : 1.0f;
    const float height = viewportHeight > 0 ? static_cast<float>(viewportHeight) : 1.0f;

    DirectX::XMFLOAT4X4 projection{};
    projection._11 = 2.0f / width;
    projection._22 = -2.0f / height;
    projection._33 = 1.0f;
    projection._41 = -1.0f;
    projection._42 = 1.0f;
    projection._44 = 1.0f;
    return projection;
}

}  // namespace hmi

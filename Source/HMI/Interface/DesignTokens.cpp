#include "HMI/Interface/DesignTokens.h"

#include <array>
#include <cstdio>

namespace hmi {

namespace {

// Échelles partagées par les deux portées (TACHE-01) : seules les couleurs diffèrent entre elles.
constexpr SpacingTokens SHARED_SPACING{};

[[nodiscard]] TypographyTokens sharedTypography() noexcept {
    TypographyTokens typography;
    typography.screenTitle = TypographyLevel{22, 700};   // QFont::Bold
    typography.sectionTitle = TypographyLevel{13, 700};
    typography.body = TypographyLevel{10, 400};           // QFont::Normal
    typography.caption = TypographyLevel{9, 400};
    typography.monospaceBody = TypographyLevel{22, 700};
    return typography;
}

constexpr SizeTokens SHARED_SIZE{};

// Identité du jeu (menu principal, écran Options, jeu) : reprend les couleurs du thème historique
// (fond sombre, accent ambre), désormais nommées par rôle plutôt qu'éparpillées en littéraux.
[[nodiscard]] DesignTokens buildIdentityTokens() noexcept {
    DesignTokens tokens;
    tokens.color.background = DesignColor{0x1a, 0x1f, 0x29};
    tokens.color.surface = DesignColor{0x1e, 0x25, 0x31};
    tokens.color.surfaceAlt = DesignColor{0x23, 0x2a, 0x36};
    tokens.color.border = DesignColor{0x33, 0x3a, 0x48};
    tokens.color.text = DesignColor{0xf2, 0xf2, 0xff};
    tokens.color.textMuted = DesignColor{0xb3, 0xb8, 0xc7};
    tokens.color.accent = DesignColor{0xff, 0xd1, 0x33};
    tokens.color.accentHover = DesignColor{0xff, 0xdb, 0x5c};
    tokens.color.error = DesignColor{0xff, 0x5c, 0x5c};
    tokens.spacing = SHARED_SPACING;
    tokens.typography = sharedTypography();
    tokens.size = SHARED_SIZE;
    return tokens;
}

// Châssis d'édition, thème sombre (variable, TACHE-06 lui ajoute un jeu clair) : palette neutre
// proche de l'identité, même accent ambre pour la cohérence visuelle de l'ensemble (critère
// d'acceptation 1 du lot : une seule apparence, aucune couture entre les deux portées).
[[nodiscard]] DesignTokens buildEditorDarkTokens() noexcept {
    DesignTokens tokens;
    tokens.color.background = DesignColor{0x1e, 0x22, 0x2b};
    tokens.color.surface = DesignColor{0x26, 0x2b, 0x36};
    tokens.color.surfaceAlt = DesignColor{0x2d, 0x33, 0x40};
    tokens.color.border = DesignColor{0x3a, 0x41, 0x50};
    tokens.color.text = DesignColor{0xe6, 0xe8, 0xee};
    tokens.color.textMuted = DesignColor{0x9a, 0xa1, 0xb0};
    tokens.color.accent = DesignColor{0xff, 0xd1, 0x33};
    tokens.color.accentHover = DesignColor{0xff, 0xdb, 0x5c};
    tokens.color.error = DesignColor{0xff, 0x6b, 0x6b};
    tokens.spacing = SHARED_SPACING;
    tokens.typography = sharedTypography();
    tokens.size = SHARED_SIZE;
    return tokens;
}

}  // namespace

const DesignTokens& identityTokens() noexcept {
    static const DesignTokens tokens = buildIdentityTokens();
    return tokens;
}

const DesignTokens& editorDarkTokens() noexcept {
    static const DesignTokens tokens = buildEditorDarkTokens();
    return tokens;
}

DesignColor viewportClearColor(bool editorMode) noexcept {
    return editorMode ? editorDarkTokens().color.background : identityTokens().color.background;
}

std::string toCssColor(DesignColor color) {
    std::array<char, 8> buffer{};
    std::snprintf(buffer.data(), buffer.size(), "#%02x%02x%02x", color.r, color.g, color.b);
    return std::string(buffer.data());
}

std::string toCssRgba(DesignColor color) {
    return "rgba(" + std::to_string(color.r) + ", " + std::to_string(color.g) + ", " +
           std::to_string(color.b) + ", " + std::to_string(color.a) + ")";
}

}  // namespace hmi

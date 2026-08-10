#include "HMI/Interface/DesignTokens.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace hmi {

namespace {

// Échelles partagées par les deux portées (TACHE-01) : seules les couleurs diffèrent entre elles.
constexpr SpacingTokens SHARED_SPACING{};

[[nodiscard]] TypographyTokens sharedTypography() noexcept {
    TypographyTokens typography;
    typography.screenTitle = TypographyLevel{32, 700};  // QFont::Bold -- titre du menu/des Options.
    typography.sectionTitle =
        TypographyLevel{16, 700};                // Boutons de navigation, titres de panneau.
    typography.body = TypographyLevel{10, 400};  // QFont::Normal
    typography.caption = TypographyLevel{9, 400};
    typography.monospaceBody = TypographyLevel{10, 400};
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

// Chassis d'edition, theme CLAIR (LOT-56 TACHE-06) : pas le sombre invers -- ecarts de luminosite
// entre fond/surface/bordure/lignes alternees choisis independamment, accent assombri (l'ambre vif
// du theme sombre manque de contraste utilise comme texte sur un fond clair).
[[nodiscard]] DesignTokens buildEditorLightTokens() noexcept {
    DesignTokens tokens;
    tokens.color.background = DesignColor{0xf5, 0xf6, 0xf8};
    tokens.color.surface = DesignColor{0xff, 0xff, 0xff};
    tokens.color.surfaceAlt = DesignColor{0xec, 0xee, 0xf2};
    tokens.color.border = DesignColor{0xc9, 0xce, 0xd6};
    tokens.color.text = DesignColor{0x1c, 0x21, 0x28};
    tokens.color.textMuted = DesignColor{0x5b, 0x64, 0x72};
    tokens.color.accent = DesignColor{0xb3, 0x6b, 0x00};
    tokens.color.accentHover = DesignColor{0xcc, 0x7a, 0x00};
    tokens.color.error = DesignColor{0xc0, 0x26, 0x26};
    tokens.spacing = SHARED_SPACING;
    tokens.typography = sharedTypography();
    tokens.size = SHARED_SIZE;
    return tokens;
}

// Ajoute les neuf roles de couleur d'une portee au tableau de substitution, sous le prefixe donne
// (ex. "identity.color.background" -> "#1a1f29").
void addColorValues(std::unordered_map<std::string, std::string>& values, const std::string& prefix,
                    const ColorTokens& color) {
    values[prefix + ".background"] = toCssColor(color.background);
    values[prefix + ".surface"] = toCssColor(color.surface);
    values[prefix + ".surfaceAlt"] = toCssColor(color.surfaceAlt);
    values[prefix + ".border"] = toCssColor(color.border);
    values[prefix + ".text"] = toCssColor(color.text);
    values[prefix + ".textMuted"] = toCssColor(color.textMuted);
    values[prefix + ".accent"] = toCssColor(color.accent);
    values[prefix + ".accentHover"] = toCssColor(color.accentHover);
    values[prefix + ".error"] = toCssColor(color.error);
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

const DesignTokens& editorLightTokens() noexcept {
    static const DesignTokens tokens = buildEditorLightTokens();
    return tokens;
}

DesignColor viewportClearColor(bool editorMode, const DesignTokens& activeEditorTokens) noexcept {
    return editorMode ? activeEditorTokens.color.background : identityTokens().color.background;
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

namespace {

// Composante -> canal lineaire WCAG (gamma sRGB inverse).
[[nodiscard]] double linearChannel(std::uint8_t component) noexcept {
    const double c = static_cast<double>(component) / 255.0;
    return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

}  // namespace

double relativeLuminance(DesignColor color) noexcept {
    return 0.2126 * linearChannel(color.r) + 0.7152 * linearChannel(color.g) +
           0.0722 * linearChannel(color.b);
}

double contrastRatio(DesignColor a, DesignColor b) noexcept {
    const double la = relativeLuminance(a);
    const double lb = relativeLuminance(b);
    const double lighter = std::max(la, lb);
    const double darker = std::min(la, lb);
    return (lighter + 0.05) / (darker + 0.05);
}

std::unordered_map<std::string, std::string> buildStyleSheetValues(
    const DesignTokens& editorTokens) {
    std::unordered_map<std::string, std::string> values;
    addColorValues(values, "identity.color", identityTokens().color);
    addColorValues(values, "editor.color", editorTokens.color);
    // Echelle d'espacement : partagee entre les deux portees (DesignTokensTest.
    // LesDeuxPorteesPartagentLesMemesEchelles), la source choisie ici est arbitraire.
    const SpacingTokens& spacing = identityTokens().spacing;
    values["tokens.spacing.extraSmall"] = std::to_string(spacing.extraSmall);
    values["tokens.spacing.small"] = std::to_string(spacing.small);
    values["tokens.spacing.medium"] = std::to_string(spacing.medium);
    values["tokens.spacing.large"] = std::to_string(spacing.large);
    values["tokens.spacing.extraLarge"] = std::to_string(spacing.extraLarge);
    // Echelle typographique (LOT-56 TACHE-03) : elle aussi partagee entre les deux portees.
    const TypographyTokens& typography = identityTokens().typography;
    values["tokens.typography.screenTitle.pointSize"] =
        std::to_string(typography.screenTitle.pointSize);
    values["tokens.typography.sectionTitle.pointSize"] =
        std::to_string(typography.sectionTitle.pointSize);
    return values;
}

}  // namespace hmi

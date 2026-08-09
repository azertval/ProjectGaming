#include "HMI/Interface/ApplicationTheme.h"

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QString>
#include <QStyle>
#include <QStyleFactory>

#include "HMI/HmiLog.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/StyleSheetTemplate.h"

namespace hmi {

namespace {

[[nodiscard]] QColor toQColor(DesignColor color) {
    return QColor(color.r, color.g, color.b, color.a);
}

// Ajoute les neuf roles de couleur d'une portee au tableau de substitution, sous le prefixe donne
// (ex. "identity.color.background" -> "#1a1f29").
void addColorValues(std::unordered_map<std::string, std::string>& values,
                    const std::string& prefix, const ColorTokens& color) {
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

// Applique les rôles communs aux trois groupes de palette pour un jeu de jetons donné.
void setCommonRoles(QPalette& palette, QPalette::ColorGroup group, const ColorTokens& color) {
    palette.setColor(group, QPalette::Window, toQColor(color.background));
    palette.setColor(group, QPalette::WindowText, toQColor(color.text));
    palette.setColor(group, QPalette::Base, toQColor(color.surface));
    palette.setColor(group, QPalette::AlternateBase, toQColor(color.surfaceAlt));
    palette.setColor(group, QPalette::ToolTipBase, toQColor(color.surface));
    palette.setColor(group, QPalette::ToolTipText, toQColor(color.text));
    palette.setColor(group, QPalette::Text, toQColor(color.text));
    palette.setColor(group, QPalette::Button, toQColor(color.surface));
    palette.setColor(group, QPalette::ButtonText, toQColor(color.text));
    palette.setColor(group, QPalette::BrightText, toQColor(color.error));
    palette.setColor(group, QPalette::Highlight, toQColor(color.accent));
    // Texte de sélection lu sur l'accent (clair) : le fond, sombre dans les deux portées, contraste
    // mieux que le texte principal (lui aussi clair).
    palette.setColor(group, QPalette::HighlightedText, toQColor(color.background));
    palette.setColor(group, QPalette::Link, toQColor(color.accent));
    palette.setColor(group, QPalette::PlaceholderText, toQColor(color.textMuted));
}

}  // namespace

void applyApplicationStyle() {
    // Fusion : seul style Qt fourni sur toutes les plate-formes qui honore intégralement une
    // QPalette et une feuille de style personnalisées -- les styles natifs dessinent une partie de
    // leurs contrôles via l'API du système et ignorent le reste (cf. epic.md, constat du lot).
    if (QStyle* const style = QStyleFactory::create(QStringLiteral("Fusion"))) {
        QApplication::setStyle(style);
    }
}

QPalette buildApplicationPalette(const DesignTokens& tokens) {
    QPalette palette;
    setCommonRoles(palette, QPalette::Active, tokens.color);
    setCommonRoles(palette, QPalette::Inactive, tokens.color);

    // Groupe désactivé : texte ramené au jeton atténué sur les trois rôles de texte, faute de quoi
    // Qt se rabat sur un gris calculé automatiquement, jamais garanti lisible sur le fond posé ici.
    setCommonRoles(palette, QPalette::Disabled, tokens.color);
    const QColor muted = toQColor(tokens.color.textMuted);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, muted);
    palette.setColor(QPalette::Disabled, QPalette::Text, muted);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, muted);

    return palette;
}

std::unordered_map<std::string, std::string> buildStyleSheetValues(const DesignTokens& editorTokens) {
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
    return values;
}

void applyStyleSheet(const DesignTokens& editorTokens) {
    QFile themeFile(QStringLiteral(":/resources/theme.qss"));
    if (!themeFile.open(QFile::ReadOnly | QFile::Text)) {
        HMI_LOG_WARNING("Theme d'interface introuvable (:/resources/theme.qss) : style par defaut.");
        return;
    }
    const std::string templateText = QString::fromUtf8(themeFile.readAll()).toStdString();
    const StyleSheetSubstitutionResult substituted =
        substituteStyleSheetTemplate(templateText, buildStyleSheetValues(editorTokens));
    if (!substituted.ok) {
        HMI_LOG_WARNING("Theme d'interface invalide (" + substituted.error + ") : style par defaut.");
        return;
    }
    qApp->setStyleSheet(QString::fromStdString(substituted.text));
}

void applyEditorTheme() {
    QApplication::setPalette(buildApplicationPalette(editorDarkTokens()));
    applyStyleSheet(editorDarkTokens());
}

}  // namespace hmi

#include "HMI/Interface/ApplicationTheme.h"

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>

#include <filesystem>

#include "HMI/HmiLog.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/FontResolution.h"
#include "HMI/Interface/StyleSheetTemplate.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

namespace {

// Cle de preference du theme de l'editeur, meme portee QSettings que la langue et le mode de rendu
// (EX-IHM-011) : aucun nouveau mecanisme de persistance a inventer.
const char* const THEME_SETTINGS_KEY = "editor_theme";

[[nodiscard]] QColor toQColor(DesignColor color) {
    return QColor(color.r, color.g, color.b, color.a);
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
    // Texte de sélection lu sur l'accent : noir ou blanc, celui des deux qui contraste le mieux
    // avec l'accent -- fonctionne aussi bien pour l'accent clair du thème sombre que pour l'accent
    // assombri du thème clair (LOT-56 TACHE-06), sans étude de cas par thème.
    constexpr DesignColor BLACK{0, 0, 0};
    constexpr DesignColor WHITE{255, 255, 255};
    const DesignColor highlightedText = contrastRatio(color.accent, BLACK) >=
                                                contrastRatio(color.accent, WHITE)
                                            ? BLACK
                                            : WHITE;
    palette.setColor(group, QPalette::HighlightedText, toQColor(highlightedText));
    palette.setColor(group, QPalette::Link, toQColor(color.accent));
    palette.setColor(group, QPalette::PlaceholderText, toQColor(color.textMuted));
}

[[nodiscard]] EditorThemeSetting parseThemeSetting(const QString& value) {
    if (value == QLatin1String("light")) {
        return EditorThemeSetting::Light;
    }
    if (value == QLatin1String("dark")) {
        return EditorThemeSetting::Dark;
    }
    return EditorThemeSetting::System;
}

[[nodiscard]] const char* themeSettingName(EditorThemeSetting setting) {
    switch (setting) {
        case EditorThemeSetting::Light:
            return "light";
        case EditorThemeSetting::Dark:
            return "dark";
        case EditorThemeSetting::System:
            return "system";
    }
    return "system";
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

void applyFont() {
    const std::filesystem::path fontsDirectory = executableDirectory() / "Assets" / "Fonts";
    const int regularId = QFontDatabase::addApplicationFont(
        QString::fromStdString((fontsDirectory / "Inter-Regular.ttf").string()));
    const int boldId = QFontDatabase::addApplicationFont(
        QString::fromStdString((fontsDirectory / "Inter-Bold.ttf").string()));

    std::string family;
    if (regularId != -1) {
        const QStringList families = QFontDatabase::applicationFontFamilies(regularId);
        if (!families.isEmpty()) {
            family = families.first().toStdString();
        }
    }
    const FontFamilyResolution resolution =
        resolveFontFamily(regularId != -1 && boldId != -1 && !family.empty(), family);

    QFont font;
    if (resolution.useEmbeddedFamily) {
        font = QFont(QString::fromStdString(resolution.embeddedFamily));
    } else {
        // Famille generique demandee a Qt : jamais un second nom de police code en dur (voir
        // hmi::resolveFontFamily).
        font.setStyleHint(QFont::SansSerif);
        HMI_LOG_WARNING(
            "Police embarquee introuvable ou invalide (Assets/Fonts/Inter-*.ttf) : famille "
            "generique.");
    }
    font.setPointSize(identityTokens().typography.body.pointSize);
    QApplication::setFont(font);
}

EditorThemeSetting editorThemeSetting() {
    return parseThemeSetting(
        QSettings().value(QString::fromLatin1(THEME_SETTINGS_KEY), QStringLiteral("system")).toString());
}

void setEditorThemeSetting(EditorThemeSetting setting) {
    QSettings().setValue(QString::fromLatin1(THEME_SETTINGS_KEY),
                         QString::fromLatin1(themeSettingName(setting)));
}

bool systemPrefersDarkTheme() {
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

const DesignTokens& currentEditorTokens() {
    const EditorThemeMode mode =
        resolveEffectiveEditorTheme(editorThemeSetting(), systemPrefersDarkTheme());
    return mode == EditorThemeMode::Light ? editorLightTokens() : editorDarkTokens();
}

void applyEditorTheme() {
    applyFont();
    reapplyEditorTheme();
}

void reapplyEditorTheme() {
    const DesignTokens& tokens = currentEditorTokens();
    QApplication::setPalette(buildApplicationPalette(tokens));
    applyStyleSheet(tokens);
}

}  // namespace hmi

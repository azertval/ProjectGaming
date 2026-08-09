#include "HMI/Interface/ApplicationTheme.h"

#include <QApplication>
#include <QColor>
#include <QStyle>
#include <QStyleFactory>

#include "HMI/Interface/DesignTokens.h"

namespace hmi {

namespace {

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

void applyEditorTheme() {
    QApplication::setPalette(buildApplicationPalette(editorDarkTokens()));
}

}  // namespace hmi

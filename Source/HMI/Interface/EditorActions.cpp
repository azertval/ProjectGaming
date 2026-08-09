#include "HMI/Interface/EditorActions.h"

#include <QAction>
#include <QActionGroup>
#include <QKeySequence>
#include <QSignalBlocker>
#include <QString>
#include <QToolBar>

#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/ThemeIcons.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

EditorActions::EditorActions(const DesignTokens& tokens, QObject* parent)
    : QObject(parent), _toolGroup(new QActionGroup(this)) {
    _toolGroup->setExclusive(true);

    for (std::size_t i = 0; i < editorActionCatalog().size(); ++i) {
        const EditorActionSpec& spec = editorActionCatalog()[i];
        auto* const act = new QAction(this);
        act->setIcon(themeIcon(spec.id, tokens.size.iconMedium, tokens));
        act->setCheckable(spec.checkable);
        if (spec.shortcut[0] != '\0') {
            act->setShortcut(QKeySequence(QString::fromLatin1(spec.shortcut)));
        }
        if (spec.group == EditorActionGroup::LevelTools) {
            act->setActionGroup(_toolGroup);
        }
        _actions[i] = act;
    }
    // Pinceau actif par défaut (même défaut que l'ancien panneau Outils, EX-EDIT-014).
    action(IconId::ToolPaint)->setChecked(true);
}

QAction* EditorActions::action(IconId id) const {
    const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& catalog = editorActionCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        if (catalog[i].id == id) {
            return _actions[i];
        }
    }
    return nullptr;
}

QAction* EditorActions::toolAction(EditorTool tool) const {
    return action(editorActionForTool(tool));
}

void EditorActions::populateToolBar(QToolBar& toolBar) const {
    bool separatorInserted = false;
    for (const EditorActionSpec& spec : editorActionCatalog()) {
        if (!separatorInserted && spec.group != EditorActionGroup::LevelTools) {
            toolBar.addSeparator();
            separatorInserted = true;
        }
        toolBar.addAction(action(spec.id));
    }
}

void EditorActions::retranslateUi(const Localization& loc) {
    const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& catalog = editorActionCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        const QString label = QString::fromStdString(loc.text(catalog[i].labelKey));
        _actions[i]->setText(label);
        // Infobulle = libelle + raccourci de l'ACTION elle-meme (jamais une chaine traduite
        // decrivant la touche) : le remappage ne peut donc jamais la rendre fausse.
        const QKeySequence shortcut = _actions[i]->shortcut();
        _actions[i]->setToolTip(shortcut.isEmpty()
                                    ? label
                                    : label + QStringLiteral(" (") +
                                          shortcut.toString(QKeySequence::NativeText) +
                                          QStringLiteral(")"));
    }
}

void EditorActions::setActiveTool(EditorTool tool) {
    QAction* const act = toolAction(tool);
    if (act == nullptr || act->isChecked()) {
        return;
    }
    const QSignalBlocker blocker(act);
    act->setChecked(true);
}

void EditorActions::setEditingCommandsEnabled(bool enabled) {
    for (const EditorActionSpec& spec : editorActionCatalog()) {
        // Le mode de rendu reste toujours actif : en edition, en essai et en jeu reel
        // (EX-REN-046) -- jamais desactive, contrairement aux six autres commandes.
        if (spec.group == EditorActionGroup::None && spec.id != IconId::ToggleRenderMode) {
            action(spec.id)->setEnabled(enabled);
        }
    }
}

void EditorActions::refreshIcons(const DesignTokens& tokens) {
    const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& catalog = editorActionCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        _actions[i]->setIcon(themeIcon(catalog[i].id, tokens.size.iconMedium, tokens));
    }
}

}  // namespace hmi

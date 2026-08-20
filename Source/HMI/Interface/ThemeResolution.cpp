// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/ThemeResolution.h"

namespace hmi {

EditorThemeMode resolveEffectiveEditorTheme(EditorThemeSetting setting,
                                            bool systemPrefersDark) noexcept {
    switch (setting) {
        case EditorThemeSetting::Light:
            return EditorThemeMode::Light;
        case EditorThemeSetting::Dark:
            return EditorThemeMode::Dark;
        case EditorThemeSetting::System:
            return systemPrefersDark ? EditorThemeMode::Dark : EditorThemeMode::Light;
    }
    return EditorThemeMode::Dark;
}

}  // namespace hmi

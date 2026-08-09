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

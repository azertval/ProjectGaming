# Elements/UI/

Assets **Qt déclaratifs** de l'IHM, éditables hors code (Qt Designer) et compilés par la cible
`ProjectGaming` (`AUTOUIC`/`AUTORCC`, cf. `Source/HMI/CMakeLists.txt`, propriété
`AUTOUIC_SEARCH_PATHS`).

- `MainMenu.ui`, `OptionsPage.ui`, `ToolPanel.ui` — mises en page Qt Designer ; `uic` génère les
  en-têtes `ui_*.h` inclus par les widgets correspondants (`Source/HMI/Interface/`,
  `Source/HMI/Editor/`).
- `resources.qrc` — ressource Qt embarquée ; référence le thème `../Themes/theme.qss` (alias
  `:/resources/theme.qss`, chargé par `HMI/main.cpp`).

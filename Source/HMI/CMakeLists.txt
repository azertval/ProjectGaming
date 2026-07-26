# Couche de présentation HMI.
#
# LOT-34 (refonte IHM vers Qt) : `HMI` est scindé en deux.
#  - `HmiRuntime` : bibliothèque statique de **rendu Direct3D 11, entrées et localisation**,
#    réutilisable telle quelle par l'exécutable historique ET par la future cible Qt
#    (`ProjectGamingEditor`, `Source/Editor`). Elle ne contient AUCUNE UI « maison » (écrans,
#    widgets d'éditeur), qui reste dans l'exécutable historique jusqu'à son retrait (LOT-38).
#  - `ProjectGaming` : l'exécutable historique (fenêtre Win32, boucle, écrans, éditeur intégré),
#    conservé en parallèle de la cible Qt pendant toute la migration pour ne jamais casser le jeu.

# --- Bibliothèque de rendu/plateforme réutilisable ---------------------------------------------
add_library(HmiRuntime STATIC
    Platform/ExecutableDirectory.cpp
    Input/InputState.cpp
    Input/GamepadPoller.cpp
    Input/PlayerInputMapper.cpp
    Input/GameKeyBindings.cpp
    Input/EditorKeyBindings.cpp
    Input/KeyName.cpp
    Input/GamepadBindings.cpp
    Input/GamepadButtonName.cpp
    Graphics/GraphicsDevice.cpp
    Graphics/SpriteBatch.cpp
    Graphics/TextureAtlas.cpp
    Graphics/Camera2D.cpp
    Graphics/RoomGrid.cpp
    Graphics/SpriteRenderer.cpp
    Graphics/DraftRenderer.cpp
    Graphics/BitmapFont.cpp
    Graphics/FlagIcons.cpp
    Graphics/SaveIcon.cpp
    Graphics/TileVisuals.cpp
    Game/GameSession.cpp
    Localization/Localization.cpp
)

# Core en PUBLIC : propage la racine d'inclusion `Source/` et l'API Core aux consommateurs.
# Les bibliothèques système Direct3D/XInput en PUBLIC : un exécutable qui lie HmiRuntime en a
# besoin au link (bibliothèque statique).
target_link_libraries(HmiRuntime
    PUBLIC
        Core
        d3d11
        dxgi
        d3dcompiler
        xinput
    PRIVATE
        project_warnings
        project_options
        # Les *KeyBindings persistent en JSON ; Core ne propage pas nlohmann_json (lien PRIVATE).
        nlohmann_json::nlohmann_json
)

# Réduit la surface de Windows.h, évite les macros min/max, API Unicode. PUBLIC : les en-têtes de
# HmiRuntime (rendu D3D11, entrées) incluent Windows.h ; les consommateurs doivent voir les mêmes
# macros pour une compilation cohérente.
target_compile_definitions(HmiRuntime PUBLIC
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

if(ENABLE_PCH)
    target_precompile_headers(HmiRuntime PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# --- Exécutable historique du jeu (UI « maison ») ----------------------------------------------
add_executable(ProjectGaming
    main.cpp
    Platform/Window.cpp
    Interface/ScreenManager.cpp
    Interface/MenuModel.cpp
    Interface/MenuScreen.cpp
    Interface/OptionsModel.cpp
    Interface/OptionsScreen.cpp
    Interface/GameKeybindingsModel.cpp
    Interface/GameKeybindingsScreen.cpp
    Interface/EditorKeybindingsModel.cpp
    Interface/EditorKeybindingsScreen.cpp
    Interface/GamepadBindingsModel.cpp
    Interface/GamepadBindingsScreen.cpp
    Interface/GameScreen.cpp
    Interface/EditorScreen.cpp
    Interface/LanguageSelector.cpp
    Interface/SaveLogButton.cpp
    Interface/SessionLog.cpp
    Editor/TilePalette.cpp
    Editor/LevelPicker.cpp
    Editor/TextInputField.cpp
    Editor/LevelNameValidation.cpp
    Editor/LevelSizeValidation.cpp
    Editor/ToolBar.cpp
)

# HmiRuntime propage Core, les libs D3D et les définitions Unicode (PUBLIC) : rien à redéclarer ici.
target_link_libraries(ProjectGaming PRIVATE
    HmiRuntime
    project_warnings
    project_options
)

# En Release, pas de fenêtre console (sous-système Windows) pour une expérience utilisateur
# propre ; en Debug, on conserve la console (logs visibles). Le point d'entrée reste le `main()`
# standard grâce à `/ENTRY:mainCRTStartup` (la CRT s'initialise puis appelle `main`).
if(MSVC)
    set_target_properties(ProjectGaming PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
    target_link_options(ProjectGaming PRIVATE $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
endif()

if(ENABLE_PCH)
    # PCH propre à la cible : ses définitions (UNICODE…) diffèrent de Core,
    # on ne peut donc pas réutiliser le PCH de Core (mismatch _MBCS/_UNICODE).
    target_precompile_headers(ProjectGaming PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# Copie les catalogues de traduction (.lang) à côté de l'exécutable : ils sont chargés à
# l'exécution par hmi::Localization, relativement au répertoire du binaire.
add_custom_command(TARGET ProjectGaming POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Localization
        $<TARGET_FILE_DIR:ProjectGaming>/Localization
    COMMENT "Copie des catalogues de traduction (.lang)")

# Copie les fichiers de niveaux (.json) à côté de l'exécutable : ils sont chargés à l'exécution
# par core::LevelLoader, relativement au répertoire du binaire.
add_custom_command(TARGET ProjectGaming POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Levels
        $<TARGET_FILE_DIR:ProjectGaming>/Levels
    COMMENT "Copie des niveaux (.json)")

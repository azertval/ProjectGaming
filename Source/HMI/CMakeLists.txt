# Exécutable du jeu : fenêtre, rendu Direct3D 11 et boucle de jeu.
add_executable(ProjectGaming
    main.cpp
    Platform/Window.cpp
    Platform/ExecutableDirectory.cpp
    Input/InputState.cpp
    Input/PlayerInputMapper.cpp
    Graphics/GraphicsDevice.cpp
    Graphics/SpriteBatch.cpp
    Graphics/TextureAtlas.cpp
    Graphics/Camera2D.cpp
    Graphics/SpriteRenderer.cpp
    Graphics/BitmapFont.cpp
    Graphics/FlagIcons.cpp
    Graphics/SaveIcon.cpp
    Graphics/TileVisuals.cpp
    Localization/Localization.cpp
    Interface/ScreenManager.cpp
    Interface/MenuModel.cpp
    Interface/MenuScreen.cpp
    Interface/GameScreen.cpp
    Interface/EditorScreen.cpp
    Interface/LanguageSelector.cpp
    Interface/SaveLogButton.cpp
    Interface/SessionLog.cpp
    Editor/TilePalette.cpp
    Editor/LevelPicker.cpp
    Editor/TextInputField.cpp
    Editor/LevelNameValidation.cpp
    Editor/ToolBar.cpp
)

target_link_libraries(ProjectGaming PRIVATE
    Core
    project_warnings
    project_options
    d3d11
    dxgi
    d3dcompiler
)

# Réduit la surface de Windows.h et évite les macros min/max ; API Unicode.
target_compile_definitions(ProjectGaming PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
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

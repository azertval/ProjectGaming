# Exécutable du jeu : fenêtre, rendu Direct3D 11 et boucle de jeu.
add_executable(ProjectGaming
    main.cpp
    Platform/Window.cpp
    Input/InputState.cpp
    Graphics/GraphicsDevice.cpp
    Graphics/SpriteBatch.cpp
    Graphics/TextureAtlas.cpp
    Graphics/Camera2D.cpp
    Graphics/SpriteRenderer.cpp
    Graphics/BitmapFont.cpp
    Localization/Localization.cpp
    Interface/ScreenManager.cpp
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

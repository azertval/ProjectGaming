# Couche de présentation HMI : **application Qt** (`ProjectGaming`).
#
# Depuis le LOT-38, l'IHM « maison » et l'exécutable historique `ProjectGaming` ont été retirés :
# l'unique application est l'éditeur Qt, qui embarque le rendu Direct3D 11 du jeu dans un viewport et
# l'entoure de panneaux d'édition. Le code est réparti par domaine, au plus près de l'architecture
# d'origine du projet :
#   Platform/     provisionnement bas niveau (répertoire exécutable) ;
#   Input/        entrées (état, mapping, remappage clavier/manette, pont Qt -> Key) ;
#   Graphics/     rendu Direct3D 11 réutilisable (device, sprites, caméra, brouillon d'édition) ;
#   Game/         simulation d'un niveau (GameSession) et viewport Qt jeu/édition ;
#   Localization/ catalogue de traduction ;
#   Interface/    fenêtre principale, menu, options, remappage (widgets Qt) ;
#   Editor/       périmètre éditeur de niveau (palette, outils, navigateur, logique pure).
# Les fichiers déclaratifs Qt (.ui, .qrc) et le thème (.qss) vivent dans Source/Elements (assets).

# La cible reste **optionnelle** : sur un poste sans Qt (ex. CI n'exécutant que les tests unitaires,
# qui compilent les sources pures directement), la configuration n'échoue pas — l'application est
# simplement ignorée. Fournir Qt via CMAKE_PREFIX_PATH (ou l'action CI install-qt-action) pour
# l'activer, ou passer -DBUILD_EDITOR_QT=OFF pour l'ignorer explicitement.
option(BUILD_EDITOR_QT "Construire l'éditeur Qt (ProjectGaming). Nécessite Qt6." ON)
if(NOT BUILD_EDITOR_QT)
    message(STATUS "ProjectGaming : désactivé (BUILD_EDITOR_QT=OFF).")
    return()
endif()

find_package(Qt6 QUIET COMPONENTS Widgets Gui)
if(NOT Qt6_FOUND)
    message(STATUS
        "ProjectGaming : Qt6 introuvable, cible ignorée. "
        "Définir CMAKE_PREFIX_PATH vers une installation Qt (ex. C:/Qt/6.8.1/msvc2022_64) "
        "ou passer -DBUILD_EDITOR_QT=OFF.")
    return()
endif()

message(STATUS "ProjectGaming : Qt6 ${Qt6_VERSION} trouvé (${Qt6_DIR}).")

# Traitement automatique des métacompilateurs Qt, restreint à cette cible (moc/uic/rcc).
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Assets Qt déclaratifs (hors code) : mises en page Designer + ressource embarquée, dans Elements.
set(ELEMENTS_UI ${PROJECT_SOURCE_DIR}/Source/Elements/UI)

add_executable(ProjectGaming
    main.cpp
    # Plateforme
    Platform/ExecutableDirectory.cpp
    # Entrées (dont le pont Qt -> Key)
    Input/InputState.cpp
    Input/GamepadPoller.cpp
    Input/PlayerInputMapper.cpp
    Input/GameKeyBindings.cpp
    Input/EditorKeyBindings.cpp
    Input/KeyName.cpp
    Input/GamepadBindings.cpp
    Input/GamepadButtonName.cpp
    Input/QtKeyMap.cpp
    # Rendu Direct3D 11
    Graphics/GraphicsDevice.cpp
    Graphics/SpriteBatch.cpp
    Graphics/TextureAtlas.cpp
    Graphics/Camera2D.cpp
    Graphics/RoomGrid.cpp
    Graphics/SpriteRenderer.cpp
    Graphics/DraftRenderer.cpp
    Graphics/TileVisuals.cpp
    # Jeu (simulation d'un niveau + viewport Qt jeu/édition)
    Game/GameSession.cpp
    Game/GameViewport.cpp
    # Localisation
    Localization/Localization.cpp
    # Diagnostics (enregistrement des journaux de session)
    Diagnostics/SessionLog.cpp
    # Interface (fenêtre, menu, options, remappage)
    Interface/MainWindow.cpp
    Interface/MainMenu.cpp
    Interface/OptionsPage.cpp
    Interface/KeybindingsWidget.cpp
    Interface/GamepadBindingsWidget.cpp
    # Éditeur de niveau
    Editor/PalettePanel.cpp
    Editor/ToolPanel.cpp
    Editor/LevelBrowserPanel.cpp
    Editor/TileTaxonomy.cpp
    Editor/LevelFileOperations.cpp
    Editor/LevelNameValidation.cpp
    # Ressource embarquée (thème .qss) — traitée par AUTORCC.
    ${ELEMENTS_UI}/resources.qrc
    # Mises en page Qt Designer — traitées par AUTOUIC (génèrent ui_*.h).
    ${ELEMENTS_UI}/MainWindow.ui
    ${ELEMENTS_UI}/MainMenu.ui
    ${ELEMENTS_UI}/OptionsPage.ui
    ${ELEMENTS_UI}/ToolPanel.ui
    ${ELEMENTS_UI}/LevelBrowserPanel.ui
)

# Racine d'inclusion Source/ (pour les includes "HMI/..." et "Core/...").
target_include_directories(ProjectGaming PRIVATE ${PROJECT_SOURCE_DIR}/Source)

# Les mises en page .ui vivent dans Source/Elements/UI (assets), pas à côté des widgets : AUTOUIC
# doit y chercher le .ui correspondant à chaque include "ui_*.h".
set_target_properties(ProjectGaming PROPERTIES AUTOUIC_SEARCH_PATHS ${ELEMENTS_UI})

target_link_libraries(ProjectGaming PRIVATE
    Core
    Qt6::Widgets
    Qt6::Gui
    d3d11
    dxgi
    d3dcompiler
    xinput
    project_warnings
    project_options
    # Les *KeyBindings persistent en JSON ; Core ne propage pas nlohmann_json (lien PRIVATE).
    nlohmann_json::nlohmann_json
)

# Réduit la surface de Windows.h, évite les macros min/max, API Unicode.
target_compile_definitions(ProjectGaming PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

if(ENABLE_PCH)
    target_precompile_headers(ProjectGaming PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# Copie des assets à côté de l'exécutable, pour que l'éditeur charge niveaux et traductions
# relativement à son binaire.
add_custom_command(TARGET ProjectGaming POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Localization
        $<TARGET_FILE_DIR:ProjectGaming>/Localization
    COMMENT "Copie des catalogues de traduction (.lang)")
add_custom_command(TARGET ProjectGaming POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Levels
        $<TARGET_FILE_DIR:ProjectGaming>/Levels
    COMMENT "Copie des niveaux (.json)")

# Déploiement Qt : copie les DLL Qt (plateforme, styles) à côté de l'exécutable pour que
# l'utilisateur final n'ait **aucune bibliothèque à installer** — un simple dossier exécutable
# autonome. windeployqt est fourni par l'installation Qt (même répertoire bin que qmake).
get_target_property(_qmake_exe Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_exe}" DIRECTORY)
find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_qt_bin_dir}")
if(WINDEPLOYQT_EXE)
    add_custom_command(TARGET ProjectGaming POST_BUILD
        COMMAND "${WINDEPLOYQT_EXE}"
            --no-translations --compiler-runtime
            "$<TARGET_FILE:ProjectGaming>"
        COMMENT "Déploiement des DLL Qt + runtime à côté de l'exécutable (windeployqt)")
else()
    message(STATUS "ProjectGaming : windeployqt introuvable, DLL Qt non déployées "
        "automatiquement (exécution possible depuis un environnement Qt).")
endif()

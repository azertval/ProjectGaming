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

# --- Découverte de Qt sans configuration manuelle ---
# find_package() ne consulte que CMAKE_PREFIX_PATH. Or l'installateur officiel Qt place ses
# versions dans <racine>/<version>/<compilateur> — un emplacement que CMake ne connaît pas. Sur un
# poste neuf, la configuration ne trouvait donc rien et la cible était ignorée *en silence* : le
# build « réussissait » sans jamais produire d'exécutable. On complète la recherche par les
# emplacements conventionnels, en les ajoutant **en fin** de liste pour qu'un choix explicite
# (-DCMAKE_PREFIX_PATH, -DQt6_DIR) garde toujours la priorité.
set(_qt_hints "")

# Variables d'environnement usuelles. QT_ROOT_DIR est exporté par install-qt-action (CI) ;
# QTDIR est la convention historique de l'installateur.
foreach(_qt_var QT_ROOT_DIR Qt6_ROOT QTDIR)
    if(DEFINED ENV{${_qt_var}})
        list(APPEND _qt_hints "$ENV{${_qt_var}}")
    endif()
endforeach()

# Arborescences de l'installateur officiel. Le tri décroissant naturel place la version la plus
# récente en tête (6.10 avant 6.8, ce qu'un tri lexicographique ferait à l'envers).
if(WIN32)
    file(GLOB _qt_installs
        "$ENV{SystemDrive}/Qt/*/msvc*_64"
        "C:/Qt/*/msvc*_64"
        "D:/Qt/*/msvc*_64")
else()
    file(GLOB _qt_installs
        "$ENV{HOME}/Qt/*/gcc_64"
        "/opt/Qt/*/gcc_64")
endif()
list(SORT _qt_installs COMPARE NATURAL ORDER DESCENDING)
list(APPEND _qt_hints ${_qt_installs})

if(_qt_hints)
    list(REMOVE_DUPLICATES _qt_hints)
    list(APPEND CMAKE_PREFIX_PATH ${_qt_hints})
endif()

find_package(Qt6 QUIET COMPONENTS Widgets Gui Multimedia)
if(NOT Qt6_FOUND)
    # Avertissement et non simple statut : sans Qt il n'y a **aucun exécutable**, seulement les
    # tests. Passer inaperçu est précisément ce qui rendait l'absence d'exe difficile à diagnostiquer.
    message(WARNING
        "ProjectGaming : Qt6 introuvable, cible ignorée — AUCUN EXÉCUTABLE ne sera produit.\n"
        "Emplacements examinés : ${_qt_hints}\n"
        "Corriger en installant Qt via l'installateur officiel, ou en désignant l'installation :\n"
        "  cmake --preset ninja -DCMAKE_PREFIX_PATH=<chemin>/msvc2022_64\n"
        "Passer -DBUILD_EDITOR_QT=OFF pour ne construire que les tests, sans cet avertissement.")
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
    Graphics/AssetPaths.cpp
    Graphics/TextureLoader.cpp
    Graphics/AssetContract.cpp
    Graphics/RenderMode.cpp
    Graphics/TileAppearance.cpp
    Graphics/SkinCatalog.cpp
    Graphics/TileAutotile.cpp
    Graphics/SlopeMask.cpp
    Graphics/ProceduralAtlas.cpp
    Graphics/AnimationCatalog.cpp
    Graphics/PlayerSprite.cpp
    Graphics/MissingTexture.cpp
    Graphics/TextureAtlas.cpp
    Graphics/TextureCache.cpp
    Graphics/TextureAtlasRegions.cpp
    Graphics/Camera2D.cpp
    Graphics/RoomGrid.cpp
    Graphics/ComposedScene.cpp
    Graphics/QuadRecorder.cpp
    Graphics/BackgroundRenderer.cpp
    Graphics/ShadowRenderer.cpp
    Graphics/SpriteRenderer.cpp
    Graphics/DraftRenderer.cpp
    Graphics/TileVisuals.cpp
    Graphics/MechanismVisuals.cpp
    Graphics/DecorVisuals.cpp
    Graphics/Parallax.cpp
    Graphics/ProceduralFont.cpp
    Graphics/BitmapFont.cpp
    Graphics/TextRenderer.cpp
    # Jeu (simulation d'un niveau + viewport Qt jeu/édition)
    Game/GameSession.cpp
    Game/GameViewport.cpp
    Game/Progression.cpp
    Game/GameHud.cpp
    # Localisation
    Localization/Localization.cpp
    # Audio (bruitages, LOT-60)
    Audio/AudioEngine.cpp
    # Diagnostics (enregistrement des journaux de session)
    Diagnostics/SessionLog.cpp
    # Interface (fenêtre, menu, options, remappage)
    Interface/DesignTokens.cpp
    Interface/StyleSheetTemplate.cpp
    Interface/FontResolution.cpp
    Interface/ThemeResolution.cpp
    Interface/ApplicationTheme.cpp
    Interface/IconGeometry.cpp
    Interface/ThemeIcons.cpp
    Interface/ActionCatalog.cpp
    Interface/EditorActions.cpp
    Interface/ScreenFlow.cpp
    Interface/MainWindow.cpp
    Interface/MainMenu.cpp
    Interface/PauseScreen.cpp
    Interface/LevelCompleteScreen.cpp
    Interface/LevelSelectScreen.cpp
    Interface/OptionsPage.cpp
    Interface/KeybindingsWidget.cpp
    Interface/EditorKeybindingsWidget.cpp
    Interface/GamepadBindingsWidget.cpp
    # Éditeur de niveau
    Editor/PalettePanel.cpp
    Editor/DecorsPanel.cpp
    Editor/LevelBrowserPanel.cpp
    Editor/LinkPanel.cpp
    Editor/TileTaxonomy.cpp
    Editor/TaxonomyLabels.cpp
    Editor/TexturePanel.cpp
    Editor/SkinAssignments.cpp
    Editor/PaletteAppearance.cpp
    Editor/LevelFileOperations.cpp
    Editor/LevelNameValidation.cpp
    Editor/LinkGeometry.cpp
    Editor/LinkGesture.cpp
    Editor/TextureAssignGesture.cpp
    Editor/DecorGeometry.cpp
    Editor/DecorGesture.cpp
    Editor/DecorListModel.cpp
    Editor/AssetLibrary.cpp
    Editor/ThumbnailGeometry.cpp
    Editor/AssetThumbnailView.cpp
    Editor/AssetFileOperations.cpp
    Editor/AssetReferences.cpp
    Editor/MechanismAnimationAssignments.cpp
    Editor/EditorStatus.cpp
    Editor/PanelFocus.cpp
    Editor/PixelOperations.cpp
    Editor/PixelHistory.cpp
    Editor/PixelCanvasGeometry.cpp
    Editor/PixelCanvas.cpp
    Editor/PixelHistoryPanel.cpp
    Editor/PixelAssetIO.cpp
    Editor/PixelPalette.cpp
    Editor/PixelPalettePanel.cpp
    Editor/PixelAutotilePreview.cpp
    # Ressource embarquée (thème .qss) — traitée par AUTORCC.
    ${ELEMENTS_UI}/resources.qrc
    # Mises en page Qt Designer — traitées par AUTOUIC (génèrent ui_*.h).
    ${ELEMENTS_UI}/MainWindow.ui
    ${ELEMENTS_UI}/MainMenu.ui
    ${ELEMENTS_UI}/PauseScreen.ui
    ${ELEMENTS_UI}/LevelCompleteScreen.ui
    ${ELEMENTS_UI}/LevelSelectScreen.ui
    ${ELEMENTS_UI}/OptionsPage.ui
    ${ELEMENTS_UI}/DecorsPanel.ui
    ${ELEMENTS_UI}/TexturePanel.ui
    ${ELEMENTS_UI}/LevelBrowserPanel.ui
    ${ELEMENTS_UI}/AssetThumbnailView.ui
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
    Qt6::Multimedia
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
add_custom_command(TARGET ProjectGaming POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Assets
        $<TARGET_FILE_DIR:ProjectGaming>/Assets
    COMMENT "Copie des assets graphiques (atlas de textures, LOT-39)")

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

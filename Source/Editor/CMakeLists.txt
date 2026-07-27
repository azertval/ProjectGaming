# Éditeur / application Qt (LOT-34, refonte IHM).
#
# Nouvelle cible exécutable `ProjectGamingEditor` : application Qt qui, à terme, embarque le rendu
# Direct3D 11 du jeu (via `HmiRuntime`) dans un viewport et l'entoure de panneaux d'édition. À cette
# tâche (LOT-34 TACHE-01), elle se limite à un `main` Qt ouvrant une fenêtre nue — le viewport et la
# boucle arrivent aux tâches suivantes.
#
# La cible est **optionnelle** : sur un poste sans Qt, la configuration n'échoue pas (l'exécutable
# historique reste construit). Fournir Qt via `CMAKE_PREFIX_PATH` (ou l'action CI `install-qt-action`)
# pour l'activer, ou passer `-DBUILD_EDITOR_QT=OFF` pour l'ignorer explicitement.

option(BUILD_EDITOR_QT "Construire l'éditeur Qt (ProjectGamingEditor). Nécessite Qt6." ON)
if(NOT BUILD_EDITOR_QT)
    message(STATUS "ProjectGamingEditor : désactivé (BUILD_EDITOR_QT=OFF).")
    return()
endif()

find_package(Qt6 QUIET COMPONENTS Widgets Gui)
if(NOT Qt6_FOUND)
    message(STATUS
        "ProjectGamingEditor : Qt6 introuvable, cible ignorée. "
        "Définir CMAKE_PREFIX_PATH vers une installation Qt (ex. C:/Qt/6.8.1/msvc2022_64) "
        "ou passer -DBUILD_EDITOR_QT=OFF.")
    return()
endif()

message(STATUS "ProjectGamingEditor : Qt6 ${Qt6_VERSION} trouvé (${Qt6_DIR}).")

# Traitement automatique des métacompilateurs Qt, restreint à cette cible.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(ProjectGamingEditor
    main_qt.cpp
    GameViewport.cpp
    MainWindow.cpp
    PalettePanel.cpp
    ToolPanel.cpp
    LevelBrowserPanel.cpp
    TileTaxonomy.cpp
    LevelFileOperations.cpp
    # Validation de nom de niveau (partagée avec l'éditeur historique jusqu'au LOT-38).
    ${PROJECT_SOURCE_DIR}/Source/HMI/Editor/LevelNameValidation.cpp
)

# HmiRuntime propage Core, les libs Direct3D et les définitions Unicode (PUBLIC).
target_link_libraries(ProjectGamingEditor PRIVATE
    HmiRuntime
    Qt6::Widgets
    Qt6::Gui
    project_warnings
    project_options
)

if(ENABLE_PCH)
    target_precompile_headers(ProjectGamingEditor PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# Copie des assets à côté de l'exécutable (mêmes catalogues/niveaux que l'exe historique), pour que
# l'éditeur charge niveaux et traductions relativement à son binaire.
add_custom_command(TARGET ProjectGamingEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Localization
        $<TARGET_FILE_DIR:ProjectGamingEditor>/Localization
    COMMENT "Copie des catalogues de traduction (.lang)")
add_custom_command(TARGET ProjectGamingEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Source/Elements/Levels
        $<TARGET_FILE_DIR:ProjectGamingEditor>/Levels
    COMMENT "Copie des niveaux (.json)")

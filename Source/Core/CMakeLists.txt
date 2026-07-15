# Bibliothèque Core : logique et moteur du jeu.
add_library(Core STATIC
    Core.cpp
    Time/FixedTimestep.cpp
)

# Racine d'inclusion = Source/ : les en-têtes s'incluent par chemin complet
# depuis la racine (ex. "Core/Core.h"), jamais par nom seul.
target_include_directories(Core PUBLIC ${PROJECT_SOURCE_DIR}/Source)

target_link_libraries(Core PRIVATE project_warnings project_options)

if(ENABLE_PCH)
    target_precompile_headers(Core PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# Bibliothèque Core : logique et moteur du jeu.
add_library(Core STATIC
    Core.cpp
)

# Racine d'inclusion = Source/ : les en-têtes s'incluent par chemin complet
# depuis la racine (ex. "Core/Core.h"), jamais par nom seul.
target_include_directories(Core PUBLIC ${PROJECT_SOURCE_DIR}/Source)

target_link_libraries(Core PRIVATE project_warnings project_options)

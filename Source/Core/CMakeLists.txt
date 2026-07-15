# Bibliothèque Core : logique et moteur du jeu.
add_library(Core STATIC
    Core.cpp
)

target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

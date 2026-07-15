# Exécutable du jeu : bootstrap de la présentation.
# (Le rendu DirectX sera ajouté lors du lot dédié ; pour l'instant, point d'entrée minimal.)
add_executable(ProjectGaming
    main.cpp
)

target_link_libraries(ProjectGaming PRIVATE Core)

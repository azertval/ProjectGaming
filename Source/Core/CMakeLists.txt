# Bibliothèque Core : logique et moteur du jeu.
add_library(Core STATIC
    Core.cpp
    Math/Vector2.cpp
    Math/Rect.cpp
    Ecs/EntityManager.cpp
    Ecs/World.cpp
    Ecs/Systems/MovementSystem.cpp
    Ecs/Systems/CharacterPhysicsSystem.cpp
    Ecs/Systems/AnimationSystem.cpp
    Levels/TileMap.cpp
    Levels/LevelLoader.cpp
    Levels/LevelWriter.cpp
    Levels/LevelDraft.cpp
    Levels/LevelScene.cpp
    Levels/LevelOutcome.cpp
    Levels/DangerGeometry.cpp
    Gameplay/MechanismController.cpp
    Gameplay/BlockController.cpp
    Physics/SweptCollision.cpp
    Physics/SlopeGeometry.cpp
    Physics/AabbVsAabb.cpp
    Time/FixedTimestep.cpp
    Diagnostics/LogFormat.cpp
    Diagnostics/Logger.cpp
    Diagnostics/ConsoleLogSink.cpp
    Diagnostics/Assert.cpp
)

# Racine d'inclusion = Source/ : les en-têtes s'incluent par chemin complet
# depuis la racine (ex. "Core/Core.h"), jamais par nom seul.
target_include_directories(Core PUBLIC ${PROJECT_SOURCE_DIR}/Source)

target_link_libraries(Core PRIVATE project_warnings project_options nlohmann_json::nlohmann_json)

if(ENABLE_PCH)
    target_precompile_headers(Core PRIVATE ${PROJECT_SOURCE_DIR}/Source/pch.h)
endif()

# Tests unitaires (GoogleTest).
include(GoogleTest)

add_executable(UnitTests
    Unit/test_core.cpp
    Unit/test_vector2.cpp
    Unit/test_rect.cpp
    Unit/test_entity_manager.cpp
    Unit/test_component_pool.cpp
    Unit/test_view.cpp
    Unit/test_world.cpp
    Unit/test_sprite.cpp
    Unit/test_camera2d.cpp
    Unit/test_fixed_timestep.cpp
    Unit/test_logger.cpp
    Unit/test_sinks.cpp
    Unit/test_log_format.cpp
    Unit/test_assert.cpp
    Unit/test_input_state.cpp
    # La caméra 2D est de la logique pure (DirectXMath), testable hors GPU :
    # on compile sa source dans les tests unitaires.
    ../HMI/Graphics/Camera2D.cpp
    # L'état des entrées est indépendant de la fenêtre (aucun <Windows.h>) : sa
    # source est compilée dans les tests pour valider les fronts en isolation.
    ../HMI/Input/InputState.cpp
)

target_link_libraries(UnitTests PRIVATE
    Core
    GTest::gtest_main
    project_warnings
    project_options
)

# Tests d'intégration : plusieurs briques assemblées collaborant de bout en bout.
add_executable(IntegrationTests
    Integration/test_ecs_mouvement.cpp
    Integration/test_boucle_simulation.cpp
)

target_link_libraries(IntegrationTests PRIVATE
    Core
    GTest::gtest_main
    project_warnings
    project_options
)

# Enregistre chaque TEST() auprès de CTest.
gtest_discover_tests(UnitTests)
gtest_discover_tests(IntegrationTests)

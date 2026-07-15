# Tests unitaires (GoogleTest).
include(GoogleTest)

add_executable(UnitTests
    Unit/test_core.cpp
    Unit/test_vector2.cpp
    Unit/test_rect.cpp
    Unit/test_entity_manager.cpp
    Unit/test_component_pool.cpp
    Unit/test_view.cpp
    Unit/test_fixed_timestep.cpp
    Unit/test_logger.cpp
    Unit/test_sinks.cpp
    Unit/test_log_format.cpp
    Unit/test_assert.cpp
)

target_link_libraries(UnitTests PRIVATE
    Core
    GTest::gtest_main
    project_warnings
    project_options
)

# Enregistre chaque TEST() auprès de CTest.
gtest_discover_tests(UnitTests)

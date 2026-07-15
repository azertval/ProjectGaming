# Tests unitaires (GoogleTest).
include(GoogleTest)

add_executable(UnitTests
    Unit/test_core.cpp
)

target_link_libraries(UnitTests PRIVATE
    Core
    GTest::gtest_main
)

# Enregistre chaque TEST() auprès de CTest.
gtest_discover_tests(UnitTests)

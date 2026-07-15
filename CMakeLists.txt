cmake_minimum_required(VERSION 3.21)

project(ProjectGaming
    VERSION 0.1.0
    DESCRIPTION "Jeu 2D de plateforme/puzzle en C++/DirectX"
    LANGUAGES CXX)

# --- Standard C++ ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Dossiers de sortie ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# --- Options ---
option(BUILD_TESTING "Construire les tests (GoogleTest)" ON)

# --- Dépendances tierces (External/) ---
add_subdirectory(External)

# --- Code source ---
add_subdirectory(Source)

# --- Tests ---
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(Source/Test)
endif()

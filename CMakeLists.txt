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
option(ENABLE_WARNINGS_AS_ERRORS "Traiter les avertissements comme des erreurs" ON)
option(ENABLE_ASAN "Activer AddressSanitizer" OFF)

# --- Avertissements (appliqués uniquement à NOS cibles, pas aux dépendances) ---
add_library(project_warnings INTERFACE)
if(MSVC)
    # /external:* : les en-têtes inclus via <...> (STL, DirectX, GoogleTest) sont
    # traités comme externes et n'émettent pas d'avertissement — cohérent avec
    # notre règle « <...> = externe, "..." = projet ».
    target_compile_options(project_warnings INTERFACE
        /W4 /permissive- /external:anglebrackets /external:W0)
    if(ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(project_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)
    if(ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(project_warnings INTERFACE -Werror)
    endif()
endif()

# --- Options de compilation transverses (sanitizers) ---
add_library(project_options INTERFACE)
if(ENABLE_ASAN)
    if(MSVC)
        target_compile_options(project_options INTERFACE /fsanitize=address)
    else()
        target_compile_options(project_options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(project_options INTERFACE -fsanitize=address)
    endif()
endif()

# --- Dépendances tierces (External/) ---
add_subdirectory(External)

# --- Code source ---
add_subdirectory(Source)

# --- Tests ---
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(Source/Test)
endif()

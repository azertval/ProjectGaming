# Dépendances tierces
#
# Deux stratégies possibles :
#  - FetchContent : la dépendance est téléchargée à la configuration (cas GoogleTest).
#  - Vendored     : la dépendance est copiée dans un sous-dossier de External/ et
#                   ajoutée ici via add_subdirectory().

include(FetchContent)

# --- GoogleTest (framework de test) ---
if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    # Utilise la même CRT que le reste du projet sous MSVC.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

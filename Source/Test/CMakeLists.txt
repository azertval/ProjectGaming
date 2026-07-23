# Tests unitaires (GoogleTest).
include(GoogleTest)

add_executable(UnitTests
    # L'arborescence des tests unitaires reflète celle des sources (Source/) :
    # le test de Source/<Module>/<X> vit sous Unit/<Module>/test_<x>.cpp.
    Unit/Core/test_core.cpp
    Unit/Core/Math/test_vector2.cpp
    Unit/Core/Math/test_rect.cpp
    Unit/Core/Math/test_math_utils.cpp
    Unit/Core/Ecs/test_entity_manager.cpp
    Unit/Core/Ecs/test_component_pool.cpp
    Unit/Core/Ecs/test_view.cpp
    Unit/Core/Ecs/test_world.cpp
    Unit/Core/Ecs/test_sprite.cpp
    Unit/Core/Ecs/test_player_components.cpp
    Unit/Core/Levels/test_level.cpp
    Unit/Core/Levels/test_level_loader.cpp
    Unit/Core/Levels/test_level_writer.cpp
    Unit/Core/Levels/test_level_draft.cpp
    Unit/Core/Levels/test_level_outcome.cpp
    Unit/Core/Gameplay/test_mechanism_controller.cpp
    Unit/Core/Gameplay/test_block_controller.cpp
    Unit/Core/Physics/test_swept_collision.cpp
    Unit/Core/Physics/test_slope_geometry.cpp
    Unit/Core/Time/test_fixed_timestep.cpp
    Unit/Core/Diagnostics/test_logger.cpp
    Unit/Core/Diagnostics/test_sinks.cpp
    Unit/Core/Diagnostics/test_log_format.cpp
    Unit/Core/Diagnostics/test_assert.cpp
    Unit/Core/Diagnostics/test_log_level_parse.cpp
    Unit/HMI/Input/test_input_state.cpp
    Unit/HMI/Input/test_player_input_mapper.cpp
    Unit/HMI/Graphics/test_camera2d.cpp
    Unit/HMI/Localization/test_localization.cpp
    Unit/HMI/Interface/test_screen_manager.cpp
    Unit/HMI/Interface/test_level_sequence.cpp
    Unit/HMI/Interface/test_menu_model.cpp
    Unit/HMI/Interface/test_options_model.cpp
    Unit/HMI/Interface/test_language_selector.cpp
    Unit/HMI/Interface/test_save_log_button.cpp
    Unit/HMI/Interface/test_session_log.cpp
    Unit/HMI/Editor/test_tile_palette.cpp
    Unit/HMI/Editor/test_level_picker.cpp
    Unit/HMI/Editor/test_text_input_field.cpp
    Unit/HMI/Editor/test_level_name_validation.cpp
    Unit/HMI/Editor/test_tool_bar.cpp
    Unit/HMI/Editor/test_level_size_validation.cpp
    # La caméra 2D est de la logique pure (DirectXMath), testable hors GPU :
    # on compile sa source dans les tests unitaires.
    ../HMI/Graphics/Camera2D.cpp
    # L'état des entrées est indépendant de la fenêtre (aucun <Windows.h>) : sa
    # source est compilée dans les tests pour valider les fronts en isolation.
    ../HMI/Input/InputState.cpp
    # Le mapping touches → intention est une logique pure, testable hors GPU.
    ../HMI/Input/PlayerInputMapper.cpp
    # Le catalogue de traduction est une logique pure (fichiers/chaînes), testable hors GPU.
    ../HMI/Localization/Localization.cpp
    # La navigation entre écrans (transitions) est testable via une fabrique factice, hors GPU.
    ../HMI/Interface/ScreenManager.cpp
    # La logique du menu (sélection, survol, transitions) est pure, testable hors GPU.
    ../HMI/Interface/MenuModel.cpp
    # La logique du menu d'options (sélection, survol, actions) est pure, testable hors GPU.
    ../HMI/Interface/OptionsModel.cpp
    # Le bouton de langue (ancrage, bascule, clic) est une logique pure, testable hors GPU.
    ../HMI/Interface/LanguageSelector.cpp
    # Le bouton d'enregistrement des logs (ancrage, clic) est pur, testable hors GPU.
    ../HMI/Interface/SaveLogButton.cpp
    # La sérialisation des logs de session est pure, testable hors GPU.
    ../HMI/Interface/SessionLog.cpp
    # La palette de tuiles de l'éditeur (géométrie/sélection) est pure, testable hors GPU.
    ../HMI/Editor/TilePalette.cpp
    # Le sélecteur de niveau (navigation/sélection) est pur, testable hors GPU.
    ../HMI/Editor/LevelPicker.cpp
    # Le champ de saisie de texte de l'éditeur est pur, testable hors GPU.
    ../HMI/Editor/TextInputField.cpp
    # La validation d'un nom de niveau saisi est une fonction pure, testable hors GPU.
    ../HMI/Editor/LevelNameValidation.cpp
    # La barre d'outils de l'éditeur (géométrie/sélection) est pure, testable hors GPU.
    ../HMI/Editor/ToolBar.cpp
    # La validation d'une taille de grille saisie est une fonction pure, testable hors GPU.
    ../HMI/Editor/LevelSizeValidation.cpp
)

target_link_libraries(UnitTests PRIVATE
    Core
    GTest::gtest_main
    project_warnings
    project_options
)

# Chemins (source) des assets, pour vérifier que le contenu livré (niveaux, catalogues) se charge.
target_compile_definitions(UnitTests PRIVATE
    PROJECTGAMING_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels"
    PROJECTGAMING_LOCALIZATION_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Localization")

# Tests d'intégration : plusieurs briques assemblées collaborant de bout en bout.
add_executable(IntegrationTests
    Integration/test_ecs_mouvement.cpp
    Integration/test_boucle_simulation.cpp
    Integration/test_niveau_ecs.cpp
    Integration/test_physique_personnage.cpp
    Integration/test_animation_personnage.cpp
)

target_link_libraries(IntegrationTests PRIVATE
    Core
    GTest::gtest_main
    project_warnings
    project_options
)

# Chemin (source) des niveaux, pour le test d'intégration « fichier livré → ECS ».
target_compile_definitions(IntegrationTests PRIVATE
    PROJECTGAMING_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels")

# Tests système : un parcours de jeu complet sur le contenu livré, sans la couche GPU.
add_executable(SystemTests
    Systeme/test_parcours_complet.cpp
    Systeme/test_parcours_edition.cpp
)

target_link_libraries(SystemTests PRIVATE
    Core
    GTest::gtest_main
    project_warnings
    project_options
)

# Chemin (source) des niveaux : le parcours complet rejoue les niveaux livrés.
target_compile_definitions(SystemTests PRIVATE
    PROJECTGAMING_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels")

# Enregistre chaque TEST() auprès de CTest.
gtest_discover_tests(UnitTests)
gtest_discover_tests(IntegrationTests)
gtest_discover_tests(SystemTests)

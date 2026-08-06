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
    Unit/Core/Levels/test_danger_geometry.cpp
    Unit/Core/Levels/test_tile_type_name.cpp
    Unit/Core/Gameplay/test_mechanism_controller.cpp
    Unit/Core/Gameplay/test_danger_controller.cpp
    Unit/Core/Gameplay/test_block_controller.cpp
    Unit/Core/Physics/test_swept_collision.cpp
    Unit/Core/Physics/test_slope_geometry.cpp
    Unit/Core/Physics/test_aabb_vs_aabb.cpp
    Unit/Core/Time/test_fixed_timestep.cpp
    Unit/Core/Diagnostics/test_logger.cpp
    Unit/Core/Diagnostics/test_sinks.cpp
    Unit/Core/Diagnostics/test_file_log_sink.cpp
    Unit/Core/Diagnostics/test_log_format.cpp
    Unit/Core/Diagnostics/test_assert.cpp
    Unit/Core/Diagnostics/test_log_level_parse.cpp
    # Bibliothèque de calcul numérique maison pour l'IA de résolution autonome (LOT-ANNEXE-01,
    # programme annexe @ref lots-annexe) : tenseurs, opérations, RNG déterministe.
    Unit/AiSolver/Math/test_rng.cpp
    Unit/AiSolver/Math/test_tensor.cpp
    Unit/AiSolver/Math/test_tensor_ops.cpp
    Unit/AiSolver/Math/test_matmul.cpp
    Unit/AiSolver/Math/test_tensor_integration.cpp
    # Graphe de calcul dynamique de l'autodiff maison (LOT-ANNEXE-02, TACHE-01).
    Unit/AiSolver/Math/test_autodiff_node.cpp
    # Operations differentiables de base de l'autodiff maison (LOT-ANNEXE-02, TACHE-02).
    Unit/AiSolver/Math/test_autodiff_ops.cpp
    Unit/HMI/Input/test_input_state.cpp
    Unit/HMI/Input/test_player_input_mapper.cpp
    Unit/HMI/Input/test_game_key_bindings.cpp
    Unit/HMI/Input/test_editor_key_bindings.cpp
    Unit/HMI/Input/test_key_name.cpp
    Unit/HMI/Input/test_gamepad_bindings.cpp
    Unit/HMI/Input/test_gamepad_button_name.cpp
    Unit/HMI/Graphics/test_camera2d.cpp
    Unit/HMI/Graphics/test_room_grid.cpp
    Unit/HMI/Graphics/test_asset_paths.cpp
    Unit/HMI/Graphics/test_procedural_atlas.cpp
    Unit/HMI/Graphics/test_texture_atlas.cpp
    Unit/HMI/Graphics/test_asset_contract.cpp
    Unit/HMI/Graphics/test_missing_texture.cpp
    Unit/HMI/Graphics/test_quad_recorder.cpp
    Unit/HMI/Graphics/test_render_culling.cpp
    Unit/HMI/Graphics/test_render_mode.cpp
    Unit/HMI/Graphics/test_skin_catalog.cpp
    Unit/HMI/Graphics/test_tile_autotile.cpp
    Unit/HMI/Graphics/test_tile_skin_resolution.cpp
    Unit/HMI/Graphics/test_slope_mask.cpp
    Unit/HMI/Localization/test_localization.cpp
    Unit/HMI/Diagnostics/test_session_log.cpp
    Unit/HMI/Editor/test_level_name_validation.cpp
    Unit/HMI/Editor/test_tile_taxonomy.cpp
    Unit/HMI/Editor/test_skin_assignments.cpp
    Unit/HMI/Editor/test_palette_appearance.cpp
    Unit/HMI/Editor/test_level_file_operations.cpp
    Unit/HMI/Editor/test_link_geometry.cpp
    Unit/HMI/Editor/test_link_gesture.cpp
    # La caméra 2D est de la logique pure (DirectXMath), testable hors GPU :
    # on compile sa source dans les tests unitaires.
    ../HMI/Graphics/Camera2D.cpp
    # La partition d'un niveau en salles (LOT-32) est une logique pure (aucune dépendance
    # rendu) : testable hors GPU.
    ../HMI/Graphics/RoomGrid.cpp
    # La résolution de chemins d'assets (LOT-39) est une logique pure (std::filesystem),
    # testable hors GPU/Qt.
    ../HMI/Graphics/AssetPaths.cpp
    # La correspondance type de tuile -> région d'atlas (LOT-39, dépendance de ProceduralAtlas.cpp
    # ci-dessous) est une logique pure, testable hors GPU.
    ../HMI/Graphics/TileVisuals.cpp
    # La génération procédurale de l'atlas de repli (LOT-39) ne fait que produire des pixels en
    # mémoire (aucun appel Direct3D/Qt) : testable hors GPU.
    ../HMI/Graphics/ProceduralAtlas.cpp
    # Le mapping (colonne, ligne) -> région d'atlas de TextureAtlas (LOT-39) est de la pure
    # arithmétique de grille, séparée du chargement fichier/procédural (Qt/Direct3D 11) : testable
    # hors GPU.
    ../HMI/Graphics/TextureAtlasRegions.cpp
    # Le contrat de dimensions par famille d'assets (LOT-40) est une validation pure, sans
    # decodage ni upload : testable hors GPU.
    ../HMI/Graphics/AssetContract.cpp
    # Le mode de rendu (LOT-41) et la resolution d'apparence qui en decoule sont de la logique
    # pure (aucune dependance Qt/GPU) : testables hors GPU.
    ../HMI/Graphics/RenderMode.cpp
    ../HMI/Graphics/TileAppearance.cpp
    # Le catalogue de skins (LOT-42) est une configuration de presentation lue/ecrite en JSON,
    # sans dependance Qt/GPU : testable hors GPU (EX-NFR-010).
    ../HMI/Graphics/SkinCatalog.cpp
    # La table de raccords automatiques (LOT-42) est de la pure arithmetique de voisinage, sans
    # texture ni GPU : testable exhaustivement hors GPU (EX-NFR-004).
    ../HMI/Graphics/TileAutotile.cpp
    # La silhouette des pentes (LOT-42) ne fait que decider quels pixels sont dans la matiere,
    # a partir de la geometrie de Core : testable hors GPU, et comparable a l'atlas procedural.
    ../HMI/Graphics/SlopeMask.cpp
    # Le damier magenta de repli (LOT-40) ne produit que des pixels en memoire, comme
    # ProceduralAtlas.cpp : testable hors GPU.
    ../HMI/Graphics/MissingTexture.cpp
    # La composition du rendu (LOT-40) -- liste ordonnee des primitives, tri par calque/texture,
    # culling -- est separee de la soumission Direct3D : c'est ce qui rend le rendu verifiable
    # sans GPU (EX-NFR-004).
    ../HMI/Graphics/ComposedScene.cpp
    ../HMI/Graphics/QuadRecorder.cpp
    # L'état des entrées est indépendant de la fenêtre (aucun <Windows.h>) : sa
    # source est compilée dans les tests pour valider les fronts en isolation.
    ../HMI/Input/InputState.cpp
    # Le mapping touches → intention est une logique pure, testable hors GPU.
    ../HMI/Input/PlayerInputMapper.cpp
    # Le remappage des touches (jeu/éditeur) et son affichage sont une logique pure, testable
    # hors GPU (persistance JSON sur disque, aucune dépendance fenêtre/GPU).
    ../HMI/Input/GameKeyBindings.cpp
    ../HMI/Input/EditorKeyBindings.cpp
    ../HMI/Input/KeyName.cpp
    ../HMI/Input/GamepadBindings.cpp
    ../HMI/Input/GamepadButtonName.cpp
    # Le catalogue de traduction est une logique pure (fichiers/chaînes), testable hors GPU.
    ../HMI/Localization/Localization.cpp
    # La sérialisation des logs de session est pure (aucune dépendance fenêtre/GPU), testable.
    ../HMI/Diagnostics/SessionLog.cpp
    # La validation d'un nom de niveau saisi est une fonction pure, testable hors GPU.
    ../HMI/Editor/LevelNameValidation.cpp
    # La taxonomie de la palette Qt (catégories/sous-groupes) est pure, testable hors GPU/Qt.
    ../HMI/Editor/TileTaxonomy.cpp
    # La logique alimentant le panneau « Textures » (LOT-42) est separee du widget Qt : lignes a
    # afficher, balayage du dossier de skins, effet d'une assignation. Testable sans Qt.
    ../HMI/Editor/TaxonomyLabels.cpp
    ../HMI/Editor/SkinAssignments.cpp
    # Le choix de vignette de la palette (LOT-42) applique la meme priorite que le canevas, sans
    # aucune dependance Qt/GPU : c'est ce qui permet de verifier qu'ils ne divergent pas.
    ../HMI/Editor/PaletteAppearance.cpp
    # Les opérations fichiers de niveaux (créer/renommer/dupliquer/supprimer) sont pures, testables.
    # (LevelNameValidation.cpp est déjà compilé plus haut.)
    ../HMI/Editor/LevelFileOperations.cpp
    # Géométrie/modèle des liens de mécanismes (LOT-37) et machine à état du geste de l'outil Lien :
    # logique pure, testable hors GPU/Qt.
    ../HMI/Editor/LinkGeometry.cpp
    ../HMI/Editor/LinkGesture.cpp
)

target_link_libraries(UnitTests PRIVATE
    Core
    AiSolver
    GTest::gtest_main
    project_warnings
    project_options
    # GameKeyBindings.cpp/EditorKeyBindings.cpp (ci-dessus) sont compiles directement dans cette
    # cible (comme PlayerInputMapper.cpp) et persistent en JSON : besoin direct de nlohmann_json,
    # que Core ne propage pas (lien PRIVATE cote Core).
    nlohmann_json::nlohmann_json
)

# Chemins (source) des assets, pour vérifier que le contenu livré (niveaux, catalogues) se charge.
target_compile_definitions(UnitTests PRIVATE
    PROJECTGAMING_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels"
    PROJECTGAMING_LOCALIZATION_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Localization"
    PROJECTGAMING_ASSETS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Assets")

# Tests d'intégration : plusieurs briques assemblées collaborant de bout en bout.
add_executable(IntegrationTests
    Integration/test_ecs_mouvement.cpp
    Integration/test_boucle_simulation.cpp
    Integration/test_niveau_ecs.cpp
    Integration/test_physique_personnage.cpp
    Integration/test_animation_personnage.cpp
    Integration/test_bloc_reduit.cpp
    Integration/test_danger_avance.cpp
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

# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Tests unitaires (GoogleTest).
include(GoogleTest)

add_executable(UnitTests
    # L'arborescence des tests unitaires reflète celle des sources (Source/) :
    # le test de Source/<Module>/<X> vit sous Unit/<Module>/test_<x>.cpp.
    Unit/Core/test_core.cpp
    Unit/Core/Math/test_vector2.cpp
    Unit/Core/Math/test_rect.cpp
    Unit/Core/Math/test_math_utils.cpp
    Unit/Core/Math/test_deterministic_random.cpp
    Unit/Core/Ecs/test_entity_manager.cpp
    Unit/Core/Ecs/test_animation_clip.cpp
    Unit/Core/Ecs/test_component_pool.cpp
    Unit/Core/Ecs/test_view.cpp
    Unit/Core/Ecs/test_world.cpp
    Unit/Core/Ecs/test_sprite.cpp
    Unit/Core/Ecs/test_player_components.cpp
    Unit/Core/Ecs/test_particle_system.cpp
    Unit/Core/Ecs/test_particle_triggers.cpp
    Unit/Core/Levels/test_level.cpp
    Unit/Core/Levels/test_camera_framing.cpp
    Unit/Core/Levels/test_level_loader.cpp
    Unit/Core/Levels/test_level_sequence.cpp
    Unit/Core/Levels/test_level_writer.cpp
    Unit/Core/Levels/test_level_draft.cpp
    Unit/Core/Levels/test_plane.cpp
    Unit/Core/Levels/test_level_outcome.cpp
    Unit/Core/Levels/test_danger_geometry.cpp
    Unit/Core/Levels/test_tile_type_name.cpp
    Unit/Core/Levels/test_tile_texture_override.cpp
    Unit/Core/Gameplay/test_mechanism_controller.cpp
    Unit/Core/Gameplay/test_danger_controller.cpp
    Unit/Core/Gameplay/test_block_controller.cpp
    Unit/Core/Gameplay/test_platform_controller.cpp
    Unit/Core/Gameplay/test_platform_path.cpp
    Unit/Core/Physics/test_swept_collision.cpp
    Unit/Core/Physics/test_slope_geometry.cpp
    Unit/Core/Physics/test_aabb_vs_aabb.cpp
    Unit/Core/Time/test_fixed_timestep.cpp
    Unit/Core/Diagnostics/test_logger.cpp
    Unit/Core/Diagnostics/test_scoped_log_level.cpp
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
    # backward() : parcours topologique inverse de l'autodiff maison (LOT-ANNEXE-02, TACHE-03).
    Unit/AiSolver/Math/test_autodiff_backward.cpp
    # Verification de gradient par differences finies (LOT-ANNEXE-02, TACHE-04) ; GradientCheck.h
    # est header-only, inclus directement par ce fichier de test (pas d'entree propre ici).
    Unit/AiSolver/Math/test_autodiff_gradient_checking.cpp
    # Bibliotheque de reseaux de neurones maison (LOT-ANNEXE-03) : couche dense (TACHE-01).
    Unit/AiSolver/Nn/test_dense.cpp
    # Activations sigmoid/softmax (LOT-ANNEXE-03, TACHE-02).
    Unit/AiSolver/Nn/test_activations.cpp
    # Composition en reseau (LOT-ANNEXE-03, TACHE-03).
    Unit/AiSolver/Nn/test_network.cpp
    # Initialisation des poids et serialisation (LOT-ANNEXE-03, TACHE-04).
    Unit/AiSolver/Nn/test_weight_init.cpp
    Unit/AiSolver/Nn/test_serialization.cpp
    # Tests transversaux du reseau complet, activations melees (LOT-ANNEXE-03, TACHE-06).
    Unit/AiSolver/Nn/test_network_reproducibility.cpp
    Unit/AiSolver/Nn/test_network_stability.cpp
    # Optimiseurs maison (LOT-ANNEXE-04) : SGD avec/sans inertie (TACHE-01), zeroGrad partage.
    Unit/AiSolver/Optim/test_optimizer_utils.cpp
    Unit/AiSolver/Optim/test_sgd.cpp
    # Adam, moments d'ordre 1/2, correction de biais (LOT-ANNEXE-04, TACHE-02).
    Unit/AiSolver/Optim/test_adam.cpp
    # Convergence sur problemes jouets (LOT-ANNEXE-04, TACHE-03), bloquant avant la generation 3 ;
    # ToyProblems.h est header-only, inclus directement par ce fichier de test.
    Unit/AiSolver/Optim/test_convergence.cpp
    # Environnement de simulation headless (LOT-ANNEXE-05) : reset/step, budget de pas et mesure de
    # progression, sans fenetre ni GPU.
    Unit/AiSolver/Env/test_headless_level_environment.cpp
    # Encodage de l'observation (LOT-ANNEXE-06) : fenetre de tuiles categorielle (TACHE-01), vecteur
    # d'etat joueur (TACHE-02), etat des mecanismes dans la fenetre (TACHE-03), determinisme et
    # stabilite dimensionnelle aux bords (TACHE-04).
    Unit/AiSolver/Env/test_tile_window_encoder.cpp
    Unit/AiSolver/Env/test_player_state_encoder.cpp
    Unit/AiSolver/Env/test_mechanism_state_encoder.cpp
    Unit/AiSolver/Env/test_observation_determinisme.cpp
    Unit/AiSolver/Env/test_observation_bords_carte.cpp
    # Espace d'action discret (TACHE-01), decodage deterministe/stochastique (TACHE-02) et format
    # de rejeu v1 (TACHE-03), LOT-ANNEXE-07 : test de bout en bout inclus dans test_replay_file.cpp
    # (TACHE-04).
    Unit/AiSolver/Env/test_action_space.cpp
    Unit/AiSolver/Env/test_action_decoding.cpp
    Unit/AiSolver/Replay/test_replay_file.cpp
    # Fonction de récompense (TACHE-01), classification de fin d'épisode (TACHE-02) et scénarios
    # combinés (TACHE-03), LOT-ANNEXE-08 : signal d'apprentissage unique et partagé.
    Unit/AiSolver/Env/test_reward.cpp
    Unit/AiSolver/Env/test_episode.cpp
    Unit/AiSolver/Env/test_reward_episode.cpp
    # Champ de distances de plus court chemin sur la grille (amendement LOT-ANNEXE-08, EX-IA-023) :
    # remplace la distance euclidienne dans Env/Reward.cpp pour les niveaux à détour forcé par un
    # mur.
    Unit/AiSolver/Env/test_grid_distance_field.cpp
    # Journalisation CSV incrementale des statistiques d'entrainement (LOT-ANNEXE-09) : moyenne
    # mobile/delta (TACHE-03), format de colonnes (TACHE-02), chemin de fichier par run (TACHE-04),
    # recorder bout en bout (TACHE-01/05).
    Unit/AiSolver/Stats/test_moving_average.cpp
    Unit/AiSolver/Stats/test_csv_format.cpp
    Unit/AiSolver/Stats/test_training_run_path.cpp
    Unit/AiSolver/Stats/test_training_stats_recorder.cpp
    # Algorithme evolutionniste maison (LOT-ANNEXE-10) : population (TACHE-01), evaluation de
    # fitness (TACHE-02), operateurs genetiques (TACHE-03), boucle de generation (TACHE-04),
    # reproductibilite et non-regression (TACHE-05).
    Unit/AiSolver/Training/test_population.cpp
    Unit/AiSolver/Training/test_fitness_evaluator.cpp
    Unit/AiSolver/Training/test_genetic_operators.cpp
    Unit/AiSolver/Training/test_evolutionary_trainer.cpp
    Unit/AiSolver/Training/test_evolutionary_reproducibility.cpp
    Unit/AiSolver/Training/test_evolutionary_non_regression.cpp
    Unit/AiSolver/Training/test_level_training_session.cpp
    Unit/AiSolver/Training/test_deterministic_replay.cpp
    Unit/AiSolver/Training/test_argmax_rollout.cpp
    Unit/AiSolver/Training/test_replay_export.cpp
    # Policy gradient maison REINFORCE (LOT-ANNEXE-12) : collecte de trajectoire (TACHE-01), retour
    # actualise (TACHE-02), perte REINFORCE et gradient checking -- le test le plus important du lot
    # (TACHE-03/05), boucle d'entrainement bout en bout sur le niveau de controle trivial (TACHE-04).
    Unit/AiSolver/Training/test_trajectory_collector.cpp
    Unit/AiSolver/Training/test_return_calculator.cpp
    Unit/AiSolver/Training/test_reinforce_loss.cpp
    Unit/AiSolver/Training/test_policy_gradient_loss.cpp
    Unit/AiSolver/Training/test_reinforce_trainer.cpp
    # Acteur-critique (LOT-ANNEXE-13) : reseau critique (TACHE-01), avantage et non-regression de
    # la perte REINFORCE apres factorisation (TACHE-02), perte du critique et boucle d'optimisation
    # conjointe (TACHE-03), comparaison chiffree de convergence vs REINFORCE brut (TACHE-04).
    Unit/AiSolver/Training/test_critic_network.cpp
    Unit/AiSolver/Training/test_advantage_calculator.cpp
    Unit/AiSolver/Training/test_actor_critic_loss.cpp
    Unit/AiSolver/Training/test_critic_loss.cpp
    Unit/AiSolver/Training/test_actor_critic_trainer.cpp
    Unit/AiSolver/Eval/test_convergence_comparator.cpp
    # Algorithme avance DQN (LOT-ANNEXE-14) : reseau Q(s,a) et memoire de rejeu (TACHE-01), perte de
    # Bellman/gradient checking (TACHE-01), boucle d'entrainement integree au harnais existant
    # (TACHE-02), comparaison chiffree generalisee a quatre series (TACHE-03).
    Unit/AiSolver/Training/test_q_network.cpp
    Unit/AiSolver/Training/test_replay_buffer.cpp
    Unit/AiSolver/Training/test_dqn_loss.cpp
    Unit/AiSolver/Training/test_dqn_trainer.cpp
    Unit/AiSolver/Eval/test_generation_comparator.cpp
    # Harnais de benchmark multi-algorithmes (LOT-ANNEXE-15), premier lot de la generation 4 :
    # adaptateurs TrainedPolicy uniformes (TACHE-01), execution automatisee repetee et reproductible
    # (TACHE-01), rapport comparatif CSV par niveau x algorithme reutilisant l'echappement partage
    # (TACHE-02), robustesse au bruit d'observation sans jamais toucher l'etat reel simule (TACHE-03).
    Unit/AiSolver/Eval/test_trained_policy.cpp
    Unit/AiSolver/Eval/test_benchmark_runner.cpp
    Unit/AiSolver/Eval/test_benchmark_report.cpp
    Unit/AiSolver/Eval/test_noisy_observation.cpp
    Unit/AiSolver/Eval/test_cross_level_benchmark.cpp
    # Validation et stabilisation du format de rejeu v1 (LOT-ANNEXE-17) : empreinte FNV-1a
    # deterministe du fichier de niveau (TACHE-01), validation a la lecture (fichier absent,
    # empreinte divergente, rejeu valide), stabilisation du format (formatVersion 1 -> 2,
    # compatibilite ascendante stricte, TACHE-02).
    Unit/AiSolver/Replay/test_level_fingerprint.cpp
    Unit/AiSolver/Replay/test_replay_validation.cpp
    Unit/AiSolver/Replay/test_replay_format.cpp
    # Outillage CLI (LOT-ANNEXE-19) : Commands.cpp/ArgumentParsing.cpp/TrainingConfig.cpp n'appartiennent
    # pas a la bibliotheque AiSolver (elles sont l'executable distinct aisolver-cli, cf.
    # Source/AiSolver/CMakeLists.txt), meme patron que GameEvents.cpp/ReplayPlayback.cpp ci-dessous
    # pour HMI : compilees directement dans cette cible pour rester testables sans lancer le vrai
    # executable (Main.cpp, lui, n'est jamais compile ici).
    ../AiSolver/Cli/ArgumentParsing.cpp
    # Cli/TrainingConfig.cpp n'est plus compile ici : deplace dans la bibliotheque AiSolver
    # (LOT-ANNEXE-21) pour etre reutilise par HMI/Ai/TrainingWorker -- deja fourni par le lien
    # AiSolver ci-dessous, le compiler aussi ici produirait des symboles dupliques.
    ../AiSolver/Cli/Commands.cpp
    Unit/AiSolver/Cli/test_argument_parsing.cpp
    Unit/AiSolver/Cli/test_training_config.cpp
    Unit/AiSolver/Cli/test_cli_commands.cpp
    Unit/HMI/Input/test_input_state.cpp
    Unit/HMI/Input/test_player_input_mapper.cpp
    Unit/HMI/Input/test_game_key_bindings.cpp
    Unit/HMI/Input/test_editor_key_bindings.cpp
    Unit/HMI/Input/test_key_name.cpp
    Unit/HMI/Input/test_gamepad_bindings.cpp
    Unit/HMI/Input/test_gamepad_probe.cpp
    Unit/HMI/Input/test_gamepad_button_name.cpp
    Unit/HMI/Graphics/test_camera2d.cpp
    Unit/HMI/Graphics/test_room_grid.cpp
    Unit/HMI/Graphics/test_follow_camera.cpp
    Unit/HMI/Graphics/test_camera_zones.cpp
    Unit/HMI/Graphics/test_asset_paths.cpp
    Unit/HMI/Graphics/test_procedural_atlas.cpp
    Unit/HMI/Graphics/test_tile_visuals.cpp
    Unit/HMI/Graphics/test_texture_atlas.cpp
    Unit/HMI/Graphics/test_animation_catalog.cpp
    Unit/HMI/Graphics/test_animated_tiles.cpp
    Unit/HMI/Graphics/test_player_sprite.cpp
    Unit/HMI/Graphics/test_asset_contract.cpp
    Unit/HMI/Graphics/test_missing_texture.cpp
    Unit/HMI/Graphics/test_quad_recorder.cpp
    Unit/HMI/Graphics/test_render_culling.cpp
    Unit/HMI/Graphics/test_background_fit.cpp
    Unit/HMI/Graphics/test_render_mode.cpp
    Unit/HMI/Graphics/test_skin_catalog.cpp
    Unit/HMI/Graphics/test_tile_autotile.cpp
    Unit/HMI/Graphics/test_tile_skin_resolution.cpp
    Unit/HMI/Graphics/test_texture_resolution.cpp
    Unit/HMI/Graphics/test_slope_mask.cpp
    Unit/HMI/Graphics/test_cache_registry.cpp
    Unit/HMI/Graphics/test_mechanism_visuals.cpp
    Unit/HMI/Graphics/test_mechanism_transitions.cpp
    Unit/HMI/Graphics/test_parallax.cpp
    Unit/HMI/Graphics/test_layer_visibility.cpp
    Unit/HMI/Graphics/test_plane_render.cpp
    Unit/HMI/Graphics/test_bitmap_font.cpp
    Unit/HMI/Graphics/test_text_renderer.cpp
    Unit/HMI/Graphics/test_shadow_render.cpp
    Unit/HMI/Graphics/test_render_budget.cpp
    Unit/HMI/Graphics/test_particle_render.cpp
    Unit/HMI/Graphics/test_screen_shake.cpp
    Unit/HMI/Game/test_game_hud.cpp
    Unit/HMI/Game/test_diagnostics_hud.cpp
    Unit/HMI/Game/test_progression.cpp
    Unit/HMI/Game/test_game_events.cpp
    Unit/HMI/Game/test_level_run_stats.cpp
    Unit/HMI/Game/test_replay_playback.cpp
    Unit/HMI/Audio/test_sound_triggers.cpp
    Unit/HMI/Localization/test_localization.cpp
    Unit/HMI/Diagnostics/test_session_log.cpp
    Unit/HMI/Audio/test_sound_catalog.cpp
    Unit/HMI/Interface/test_design_tokens.cpp
    Unit/HMI/Interface/test_application_theme.cpp
    Unit/HMI/Interface/test_theme_teardown_guards.cpp
    Unit/HMI/Interface/test_editor_actions.cpp
    Unit/HMI/Interface/test_screen_flow.cpp
    Unit/HMI/Interface/test_pixel_art_scale.cpp
    Unit/HMI/Interface/test_editor_workspace.cpp
    Unit/HMI/Interface/test_pixel_frame_geometry.cpp
    Unit/HMI/Editor/test_thumbnail_geometry.cpp
    Unit/HMI/Editor/test_level_name_validation.cpp
    Unit/HMI/Editor/test_tile_taxonomy.cpp
    Unit/HMI/Editor/test_skin_assignments.cpp
    Unit/HMI/Editor/test_palette_appearance.cpp
    Unit/HMI/Editor/test_level_file_operations.cpp
    Unit/HMI/Editor/test_link_geometry.cpp
    Unit/HMI/Editor/test_link_gesture.cpp
    Unit/HMI/Editor/test_texture_assign_gesture.cpp
    Unit/HMI/Editor/test_asset_library.cpp
    Unit/HMI/Editor/test_asset_file_operations.cpp
    Unit/HMI/Editor/test_asset_references.cpp
    Unit/HMI/Editor/test_path_geometry.cpp
    Unit/HMI/Editor/test_path_gesture.cpp
    Unit/HMI/Editor/test_mechanism_animation_assignments.cpp
    Unit/HMI/Editor/test_editor_status.cpp
    Unit/HMI/Editor/test_panel_focus.cpp
    Unit/HMI/Editor/test_pixel_operations.cpp
    Unit/HMI/Editor/test_pixel_history.cpp
    Unit/HMI/Editor/test_pixel_canvas_geometry.cpp
    Unit/HMI/Editor/test_plane_reference.cpp
    Unit/HMI/Editor/test_plane_file_naming.cpp
    Unit/HMI/Editor/test_pixel_asset_io.cpp
    Unit/HMI/Editor/test_pixel_palette.cpp
    Unit/HMI/Editor/test_pixel_autotile_preview.cpp
    # La caméra 2D est de la logique pure (DirectXMath), testable hors GPU :
    # on compile sa source dans les tests unitaires.
    ../HMI/Graphics/Camera2D.cpp
    # La partition d'un niveau en salles (LOT-32) est une logique pure (aucune dépendance
    # rendu) : testable hors GPU.
    ../HMI/Graphics/RoomGrid.cpp
    # La caméra de suivi (LOT-64 TACHE-02 : zone morte, anticipation, lissage, bornage) est une
    # fonction pure, sans dépendance rendu : testable hors GPU, même patron que RoomGrid.cpp.
    ../HMI/Graphics/FollowCamera.cpp
    # La resolution de la zone de camera active parmi des zones dessinees a la main (LOT-64bis)
    # est une fonction pure, meme patron que RoomGrid.cpp : testable hors GPU.
    ../HMI/Graphics/CameraZones.cpp
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
    # Le format d'animation nom-asset.anim.json (LOT-46) est une lecture/validation JSON pure,
    # sans dependance Qt/GPU : testable hors GPU (EX-NFR-010).
    ../HMI/Graphics/AnimationCatalog.cpp
    # L'ancrage image/hitbox et la resolution de clip par repli du personnage (LOT-48) sont des
    # fonctions pures (aucune dependance GPU/fichier) : testables hors GPU.
    ../HMI/Graphics/PlayerSprite.cpp
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
    # Le calcul de recadrage du fond et sa composition (LOT-44) sont purs -- la resolution de
    # texture (acces TextureCache/GPU) est separee dans hmi::resolveBackgroundTexture
    # (SpriteRenderer.cpp, non compile ici), meme patron que composeWorldSprites/sceneTextures.
    ../HMI/Graphics/BackgroundRenderer.cpp
    # Les ombres du plan physique (LOT-55) ne font que composer des quads a partir de l'ECS et de
    # l'atlas procedural (regions deja calculees) -- pure, sans GPU, meme patron que ci-dessus.
    ../HMI/Graphics/ShadowRenderer.cpp
    # Le rendu des particules (LOT-53 TACHE-03) ne fait que composer des quads a partir de l'ECS
    # et d'une region d'atlas deja calculee (TextureAtlas::tile, pure arithmetique de grille,
    # cf. TextureAtlasRegions.cpp ci-dessous) -- meme patron que ShadowRenderer.cpp, sans GPU.
    ../HMI/Graphics/ParticleRenderer.cpp
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
    # Le catalogue de sons (LOT-60 TACHE-02) est une resolution de chemins pure, sur le patron de
    # SkinCatalog.cpp : sans dependance Qt/peripherique, testable hors Qt Multimedia (EX-NFR-010).
    ../HMI/Audio/SoundCatalog.cpp
    # La detection des transitions de jeu et la table evenement -> son (LOT-60 TACHE-03) sont des
    # fonctions pures (aucune dependance Qt/peripherique), meme patron que MechanismVisuals.cpp :
    # testables hors GPU/Qt (EX-NFR-010).
    ../HMI/Game/GameEvents.cpp
    # Lecture/validation de rejeu (LOT-ANNEXE-18, TACHE-01) : aucune dependance Qt/GPU (uniquement
    # AiSolver/Replay), meme patron que GameEvents.cpp ci-dessus -- testable hors fenetre.
    ../HMI/Game/ReplayPlayback.cpp
    ../HMI/Audio/SoundTriggers.cpp
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
    # Geste pur de l'outil « Texture par instance » (LOT-45) : meme patron que LinkGesture.cpp,
    # testable hors Qt/GPU.
    ../HMI/Editor/TextureAssignGesture.cpp
    # Geometrie des poignees de parcours et machine a etats du geste de l'outil « Parcours »
    # (LOT-67) : fonctions et machine a etats pures, sans dependance GPU/Qt, meme patron que
    # LinkGesture/LinkGeometry (LOT-37).
    ../HMI/Editor/PathGeometry.cpp
    ../HMI/Editor/PathGesture.cpp
    # Balayage/filtrage d'un dossier d'assets (LOT-43), partagé par AssetThumbnailView : pur,
    # testable hors Qt.
    ../HMI/Editor/AssetLibrary.cpp
    # Opérations fichiers sur les assets (LOT-43), pendant d'AssetLibrary/LevelFileOperations :
    # le décodage (Qt) reste chez l'appelant, cette classe ne fait que std::filesystem.
    ../HMI/Editor/AssetFileOperations.cpp
    # Détection des références à un asset dans skins.json (LOT-43) : pure, sans Qt/GPU/fichier.
    ../HMI/Editor/AssetReferences.cpp
    # La correspondance etat logique -> clip des mecanismes (LOT-47) est une fonction pure, sans
    # dependance GPU/Qt/ECS : testable exhaustivement hors GPU.
    ../HMI/Graphics/MechanismVisuals.cpp
    # La projection profondeur -> calque et la composition des plans picturaux (LOT-69 TACHE-05)
    # sont des fonctions pures, sans dependance GPU/Qt : testables hors GPU, meme patron que
    # MechanismVisuals.cpp ci-dessus. Seule resolvePlaneTextures touche au cache, et n'est pas
    # exercee par les tests unitaires.
    ../HMI/Graphics/PlaneVisuals.cpp
    # Le calcul du decalage de parallaxe (LOT-49 TACHE-03, generalise au LOT-69) est fait de
    # fonctions pures, sans dependance GPU : testables hors GPU.
    ../HMI/Graphics/Parallax.cpp
    # Les lignes du panneau « Textures » / section Animations et leur diagnostic de clips
    # manquants (LOT-47) sont une logique pure (filesystem + AnimationCatalog), testable sans Qt.
    ../HMI/Editor/MechanismAnimationAssignments.cpp
    # Choix des lignes de la barre d'etat de l'editeur (LOT-57) a partir d'un contexte d'edition :
    # fonction pure, meme patron que GameHud.cpp, testable hors GPU.
    ../HMI/Editor/EditorStatus.cpp
    # Correspondance outil -> panneau a mettre en avant (LOT-57) : table pure, meme patron que
    # ActionCatalog.cpp, testable hors Qt.
    ../HMI/Editor/PanelFocus.cpp
    # Machine a etats des ecrans de la fenetre principale (LOT-59) : table de transitions et
    # habillage de fenetre, purs, meme patron que PanelFocus.cpp -- testable hors Qt.
    ../HMI/Interface/ScreenFlow.cpp
    # Métriques de police bitmap, mesure pure de texte, lecture JSON et repli procédural (LOT-52) :
    # aucune dépendance GPU/Qt (contrairement à BitmapFont.cpp, qui charge la texture réelle et
    # n'est donc pas compilé ici, comme TextureAtlas.cpp/TextureCache.cpp) : testable hors GPU.
    ../HMI/Graphics/ProceduralFont.cpp
    # Composition d'une chaîne en quads sur le calque UI et projection écran (LOT-52) : logique
    # pure sur des SpriteQuad/hmi::ComposedScene, sans dépendance GPU : testable hors GPU.
    ../HMI/Graphics/TextRenderer.cpp
    # Choix des lignes du HUD (budgets, nom du tableau) à partir de core::Player/core::Level
    # (LOT-52) : fonction pure, testable hors GPU.
    ../HMI/Game/GameHud.cpp
    # Moyenne glissante de cadence et lignes du compteur de diagnostic (LOT-62 TACHE-02) : logique
    # pure (aucune horloge systeme, aucun rendu), meme patron que GameHud.cpp ci-dessus.
    ../HMI/Game/DiagnosticsHud.cpp
    # Progression persistée (LOT-59 TACHE-05, EX-LVL-014) : logique et persistance JSON pures,
    # aucune dépendance Qt/GPU (fichier/écriture atomique via std::filesystem/std::ofstream) --
    # testable hors GPU, même patron que GameKeyBindings.cpp ci-dessous.
    ../HMI/Game/Progression.cpp
    # Jetons de design de l'IHM (LOT-56) : couleurs par rôle, échelles, conversion CSS -- aucune
    # dépendance Qt/GPU (contrairement à ApplicationTheme.cpp, qui les applique via QPalette/
    # QApplication et n'est donc pas compilé ici, même distinction que BitmapFont.cpp/
    # ProceduralFont.cpp) : testable hors GPU.
    ../HMI/Interface/DesignTokens.cpp
    # Substitution ${...} du modele de feuille de style (LOT-56 TACHE-02) : logique pure sur des
    # std::string, aucune dependance Qt (contrairement a ApplicationTheme.cpp, qui charge le
    # fichier via QFile et l'applique via QApplication::setStyleSheet).
    ../HMI/Interface/StyleSheetTemplate.cpp
    # Resolution de la famille de police embarquee (LOT-56 TACHE-03) : decision pure (booleen +
    # chaines), aucune dependance Qt (l'enregistrement reel via QFontDatabase reste dans
    # ApplicationTheme.cpp, non compile ici).
    ../HMI/Interface/FontResolution.cpp
    # Resolution du theme effectif de l'editeur (LOT-56 TACHE-06) : decision pure (reglage +
    # preference systeme -> thème a appliquer), aucune dependance Qt.
    ../HMI/Interface/ThemeResolution.cpp
    # Geometrie des icones et catalogue d'actions de l'editeur (LOT-56 TACHE-04) : logique pure
    # (listes de primitives, tables statiques), aucune dependance Qt (le rendu QPainter reel reste
    # dans ThemeIcons.cpp/EditorActions.cpp, non compiles ici).
    ../HMI/Interface/IconGeometry.cpp
    ../HMI/Game/LevelRunStats.cpp
    ../HMI/Interface/PixelArtScale.cpp
    ../HMI/Interface/EditorWorkspace.cpp
    ../HMI/Interface/PixelFrameGeometry.cpp
    ../HMI/Interface/MenuBackdropGeometry.cpp
    ../HMI/Interface/KeyHintText.cpp
    ../HMI/Interface/ActionCatalog.cpp
    # Dimensionnement des vignettes a l'echelle d'affichage (LOT-56 TACHE-05) : arithmetique pure,
    # aucune dependance Qt/GPU.
    ../HMI/Editor/ThumbnailGeometry.cpp
    # Operations sur tampon de pixels et historique nomme de l'atelier pixel art (LOT-54
    # TACHE-02) : logique pure sur hmi::DecodedImage (meme type que decodeImageFile, sans les
    # dependances Qt de son implementation), aucune dependance Qt/GPU.
    ../HMI/Editor/PixelOperations.cpp
    ../HMI/Editor/PixelHistory.cpp
    # Conversions vue <-> image du canevas pixel art (LOT-54 TACHE-03) : logique pure (aucune
    # dependance Qt/GPU), le widget PixelCanvas lui-meme (Qt) n'est compile que dans ProjectGaming,
    # meme distinction que ThumbnailGeometry.cpp/AssetThumbnailView.cpp.
    ../HMI/Editor/PixelCanvasGeometry.cpp
    # Reperes du mode creation (LOT-69 TACHE-07) : pelure d'oignon des tuiles, aplatissement des
    # plans voisins et reechantillonnage. Logique pure sur DecodedImage, sans Qt ni GPU.
    ../HMI/Editor/PlaneReference.cpp
    # Nommage et dimensions des fichiers de plan (LOT-69 TACHE-08) : derivation pure, sans disque.
    ../HMI/Editor/PlaneFileNaming.cpp
    # Tailles d'asset proposees a la creation dans l'atelier pixel art (LOT-54 TACHE-05) : logique
    # pure, derivee du contrat de dimensions (AssetContract.cpp, deja compile ci-dessus).
    ../HMI/Editor/PixelAssetIO.cpp
    # Palette de projet, extraction et contrainte de couleur (LOT-54 TACHE-07) : logique pure
    # (std::filesystem + nlohmann::json, deja lie ci-dessous), aucune dependance Qt/GPU.
    ../HMI/Editor/PixelPalette.cpp
    # Mode planche a raccords de l'atelier pixel art (LOT-54 TACHE-08) : logique pure, reutilise
    # la table canonique de TileAutotile.cpp (deja compilee ci-dessus), aucune dependance Qt/GPU.
    ../HMI/Editor/PixelAutotilePreview.cpp
)

# Encodage/decodage d'image (LOT-54 TACHE-01) : QImage (Qt) lit et ecrit le PNG, mais aucun device
# Direct3D n'est requis -- seul TextureLoader.cpp touche a Qt parmi les sources de UnitTests.
# Ajoute conditionnellement : sur un poste sans Qt, UnitTests doit continuer a se configurer et
# construire (meme garde que BUILD_EDITOR_QT dans Source/HMI/CMakeLists.txt, qui a deja resolu
# Qt6_DIR en cache si l'application a ete traitee avant ce dossier). Qt est toujours installe en
# CI (cf. ci.yml) : le test s'y execute donc systematiquement.
# GuiPrivate en plus de Gui depuis le LOT-69 TACHE-02 : TextureLoader.cpp cree desormais des
# QRhiTexture, et <rhi/qrhi.h> vit dans les en-tetes prives de QtGui. Le test, lui, ne touche que
# le decodage/encodage d'image -- aucune ressource GPU n'est creee sans QRhi.
find_package(Qt6 QUIET COMPONENTS Gui GuiPrivate)
if(Qt6_FOUND)
    target_sources(UnitTests PRIVATE
        Unit/HMI/Graphics/test_image_encode.cpp
        # Non-regression VISUELLE du portage QRhi (LOT-69 TACHE-02) : rendu hors ecran d'un motif
        # temoin, relu pixel par pixel. Se saute proprement si la machine n'offre aucune interface
        # QRhi -- c'est le seul test qui protege la nettete du pixel art.
        Unit/HMI/Graphics/test_rhi_offscreen.cpp
        # Table de correspondance Qt <-> Win32 (LOT-57 TACHE-04) : logique PURE, mais qui lit les
        # constantes de <qnamespace.h> -- elle depend des en-tetes Qt, jamais d'une QApplication.
        # Elle vit donc ici plutot qu'avec les autres sources HMI/Input compilees sans condition.
        Unit/HMI/Input/test_qt_key_map.cpp
        ../HMI/Input/QtKeyMap.cpp
        ../HMI/Graphics/TextureLoader.cpp
        ../HMI/Graphics/SpriteBatch.cpp)
    target_link_libraries(UnitTests PRIVATE Qt6::Gui Qt6::GuiPrivate)
    # Les shaders du pipeline sont embarques dans UnitTests comme dans l'executable : le test de
    # rendu hors ecran construit un vrai hmi::SpriteBatch, qui les charge depuis les ressources.
    qt6_add_shaders(UnitTests "unittests_shaders"
        PREFIX "/shaders"
        BASE "${PROJECT_SOURCE_DIR}/Source/HMI/Graphics/Shaders"
        FILES
            ${PROJECT_SOURCE_DIR}/Source/HMI/Graphics/Shaders/sprite.vert
            ${PROJECT_SOURCE_DIR}/Source/HMI/Graphics/Shaders/sprite.frag
    )
else()
    message(STATUS
        "UnitTests : Qt6 introuvable -- test d'encodage/decodage d'image (LOT-54) ignore.")
endif()

# Le moteur audio (LOT-60 TACHE-01) enveloppe QSoundEffect (Qt Multimedia) : sa dependance a la
# bibliotheque n'est testable qu'avec le module present. Ajoute conditionnellement, meme garde que
# Qt6::Gui/TextureLoader.cpp ci-dessus -- sur un poste sans le module, UnitTests continue de se
# configurer et de construire (DoD de la tache-01 : testable sans Qt6::Multimedia).
find_package(Qt6 QUIET COMPONENTS Multimedia)
if(Qt6_FOUND AND TARGET Qt6::Multimedia)
    target_sources(UnitTests PRIVATE
        Unit/HMI/Audio/test_audio_engine.cpp
        ../HMI/Audio/AudioEngine.cpp)
    target_link_libraries(UnitTests PRIVATE Qt6::Multimedia)
else()
    message(STATUS
        "UnitTests : Qt6::Multimedia introuvable -- test du moteur audio (LOT-60) ignore.")
endif()

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
    PROJECTGAMING_ASSETS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Assets"
    PROJECTGAMING_AUDIO_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Audio"
    PROJECTGAMING_THEME_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/Themes/theme.qss"
    PROJECTGAMING_MAIN_MENU_UI_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/UI/MainMenu.ui"
    PROJECTGAMING_OPTIONS_PAGE_UI_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/UI/OptionsPage.ui"
    PROJECTGAMING_FR_LANG_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/Localization/fr.lang"
    PROJECTGAMING_EN_LANG_PATH="${PROJECT_SOURCE_DIR}/Source/Elements/Localization/en.lang")

# Tests d'intégration : plusieurs briques assemblées collaborant de bout en bout.
add_executable(IntegrationTests
    Integration/test_ecs_mouvement.cpp
    Integration/test_boucle_simulation.cpp
    Integration/test_niveau_ecs.cpp
    Integration/test_physique_personnage.cpp
    Integration/test_animation_personnage.cpp
    Integration/test_bloc_reduit.cpp
    Integration/test_danger_avance.cpp
    Integration/test_plateforme_composition.cpp
    Integration/test_croisements_mecaniques.cpp
    # Non-régression récompense/épisode sur les niveaux demo-*.json livrés (LOT-ANNEXE-08,
    # TACHE-03) : rejoue le script d'entrées de test_parcours_complet.cpp via
    # aisolver::HeadlessLevelEnvironment.
    Integration/test_recompense_demo_niveaux.cpp
)

target_link_libraries(IntegrationTests PRIVATE
    Core
    # AiSolver (LOT-ANNEXE-08) : test_recompense_demo_niveaux.cpp rejoue les niveaux livrés via
    # aisolver::HeadlessLevelEnvironment et sa fonction de récompense.
    AiSolver
    GTest::gtest_main
    project_warnings
    project_options
)

# Chemin (source) des niveaux, pour le test d'intégration « fichier livré → ECS ».
target_compile_definitions(IntegrationTests PRIVATE
    PROJECTGAMING_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels")

# Tests système : un parcours de jeu complet sur le contenu livré, sans la couche GPU.
add_executable(SystemTests
    Systeme/test_couverture_mecaniques.cpp
    Systeme/test_parcours_complet.cpp
    Systeme/test_parcours_edition.cpp
    Systeme/test_plans_livres.cpp
    Systeme/test_rejeu_ia.cpp
)

target_link_libraries(SystemTests PRIVATE
    Core
    # AiSolver (LOT-ANNEXE-05, TACHE-05) : garde de fidelite pas-a-pas entre l'orchestration de
    # reference (playLevelTraced, test_parcours_complet.cpp) et aisolver::HeadlessLevelEnvironment.
    AiSolver
    GTest::gtest_main
    project_warnings
    project_options
)

# Chemin (source) des niveaux : le parcours complet rejoue les niveaux livrés.
# PROJECTGAMING_REPLAY_FIXTURES_DIR (LOT-ANNEXE-18, TACHE-03) : fixture de rejeu committee.
target_compile_definitions(SystemTests PRIVATE
    PROJECTGAMING_LEVELS_DIR="${PROJECT_SOURCE_DIR}/Source/Elements/Levels"
    PROJECTGAMING_REPLAY_FIXTURES_DIR="${PROJECT_SOURCE_DIR}/Source/Test/Systeme/Fixtures")

# Enregistre chaque TEST() aupres de CTest.
#
# LABELS : l'etage d'appartenance, pour pouvoir ne rejouer qu'une famille --
# `ctest --preset vs -L unitaire` pendant une boucle de developpement, les trois en CI.
# Les proprietes de `gtest_discover_tests` s'appliquent a TOUTE la cible : un etiquetage plus fin
# (les quelques tests d'entrainement IA qui pesent des dizaines de secondes) se fait par nom, ces
# tests etant tous nommes d'apres leur sujet -- p. ex.
# `ctest --preset vs -E 'Trainer|Comparator|CrossLevel|NonRegression'`.
#
# TIMEOUT : une borne de TERMINAISON, pas un objectif de performance. Sans elle, un test parti en
# boucle bloque le job jusqu'au delai global du runner, sans dire lequel. Le seuil laisse une marge
# large au test le plus lourd connu (~42 s en Debug) : il n'attrape qu'un veritable emballement.
gtest_discover_tests(UnitTests PROPERTIES LABELS unitaire TIMEOUT 300)
gtest_discover_tests(IntegrationTests PROPERTIES LABELS integration TIMEOUT 300)
# Le parcours complet rejoue les 24 tableaux livres, avec un plafond de 9 000 pas chacun.
gtest_discover_tests(SystemTests PROPERTIES LABELS systeme TIMEOUT 900)

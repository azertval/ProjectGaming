/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "ProjectGaming", "index.html", [
    [ "En bref", "index.html#autotoc_md1", null ],
    [ "Avancement", "index.html#autotoc_md2", null ],
    [ "Navigation", "index.html#autotoc_md3", null ],
    [ "Guide du développeur", "guide.html", [
      [ "Comment lire ce guide", "guide.html#autotoc_md158", null ],
      [ "Architecture en deux couches", "guide.html#autotoc_md159", null ],
      [ "Plan du guide", "guide.html#autotoc_md160", null ],
      [ "Boucle de jeu et pas de temps fixe", "guide-boucle.html", [
        [ "Qu'est-ce qu'une boucle de jeu ?", "guide-boucle.html#autotoc_md30", null ],
        [ "Le piège du framerate variable", "guide-boucle.html#autotoc_md31", null ],
        [ "Le principe du pas de temps fixe", "guide-boucle.html#autotoc_md32", null ],
        [ "L'accumulateur : \\ref core::FixedTimestep \"core::FixedTimestep\"", "guide-boucle.html#autotoc_md33", [
          [ "Exemple chiffré", "guide-boucle.html#autotoc_md34", null ],
          [ "La « spirale de la mort »", "guide-boucle.html#autotoc_md35", null ],
          [ "\\ref core::FixedTimestep::interpolationAlpha \"interpolationAlpha\"", "guide-boucle.html#autotoc_md36", null ],
          [ "Les frames sans pas de simulation et les entrées", "guide-boucle.html#autotoc_md37", null ]
        ] ],
        [ "Conséquence pratique pour tout le code de simulation", "guide-boucle.html#autotoc_md38", null ],
        [ "Voir aussi", "guide-boucle.html#autotoc_md39", null ]
      ] ],
      [ "ECS : entités, composants, systèmes", "guide-ecs.html", [
        [ "Le problème que l'ECS résout", "guide-ecs.html#autotoc_md49", null ],
        [ "L'entité : \\ref core::Entity \"core::Entity\"", "guide-ecs.html#autotoc_md50", null ],
        [ "Le \\ref core::World \"World\"", "guide-ecs.html#autotoc_md51", null ],
        [ "Le stockage : sparse set (core::ComponentPool<T>)", "guide-ecs.html#autotoc_md52", [
          [ "Ajout et suppression : <em>swap-and-pop</em>", "guide-ecs.html#autotoc_md53", null ],
          [ "Exemple pas à pas", "guide-ecs.html#autotoc_md54", null ]
        ] ],
        [ "Les vues : core::View<Components...>", "guide-ecs.html#autotoc_md55", null ],
        [ "Les systèmes et l'ordre d'exécution", "guide-ecs.html#autotoc_md56", null ],
        [ "Voir aussi", "guide-ecs.html#autotoc_md57", null ]
      ] ],
      [ "Mathématiques du moteur", "guide-maths.html", [
        [ "\\ref core::Vector2 \"Vector2\" : un point ou une direction dans le monde", "guide-maths.html#autotoc_md100", [
          [ "\\ref core::Vector2::lengthSquared \"lengthSquared\" : éviter la racine carrée", "guide-maths.html#autotoc_md101", null ],
          [ "Égalité approchée", "guide-maths.html#autotoc_md102", null ]
        ] ],
        [ "\\ref core::Aabb \"Aabb\" : la boîte englobante alignée aux axes", "guide-maths.html#autotoc_md103", null ],
        [ "Conventions d'unités et de repère", "guide-maths.html#autotoc_md104", null ],
        [ "Comparaison flottante : pourquoi l'égalité stricte est dangereuse", "guide-maths.html#autotoc_md105", null ],
        [ "Voir aussi", "guide-maths.html#autotoc_md106", null ]
      ] ],
      [ "Physique du personnage", "guide-physique.html", [
        [ "1. Collision par balayage continu (swept AABB)", "guide-physique.html#autotoc_md123", [
          [ "Le problème : le <em>tunneling</em>", "guide-physique.html#autotoc_md124", null ],
          [ "La solution : tester tout le trajet, pas seulement l'arrivée", "guide-physique.html#autotoc_md125", null ],
          [ "Méthode retenue : balayage <strong>par axe</strong> avec clamp direct", "guide-physique.html#autotoc_md126", null ],
          [ "Pourquoi caler directement plutôt que d'interpoler", "guide-physique.html#autotoc_md127", null ],
          [ "Lire le résultat : \\ref core::SweepResult \"core::SweepResult\"", "guide-physique.html#autotoc_md128", null ]
        ] ],
        [ "2. Suivi de pente et d'arrondi (EX-GP-003, EX-GP-004)", "guide-physique.html#autotoc_md129", [
          [ "Pourquoi une pente (ou un arrondi) n'est jamais solide", "guide-physique.html#autotoc_md130", null ],
          [ "<tt>core::slopeSurfaceHeight</tt> et <tt>core::resolveSlopeFollow</tt>", "guide-physique.html#autotoc_md131", null ],
          [ "Le piège du mur adjacent (correction du balayage horizontal)", "guide-physique.html#autotoc_md132", null ],
          [ "Particularité de l'arrondi : tangente verticale à une extrémité", "guide-physique.html#autotoc_md133", null ]
        ] ],
        [ "3. Gravité et intégration", "guide-physique.html#autotoc_md134", [
          [ "Vitesse terminale newtonienne (EX-GP-019)", "guide-physique.html#autotoc_md135", null ]
        ] ],
        [ "4. Saut et <em>game feel</em>", "guide-physique.html#autotoc_md136", null ],
        [ "5. Dash 8 directions", "guide-physique.html#autotoc_md137", null ],
        [ "6. Wall jump et wall slide", "guide-physique.html#autotoc_md138", null ],
        [ "Ordre d'un pas (résumé)", "guide-physique.html#autotoc_md139", null ],
        [ "Voir aussi", "guide-physique.html#autotoc_md140", null ]
      ] ],
      [ "Niveaux : modèle, chargement, mécanismes, budgets", "guide-niveaux.html", [
        [ "Le modèle en mémoire", "guide-niveaux.html#autotoc_md107", [
          [ "Deux systèmes de coordonnées à ne pas confondre", "guide-niveaux.html#autotoc_md108", null ],
          [ "\\ref core::TileType \"core::TileType\" : le vocabulaire des cases", "guide-niveaux.html#autotoc_md109", null ],
          [ "\\ref core::TileMap \"core::TileMap\" : la grille", "guide-niveaux.html#autotoc_md110", null ],
          [ "\\ref core::Level \"core::Level\" : le niveau assemblé", "guide-niveaux.html#autotoc_md111", null ]
        ] ],
        [ "Chargement JSON", "guide-niveaux.html#autotoc_md112", [
          [ "Exemple concret", "guide-niveaux.html#autotoc_md113", null ],
          [ "Validation", "guide-niveaux.html#autotoc_md114", null ]
        ] ],
        [ "De la grille aux entités : \\ref core::buildLevelScene \"buildLevelScene\"", "guide-niveaux.html#autotoc_md115", null ],
        [ "Mécanismes déclencheur ↔ porte", "guide-niveaux.html#autotoc_md116", null ],
        [ "Blocs poussables", "guide-niveaux.html#autotoc_md117", [
          [ "Blocs à taille réduite (<tt>×0.5</tt>/<tt>×0.25</tt>)", "guide-niveaux.html#autotoc_md118", null ]
        ] ],
        [ "Budget de mouvements", "guide-niveaux.html#autotoc_md119", null ],
        [ "Dangers avancés (<tt>LOT-31</tt>)", "guide-niveaux.html#autotoc_md120", null ],
        [ "Issue et enchaînement", "guide-niveaux.html#autotoc_md121", null ],
        [ "Voir aussi", "guide-niveaux.html#autotoc_md122", null ]
      ] ],
      [ "Entrées et actions logiques", "guide-entrees.html", [
        [ "Le principe : ne jamais coder « en dur » une touche dans le gameplay", "guide-entrees.html#autotoc_md77", null ],
        [ "Échantillonner plutôt que réagir : \\ref hmi::InputState \"hmi::InputState\"", "guide-entrees.html#autotoc_md78", [
          [ "Détecter les fronts, pas seulement l'état", "guide-entrees.html#autotoc_md79", null ],
          [ "Le cycle d'une frame", "guide-entrees.html#autotoc_md80", null ],
          [ "Un détail d'implémentation qui simplifie tout : \\ref hmi::Key \"Key\" réutilise les codes Win32", "guide-entrees.html#autotoc_md81", null ]
        ] ],
        [ "Traduire l'état en intention : \\ref hmi::toPlayerInput \"hmi::toPlayerInput\"", "guide-entrees.html#autotoc_md82", null ],
        [ "La manette : une seconde source, fusionnée en lecture (EX-CTRL-002, LOT-20)", "guide-entrees.html#autotoc_md83", null ],
        [ "Le menu d'options : la fusion manette à l'œuvre (\\ref hmi::OptionsModel \"hmi::OptionsModel\"/\\ref hmi::OptionsScreen \"OptionsScreen\")", "guide-entrees.html#autotoc_md84", null ],
        [ "Remapper les touches et boutons : \\ref hmi::GameKeyBindings \"GameKeyBindings\"/\\ref hmi::EditorKeyBindings \"EditorKeyBindings\" (LOT-29), \\ref hmi::GamepadBindings \"GamepadBindings\" (LOT-30)", "guide-entrees.html#autotoc_md85", null ],
        [ "La langue de l'interface : \\ref hmi::Localization \"hmi::Localization\" et \\ref hmi::LanguageSelector \"hmi::LanguageSelector\"", "guide-entrees.html#autotoc_md86", null ],
        [ "Voir aussi", "guide-entrees.html#autotoc_md87", null ]
      ] ],
      [ "Rendu 2D : de l'ECS à l'écran", "guide-rendu.html", [
        [ "Vocabulaire de base : GPU, swap chain, back buffer", "guide-rendu.html#autotoc_md141", null ],
        [ "\\ref hmi::GraphicsDevice \"hmi::GraphicsDevice\" : initialiser Direct3D 11 et présenter l'image", "guide-rendu.html#autotoc_md142", null ],
        [ "\\ref hmi::Window \"hmi::Window\" : la fenêtre, prérequis du rendu", "guide-rendu.html#autotoc_md143", null ],
        [ "Unités monde et pixels : \\ref hmi::Camera2D \"hmi::Camera2D\"", "guide-rendu.html#autotoc_md144", [
          [ "Cadrer un contenu plus grand que la fenêtre : <tt>fitZoom</tt> et <tt>hmi::RoomGrid</tt>", "guide-rendu.html#autotoc_md145", null ]
        ] ],
        [ "Le pipeline de dessin de sprites : \\ref hmi::SpriteBatch \"hmi::SpriteBatch\"", "guide-rendu.html#autotoc_md146", [
          [ "Pourquoi « batcher » plutôt que dessiner un sprite à la fois", "guide-rendu.html#autotoc_md147", null ],
          [ "\\ref hmi::SpriteQuad \"SpriteQuad\" : un rectangle texturé", "guide-rendu.html#autotoc_md148", null ],
          [ "Sommets, shaders, et échantillonnage <em>nearest</em>", "guide-rendu.html#autotoc_md149", null ]
        ] ],
        [ "\\ref hmi::TextureAtlas \"hmi::TextureAtlas\" : un spritesheet, généré en code", "guide-rendu.html#autotoc_md150", [
          [ "Les images du personnage : pourquoi elles vivent dans le même atlas", "guide-rendu.html#autotoc_md151", null ],
          [ "L'animation : une projection de l'état physique, pas un état séparé", "guide-rendu.html#autotoc_md152", null ]
        ] ],
        [ "\\ref hmi::SpriteRenderer \"hmi::SpriteRenderer\" : le pont ECS → écran", "guide-rendu.html#autotoc_md153", [
          [ "Interpoler le mouvement : <tt>hmi::PreviousPosition</tt> et le facteur d'interpolation", "guide-rendu.html#autotoc_md154", null ]
        ] ],
        [ "\\ref hmi::BitmapFont \"hmi::BitmapFont\" : dessiner du texte", "guide-rendu.html#autotoc_md155", null ],
        [ "Assembler la frame complète", "guide-rendu.html#autotoc_md156", null ],
        [ "Voir aussi", "guide-rendu.html#autotoc_md157", null ]
      ] ],
      [ "Journalisation et assertions", "guide-journalisation.html", [
        [ "Pourquoi journaliser dans un jeu vidéo", "guide-journalisation.html#autotoc_md88", null ],
        [ "Les niveaux de gravité : \\ref core::LogLevel \"core::LogLevel\"", "guide-journalisation.html#autotoc_md89", null ],
        [ "\\ref core::Logger \"core::Logger\" : filtrer puis diffuser", "guide-journalisation.html#autotoc_md90", null ],
        [ "Les sinks : où finissent les messages", "guide-journalisation.html#autotoc_md91", null ],
        [ "Les macros de journalisation, par catégorie", "guide-journalisation.html#autotoc_md92", [
          [ "Chaque module a sa propre catégorie", "guide-journalisation.html#autotoc_md93", null ],
          [ "Une règle de performance à respecter", "guide-journalisation.html#autotoc_md94", null ]
        ] ],
        [ "Le format d'une ligne : \\ref core::formatLogLine \"core::formatLogLine\"", "guide-journalisation.html#autotoc_md95", null ],
        [ "Configurer le niveau minimal au lancement", "guide-journalisation.html#autotoc_md96", [
          [ "Bootstrap réel : sinks différents en développement et en Release", "guide-journalisation.html#autotoc_md97", null ]
        ] ],
        [ "Assertions : \\ref PROJECTGAMING_ASSERT \"PROJECTGAMING_ASSERT\", un outil différent", "guide-journalisation.html#autotoc_md98", null ],
        [ "Voir aussi", "guide-journalisation.html#autotoc_md99", null ]
      ] ],
      [ "Éditeur de niveaux intégré", "guide-editeur.html", [
        [ "Le problème : éditer un niveau sans (re)coder le moteur", "guide-editeur.html#autotoc_md58", null ],
        [ "\\ref core::LevelDraft \"core::LevelDraft\" : un niveau qu'on peut défaire", "guide-editeur.html#autotoc_md59", [
          [ "Mécanismes : qui a le droit de se lier à qui", "guide-editeur.html#autotoc_md60", null ]
        ] ],
        [ "\\ref core::LevelWriter \"core::LevelWriter\" : l'inverse du chargement, avec un piège", "guide-editeur.html#autotoc_md61", null ],
        [ "\\ref hmi::EditorScreen \"EditorScreen\" : peindre, c'est convertir un pixel en case", "guide-editeur.html#autotoc_md62", [
          [ "La palette : une simple colonne de rectangles cliquables", "guide-editeur.html#autotoc_md63", null ],
          [ "Un clic, plusieurs significations possibles", "guide-editeur.html#autotoc_md64", null ],
          [ "Trois outils, une même grille : \\ref hmi::EditorTool \"EditorTool\"", "guide-editeur.html#autotoc_md65", null ],
          [ "Peindre par lot sans dupliquer la logique de peinture : \\ref core::LevelDraft::paintRegion \"LevelDraft::paintRegion\"", "guide-editeur.html#autotoc_md66", null ],
          [ "Lier deux tuiles sans dessiner de trait", "guide-editeur.html#autotoc_md67", null ]
        ] ],
        [ "Annuler/refaire : pourquoi des instantanés complets", "guide-editeur.html#autotoc_md68", null ],
        [ "Essai immédiat : jouer sans quitter l'éditeur", "guide-editeur.html#autotoc_md69", null ],
        [ "Enregistrer : valider avant d'écrire, jamais l'inverse", "guide-editeur.html#autotoc_md70", null ],
        [ "Garde-fous contre la perte de travail", "guide-editeur.html#autotoc_md71", null ],
        [ "Un champ de saisie de texte générique : nommer, renommer, redimensionner", "guide-editeur.html#autotoc_md72", null ],
        [ "Cadrer un niveau plus grand que la fenêtre", "guide-editeur.html#autotoc_md73", null ],
        [ "Un panneau plutôt que des bandes empilées", "guide-editeur.html#autotoc_md74", null ],
        [ "Choisir un niveau à éditer : \\ref hmi::LevelPicker \"hmi::LevelPicker\"", "guide-editeur.html#autotoc_md75", null ],
        [ "Voir aussi", "guide-editeur.html#autotoc_md76", null ]
      ] ],
      [ "Écrans et navigation", "guide-ecrans.html", [
        [ "Le problème : plusieurs écrans, une seule boucle", "guide-ecrans.html#autotoc_md40", null ],
        [ "Le contrat d'un écran : \\ref hmi::IScreen \"IScreen\" et \\ref hmi::ScreenTransition \"ScreenTransition\"", "guide-ecrans.html#autotoc_md41", null ],
        [ "Qui applique les transitions : \\ref hmi::ScreenManager \"ScreenManager\"", "guide-ecrans.html#autotoc_md42", null ],
        [ "La fabrique réelle : assembler les écrans dans <tt>main</tt>", "guide-ecrans.html#autotoc_md43", null ],
        [ "Où ça s'insère dans la boucle de jeu", "guide-ecrans.html#autotoc_md44", null ],
        [ "Les ressources partagées : \\ref hmi::RenderContext \"RenderContext\"", "guide-ecrans.html#autotoc_md45", null ],
        [ "Enchaîner des niveaux : \\ref hmi::LevelSequence \"LevelSequence\"", "guide-ecrans.html#autotoc_md46", null ],
        [ "Un cas particulier : l'essai immédiat de l'éditeur", "guide-ecrans.html#autotoc_md47", null ],
        [ "Voir aussi", "guide-ecrans.html#autotoc_md48", null ]
      ] ]
    ] ],
    [ "Cahier de test", "cahiertest.html", [
      [ "Tests unitaires (459)", "cahiertest.html#autotoc_md4", [
        [ "Core", "cahiertest.html#autotoc_md5", [
          [ "Diagnostics (14)", "cahiertest.html#autotoc_md6", null ],
          [ "Ecs (34)", "cahiertest.html#autotoc_md7", null ],
          [ "Gameplay (29)", "cahiertest.html#autotoc_md8", null ],
          [ "Levels (115)", "cahiertest.html#autotoc_md9", null ],
          [ "Math (20)", "cahiertest.html#autotoc_md10", null ],
          [ "Physics (39)", "cahiertest.html#autotoc_md11", null ],
          [ "Time (6)", "cahiertest.html#autotoc_md12", null ]
        ] ],
        [ "HMI", "cahiertest.html#autotoc_md13", [
          [ "Editor (57)", "cahiertest.html#autotoc_md14", null ],
          [ "Graphics (14)", "cahiertest.html#autotoc_md15", null ],
          [ "Input (63)", "cahiertest.html#autotoc_md16", null ],
          [ "Interface (59)", "cahiertest.html#autotoc_md17", null ],
          [ "Localization (8)", "cahiertest.html#autotoc_md18", null ]
        ] ]
      ] ],
      [ "Tests d'intégration (79)", "cahiertest.html#autotoc_md19", [
        [ "Animation Personnage — <tt>test_animation_personnage.cpp</tt> (5)", "cahiertest.html#autotoc_md20", null ],
        [ "Bloc Réduit — <tt>test_bloc_reduit.cpp</tt> (2)", "cahiertest.html#autotoc_md21", null ],
        [ "Boucle Simulation — <tt>test_boucle_simulation.cpp</tt> (2)", "cahiertest.html#autotoc_md22", null ],
        [ "Dangers avancés — <tt>test_danger_avance.cpp</tt> (4)", "cahiertest.html#autotoc_md23", null ],
        [ "Ecs Mouvement — <tt>test_ecs_mouvement.cpp</tt> (4)", "cahiertest.html#autotoc_md24", null ],
        [ "Niveau Ecs — <tt>test_niveau_ecs.cpp</tt> (2)", "cahiertest.html#autotoc_md25", null ],
        [ "Physique Personnage — <tt>test_physique_personnage.cpp</tt> (60)", "cahiertest.html#autotoc_md26", null ]
      ] ],
      [ "Tests système (3)", "cahiertest.html#autotoc_md27", [
        [ "Parcours Complet — <tt>test_parcours_complet.cpp</tt> (1)", "cahiertest.html#autotoc_md28", null ],
        [ "Éditeur de niveaux — <tt>test_parcours_edition.cpp</tt> (2)", "cahiertest.html#autotoc_md29", null ]
      ] ]
    ] ],
    [ "Spécifications", "specifications.html", [
      [ "Documents", "specifications.html#autotoc_md243", null ],
      [ "Vision & périmètre", "spec-vision.html", [
        [ "Concept", "spec-vision.html#autotoc_md244", [
          [ "Mécanique de jeu (décidée)", "spec-vision.html#autotoc_md245", null ]
        ] ],
        [ "Boucle de gameplay", "spec-vision.html#autotoc_md246", null ],
        [ "Objectifs (MVP)", "spec-vision.html#autotoc_md247", null ],
        [ "Objectifs produit (au-delà du moteur)", "spec-vision.html#autotoc_md248", null ],
        [ "Hors périmètre (MVP)", "spec-vision.html#autotoc_md249", null ],
        [ "Traçabilité", "spec-vision.html#autotoc_md250", null ]
      ] ],
      [ "Gameplay", "spec-gameplay.html", [
        [ "1. Monde en tuiles", "spec-gameplay.html#autotoc_md223", [
          [ "Dangers avancés (<tt>LOT-31</tt>)", "spec-gameplay.html#autotoc_md224", null ]
        ] ],
        [ "2. Personnage & déplacement", "spec-gameplay.html#autotoc_md225", [
          [ "Mécaniques aériennes avancées (au-delà du MVP)", "spec-gameplay.html#autotoc_md226", null ],
          [ "Ressenti (game feel) — ⚠️ à affiner par tests", "spec-gameplay.html#autotoc_md227", null ]
        ] ],
        [ "3. Mécanismes de puzzle", "spec-gameplay.html#autotoc_md228", null ],
        [ "4. Conditions de fin de niveau", "spec-gameplay.html#autotoc_md229", null ],
        [ "5. États de jeu", "spec-gameplay.html#autotoc_md230", null ],
        [ "Traçabilité", "spec-gameplay.html#autotoc_md231", null ]
      ] ],
      [ "Contrôles & entrées", "spec-controles.html", [
        [ "1. Périphériques", "spec-controles.html#autotoc_md173", null ],
        [ "2. Actions du jeu (mapping logique)", "spec-controles.html#autotoc_md174", null ],
        [ "3. Réactivité", "spec-controles.html#autotoc_md175", null ],
        [ "Traçabilité", "spec-controles.html#autotoc_md176", null ]
      ] ],
      [ "Rendu & cible technique", "spec-rendu-technique.html", [
        [ "1. Cible technique", "spec-rendu-technique.html#autotoc_md237", null ],
        [ "2. Rendu 2D", "spec-rendu-technique.html#autotoc_md238", null ],
        [ "3. Boucle & temps", "spec-rendu-technique.html#autotoc_md239", null ],
        [ "4. Interface (HMI)", "spec-rendu-technique.html#autotoc_md240", null ],
        [ "5. Audio (⚠️ minimal MVP)", "spec-rendu-technique.html#autotoc_md241", null ],
        [ "Traçabilité", "spec-rendu-technique.html#autotoc_md242", null ]
      ] ],
      [ "Niveaux & contenu", "spec-niveaux.html", [
        [ "1. Représentation des niveaux", "spec-niveaux.html#autotoc_md232", [
          [ "Format retenu (JSON, liste de tuiles-objets)", "spec-niveaux.html#autotoc_md233", null ]
        ] ],
        [ "2. Progression", "spec-niveaux.html#autotoc_md234", null ],
        [ "3. Conception (lignes directrices)", "spec-niveaux.html#autotoc_md235", null ],
        [ "Traçabilité", "spec-niveaux.html#autotoc_md236", null ]
      ] ],
      [ "Exigences non fonctionnelles", "spec-exigences.html", [
        [ "1. Performance", "spec-exigences.html#autotoc_md217", null ],
        [ "2. Architecture & maintenabilité", "spec-exigences.html#autotoc_md218", null ],
        [ "3. Qualité & vérification", "spec-exigences.html#autotoc_md219", null ],
        [ "4. Portabilité & reproductibilité", "spec-exigences.html#autotoc_md220", null ],
        [ "5. Robustesse", "spec-exigences.html#autotoc_md221", null ],
        [ "Traçabilité", "spec-exigences.html#autotoc_md222", null ]
      ] ],
      [ "Éditeur de niveaux", "spec-editeur.html", [
        [ "Objectif", "spec-editeur.html#autotoc_md204", null ],
        [ "1. Exigences fonctionnelles", "spec-editeur.html#autotoc_md205", null ],
        [ "2. Réutilisation & cohérence", "spec-editeur.html#autotoc_md206", null ],
        [ "3. Distribution & collaboration", "spec-editeur.html#autotoc_md207", null ],
        [ "4. Approche d'implémentation (décidée)", "spec-editeur.html#autotoc_md208", null ],
        [ "4bis. Décors & pixel art (post-MVP, intégré à l'éditeur)", "spec-editeur.html#autotoc_md209", null ],
        [ "5. Non-objectifs (éditeur, MVP)", "spec-editeur.html#autotoc_md210", null ],
        [ "6. Robustesse et confort d'édition (LOT-15)", "spec-editeur.html#autotoc_md211", null ],
        [ "7. Niveaux de grande taille (LOT-16)", "spec-editeur.html#autotoc_md212", null ],
        [ "8. Palette organisée par catégories (LOT-27)", "spec-editeur.html#autotoc_md213", null ],
        [ "9. Dangers avancés (<tt>LOT-31</tt>)", "spec-editeur.html#autotoc_md214", null ],
        [ "10. Niveaux à salles (<tt>LOT-32</tt>)", "spec-editeur.html#autotoc_md215", null ],
        [ "Traçabilité", "spec-editeur.html#autotoc_md216", null ]
      ] ],
      [ "Architecture (décisions dimensionnantes)", "spec-architecture.html", [
        [ "1. Modules & dépendances", "spec-architecture.html#autotoc_md161", null ],
        [ "2. Modèle d'entités : ECS", "spec-architecture.html#autotoc_md162", null ],
        [ "3. Coordonnées & unités — trois espaces distincts", "spec-architecture.html#autotoc_md163", null ],
        [ "4. Frontière simulation ↔ rendu", "spec-architecture.html#autotoc_md164", null ],
        [ "5. Mathématiques dans Core", "spec-architecture.html#autotoc_md165", null ],
        [ "6. Abstraction de rendu", "spec-architecture.html#autotoc_md166", null ],
        [ "7. Modèle de threading", "spec-architecture.html#autotoc_md167", null ],
        [ "8. Communication inter-systèmes", "spec-architecture.html#autotoc_md168", null ],
        [ "9. Gestion des ressources", "spec-architecture.html#autotoc_md169", null ],
        [ "10. Contrainte « éditeur intégré »", "spec-architecture.html#autotoc_md170", null ],
        [ "11. Décors dynamiques (accommodation dimensionnante)", "spec-architecture.html#autotoc_md171", null ],
        [ "Traçabilité", "spec-architecture.html#autotoc_md172", null ]
      ] ],
      [ "Décors & pipeline pixel art", "spec-decors.html", [
        [ "Vision", "spec-decors.html#autotoc_md196", null ],
        [ "1. Système de décors", "spec-decors.html#autotoc_md197", null ],
        [ "2. Manipulation", "spec-decors.html#autotoc_md198", [
          [ "À la conception (éditeur)", "spec-decors.html#autotoc_md199", null ],
          [ "En jeu (mécanique, à terme)", "spec-decors.html#autotoc_md200", null ]
        ] ],
        [ "3. Pipeline photo → pixel art (intégré à l'éditeur)", "spec-decors.html#autotoc_md201", null ],
        [ "4. Périmètre & séquencement", "spec-decors.html#autotoc_md202", null ],
        [ "Traçabilité", "spec-decors.html#autotoc_md203", null ]
      ] ],
      [ "Conventions de code", "spec-conventions.html", [
        [ "1. Langage & standard", "spec-conventions.html#autotoc_md178", null ],
        [ "2. Nommage", "spec-conventions.html#autotoc_md179", null ],
        [ "3. Mise en forme", "spec-conventions.html#autotoc_md180", null ],
        [ "4. Inclusions (#include)", "spec-conventions.html#autotoc_md181", [
          [ "Chemins complets depuis Source/", "spec-conventions.html#autotoc_md182", null ],
          [ "Ordre des groupes", "spec-conventions.html#autotoc_md183", null ]
        ] ],
        [ "5. Architecture (dépendances entre modules)", "spec-conventions.html#autotoc_md184", [
          [ "Classes plutôt que fonctions libres", "spec-conventions.html#autotoc_md185", null ],
          [ "RAII obligatoire", "spec-conventions.html#autotoc_md186", null ]
        ] ],
        [ "6. Documentation Doxygen", "spec-conventions.html#autotoc_md187", [
          [ "Doxygen dans le header, commentaires simples // dans le .cpp", "spec-conventions.html#autotoc_md188", null ],
          [ "Documentation du corps (.cpp)", "spec-conventions.html#autotoc_md189", null ]
        ] ],
        [ "7. Bonnes pratiques", "spec-conventions.html#autotoc_md190", null ],
        [ "8. Tests", "spec-conventions.html#autotoc_md191", null ],
        [ "9. Gestion des erreurs", "spec-conventions.html#autotoc_md192", null ],
        [ "10. Assertions & journalisation", "spec-conventions.html#autotoc_md193", null ],
        [ "11. Outillage qualité (automatisé)", "spec-conventions.html#autotoc_md194", null ],
        [ "12. Identifiants d'exigences (EX-…)", "spec-conventions.html#autotoc_md195", null ]
      ] ]
    ] ],
    [ "Lots", "lots.html", [
      [ "Lots", "lots.html#autotoc_md1589", null ],
      [ "LOT-01 — Fenêtre & boucle de jeu (Direct3D 11)", "lot-01.html", [
        [ "Objectif", "lot-01.html#autotoc_md251", null ],
        [ "Périmètre", "lot-01.html#autotoc_md252", [
          [ "Inclus", "lot-01.html#autotoc_md253", null ],
          [ "Exclus (lots ultérieurs)", "lot-01.html#autotoc_md254", null ]
        ] ],
        [ "Exigences couvertes", "lot-01.html#autotoc_md255", null ],
        [ "Découpage", "lot-01.html#autotoc_md256", null ],
        [ "Critères d'acceptation du lot", "lot-01.html#autotoc_md257", null ],
        [ "Navigation des tâches", "lot-01.html#autotoc_md258", null ],
        [ "TACHE-01 — Fenêtre Win32 & pompe de messages", "lot-01-tache-01-fenetre-win32.html", [
          [ "Contexte", "lot-01-tache-01-fenetre-win32.html#autotoc_md259", null ],
          [ "Travail à réaliser", "lot-01-tache-01-fenetre-win32.html#autotoc_md260", null ],
          [ "Fichiers impactés", "lot-01-tache-01-fenetre-win32.html#autotoc_md261", null ],
          [ "Points d'attention", "lot-01-tache-01-fenetre-win32.html#autotoc_md262", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-01-fenetre-win32.html#autotoc_md263", null ],
          [ "Exigences", "lot-01-tache-01-fenetre-win32.html#autotoc_md264", null ]
        ] ],
        [ "TACHE-02 — Initialisation Direct3D 11 (RAII)", "lot-01-tache-02-init-direct3d11.html", [
          [ "Contexte", "lot-01-tache-02-init-direct3d11.html#autotoc_md265", null ],
          [ "Travail à réaliser", "lot-01-tache-02-init-direct3d11.html#autotoc_md266", null ],
          [ "Fichiers impactés", "lot-01-tache-02-init-direct3d11.html#autotoc_md267", null ],
          [ "Points d'attention", "lot-01-tache-02-init-direct3d11.html#autotoc_md268", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-02-init-direct3d11.html#autotoc_md269", null ],
          [ "Exigences", "lot-01-tache-02-init-direct3d11.html#autotoc_md270", null ]
        ] ],
        [ "TACHE-03 — Boucle à pas de temps fixe (testable)", "lot-01-tache-03-boucle-pas-fixe.html", [
          [ "Contexte", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md271", null ],
          [ "Travail à réaliser", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md272", null ],
          [ "Fichiers impactés", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md273", null ],
          [ "Tests (obligatoires)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md274", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md275", null ],
          [ "Exigences", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md276", null ]
        ] ],
        [ "TACHE-04 — Effacement écran, présentation & redimensionnement", "lot-01-tache-04-effacement-presentation.html", [
          [ "Contexte", "lot-01-tache-04-effacement-presentation.html#autotoc_md277", null ],
          [ "Travail à réaliser", "lot-01-tache-04-effacement-presentation.html#autotoc_md278", null ],
          [ "Fichiers impactés", "lot-01-tache-04-effacement-presentation.html#autotoc_md279", null ],
          [ "Points d'attention", "lot-01-tache-04-effacement-presentation.html#autotoc_md280", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-04-effacement-presentation.html#autotoc_md281", null ],
          [ "Exigences", "lot-01-tache-04-effacement-presentation.html#autotoc_md282", null ]
        ] ],
        [ "TACHE-05 — Intégration main & vérification", "lot-01-tache-05-integration.html", [
          [ "Contexte", "lot-01-tache-05-integration.html#autotoc_md283", null ],
          [ "Travail à réaliser", "lot-01-tache-05-integration.html#autotoc_md284", null ],
          [ "Fichiers impactés", "lot-01-tache-05-integration.html#autotoc_md285", null ],
          [ "Vérification (manuelle + automatique)", "lot-01-tache-05-integration.html#autotoc_md286", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-05-integration.html#autotoc_md287", null ],
          [ "Exigences", "lot-01-tache-05-integration.html#autotoc_md288", null ]
        ] ]
      ] ],
      [ "LOT-02 — Journalisation & diagnostics", "lot-02.html", [
        [ "Objectif", "lot-02.html#autotoc_md289", null ],
        [ "Périmètre", "lot-02.html#autotoc_md290", [
          [ "Inclus", "lot-02.html#autotoc_md291", null ],
          [ "Exclus (plus tard)", "lot-02.html#autotoc_md292", null ]
        ] ],
        [ "Exigences couvertes", "lot-02.html#autotoc_md293", null ],
        [ "Découpage", "lot-02.html#autotoc_md294", null ],
        [ "Critères d'acceptation du lot", "lot-02.html#autotoc_md295", null ],
        [ "Dépendances", "lot-02.html#autotoc_md296", null ],
        [ "Navigation des tâches", "lot-02.html#autotoc_md297", null ],
        [ "TACHE-01 — Niveaux de log & interface Logger", "lot-02-tache-01-niveaux-logger.html", [
          [ "Contexte", "lot-02-tache-01-niveaux-logger.html#autotoc_md298", null ],
          [ "Travail à réaliser", "lot-02-tache-01-niveaux-logger.html#autotoc_md299", null ],
          [ "Fichiers impactés", "lot-02-tache-01-niveaux-logger.html#autotoc_md300", null ],
          [ "Tests (obligatoires)", "lot-02-tache-01-niveaux-logger.html#autotoc_md301", null ],
          [ "Points d'attention", "lot-02-tache-01-niveaux-logger.html#autotoc_md302", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-01-niveaux-logger.html#autotoc_md303", null ],
          [ "Exigences", "lot-02-tache-01-niveaux-logger.html#autotoc_md304", null ]
        ] ],
        [ "TACHE-02 — Sinks enfichables", "lot-02-tache-02-sinks.html", [
          [ "Contexte", "lot-02-tache-02-sinks.html#autotoc_md305", null ],
          [ "Travail à réaliser", "lot-02-tache-02-sinks.html#autotoc_md306", null ],
          [ "Fichiers impactés", "lot-02-tache-02-sinks.html#autotoc_md307", null ],
          [ "Tests (obligatoires)", "lot-02-tache-02-sinks.html#autotoc_md308", null ],
          [ "Points d'attention", "lot-02-tache-02-sinks.html#autotoc_md309", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-02-sinks.html#autotoc_md310", null ],
          [ "Exigences", "lot-02-tache-02-sinks.html#autotoc_md311", null ]
        ] ],
        [ "TACHE-03 — Macros de log (fichier/ligne, horodatage)", "lot-02-tache-03-macros-log.html", [
          [ "Contexte", "lot-02-tache-03-macros-log.html#autotoc_md312", null ],
          [ "Travail à réaliser", "lot-02-tache-03-macros-log.html#autotoc_md313", null ],
          [ "Fichiers impactés", "lot-02-tache-03-macros-log.html#autotoc_md314", null ],
          [ "Tests (obligatoires)", "lot-02-tache-03-macros-log.html#autotoc_md315", null ],
          [ "Points d'attention", "lot-02-tache-03-macros-log.html#autotoc_md316", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-03-macros-log.html#autotoc_md317", null ],
          [ "Exigences", "lot-02-tache-03-macros-log.html#autotoc_md318", null ]
        ] ],
        [ "TACHE-04 — Assertions PROJECTGAMING_ASSERT", "lot-02-tache-04-assertions.html", [
          [ "Contexte", "lot-02-tache-04-assertions.html#autotoc_md319", null ],
          [ "Travail à réaliser", "lot-02-tache-04-assertions.html#autotoc_md320", null ],
          [ "Fichiers impactés", "lot-02-tache-04-assertions.html#autotoc_md321", null ],
          [ "Tests (obligatoires)", "lot-02-tache-04-assertions.html#autotoc_md322", null ],
          [ "Points d'attention", "lot-02-tache-04-assertions.html#autotoc_md323", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-04-assertions.html#autotoc_md324", null ],
          [ "Exigences", "lot-02-tache-04-assertions.html#autotoc_md325", null ]
        ] ],
        [ "TACHE-05 — Intégration dans main & documentation", "lot-02-tache-05-integration.html", [
          [ "Contexte", "lot-02-tache-05-integration.html#autotoc_md326", null ],
          [ "Travail à réaliser", "lot-02-tache-05-integration.html#autotoc_md327", null ],
          [ "Fichiers impactés", "lot-02-tache-05-integration.html#autotoc_md328", null ],
          [ "Vérification", "lot-02-tache-05-integration.html#autotoc_md329", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-05-integration.html#autotoc_md330", null ],
          [ "Exigences", "lot-02-tache-05-integration.html#autotoc_md331", null ]
        ] ]
      ] ],
      [ "LOT-03 — Fondation ECS & mathématiques Core", "lot-03.html", [
        [ "Objectif", "lot-03.html#autotoc_md332", null ],
        [ "⚠️ Décision préalable : ECS maison vs bibliothèque", "lot-03.html#autotoc_md333", null ],
        [ "Périmètre", "lot-03.html#autotoc_md334", [
          [ "Inclus", "lot-03.html#autotoc_md335", null ],
          [ "Exclus (lots ultérieurs)", "lot-03.html#autotoc_md336", null ]
        ] ],
        [ "Exigences couvertes", "lot-03.html#autotoc_md337", null ],
        [ "Découpage", "lot-03.html#autotoc_md338", null ],
        [ "Critères d'acceptation du lot", "lot-03.html#autotoc_md339", null ],
        [ "Dépendances", "lot-03.html#autotoc_md340", null ],
        [ "Navigation des tâches", "lot-03.html#autotoc_md341", null ],
        [ "TACHE-01 — Types mathématiques de Core", "lot-03-tache-01-math-core.html", [
          [ "Contexte", "lot-03-tache-01-math-core.html#autotoc_md342", null ],
          [ "Travail à réaliser", "lot-03-tache-01-math-core.html#autotoc_md343", null ],
          [ "Fichiers impactés", "lot-03-tache-01-math-core.html#autotoc_md344", null ],
          [ "Tests (obligatoires)", "lot-03-tache-01-math-core.html#autotoc_md345", null ],
          [ "Points d'attention", "lot-03-tache-01-math-core.html#autotoc_md346", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-01-math-core.html#autotoc_md347", null ],
          [ "Exigences", "lot-03-tache-01-math-core.html#autotoc_md348", null ]
        ] ],
        [ "TACHE-02 — Entités : handles générationnels & cycle de vie", "lot-03-tache-02-entites.html", [
          [ "Contexte", "lot-03-tache-02-entites.html#autotoc_md349", null ],
          [ "Travail à réaliser", "lot-03-tache-02-entites.html#autotoc_md350", null ],
          [ "Fichiers impactés", "lot-03-tache-02-entites.html#autotoc_md351", null ],
          [ "Tests (obligatoires)", "lot-03-tache-02-entites.html#autotoc_md352", null ],
          [ "Points d'attention", "lot-03-tache-02-entites.html#autotoc_md353", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-02-entites.html#autotoc_md354", null ],
          [ "Exigences", "lot-03-tache-02-entites.html#autotoc_md355", null ]
        ] ],
        [ "TACHE-03 — Stockage de composants (sparse set typé)", "lot-03-tache-03-stockage-composants.html", [
          [ "Contexte", "lot-03-tache-03-stockage-composants.html#autotoc_md356", null ],
          [ "Travail à réaliser", "lot-03-tache-03-stockage-composants.html#autotoc_md357", null ],
          [ "Fichiers impactés", "lot-03-tache-03-stockage-composants.html#autotoc_md358", null ],
          [ "Tests (obligatoires)", "lot-03-tache-03-stockage-composants.html#autotoc_md359", null ],
          [ "Points d'attention", "lot-03-tache-03-stockage-composants.html#autotoc_md360", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-03-stockage-composants.html#autotoc_md361", null ],
          [ "Exigences", "lot-03-tache-03-stockage-composants.html#autotoc_md362", null ]
        ] ],
        [ "TACHE-04 — Requêtes / vues multi-composants", "lot-03-tache-04-vues-requetes.html", [
          [ "Contexte", "lot-03-tache-04-vues-requetes.html#autotoc_md363", null ],
          [ "Travail à réaliser", "lot-03-tache-04-vues-requetes.html#autotoc_md364", null ],
          [ "Fichiers impactés", "lot-03-tache-04-vues-requetes.html#autotoc_md365", null ],
          [ "Tests (obligatoires)", "lot-03-tache-04-vues-requetes.html#autotoc_md366", null ],
          [ "Points d'attention", "lot-03-tache-04-vues-requetes.html#autotoc_md367", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-04-vues-requetes.html#autotoc_md368", null ],
          [ "Exigences", "lot-03-tache-04-vues-requetes.html#autotoc_md369", null ]
        ] ],
        [ "TACHE-05 — Systèmes & World (orchestration au pas fixe)", "lot-03-tache-05-systemes-world.html", [
          [ "Contexte", "lot-03-tache-05-systemes-world.html#autotoc_md370", null ],
          [ "Travail à réaliser", "lot-03-tache-05-systemes-world.html#autotoc_md371", null ],
          [ "Fichiers impactés", "lot-03-tache-05-systemes-world.html#autotoc_md372", null ],
          [ "Tests (obligatoires)", "lot-03-tache-05-systemes-world.html#autotoc_md373", null ],
          [ "Points d'attention", "lot-03-tache-05-systemes-world.html#autotoc_md374", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-05-systemes-world.html#autotoc_md375", null ],
          [ "Exigences", "lot-03-tache-05-systemes-world.html#autotoc_md376", null ]
        ] ],
        [ "TACHE-06 — Composant Transform + système de mouvement (démo)", "lot-03-tache-06-transform-demo.html", [
          [ "Contexte", "lot-03-tache-06-transform-demo.html#autotoc_md377", null ],
          [ "Travail à réaliser", "lot-03-tache-06-transform-demo.html#autotoc_md378", null ],
          [ "Fichiers impactés", "lot-03-tache-06-transform-demo.html#autotoc_md379", null ],
          [ "Tests (obligatoires)", "lot-03-tache-06-transform-demo.html#autotoc_md380", null ],
          [ "Points d'attention", "lot-03-tache-06-transform-demo.html#autotoc_md381", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-06-transform-demo.html#autotoc_md382", null ],
          [ "Exigences", "lot-03-tache-06-transform-demo.html#autotoc_md383", null ]
        ] ]
      ] ],
      [ "LOT-04 — Documentation Doxygen & réorganisation de l'arborescence documentaire", "lot-04.html", [
        [ "Objectif", "lot-04.html#autotoc_md384", null ],
        [ "Périmètre", "lot-04.html#autotoc_md385", [
          [ "Inclus", "lot-04.html#autotoc_md386", null ],
          [ "Exclus (lots ultérieurs)", "lot-04.html#autotoc_md387", null ]
        ] ],
        [ "Décisions de cadrage", "lot-04.html#autotoc_md388", null ],
        [ "Exigences couvertes", "lot-04.html#autotoc_md389", null ],
        [ "Découpage", "lot-04.html#autotoc_md390", null ],
        [ "Critères d'acceptation du lot", "lot-04.html#autotoc_md391", null ],
        [ "Dépendances", "lot-04.html#autotoc_md392", null ],
        [ "Navigation des tâches", "lot-04.html#autotoc_md393", null ],
        [ "TACHE-01 — Réorganisation de l'arborescence documentaire", "lot-04-tache-01-reorganisation-arbo.html", [
          [ "Contexte", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md394", null ],
          [ "Travail à réaliser", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md395", null ],
          [ "Fichiers impactés", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md396", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md397", null ],
          [ "Points d'attention", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md398", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md399", null ],
          [ "Exigences", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md400", null ]
        ] ],
        [ "TACHE-02 — Configuration Doxygen pour le Markdown", "lot-04-tache-02-config-doxygen-markdown.html", [
          [ "Contexte", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md401", null ],
          [ "Travail à réaliser", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md402", null ],
          [ "Fichiers impactés", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md403", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md404", null ],
          [ "Points d'attention", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md405", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md406", null ],
          [ "Exigences", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md407", null ]
        ] ],
        [ "TACHE-03 — Page d'accueil du projet (mainpage)", "lot-04-tache-03-page-accueil.html", [
          [ "Contexte", "lot-04-tache-03-page-accueil.html#autotoc_md408", null ],
          [ "Travail à réaliser", "lot-04-tache-03-page-accueil.html#autotoc_md409", null ],
          [ "Fichiers impactés", "lot-04-tache-03-page-accueil.html#autotoc_md410", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-03-page-accueil.html#autotoc_md411", null ],
          [ "Points d'attention", "lot-04-tache-03-page-accueil.html#autotoc_md412", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-03-page-accueil.html#autotoc_md413", null ],
          [ "Exigences", "lot-04-tache-03-page-accueil.html#autotoc_md414", null ]
        ] ],
        [ "TACHE-04 — Pages de spécification navigables (conventions incluses)", "lot-04-tache-04-pages-specification.html", [
          [ "Contexte", "lot-04-tache-04-pages-specification.html#autotoc_md415", null ],
          [ "Travail à réaliser", "lot-04-tache-04-pages-specification.html#autotoc_md416", null ],
          [ "Convention d'insertion d'une nouvelle spec (à documenter dans l'index)", "lot-04-tache-04-pages-specification.html#autotoc_md417", null ],
          [ "Fichiers impactés", "lot-04-tache-04-pages-specification.html#autotoc_md418", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-04-pages-specification.html#autotoc_md419", null ],
          [ "Points d'attention", "lot-04-tache-04-pages-specification.html#autotoc_md420", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-04-pages-specification.html#autotoc_md421", null ],
          [ "Exigences", "lot-04-tache-04-pages-specification.html#autotoc_md422", null ]
        ] ],
        [ "TACHE-05 — Pages de lots navigables", "lot-04-tache-05-pages-lots.html", [
          [ "Contexte", "lot-04-tache-05-pages-lots.html#autotoc_md423", null ],
          [ "Travail à réaliser", "lot-04-tache-05-pages-lots.html#autotoc_md424", null ],
          [ "Fichiers impactés", "lot-04-tache-05-pages-lots.html#autotoc_md425", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-05-pages-lots.html#autotoc_md426", null ],
          [ "Points d'attention", "lot-04-tache-05-pages-lots.html#autotoc_md427", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-05-pages-lots.html#autotoc_md428", null ],
          [ "Exigences", "lot-04-tache-05-pages-lots.html#autotoc_md429", null ]
        ] ],
        [ "TACHE-06 — Manuel utilisateur (squelette + première page)", "lot-04-tache-06-manuel-utilisateur.html", [
          [ "Contexte", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md430", null ],
          [ "Travail à réaliser", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md431", null ],
          [ "Fichiers impactés", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md432", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md433", null ],
          [ "Points d'attention", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md434", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md435", null ],
          [ "Exigences", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md436", null ]
        ] ],
        [ "TACHE-07 — CI documentation (WARN_AS_ERROR & déploiement)", "lot-04-tache-07-ci-docs.html", [
          [ "Contexte", "lot-04-tache-07-ci-docs.html#autotoc_md437", null ],
          [ "Travail à réaliser", "lot-04-tache-07-ci-docs.html#autotoc_md438", null ],
          [ "Fichiers impactés", "lot-04-tache-07-ci-docs.html#autotoc_md439", null ],
          [ "Avertissements connus à corriger avant WARN_AS_ERROR (relevés en TACHE-02)", "lot-04-tache-07-ci-docs.html#autotoc_md440", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-07-ci-docs.html#autotoc_md441", null ],
          [ "Points d'attention", "lot-04-tache-07-ci-docs.html#autotoc_md442", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-07-ci-docs.html#autotoc_md443", null ],
          [ "Exigences", "lot-04-tache-07-ci-docs.html#autotoc_md444", null ]
        ] ],
        [ "TACHE-08 — Traçabilité des exigences (IDs stables, ancres Doxygen, lint CI)", "lot-04-tache-08-tracabilite-exigences.html", [
          [ "Contexte", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md445", null ],
          [ "Règle à formaliser (dans conventions.md)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md446", null ],
          [ "Travail à réaliser", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md447", null ],
          [ "Fichiers impactés", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md448", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md449", null ],
          [ "Points d'attention", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md450", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md451", null ],
          [ "Exigences", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md452", null ]
        ] ]
      ] ],
      [ "LOT-05 — Rendu 2D : atlas, sprites & caméra", "lot-05.html", [
        [ "Objectif", "lot-05.html#autotoc_md453", null ],
        [ "Périmètre", "lot-05.html#autotoc_md454", [
          [ "Inclus", "lot-05.html#autotoc_md455", null ],
          [ "Exclus (lots ultérieurs)", "lot-05.html#autotoc_md456", null ]
        ] ],
        [ "Décisions de cadrage", "lot-05.html#autotoc_md457", null ],
        [ "Exigences couvertes", "lot-05.html#autotoc_md458", null ],
        [ "Découpage", "lot-05.html#autotoc_md459", null ],
        [ "Critères d'acceptation du lot", "lot-05.html#autotoc_md460", null ],
        [ "Dépendances", "lot-05.html#autotoc_md461", null ],
        [ "Navigation des tâches", "lot-05.html#autotoc_md462", null ],
        [ "TACHE-01 — Composant Sprite (données pures)", "lot-05-tache-01-composant-sprite.html", [
          [ "Contexte", "lot-05-tache-01-composant-sprite.html#autotoc_md463", null ],
          [ "Travail à réaliser", "lot-05-tache-01-composant-sprite.html#autotoc_md464", null ],
          [ "Fichiers impactés", "lot-05-tache-01-composant-sprite.html#autotoc_md465", null ],
          [ "Tests (obligatoires si logique)", "lot-05-tache-01-composant-sprite.html#autotoc_md466", null ],
          [ "Points d'attention", "lot-05-tache-01-composant-sprite.html#autotoc_md467", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-01-composant-sprite.html#autotoc_md468", null ],
          [ "Exigences", "lot-05-tache-01-composant-sprite.html#autotoc_md469", null ]
        ] ],
        [ "TACHE-02 — Pipeline de quads texturés (HLSL, blend, nearest)", "lot-05-tache-02-pipeline-quads-textures.html", [
          [ "Contexte", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md470", null ],
          [ "Travail à réaliser", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md471", null ],
          [ "Fichiers impactés", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md472", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md473", null ],
          [ "Points d'attention", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md474", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md475", null ],
          [ "Exigences", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md476", null ]
        ] ],
        [ "TACHE-03 — Atlas de textures procédural", "lot-05-tache-03-atlas-procedural.html", [
          [ "Contexte", "lot-05-tache-03-atlas-procedural.html#autotoc_md477", null ],
          [ "Travail à réaliser", "lot-05-tache-03-atlas-procedural.html#autotoc_md478", null ],
          [ "Fichiers impactés", "lot-05-tache-03-atlas-procedural.html#autotoc_md479", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-03-atlas-procedural.html#autotoc_md480", null ],
          [ "Points d'attention", "lot-05-tache-03-atlas-procedural.html#autotoc_md481", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-03-atlas-procedural.html#autotoc_md482", null ],
          [ "Exigences", "lot-05-tache-03-atlas-procedural.html#autotoc_md483", null ]
        ] ],
        [ "TACHE-04 — Caméra 2D (monde → écran)", "lot-05-tache-04-camera-2d.html", [
          [ "Contexte", "lot-05-tache-04-camera-2d.html#autotoc_md484", null ],
          [ "Travail à réaliser", "lot-05-tache-04-camera-2d.html#autotoc_md485", null ],
          [ "Fichiers impactés", "lot-05-tache-04-camera-2d.html#autotoc_md486", null ],
          [ "Tests (obligatoires)", "lot-05-tache-04-camera-2d.html#autotoc_md487", null ],
          [ "Points d'attention", "lot-05-tache-04-camera-2d.html#autotoc_md488", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-04-camera-2d.html#autotoc_md489", null ],
          [ "Exigences", "lot-05-tache-04-camera-2d.html#autotoc_md490", null ]
        ] ],
        [ "TACHE-05 — Système de rendu des sprites (ECS → écran)", "lot-05-tache-05-systeme-rendu-sprites.html", [
          [ "Contexte", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md491", null ],
          [ "Travail à réaliser", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md492", null ],
          [ "Fichiers impactés", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md493", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md494", null ],
          [ "Points d'attention", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md495", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md496", null ],
          [ "Exigences", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md497", null ]
        ] ],
        [ "TACHE-06 — Câblage du World dans la boucle + scène de démo", "lot-05-tache-06-cablage-world-demo.html", [
          [ "Contexte", "lot-05-tache-06-cablage-world-demo.html#autotoc_md498", null ],
          [ "Travail à réaliser", "lot-05-tache-06-cablage-world-demo.html#autotoc_md499", null ],
          [ "Fichiers impactés", "lot-05-tache-06-cablage-world-demo.html#autotoc_md500", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md501", null ],
          [ "Points d'attention", "lot-05-tache-06-cablage-world-demo.html#autotoc_md502", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md503", null ],
          [ "Exigences", "lot-05-tache-06-cablage-world-demo.html#autotoc_md504", null ]
        ] ]
      ] ],
      [ "LOT-06 — Menu principal", "lot-06.html", [
        [ "Objectif", "lot-06.html#autotoc_md505", null ],
        [ "Périmètre", "lot-06.html#autotoc_md506", [
          [ "Inclus", "lot-06.html#autotoc_md507", null ],
          [ "Exclus (lots ultérieurs)", "lot-06.html#autotoc_md508", null ]
        ] ],
        [ "Décisions de cadrage", "lot-06.html#autotoc_md509", null ],
        [ "Exigences couvertes", "lot-06.html#autotoc_md510", null ],
        [ "Découpage", "lot-06.html#autotoc_md511", null ],
        [ "Critères d'acceptation du lot", "lot-06.html#autotoc_md512", null ],
        [ "Dépendances", "lot-06.html#autotoc_md513", null ],
        [ "Navigation des tâches", "lot-06.html#autotoc_md514", null ],
        [ "TACHE-01 — Entrées clavier & souris", "lot-06-tache-01-entrees-clavier-souris.html", [
          [ "Contexte", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md515", null ],
          [ "Travail à réaliser", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md516", null ],
          [ "Fichiers impactés", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md517", null ],
          [ "Tests (obligatoires)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md518", null ],
          [ "Points d'attention", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md519", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md520", null ],
          [ "Exigences", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md521", null ]
        ] ],
        [ "TACHE-02 — Rendu de texte (police bitmap)", "lot-06-tache-02-rendu-texte-bitmap.html", [
          [ "Contexte", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md522", null ],
          [ "Travail à réaliser", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md523", null ],
          [ "Fichiers impactés", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md524", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md525", null ],
          [ "Points d'attention", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md526", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md527", null ],
          [ "Exigences", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md528", null ]
        ] ],
        [ "TACHE-03 — Catalogue de traduction (i18n)", "lot-06-tache-03-catalogue-traduction.html", [
          [ "Contexte", "lot-06-tache-03-catalogue-traduction.html#autotoc_md529", null ],
          [ "Travail à réaliser", "lot-06-tache-03-catalogue-traduction.html#autotoc_md530", null ],
          [ "Fichiers impactés", "lot-06-tache-03-catalogue-traduction.html#autotoc_md531", null ],
          [ "Tests (obligatoires)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md532", null ],
          [ "Points d'attention", "lot-06-tache-03-catalogue-traduction.html#autotoc_md533", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md534", null ],
          [ "Exigences", "lot-06-tache-03-catalogue-traduction.html#autotoc_md535", null ]
        ] ],
        [ "TACHE-04 — États d'application (écrans)", "lot-06-tache-04-etats-application.html", [
          [ "Contexte", "lot-06-tache-04-etats-application.html#autotoc_md536", null ],
          [ "Travail à réaliser", "lot-06-tache-04-etats-application.html#autotoc_md537", null ],
          [ "Fichiers impactés", "lot-06-tache-04-etats-application.html#autotoc_md538", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-04-etats-application.html#autotoc_md539", null ],
          [ "Points d'attention", "lot-06-tache-04-etats-application.html#autotoc_md540", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-04-etats-application.html#autotoc_md541", null ],
          [ "Exigences", "lot-06-tache-04-etats-application.html#autotoc_md542", null ]
        ] ],
        [ "TACHE-05 — Écran de menu principal", "lot-06-tache-05-ecran-menu-principal.html", [
          [ "Contexte", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md543", null ],
          [ "Travail à réaliser", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md544", null ],
          [ "Fichiers impactés", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md545", null ],
          [ "Tests (obligatoires)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md546", null ],
          [ "Points d'attention", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md547", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md548", null ],
          [ "Exigences", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md549", null ]
        ] ],
        [ "TACHE-06 — Écrans cibles (jeu démo + éditeur placeholder)", "lot-06-tache-06-ecrans-cibles.html", [
          [ "Contexte", "lot-06-tache-06-ecrans-cibles.html#autotoc_md550", null ],
          [ "Travail à réaliser", "lot-06-tache-06-ecrans-cibles.html#autotoc_md551", null ],
          [ "Fichiers impactés", "lot-06-tache-06-ecrans-cibles.html#autotoc_md552", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md553", null ],
          [ "Points d'attention", "lot-06-tache-06-ecrans-cibles.html#autotoc_md554", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md555", null ],
          [ "Exigences", "lot-06-tache-06-ecrans-cibles.html#autotoc_md556", null ]
        ] ],
        [ "TACHE-07 — Intégration main (boucle pilotée par l'écran)", "lot-06-tache-07-integration-main.html", [
          [ "Contexte", "lot-06-tache-07-integration-main.html#autotoc_md557", null ],
          [ "Travail à réaliser", "lot-06-tache-07-integration-main.html#autotoc_md558", null ],
          [ "Fichiers impactés", "lot-06-tache-07-integration-main.html#autotoc_md559", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-07-integration-main.html#autotoc_md560", null ],
          [ "Points d'attention", "lot-06-tache-07-integration-main.html#autotoc_md561", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-07-integration-main.html#autotoc_md562", null ],
          [ "Exigences", "lot-06-tache-07-integration-main.html#autotoc_md563", null ]
        ] ],
        [ "TACHE-08 — Sélecteur de langue", "lot-06-tache-08-selecteur-langue.html", [
          [ "Contexte", "lot-06-tache-08-selecteur-langue.html#autotoc_md564", null ],
          [ "Travail à réaliser", "lot-06-tache-08-selecteur-langue.html#autotoc_md565", null ],
          [ "Fichiers impactés", "lot-06-tache-08-selecteur-langue.html#autotoc_md566", null ],
          [ "Tests (obligatoires)", "lot-06-tache-08-selecteur-langue.html#autotoc_md567", null ],
          [ "Points d'attention", "lot-06-tache-08-selecteur-langue.html#autotoc_md568", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-08-selecteur-langue.html#autotoc_md569", null ],
          [ "Exigences", "lot-06-tache-08-selecteur-langue.html#autotoc_md570", null ]
        ] ]
      ] ],
      [ "LOT-07 — Niveaux : modèle et chargement", "lot-07.html", [
        [ "Objectif", "lot-07.html#autotoc_md571", null ],
        [ "Périmètre", "lot-07.html#autotoc_md572", [
          [ "Inclus", "lot-07.html#autotoc_md573", null ],
          [ "Exclus (lots ultérieurs)", "lot-07.html#autotoc_md574", null ]
        ] ],
        [ "Décisions de cadrage", "lot-07.html#autotoc_md575", null ],
        [ "Exigences couvertes", "lot-07.html#autotoc_md576", null ],
        [ "Découpage", "lot-07.html#autotoc_md577", null ],
        [ "Critères d'acceptation du lot", "lot-07.html#autotoc_md578", null ],
        [ "Dépendances", "lot-07.html#autotoc_md579", null ],
        [ "Navigation des tâches", "lot-07.html#autotoc_md580", null ],
        [ "TACHE-01 — Dépendance JSON (nlohmann/json épinglé)", "lot-07-tache-01-dependance-json.html", [
          [ "Contexte", "lot-07-tache-01-dependance-json.html#autotoc_md581", null ],
          [ "Travail à réaliser", "lot-07-tache-01-dependance-json.html#autotoc_md582", null ],
          [ "Fichiers impactés", "lot-07-tache-01-dependance-json.html#autotoc_md583", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-01-dependance-json.html#autotoc_md584", null ],
          [ "Points d'attention", "lot-07-tache-01-dependance-json.html#autotoc_md585", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-01-dependance-json.html#autotoc_md586", null ],
          [ "Exigences", "lot-07-tache-01-dependance-json.html#autotoc_md587", null ]
        ] ],
        [ "TACHE-02 — Modèle de tuiles et de niveau", "lot-07-tache-02-modele-niveau.html", [
          [ "Contexte", "lot-07-tache-02-modele-niveau.html#autotoc_md588", null ],
          [ "Travail à réaliser", "lot-07-tache-02-modele-niveau.html#autotoc_md589", null ],
          [ "Fichiers impactés", "lot-07-tache-02-modele-niveau.html#autotoc_md590", null ],
          [ "Tests (obligatoires)", "lot-07-tache-02-modele-niveau.html#autotoc_md591", null ],
          [ "Points d'attention", "lot-07-tache-02-modele-niveau.html#autotoc_md592", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-02-modele-niveau.html#autotoc_md593", null ],
          [ "Exigences", "lot-07-tache-02-modele-niveau.html#autotoc_md594", null ]
        ] ],
        [ "TACHE-03 — Chargement du niveau (JSON)", "lot-07-tache-03-chargement-json.html", [
          [ "Contexte", "lot-07-tache-03-chargement-json.html#autotoc_md595", null ],
          [ "Travail à réaliser", "lot-07-tache-03-chargement-json.html#autotoc_md596", null ],
          [ "Fichiers impactés", "lot-07-tache-03-chargement-json.html#autotoc_md597", null ],
          [ "Tests (obligatoires)", "lot-07-tache-03-chargement-json.html#autotoc_md598", null ],
          [ "Points d'attention", "lot-07-tache-03-chargement-json.html#autotoc_md599", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-03-chargement-json.html#autotoc_md600", null ],
          [ "Exigences", "lot-07-tache-03-chargement-json.html#autotoc_md601", null ]
        ] ],
        [ "TACHE-04 — Validation du niveau", "lot-07-tache-04-validation.html", [
          [ "Contexte", "lot-07-tache-04-validation.html#autotoc_md602", null ],
          [ "Travail à réaliser", "lot-07-tache-04-validation.html#autotoc_md603", null ],
          [ "Fichiers impactés", "lot-07-tache-04-validation.html#autotoc_md604", null ],
          [ "Tests (obligatoires)", "lot-07-tache-04-validation.html#autotoc_md605", null ],
          [ "Points d'attention", "lot-07-tache-04-validation.html#autotoc_md606", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-04-validation.html#autotoc_md607", null ],
          [ "Exigences", "lot-07-tache-04-validation.html#autotoc_md608", null ]
        ] ],
        [ "TACHE-05 — Niveau de démonstration", "lot-07-tache-05-niveau-demo.html", [
          [ "Contexte", "lot-07-tache-05-niveau-demo.html#autotoc_md609", null ],
          [ "Travail à réaliser", "lot-07-tache-05-niveau-demo.html#autotoc_md610", null ],
          [ "Fichiers impactés", "lot-07-tache-05-niveau-demo.html#autotoc_md611", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-05-niveau-demo.html#autotoc_md612", null ],
          [ "Points d'attention", "lot-07-tache-05-niveau-demo.html#autotoc_md613", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-05-niveau-demo.html#autotoc_md614", null ],
          [ "Exigences", "lot-07-tache-05-niveau-demo.html#autotoc_md615", null ]
        ] ],
        [ "TACHE-06 — Rendu du niveau + intégration « Charger niveau »", "lot-07-tache-06-rendu-integration.html", [
          [ "Contexte", "lot-07-tache-06-rendu-integration.html#autotoc_md616", null ],
          [ "Travail à réaliser", "lot-07-tache-06-rendu-integration.html#autotoc_md617", null ],
          [ "Fichiers impactés", "lot-07-tache-06-rendu-integration.html#autotoc_md618", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-06-rendu-integration.html#autotoc_md619", null ],
          [ "Points d'attention", "lot-07-tache-06-rendu-integration.html#autotoc_md620", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-06-rendu-integration.html#autotoc_md621", null ],
          [ "Exigences", "lot-07-tache-06-rendu-integration.html#autotoc_md622", null ]
        ] ]
      ] ],
      [ "LOT-08 — Gameplay personnage : déplacement, gravité et collisions", "lot-08.html", [
        [ "Objectif", "lot-08.html#autotoc_md623", null ],
        [ "Périmètre", "lot-08.html#autotoc_md624", [
          [ "Inclus", "lot-08.html#autotoc_md625", null ],
          [ "Exclus (lots ultérieurs)", "lot-08.html#autotoc_md626", null ]
        ] ],
        [ "Décisions de cadrage", "lot-08.html#autotoc_md627", null ],
        [ "Exigences couvertes", "lot-08.html#autotoc_md628", null ],
        [ "Découpage", "lot-08.html#autotoc_md629", null ],
        [ "Critères d'acceptation du lot", "lot-08.html#autotoc_md630", null ],
        [ "Dépendances", "lot-08.html#autotoc_md631", null ],
        [ "Navigation des tâches", "lot-08.html#autotoc_md632", null ],
        [ "TACHE-01 — Composants du personnage & intention d'entrée", "lot-08-tache-01-composants-personnage.html", [
          [ "Contexte", "lot-08-tache-01-composants-personnage.html#autotoc_md633", null ],
          [ "Travail à réaliser", "lot-08-tache-01-composants-personnage.html#autotoc_md634", null ],
          [ "Fichiers impactés", "lot-08-tache-01-composants-personnage.html#autotoc_md635", null ],
          [ "Tests (obligatoires)", "lot-08-tache-01-composants-personnage.html#autotoc_md636", null ],
          [ "Points d'attention", "lot-08-tache-01-composants-personnage.html#autotoc_md637", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-01-composants-personnage.html#autotoc_md638", null ],
          [ "Exigences", "lot-08-tache-01-composants-personnage.html#autotoc_md639", null ]
        ] ],
        [ "TACHE-02 — Balayage AABB contre la grille (géométrie pure)", "lot-08-tache-02-balayage-aabb.html", [
          [ "Contexte", "lot-08-tache-02-balayage-aabb.html#autotoc_md640", null ],
          [ "Travail à réaliser", "lot-08-tache-02-balayage-aabb.html#autotoc_md641", null ],
          [ "Fichiers impactés", "lot-08-tache-02-balayage-aabb.html#autotoc_md642", null ],
          [ "Tests (obligatoires)", "lot-08-tache-02-balayage-aabb.html#autotoc_md643", null ],
          [ "Points d'attention", "lot-08-tache-02-balayage-aabb.html#autotoc_md644", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-02-balayage-aabb.html#autotoc_md645", null ],
          [ "Exigences", "lot-08-tache-02-balayage-aabb.html#autotoc_md646", null ]
        ] ],
        [ "TACHE-03 — Physique du personnage (gravité + déplacement + collisions)", "lot-08-tache-03-physique-personnage.html", [
          [ "Contexte", "lot-08-tache-03-physique-personnage.html#autotoc_md647", null ],
          [ "Travail à réaliser", "lot-08-tache-03-physique-personnage.html#autotoc_md648", null ],
          [ "Fichiers impactés", "lot-08-tache-03-physique-personnage.html#autotoc_md649", null ],
          [ "Tests (obligatoires)", "lot-08-tache-03-physique-personnage.html#autotoc_md650", null ],
          [ "Points d'attention", "lot-08-tache-03-physique-personnage.html#autotoc_md651", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-03-physique-personnage.html#autotoc_md652", null ],
          [ "Exigences", "lot-08-tache-03-physique-personnage.html#autotoc_md653", null ]
        ] ],
        [ "TACHE-04 — Règles de fin de niveau (succès / échec)", "lot-08-tache-04-regles-fin-niveau.html", [
          [ "Contexte", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md654", null ],
          [ "Travail à réaliser", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md655", null ],
          [ "Fichiers impactés", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md656", null ],
          [ "Tests (obligatoires)", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md657", null ],
          [ "Points d'attention", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md658", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md659", null ],
          [ "Exigences", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md660", null ]
        ] ],
        [ "TACHE-05 — Actions logiques d'entrée (mapping touches → intention)", "lot-08-tache-05-actions-logiques.html", [
          [ "Contexte", "lot-08-tache-05-actions-logiques.html#autotoc_md661", null ],
          [ "Travail à réaliser", "lot-08-tache-05-actions-logiques.html#autotoc_md662", null ],
          [ "Fichiers impactés", "lot-08-tache-05-actions-logiques.html#autotoc_md663", null ],
          [ "Tests (obligatoires)", "lot-08-tache-05-actions-logiques.html#autotoc_md664", null ],
          [ "Points d'attention", "lot-08-tache-05-actions-logiques.html#autotoc_md665", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-05-actions-logiques.html#autotoc_md666", null ],
          [ "Exigences", "lot-08-tache-05-actions-logiques.html#autotoc_md667", null ]
        ] ],
        [ "TACHE-06 — Intégration jouable dans GameScreen (cadrage fixe, succès / échec)", "lot-08-tache-06-integration-jouable.html", [
          [ "Contexte", "lot-08-tache-06-integration-jouable.html#autotoc_md668", null ],
          [ "Travail à réaliser", "lot-08-tache-06-integration-jouable.html#autotoc_md669", null ],
          [ "Fichiers impactés", "lot-08-tache-06-integration-jouable.html#autotoc_md670", null ],
          [ "Vérification (visuelle, pas de test unitaire)", "lot-08-tache-06-integration-jouable.html#autotoc_md671", null ],
          [ "Points d'attention", "lot-08-tache-06-integration-jouable.html#autotoc_md672", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-06-integration-jouable.html#autotoc_md673", null ],
          [ "Exigences", "lot-08-tache-06-integration-jouable.html#autotoc_md674", null ]
        ] ]
      ] ],
      [ "LOT-09 — Saut, game feel et enchaînement de niveaux", "lot-09.html", [
        [ "Objectif", "lot-09.html#autotoc_md675", null ],
        [ "Périmètre", "lot-09.html#autotoc_md676", [
          [ "Inclus", "lot-09.html#autotoc_md677", null ],
          [ "Exclus (lots ultérieurs)", "lot-09.html#autotoc_md678", null ]
        ] ],
        [ "Décisions de cadrage", "lot-09.html#autotoc_md679", null ],
        [ "Exigences couvertes", "lot-09.html#autotoc_md680", null ],
        [ "Découpage", "lot-09.html#autotoc_md681", null ],
        [ "Critères d'acceptation du lot", "lot-09.html#autotoc_md682", null ],
        [ "Dépendances", "lot-09.html#autotoc_md683", null ],
        [ "Navigation des tâches", "lot-09.html#autotoc_md684", null ],
        [ "TACHE-01 — Données du saut : PlayerInput, Player, PhysicsConfig", "lot-09-tache-01-donnees-saut.html", [
          [ "Contexte", "lot-09-tache-01-donnees-saut.html#autotoc_md685", null ],
          [ "Travail à réaliser", "lot-09-tache-01-donnees-saut.html#autotoc_md686", null ],
          [ "Fichiers impactés", "lot-09-tache-01-donnees-saut.html#autotoc_md687", null ],
          [ "Tests (obligatoires)", "lot-09-tache-01-donnees-saut.html#autotoc_md688", null ],
          [ "Points d'attention", "lot-09-tache-01-donnees-saut.html#autotoc_md689", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-01-donnees-saut.html#autotoc_md690", null ],
          [ "Exigences", "lot-09-tache-01-donnees-saut.html#autotoc_md691", null ]
        ] ],
        [ "TACHE-02 — Mapping du saut (Espace/W → intention)", "lot-09-tache-02-mapping-saut.html", [
          [ "Contexte", "lot-09-tache-02-mapping-saut.html#autotoc_md692", null ],
          [ "Travail à réaliser", "lot-09-tache-02-mapping-saut.html#autotoc_md693", null ],
          [ "Fichiers impactés", "lot-09-tache-02-mapping-saut.html#autotoc_md694", null ],
          [ "Tests (obligatoires)", "lot-09-tache-02-mapping-saut.html#autotoc_md695", null ],
          [ "Points d'attention", "lot-09-tache-02-mapping-saut.html#autotoc_md696", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-02-mapping-saut.html#autotoc_md697", null ],
          [ "Exigences", "lot-09-tache-02-mapping-saut.html#autotoc_md698", null ]
        ] ],
        [ "TACHE-03 — Saut au sol + hauteur variable", "lot-09-tache-03-saut-hauteur-variable.html", [
          [ "Contexte", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md699", null ],
          [ "Travail à réaliser", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md700", null ],
          [ "Fichiers impactés", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md701", null ],
          [ "Tests (obligatoires)", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md702", null ],
          [ "Points d'attention", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md703", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md704", null ],
          [ "Exigences", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md705", null ]
        ] ],
        [ "TACHE-04 — Coyote time + jump buffering", "lot-09-tache-04-coyote-buffering.html", [
          [ "Contexte", "lot-09-tache-04-coyote-buffering.html#autotoc_md706", null ],
          [ "Travail à réaliser", "lot-09-tache-04-coyote-buffering.html#autotoc_md707", null ],
          [ "Fichiers impactés", "lot-09-tache-04-coyote-buffering.html#autotoc_md708", null ],
          [ "Tests (obligatoires)", "lot-09-tache-04-coyote-buffering.html#autotoc_md709", null ],
          [ "Points d'attention", "lot-09-tache-04-coyote-buffering.html#autotoc_md710", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-04-coyote-buffering.html#autotoc_md711", null ],
          [ "Exigences", "lot-09-tache-04-coyote-buffering.html#autotoc_md712", null ]
        ] ],
        [ "TACHE-05 — Enchaînement de niveaux (séquence, auto-avance, retour titre)", "lot-09-tache-05-enchainement-niveaux.html", [
          [ "Contexte", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md713", null ],
          [ "Travail à réaliser", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md714", null ],
          [ "Fichiers impactés", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md715", null ],
          [ "Vérification / tests", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md716", null ],
          [ "Points d'attention", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md717", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md718", null ],
          [ "Exigences", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md719", null ]
        ] ],
        [ "TACHE-06 — Niveaux de démo (séquence, dont saut requis) + preuve", "lot-09-tache-06-niveaux-demo.html", [
          [ "Contexte", "lot-09-tache-06-niveaux-demo.html#autotoc_md720", null ],
          [ "Travail à réaliser", "lot-09-tache-06-niveaux-demo.html#autotoc_md721", null ],
          [ "Fichiers impactés", "lot-09-tache-06-niveaux-demo.html#autotoc_md722", null ],
          [ "Tests (obligatoires)", "lot-09-tache-06-niveaux-demo.html#autotoc_md723", null ],
          [ "Points d'attention", "lot-09-tache-06-niveaux-demo.html#autotoc_md724", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-06-niveaux-demo.html#autotoc_md725", null ],
          [ "Exigences", "lot-09-tache-06-niveaux-demo.html#autotoc_md726", null ]
        ] ]
      ] ],
      [ "LOT-10 — Mécaniques aériennes avancées : double saut, wall jump, dash", "lot-10.html", [
        [ "Objectif", "lot-10.html#autotoc_md727", null ],
        [ "Périmètre", "lot-10.html#autotoc_md728", [
          [ "Inclus", "lot-10.html#autotoc_md729", null ],
          [ "Exclus (lots ultérieurs)", "lot-10.html#autotoc_md730", null ]
        ] ],
        [ "Décisions de cadrage", "lot-10.html#autotoc_md731", null ],
        [ "Exigences couvertes", "lot-10.html#autotoc_md732", null ],
        [ "Découpage", "lot-10.html#autotoc_md733", null ],
        [ "Critères d'acceptation du lot", "lot-10.html#autotoc_md734", null ],
        [ "Dépendances", "lot-10.html#autotoc_md735", null ],
        [ "Navigation des tâches", "lot-10.html#autotoc_md736", null ],
        [ "TACHE-01 — Données des mécaniques (PlayerInput, Player, PhysicsConfig)", "lot-10-tache-01-donnees.html", [
          [ "Contexte", "lot-10-tache-01-donnees.html#autotoc_md737", null ],
          [ "Travail à réaliser", "lot-10-tache-01-donnees.html#autotoc_md738", null ],
          [ "Fichiers impactés", "lot-10-tache-01-donnees.html#autotoc_md739", null ],
          [ "Tests (obligatoires)", "lot-10-tache-01-donnees.html#autotoc_md740", null ],
          [ "Points d'attention", "lot-10-tache-01-donnees.html#autotoc_md741", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-01-donnees.html#autotoc_md742", null ],
          [ "Exigences", "lot-10-tache-01-donnees.html#autotoc_md743", null ]
        ] ],
        [ "TACHE-02 — Mapping du dash + direction de visée / orientation", "lot-10-tache-02-mapping-dash.html", [
          [ "Contexte", "lot-10-tache-02-mapping-dash.html#autotoc_md744", null ],
          [ "Travail à réaliser", "lot-10-tache-02-mapping-dash.html#autotoc_md745", null ],
          [ "Fichiers impactés", "lot-10-tache-02-mapping-dash.html#autotoc_md746", null ],
          [ "Tests (obligatoires)", "lot-10-tache-02-mapping-dash.html#autotoc_md747", null ],
          [ "Points d'attention", "lot-10-tache-02-mapping-dash.html#autotoc_md748", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-02-mapping-dash.html#autotoc_md749", null ],
          [ "Exigences", "lot-10-tache-02-mapping-dash.html#autotoc_md750", null ]
        ] ],
        [ "TACHE-03 — Double saut (sauts aériens rechargés au sol)", "lot-10-tache-03-double-saut.html", [
          [ "Contexte", "lot-10-tache-03-double-saut.html#autotoc_md751", null ],
          [ "Travail à réaliser", "lot-10-tache-03-double-saut.html#autotoc_md752", null ],
          [ "Fichiers impactés", "lot-10-tache-03-double-saut.html#autotoc_md753", null ],
          [ "Tests (obligatoires)", "lot-10-tache-03-double-saut.html#autotoc_md754", null ],
          [ "Points d'attention", "lot-10-tache-03-double-saut.html#autotoc_md755", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-03-double-saut.html#autotoc_md756", null ],
          [ "Exigences", "lot-10-tache-03-double-saut.html#autotoc_md757", null ]
        ] ],
        [ "TACHE-04 — Wall jump + wall slide", "lot-10-tache-04-wall-jump.html", [
          [ "Contexte", "lot-10-tache-04-wall-jump.html#autotoc_md758", null ],
          [ "Travail à réaliser", "lot-10-tache-04-wall-jump.html#autotoc_md759", null ],
          [ "Fichiers impactés", "lot-10-tache-04-wall-jump.html#autotoc_md760", null ],
          [ "Tests (obligatoires)", "lot-10-tache-04-wall-jump.html#autotoc_md761", null ],
          [ "Points d'attention", "lot-10-tache-04-wall-jump.html#autotoc_md762", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-04-wall-jump.html#autotoc_md763", null ],
          [ "Exigences", "lot-10-tache-04-wall-jump.html#autotoc_md764", null ]
        ] ],
        [ "TACHE-05 — Dash 8 directions (burst, durée, recharge au sol)", "lot-10-tache-05-dash.html", [
          [ "Contexte", "lot-10-tache-05-dash.html#autotoc_md765", null ],
          [ "Travail à réaliser", "lot-10-tache-05-dash.html#autotoc_md766", null ],
          [ "Fichiers impactés", "lot-10-tache-05-dash.html#autotoc_md767", null ],
          [ "Tests (obligatoires)", "lot-10-tache-05-dash.html#autotoc_md768", null ],
          [ "Points d'attention", "lot-10-tache-05-dash.html#autotoc_md769", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-05-dash.html#autotoc_md770", null ],
          [ "Exigences", "lot-10-tache-05-dash.html#autotoc_md771", null ]
        ] ],
        [ "TACHE-06 — Niveau de démo « parkour » + preuve système", "lot-10-tache-06-niveau-parkour.html", [
          [ "Contexte", "lot-10-tache-06-niveau-parkour.html#autotoc_md772", null ],
          [ "Travail à réaliser", "lot-10-tache-06-niveau-parkour.html#autotoc_md773", null ],
          [ "Fichiers impactés", "lot-10-tache-06-niveau-parkour.html#autotoc_md774", null ],
          [ "Tests (obligatoires)", "lot-10-tache-06-niveau-parkour.html#autotoc_md775", null ],
          [ "Points d'attention", "lot-10-tache-06-niveau-parkour.html#autotoc_md776", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-06-niveau-parkour.html#autotoc_md777", null ],
          [ "Exigences", "lot-10-tache-06-niveau-parkour.html#autotoc_md778", null ]
        ] ]
      ] ],
      [ "LOT-11 — Ressenti avancé : personnage humanoïde, gravité asymétrique, finitions", "lot-11.html", [
        [ "Objectif", "lot-11.html#autotoc_md779", null ],
        [ "Périmètre", "lot-11.html#autotoc_md780", [
          [ "Inclus", "lot-11.html#autotoc_md781", null ],
          [ "Exclus (lots ultérieurs)", "lot-11.html#autotoc_md782", null ]
        ] ],
        [ "Décisions de cadrage", "lot-11.html#autotoc_md783", null ],
        [ "Exigences couvertes", "lot-11.html#autotoc_md784", null ],
        [ "Découpage", "lot-11.html#autotoc_md785", null ],
        [ "Critères d'acceptation du lot", "lot-11.html#autotoc_md786", null ],
        [ "Dépendances", "lot-11.html#autotoc_md787", null ],
        [ "Navigation des tâches", "lot-11.html#autotoc_md788", null ],
        [ "TACHE-01 — Données : réglages de *feel* + taille/placement du personnage", "lot-11-tache-01-donnees.html", [
          [ "Contexte", "lot-11-tache-01-donnees.html#autotoc_md789", null ],
          [ "Travail à réaliser", "lot-11-tache-01-donnees.html#autotoc_md790", null ],
          [ "Fichiers impactés", "lot-11-tache-01-donnees.html#autotoc_md791", null ],
          [ "Tests (obligatoires)", "lot-11-tache-01-donnees.html#autotoc_md792", null ],
          [ "Points d'attention", "lot-11-tache-01-donnees.html#autotoc_md793", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-01-donnees.html#autotoc_md794", null ],
          [ "Exigences", "lot-11-tache-01-donnees.html#autotoc_md795", null ]
        ] ],
        [ "TACHE-02 — Gravité asymétrique + apex hang + fast-fall", "lot-11-tache-02-gravite-asymetrique.html", [
          [ "Contexte", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md796", null ],
          [ "Travail à réaliser", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md797", null ],
          [ "Fichiers impactés", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md798", null ],
          [ "Tests (obligatoires)", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md799", null ],
          [ "Points d'attention", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md800", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md801", null ],
          [ "Exigences", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md802", null ]
        ] ],
        [ "TACHE-03 — Personnage humanoïde (spawn 0,4×0,8, sprite)", "lot-11-tache-03-personnage-humanoide.html", [
          [ "Contexte", "lot-11-tache-03-personnage-humanoide.html#autotoc_md803", null ],
          [ "Travail à réaliser", "lot-11-tache-03-personnage-humanoide.html#autotoc_md804", null ],
          [ "Fichiers impactés", "lot-11-tache-03-personnage-humanoide.html#autotoc_md805", null ],
          [ "Vérification (visuelle, pas de test unitaire — brique GPU)", "lot-11-tache-03-personnage-humanoide.html#autotoc_md806", null ],
          [ "Points d'attention", "lot-11-tache-03-personnage-humanoide.html#autotoc_md807", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-03-personnage-humanoide.html#autotoc_md808", null ],
          [ "Exigences", "lot-11-tache-03-personnage-humanoide.html#autotoc_md809", null ]
        ] ],
        [ "TACHE-04 — Rééquilibrage des niveaux + preuves à la vraie taille", "lot-11-tache-04-reequilibrage.html", [
          [ "Contexte", "lot-11-tache-04-reequilibrage.html#autotoc_md810", null ],
          [ "Travail à réaliser", "lot-11-tache-04-reequilibrage.html#autotoc_md811", null ],
          [ "Fichiers impactés", "lot-11-tache-04-reequilibrage.html#autotoc_md812", null ],
          [ "Tests (obligatoires)", "lot-11-tache-04-reequilibrage.html#autotoc_md813", null ],
          [ "Points d'attention", "lot-11-tache-04-reequilibrage.html#autotoc_md814", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-04-reequilibrage.html#autotoc_md815", null ],
          [ "Exigences", "lot-11-tache-04-reequilibrage.html#autotoc_md816", null ]
        ] ]
      ] ],
      [ "LOT-12 — Niveau puzzle : mécanismes interrupteur/porte + budget de mouvements", "lot-12.html", [
        [ "Objectif", "lot-12.html#autotoc_md817", null ],
        [ "Périmètre", "lot-12.html#autotoc_md818", [
          [ "Inclus", "lot-12.html#autotoc_md819", null ],
          [ "Exclus (lots ultérieurs)", "lot-12.html#autotoc_md820", null ]
        ] ],
        [ "Décisions de cadrage", "lot-12.html#autotoc_md821", null ],
        [ "Exigences couvertes", "lot-12.html#autotoc_md822", null ],
        [ "Découpage", "lot-12.html#autotoc_md823", null ],
        [ "Critères d'acceptation du lot", "lot-12.html#autotoc_md824", null ],
        [ "Dépendances", "lot-12.html#autotoc_md825", null ],
        [ "Navigation des tâches", "lot-12.html#autotoc_md826", null ],
        [ "TACHE-01 — Données : budget (Player, Level, LevelLoader)", "lot-12-tache-01-donnees.html", [
          [ "Contexte", "lot-12-tache-01-donnees.html#autotoc_md827", null ],
          [ "Travail à réaliser", "lot-12-tache-01-donnees.html#autotoc_md828", null ],
          [ "Fichiers impactés", "lot-12-tache-01-donnees.html#autotoc_md829", null ],
          [ "Tests (obligatoires)", "lot-12-tache-01-donnees.html#autotoc_md830", null ],
          [ "Points d'attention", "lot-12-tache-01-donnees.html#autotoc_md831", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-01-donnees.html#autotoc_md832", null ],
          [ "Exigences", "lot-12-tache-01-donnees.html#autotoc_md833", null ]
        ] ],
        [ "TACHE-02 — Mécanismes interrupteur/porte (MechanismController)", "lot-12-tache-02-mecanismes.html", [
          [ "Contexte", "lot-12-tache-02-mecanismes.html#autotoc_md834", null ],
          [ "Travail à réaliser", "lot-12-tache-02-mecanismes.html#autotoc_md835", null ],
          [ "Fichiers impactés", "lot-12-tache-02-mecanismes.html#autotoc_md836", null ],
          [ "Tests (obligatoires)", "lot-12-tache-02-mecanismes.html#autotoc_md837", null ],
          [ "Points d'attention", "lot-12-tache-02-mecanismes.html#autotoc_md838", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-02-mecanismes.html#autotoc_md839", null ],
          [ "Exigences", "lot-12-tache-02-mecanismes.html#autotoc_md840", null ]
        ] ],
        [ "TACHE-03 — Budget de sauts/dashs dans la physique", "lot-12-tache-03-budget.html", [
          [ "Contexte", "lot-12-tache-03-budget.html#autotoc_md841", null ],
          [ "Travail à réaliser", "lot-12-tache-03-budget.html#autotoc_md842", null ],
          [ "Fichiers impactés", "lot-12-tache-03-budget.html#autotoc_md843", null ],
          [ "Tests (obligatoires)", "lot-12-tache-03-budget.html#autotoc_md844", null ],
          [ "Points d'attention", "lot-12-tache-03-budget.html#autotoc_md845", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-03-budget.html#autotoc_md846", null ],
          [ "Exigences", "lot-12-tache-03-budget.html#autotoc_md847", null ]
        ] ],
        [ "TACHE-04 — Intégration GameScreen + niveau demo4 + preuves", "lot-12-tache-04-integration-puzzle.html", [
          [ "Contexte", "lot-12-tache-04-integration-puzzle.html#autotoc_md848", null ],
          [ "Travail à réaliser", "lot-12-tache-04-integration-puzzle.html#autotoc_md849", null ],
          [ "Fichiers impactés", "lot-12-tache-04-integration-puzzle.html#autotoc_md850", null ],
          [ "Tests (obligatoires)", "lot-12-tache-04-integration-puzzle.html#autotoc_md851", null ],
          [ "Points d'attention", "lot-12-tache-04-integration-puzzle.html#autotoc_md852", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-04-integration-puzzle.html#autotoc_md853", null ],
          [ "Exigences", "lot-12-tache-04-integration-puzzle.html#autotoc_md854", null ]
        ] ]
      ] ],
      [ "LOT-13 — Consolidation de la documentation", "lot-13.html", [
        [ "Objectif", "lot-13.html#autotoc_md855", null ],
        [ "Périmètre", "lot-13.html#autotoc_md856", [
          [ "Inclus", "lot-13.html#autotoc_md857", null ],
          [ "Exclus", "lot-13.html#autotoc_md858", null ]
        ] ],
        [ "Décisions de cadrage", "lot-13.html#autotoc_md859", null ],
        [ "Exigences couvertes", "lot-13.html#autotoc_md860", null ],
        [ "Découpage", "lot-13.html#autotoc_md861", null ],
        [ "Critères d'acceptation du lot", "lot-13.html#autotoc_md862", null ],
        [ "Dépendances", "lot-13.html#autotoc_md863", null ]
      ] ],
      [ "LOT-14 — Éditeur de niveaux intégré : édition de tuiles, mécanismes, essai immédiat", "lot-14.html", [
        [ "Objectif", "lot-14.html#autotoc_md864", null ],
        [ "Périmètre", "lot-14.html#autotoc_md865", [
          [ "Inclus", "lot-14.html#autotoc_md866", null ],
          [ "Exclus (lots ultérieurs)", "lot-14.html#autotoc_md867", null ]
        ] ],
        [ "Décisions de cadrage", "lot-14.html#autotoc_md868", null ],
        [ "Exigences couvertes", "lot-14.html#autotoc_md869", null ],
        [ "Découpage", "lot-14.html#autotoc_md870", null ],
        [ "Critères d'acceptation du lot", "lot-14.html#autotoc_md871", null ],
        [ "Dépendances", "lot-14.html#autotoc_md872", null ],
        [ "Navigation des tâches", "lot-14.html#autotoc_md873", null ],
        [ "TACHE-01 — Sérialisation JSON + modèle d'édition mutable", "lot-14-tache-01-serialisation-modele-edition.html", [
          [ "Contexte", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md874", null ],
          [ "Travail à réaliser", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md875", null ],
          [ "Fichiers impactés", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md876", null ],
          [ "Tests (obligatoires)", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md877", null ],
          [ "Points d'attention", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md878", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md879", null ],
          [ "Exigences", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md880", null ]
        ] ],
        [ "TACHE-02 — Écran éditeur : grille cliquable + palette de tuiles", "lot-14-tache-02-ecran-editeur-palette.html", [
          [ "Contexte", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md881", null ],
          [ "Travail à réaliser", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md882", null ],
          [ "Fichiers impactés", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md883", null ],
          [ "Tests (obligatoires)", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md884", null ],
          [ "Points d'attention", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md885", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md886", null ],
          [ "Exigences", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md887", null ]
        ] ],
        [ "TACHE-03 — Entrée/sortie, liaison de mécanismes, redimensionnement", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html", [
          [ "Contexte", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md888", null ],
          [ "Travail à réaliser", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md889", null ],
          [ "Fichiers impactés", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md890", null ],
          [ "Tests (obligatoires)", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md891", null ],
          [ "Points d'attention", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md892", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md893", null ],
          [ "Exigences", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md894", null ]
        ] ],
        [ "TACHE-04 — Historique annuler/refaire", "lot-14-tache-04-undo-redo.html", [
          [ "Contexte", "lot-14-tache-04-undo-redo.html#autotoc_md895", null ],
          [ "Travail à réaliser", "lot-14-tache-04-undo-redo.html#autotoc_md896", null ],
          [ "Fichiers impactés", "lot-14-tache-04-undo-redo.html#autotoc_md897", null ],
          [ "Tests (obligatoires)", "lot-14-tache-04-undo-redo.html#autotoc_md898", null ],
          [ "Points d'attention", "lot-14-tache-04-undo-redo.html#autotoc_md899", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-04-undo-redo.html#autotoc_md900", null ],
          [ "Exigences", "lot-14-tache-04-undo-redo.html#autotoc_md901", null ]
        ] ],
        [ "TACHE-05 — Enregistrement, validation, essai immédiat", "lot-14-tache-05-enregistrement-validation-essai.html", [
          [ "Contexte", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md902", null ],
          [ "Travail à réaliser", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md903", null ],
          [ "Fichiers impactés", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md904", null ],
          [ "Tests (obligatoires)", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md905", null ],
          [ "Points d'attention", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md906", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md907", null ],
          [ "Exigences", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md908", null ]
        ] ],
        [ "TACHE-06 — Intégration menu, tests système, guide non-codeur Git", "lot-14-tache-06-integration-guide-non-codeur.html", [
          [ "Contexte", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md909", null ],
          [ "Travail à réaliser", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md910", null ],
          [ "Fichiers impactés", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md911", null ],
          [ "Tests (obligatoires)", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md912", null ],
          [ "Points d'attention", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md913", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md914", null ],
          [ "Exigences", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md915", null ]
        ] ]
      ] ],
      [ "LOT-15 — Éditeur de niveaux : robustesse et confort d'édition", "lot-15.html", [
        [ "Objectif", "lot-15.html#autotoc_md916", null ],
        [ "Périmètre", "lot-15.html#autotoc_md917", [
          [ "Inclus", "lot-15.html#autotoc_md918", null ],
          [ "Exclus (lots ultérieurs ou non retenus)", "lot-15.html#autotoc_md919", null ]
        ] ],
        [ "Décisions de cadrage", "lot-15.html#autotoc_md920", null ],
        [ "Exigences couvertes", "lot-15.html#autotoc_md921", null ],
        [ "Découpage", "lot-15.html#autotoc_md922", null ],
        [ "Critères d'acceptation du lot", "lot-15.html#autotoc_md923", null ],
        [ "Dépendances", "lot-15.html#autotoc_md924", null ],
        [ "Navigation des tâches", "lot-15.html#autotoc_md925", null ],
        [ "TACHE-01 — Entrées bas niveau : molette et texte tapé", "lot-15-tache-01-entrees-molette-texte.html", [
          [ "Contexte", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md926", null ],
          [ "Travail à réaliser", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md927", null ],
          [ "Fichiers impactés", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md928", null ],
          [ "Tests (obligatoires)", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md929", null ],
          [ "Points d'attention", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md930", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md931", null ],
          [ "Exigences", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md932", null ],
          [ "Ajustement post-livraison (essai utilisateur)", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md933", null ]
        ] ],
        [ "TACHE-02 — Garde-fous : redimensionnement destructeur, quitter sans enregistrer", "lot-15-tache-02-garde-fous-perte-donnees.html", [
          [ "Contexte", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md934", null ],
          [ "Travail à réaliser", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md935", null ],
          [ "Fichiers impactés", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md936", null ],
          [ "Tests (obligatoires)", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md937", null ],
          [ "Points d'attention", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md938", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md939", null ],
          [ "Exigences", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md940", null ]
        ] ],
        [ "TACHE-03 — Nommage, renommage, avertissement d'écrasement", "lot-15-tache-03-nommage-renommage.html", [
          [ "Contexte", "lot-15-tache-03-nommage-renommage.html#autotoc_md941", null ],
          [ "Travail à réaliser", "lot-15-tache-03-nommage-renommage.html#autotoc_md942", null ],
          [ "Fichiers impactés", "lot-15-tache-03-nommage-renommage.html#autotoc_md943", null ],
          [ "Tests (obligatoires)", "lot-15-tache-03-nommage-renommage.html#autotoc_md944", null ],
          [ "Points d'attention", "lot-15-tache-03-nommage-renommage.html#autotoc_md945", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-03-nommage-renommage.html#autotoc_md946", null ],
          [ "Exigences", "lot-15-tache-03-nommage-renommage.html#autotoc_md947", null ]
        ] ],
        [ "TACHE-04 — Caméra : pan et zoom manuels", "lot-15-tache-04-camera-pan-zoom.html", [
          [ "Contexte", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md948", null ],
          [ "Travail à réaliser", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md949", null ],
          [ "Fichiers impactés", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md950", null ],
          [ "Tests (obligatoires)", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md951", null ],
          [ "Points d'attention", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md952", null ],
          [ "Ajustement post-livraison (essai utilisateur)", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md953", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md954", null ],
          [ "Exigences", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md955", null ]
        ] ],
        [ "TACHE-05 — Outils de zone : remplissage rectangulaire, sélection, copier/coller", "lot-15-tache-05-outils-rectangle-selection.html", [
          [ "Contexte", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md956", null ],
          [ "Travail à réaliser", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md957", null ],
          [ "Fichiers impactés", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md958", null ],
          [ "Tests (obligatoires)", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md959", null ],
          [ "Points d'attention", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md960", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md961", null ],
          [ "Exigences", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md962", null ]
        ] ],
        [ "TACHE-06 — Découvrabilité : barre d'outils, aide, libellés, liaisons lisibles", "lot-15-tache-06-decouvrabilite.html", [
          [ "Contexte", "lot-15-tache-06-decouvrabilite.html#autotoc_md963", null ],
          [ "Travail à réaliser", "lot-15-tache-06-decouvrabilite.html#autotoc_md964", null ],
          [ "Fichiers impactés", "lot-15-tache-06-decouvrabilite.html#autotoc_md965", null ],
          [ "Tests (obligatoires)", "lot-15-tache-06-decouvrabilite.html#autotoc_md966", null ],
          [ "Points d'attention", "lot-15-tache-06-decouvrabilite.html#autotoc_md967", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-06-decouvrabilite.html#autotoc_md968", null ],
          [ "Exigences", "lot-15-tache-06-decouvrabilite.html#autotoc_md969", null ],
          [ "Ajustement post-livraison (essai utilisateur)", "lot-15-tache-06-decouvrabilite.html#autotoc_md970", null ]
        ] ],
        [ "TACHE-07 — Essai immédiat en mémoire, erreurs de validation structurées", "lot-15-tache-07-essai-memoire-erreurs-structurees.html", [
          [ "Contexte", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md971", null ],
          [ "Travail à réaliser", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md972", null ],
          [ "Fichiers impactés", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md973", null ],
          [ "Tests (obligatoires)", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md974", null ],
          [ "Points d'attention", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md975", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md976", null ],
          [ "Exigences", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md977", null ]
        ] ],
        [ "TACHE-08 — Nettoyage documentaire", "lot-15-tache-08-nettoyage-documentation.html", [
          [ "Contexte", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md978", null ],
          [ "Travail à réaliser", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md979", null ],
          [ "Fichiers impactés", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md980", null ],
          [ "Tests (obligatoires)", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md981", null ],
          [ "Points d'attention", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md982", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md983", null ],
          [ "Exigences", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md984", null ]
        ] ]
      ] ],
      [ "LOT-16 — Niveaux de grande taille", "lot-16.html", [
        [ "Objectif", "lot-16.html#autotoc_md985", null ],
        [ "Périmètre", "lot-16.html#autotoc_md986", [
          [ "Inclus", "lot-16.html#autotoc_md987", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-16.html#autotoc_md988", null ]
        ] ],
        [ "Décisions de cadrage", "lot-16.html#autotoc_md989", null ],
        [ "Exigences couvertes", "lot-16.html#autotoc_md990", null ],
        [ "Découpage", "lot-16.html#autotoc_md991", null ],
        [ "Critères d'acceptation du lot", "lot-16.html#autotoc_md992", null ],
        [ "Dépendances", "lot-16.html#autotoc_md993", null ],
        [ "Navigation des tâches", "lot-16.html#autotoc_md994", null ],
        [ "TACHE-01 — Plafond de taille et validation « largeur x hauteur »", "lot-16-tache-01-plafond-validation-taille.html", [
          [ "Contexte", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md995", null ],
          [ "Travail à réaliser", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md996", null ],
          [ "Fichiers impactés", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md997", null ],
          [ "Tests (obligatoires)", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md998", null ],
          [ "Points d'attention", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md999", null ],
          [ "Définition de fait (DoD)", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md1000", null ],
          [ "Exigences", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md1001", null ]
        ] ],
        [ "TACHE-02 — Boîte de dialogue de redimensionnement (Ctrl+R)", "lot-16-tache-02-boite-dialogue-redimensionnement.html", [
          [ "Contexte", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md1002", null ],
          [ "Travail à réaliser", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md1003", null ],
          [ "Fichiers impactés", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md1004", null ],
          [ "Tests (obligatoires)", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md1005", null ],
          [ "Points d'attention", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md1006", null ],
          [ "Définition de fait (DoD)", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md1007", null ],
          [ "Exigences", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md1008", null ]
        ] ],
        [ "TACHE-03 — Caméra : englober tout le niveau (éditeur et jeu)", "lot-16-tache-03-camera-niveau-entier.html", [
          [ "Contexte", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md1009", null ],
          [ "Travail à réaliser", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md1010", null ],
          [ "Fichiers impactés", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md1011", null ],
          [ "Tests (obligatoires)", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md1012", null ],
          [ "Points d'attention", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md1013", null ],
          [ "Définition de fait (DoD)", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md1014", null ],
          [ "Exigences", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md1015", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-16-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-16-tache-04-documentation-verification.html#autotoc_md1016", null ],
          [ "Travail à réaliser", "lot-16-tache-04-documentation-verification.html#autotoc_md1017", null ],
          [ "Fichiers impactés", "lot-16-tache-04-documentation-verification.html#autotoc_md1018", null ],
          [ "Tests (obligatoires)", "lot-16-tache-04-documentation-verification.html#autotoc_md1019", null ],
          [ "Points d'attention", "lot-16-tache-04-documentation-verification.html#autotoc_md1020", null ],
          [ "Définition de fait (DoD)", "lot-16-tache-04-documentation-verification.html#autotoc_md1021", null ],
          [ "Exigences", "lot-16-tache-04-documentation-verification.html#autotoc_md1022", null ]
        ] ]
      ] ],
      [ "LOT-17 — Sprite du personnage (statique)", "lot-17.html", [
        [ "Objectif", "lot-17.html#autotoc_md1023", null ],
        [ "Périmètre", "lot-17.html#autotoc_md1024", [
          [ "Inclus", "lot-17.html#autotoc_md1025", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-17.html#autotoc_md1026", null ]
        ] ],
        [ "Décisions de cadrage", "lot-17.html#autotoc_md1027", null ],
        [ "Exigences couvertes", "lot-17.html#autotoc_md1028", null ],
        [ "Découpage", "lot-17.html#autotoc_md1029", null ],
        [ "Critères d'acceptation du lot", "lot-17.html#autotoc_md1030", null ],
        [ "Dépendances", "lot-17.html#autotoc_md1031", null ],
        [ "Navigation des tâches", "lot-17.html#autotoc_md1032", null ],
        [ "TACHE-01 — Silhouette du personnage dans l'atlas", "lot-17-tache-01-silhouette-personnage.html", [
          [ "Contexte", "lot-17-tache-01-silhouette-personnage.html#autotoc_md1033", null ],
          [ "Travail à réaliser", "lot-17-tache-01-silhouette-personnage.html#autotoc_md1034", null ],
          [ "Fichiers impactés", "lot-17-tache-01-silhouette-personnage.html#autotoc_md1035", null ],
          [ "Tests (obligatoires)", "lot-17-tache-01-silhouette-personnage.html#autotoc_md1036", null ],
          [ "Points d'attention", "lot-17-tache-01-silhouette-personnage.html#autotoc_md1037", null ],
          [ "Définition de fait (DoD)", "lot-17-tache-01-silhouette-personnage.html#autotoc_md1038", null ],
          [ "Exigences", "lot-17-tache-01-silhouette-personnage.html#autotoc_md1039", null ]
        ] ],
        [ "TACHE-02 — Documentation et vérification", "lot-17-tache-02-documentation-verification.html", [
          [ "Contexte", "lot-17-tache-02-documentation-verification.html#autotoc_md1040", null ],
          [ "Travail à réaliser", "lot-17-tache-02-documentation-verification.html#autotoc_md1041", null ],
          [ "Fichiers impactés", "lot-17-tache-02-documentation-verification.html#autotoc_md1042", null ],
          [ "Tests (obligatoires)", "lot-17-tache-02-documentation-verification.html#autotoc_md1043", null ],
          [ "Points d'attention", "lot-17-tache-02-documentation-verification.html#autotoc_md1044", null ],
          [ "Définition de fait (DoD)", "lot-17-tache-02-documentation-verification.html#autotoc_md1045", null ],
          [ "Exigences", "lot-17-tache-02-documentation-verification.html#autotoc_md1046", null ]
        ] ]
      ] ],
      [ "LOT-18 — Animation du personnage (repos, course, saut)", "lot-18.html", [
        [ "Objectif", "lot-18.html#autotoc_md1047", null ],
        [ "Périmètre", "lot-18.html#autotoc_md1048", [
          [ "Inclus", "lot-18.html#autotoc_md1049", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-18.html#autotoc_md1050", null ]
        ] ],
        [ "Décisions de cadrage", "lot-18.html#autotoc_md1051", null ],
        [ "Exigences couvertes", "lot-18.html#autotoc_md1052", null ],
        [ "Découpage", "lot-18.html#autotoc_md1053", null ],
        [ "Critères d'acceptation du lot", "lot-18.html#autotoc_md1054", null ],
        [ "Dépendances", "lot-18.html#autotoc_md1055", null ],
        [ "Navigation des tâches", "lot-18.html#autotoc_md1056", null ],
        [ "TACHE-01 — Composant et système d'animation", "lot-18-tache-01-composant-systeme-animation.html", [
          [ "Contexte", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1057", null ],
          [ "Travail à réaliser", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1058", null ],
          [ "Fichiers impactés", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1059", null ],
          [ "Tests (obligatoires)", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1060", null ],
          [ "Points d'attention", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1061", null ],
          [ "Définition de fait (DoD)", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1062", null ],
          [ "Exigences", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1063", null ]
        ] ],
        [ "TACHE-02 — Images dans l'atlas et intégration au rendu", "lot-18-tache-02-frames-atlas-integration.html", [
          [ "Contexte", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1064", null ],
          [ "Travail à réaliser", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1065", null ],
          [ "Fichiers impactés", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1066", null ],
          [ "Tests (obligatoires)", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1067", null ],
          [ "Points d'attention", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1068", null ],
          [ "Définition de fait (DoD)", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1069", null ],
          [ "Exigences", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1070", null ]
        ] ],
        [ "TACHE-03 — Documentation et vérification", "lot-18-tache-03-documentation-verification.html", [
          [ "Contexte", "lot-18-tache-03-documentation-verification.html#autotoc_md1071", null ],
          [ "Travail à réaliser", "lot-18-tache-03-documentation-verification.html#autotoc_md1072", null ],
          [ "Fichiers impactés", "lot-18-tache-03-documentation-verification.html#autotoc_md1073", null ],
          [ "Tests (obligatoires)", "lot-18-tache-03-documentation-verification.html#autotoc_md1074", null ],
          [ "Points d'attention", "lot-18-tache-03-documentation-verification.html#autotoc_md1075", null ],
          [ "Définition de fait (DoD)", "lot-18-tache-03-documentation-verification.html#autotoc_md1076", null ],
          [ "Exigences", "lot-18-tache-03-documentation-verification.html#autotoc_md1077", null ]
        ] ]
      ] ],
      [ "LOT-19 — Physique newtonienne et plaque de pression", "lot-19.html", [
        [ "Objectif", "lot-19.html#autotoc_md1078", null ],
        [ "Périmètre", "lot-19.html#autotoc_md1079", [
          [ "Inclus", "lot-19.html#autotoc_md1080", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-19.html#autotoc_md1081", null ]
        ] ],
        [ "Décisions de cadrage", "lot-19.html#autotoc_md1082", null ],
        [ "Exigences couvertes", "lot-19.html#autotoc_md1083", null ],
        [ "Découpage", "lot-19.html#autotoc_md1084", null ],
        [ "Critères d'acceptation du lot", "lot-19.html#autotoc_md1085", null ],
        [ "Dépendances", "lot-19.html#autotoc_md1086", null ],
        [ "Navigation des tâches", "lot-19.html#autotoc_md1087", null ],
        [ "TACHE-01 — Masse et chute newtonienne", "lot-19-tache-01-masse-chute-newtonienne.html", [
          [ "Contexte", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1088", null ],
          [ "Travail à réaliser", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1089", null ],
          [ "Fichiers impactés", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1090", null ],
          [ "Tests (obligatoires)", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1091", null ],
          [ "Points d'attention", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1092", null ],
          [ "Définition de fait (DoD)", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1093", null ],
          [ "Exigences", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1094", null ]
        ] ],
        [ "TACHE-02 — Plaque de pression", "lot-19-tache-02-plaque-de-pression.html", [
          [ "Contexte", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1095", null ],
          [ "Travail à réaliser", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1096", null ],
          [ "Fichiers impactés", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1097", null ],
          [ "Tests (obligatoires)", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1098", null ],
          [ "Points d'attention", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1099", null ],
          [ "Définition de fait (DoD)", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1100", null ],
          [ "Exigences", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1101", null ]
        ] ],
        [ "TACHE-03 — Intégration éditeur et niveau de démonstration", "lot-19-tache-03-editeur-niveau-demo.html", [
          [ "Contexte", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1102", null ],
          [ "Travail à réaliser", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1103", null ],
          [ "Fichiers impactés", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1104", null ],
          [ "Tests (obligatoires)", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1105", null ],
          [ "Points d'attention", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1106", null ],
          [ "Définition de fait (DoD)", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1107", null ],
          [ "Exigences", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1108", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-19-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-19-tache-04-documentation-verification.html#autotoc_md1109", null ],
          [ "Travail à réaliser", "lot-19-tache-04-documentation-verification.html#autotoc_md1110", null ],
          [ "Fichiers impactés", "lot-19-tache-04-documentation-verification.html#autotoc_md1111", null ],
          [ "Tests (obligatoires)", "lot-19-tache-04-documentation-verification.html#autotoc_md1112", null ],
          [ "Points d'attention", "lot-19-tache-04-documentation-verification.html#autotoc_md1113", null ],
          [ "Définition de fait (DoD)", "lot-19-tache-04-documentation-verification.html#autotoc_md1114", null ],
          [ "Exigences", "lot-19-tache-04-documentation-verification.html#autotoc_md1115", null ]
        ] ]
      ] ],
      [ "LOT-20 — Manette et menu d'options", "lot-20.html", [
        [ "Objectif", "lot-20.html#autotoc_md1116", null ],
        [ "Périmètre", "lot-20.html#autotoc_md1117", [
          [ "Inclus", "lot-20.html#autotoc_md1118", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-20.html#autotoc_md1119", null ]
        ] ],
        [ "Décisions de cadrage", "lot-20.html#autotoc_md1120", null ],
        [ "Exigences couvertes", "lot-20.html#autotoc_md1121", null ],
        [ "Découpage", "lot-20.html#autotoc_md1122", null ],
        [ "Critères d'acceptation du lot", "lot-20.html#autotoc_md1123", null ],
        [ "Dépendances", "lot-20.html#autotoc_md1124", null ],
        [ "Navigation des tâches", "lot-20.html#autotoc_md1125", null ],
        [ "TACHE-01 — Intégration manette (XInput)", "lot-20-tache-01-integration-manette.html", [
          [ "Contexte", "lot-20-tache-01-integration-manette.html#autotoc_md1126", null ],
          [ "Travail à réaliser", "lot-20-tache-01-integration-manette.html#autotoc_md1127", null ],
          [ "Fichiers impactés", "lot-20-tache-01-integration-manette.html#autotoc_md1128", null ],
          [ "Tests (obligatoires)", "lot-20-tache-01-integration-manette.html#autotoc_md1129", null ],
          [ "Points d'attention", "lot-20-tache-01-integration-manette.html#autotoc_md1130", null ],
          [ "Définition de fait (DoD)", "lot-20-tache-01-integration-manette.html#autotoc_md1131", null ],
          [ "Exigences", "lot-20-tache-01-integration-manette.html#autotoc_md1132", null ]
        ] ],
        [ "TACHE-02 — Menu d'options", "lot-20-tache-02-menu-options.html", [
          [ "Contexte", "lot-20-tache-02-menu-options.html#autotoc_md1133", null ],
          [ "Travail à réaliser", "lot-20-tache-02-menu-options.html#autotoc_md1134", null ],
          [ "Fichiers impactés", "lot-20-tache-02-menu-options.html#autotoc_md1135", null ],
          [ "Tests (obligatoires)", "lot-20-tache-02-menu-options.html#autotoc_md1136", null ],
          [ "Points d'attention", "lot-20-tache-02-menu-options.html#autotoc_md1137", null ],
          [ "Définition de fait (DoD)", "lot-20-tache-02-menu-options.html#autotoc_md1138", null ],
          [ "Exigences", "lot-20-tache-02-menu-options.html#autotoc_md1139", null ]
        ] ],
        [ "TACHE-03 — Documentation et vérification", "lot-20-tache-03-documentation-verification.html", [
          [ "Contexte", "lot-20-tache-03-documentation-verification.html#autotoc_md1140", null ],
          [ "Travail à réaliser", "lot-20-tache-03-documentation-verification.html#autotoc_md1141", null ],
          [ "Fichiers impactés", "lot-20-tache-03-documentation-verification.html#autotoc_md1142", null ],
          [ "Tests (obligatoires)", "lot-20-tache-03-documentation-verification.html#autotoc_md1143", null ],
          [ "Points d'attention", "lot-20-tache-03-documentation-verification.html#autotoc_md1144", null ],
          [ "Définition de fait (DoD)", "lot-20-tache-03-documentation-verification.html#autotoc_md1145", null ],
          [ "Exigences", "lot-20-tache-03-documentation-verification.html#autotoc_md1146", null ]
        ] ]
      ] ],
      [ "LOT-21 — Bloc poussable", "lot-21.html", [
        [ "Objectif", "lot-21.html#autotoc_md1147", null ],
        [ "Périmètre", "lot-21.html#autotoc_md1148", [
          [ "Inclus", "lot-21.html#autotoc_md1149", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-21.html#autotoc_md1150", null ]
        ] ],
        [ "Décisions de cadrage", "lot-21.html#autotoc_md1151", null ],
        [ "Exigences couvertes", "lot-21.html#autotoc_md1152", null ],
        [ "Découpage", "lot-21.html#autotoc_md1153", null ],
        [ "Critères d'acceptation du lot", "lot-21.html#autotoc_md1154", null ],
        [ "Dépendances", "lot-21.html#autotoc_md1155", null ],
        [ "Navigation des tâches", "lot-21.html#autotoc_md1156", null ],
        [ "TACHE-01 — Modèle et contrôleur (Core)", "lot-21-tache-01-controleur-blocs.html", [
          [ "Contexte", "lot-21-tache-01-controleur-blocs.html#autotoc_md1157", null ],
          [ "Travail à réaliser", "lot-21-tache-01-controleur-blocs.html#autotoc_md1158", null ],
          [ "Fichiers impactés", "lot-21-tache-01-controleur-blocs.html#autotoc_md1159", null ],
          [ "Tests (obligatoires)", "lot-21-tache-01-controleur-blocs.html#autotoc_md1160", null ],
          [ "Points d'attention", "lot-21-tache-01-controleur-blocs.html#autotoc_md1161", null ],
          [ "Définition de fait (DoD)", "lot-21-tache-01-controleur-blocs.html#autotoc_md1162", null ],
          [ "Exigences", "lot-21-tache-01-controleur-blocs.html#autotoc_md1163", null ]
        ] ],
        [ "TACHE-02 — Intégration éditeur et jeu (HMI)", "lot-21-tache-02-integration-editeur-jeu.html", [
          [ "Contexte", "lot-21-tache-02-integration-editeur-jeu.html#autotoc_md1164", null ],
          [ "Travail à réaliser", "lot-21-tache-02-integration-editeur-jeu.html#autotoc_md1165", null ],
          [ "Fichiers impactés", "lot-21-tache-02-integration-editeur-jeu.html#autotoc_md1166", null ],
          [ "Tests (obligatoires)", "lot-21-tache-02-integration-editeur-jeu.html#autotoc_md1167", null ],
          [ "Points d'attention", "lot-21-tache-02-integration-editeur-jeu.html#autotoc_md1168", null ],
          [ "Définition de fait (DoD)", "lot-21-tache-02-integration-editeur-jeu.html#autotoc_md1169", null ],
          [ "Exigences", "lot-21-tache-02-integration-editeur-jeu.html#autotoc_md1170", null ]
        ] ],
        [ "TACHE-03 — Documentation et vérification", "lot-21-tache-03-documentation-verification.html", [
          [ "Contexte", "lot-21-tache-03-documentation-verification.html#autotoc_md1171", null ],
          [ "Travail à réaliser", "lot-21-tache-03-documentation-verification.html#autotoc_md1172", null ],
          [ "Fichiers impactés", "lot-21-tache-03-documentation-verification.html#autotoc_md1173", null ],
          [ "Tests (obligatoires)", "lot-21-tache-03-documentation-verification.html#autotoc_md1174", null ],
          [ "Points d'attention", "lot-21-tache-03-documentation-verification.html#autotoc_md1175", null ],
          [ "Définition de fait (DoD)", "lot-21-tache-03-documentation-verification.html#autotoc_md1176", null ],
          [ "Exigences", "lot-21-tache-03-documentation-verification.html#autotoc_md1177", null ]
        ] ]
      ] ],
      [ "LOT-22 — Pentes réelles", "lot-22.html", [
        [ "Objectif", "lot-22.html#autotoc_md1178", null ],
        [ "Périmètre", "lot-22.html#autotoc_md1179", [
          [ "Inclus", "lot-22.html#autotoc_md1180", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-22.html#autotoc_md1181", null ]
        ] ],
        [ "Décisions de cadrage", "lot-22.html#autotoc_md1182", null ],
        [ "Exigences couvertes", "lot-22.html#autotoc_md1183", null ],
        [ "Découpage", "lot-22.html#autotoc_md1184", null ],
        [ "Critères d'acceptation du lot", "lot-22.html#autotoc_md1185", null ],
        [ "Dépendances", "lot-22.html#autotoc_md1186", null ],
        [ "Navigation des tâches", "lot-22.html#autotoc_md1187", null ],
        [ "TACHE-01 — Modèle de tuile et fonction de hauteur", "lot-22-tache-01-modele-tuile-pente.html", [
          [ "Contexte", "lot-22-tache-01-modele-tuile-pente.html#autotoc_md1188", null ],
          [ "Travail à réaliser", "lot-22-tache-01-modele-tuile-pente.html#autotoc_md1189", null ],
          [ "Fichiers impactés", "lot-22-tache-01-modele-tuile-pente.html#autotoc_md1190", null ],
          [ "Tests (obligatoires)", "lot-22-tache-01-modele-tuile-pente.html#autotoc_md1191", null ],
          [ "Points d'attention", "lot-22-tache-01-modele-tuile-pente.html#autotoc_md1192", null ],
          [ "Définition de fait (DoD)", "lot-22-tache-01-modele-tuile-pente.html#autotoc_md1193", null ],
          [ "Exigences", "lot-22-tache-01-modele-tuile-pente.html#autotoc_md1194", null ]
        ] ],
        [ "TACHE-02 — Collision et suivi de pente", "lot-22-tache-02-collision-suivi-pente.html", [
          [ "Contexte", "lot-22-tache-02-collision-suivi-pente.html#autotoc_md1195", null ],
          [ "Travail à réaliser", "lot-22-tache-02-collision-suivi-pente.html#autotoc_md1196", null ],
          [ "Fichiers impactés", "lot-22-tache-02-collision-suivi-pente.html#autotoc_md1197", null ],
          [ "Tests (obligatoires)", "lot-22-tache-02-collision-suivi-pente.html#autotoc_md1198", null ],
          [ "Points d'attention", "lot-22-tache-02-collision-suivi-pente.html#autotoc_md1199", null ],
          [ "Définition de fait (DoD)", "lot-22-tache-02-collision-suivi-pente.html#autotoc_md1200", null ],
          [ "Exigences", "lot-22-tache-02-collision-suivi-pente.html#autotoc_md1201", null ]
        ] ],
        [ "TACHE-03 — Éditeur et rendu", "lot-22-tache-03-editeur-rendu.html", [
          [ "Contexte", "lot-22-tache-03-editeur-rendu.html#autotoc_md1202", null ],
          [ "Travail à réaliser", "lot-22-tache-03-editeur-rendu.html#autotoc_md1203", null ],
          [ "Fichiers impactés", "lot-22-tache-03-editeur-rendu.html#autotoc_md1204", null ],
          [ "Tests (obligatoires)", "lot-22-tache-03-editeur-rendu.html#autotoc_md1205", null ],
          [ "Points d'attention", "lot-22-tache-03-editeur-rendu.html#autotoc_md1206", null ],
          [ "Définition de fait (DoD)", "lot-22-tache-03-editeur-rendu.html#autotoc_md1207", null ],
          [ "Exigences", "lot-22-tache-03-editeur-rendu.html#autotoc_md1208", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-22-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-22-tache-04-documentation-verification.html#autotoc_md1209", null ],
          [ "Travail à réaliser", "lot-22-tache-04-documentation-verification.html#autotoc_md1210", null ],
          [ "Fichiers impactés", "lot-22-tache-04-documentation-verification.html#autotoc_md1211", null ],
          [ "Tests (obligatoires)", "lot-22-tache-04-documentation-verification.html#autotoc_md1212", null ],
          [ "Points d'attention", "lot-22-tache-04-documentation-verification.html#autotoc_md1213", null ],
          [ "Définition de fait (DoD)", "lot-22-tache-04-documentation-verification.html#autotoc_md1214", null ],
          [ "Exigences", "lot-22-tache-04-documentation-verification.html#autotoc_md1215", null ]
        ] ]
      ] ],
      [ "LOT-23 — Collision arrondie", "lot-23.html", [
        [ "Objectif", "lot-23.html#autotoc_md1216", null ],
        [ "Périmètre", "lot-23.html#autotoc_md1217", [
          [ "Inclus", "lot-23.html#autotoc_md1218", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-23.html#autotoc_md1219", null ]
        ] ],
        [ "Décisions de cadrage", "lot-23.html#autotoc_md1220", null ],
        [ "Exigences couvertes", "lot-23.html#autotoc_md1221", null ],
        [ "Découpage", "lot-23.html#autotoc_md1222", null ],
        [ "Critères d'acceptation du lot", "lot-23.html#autotoc_md1223", null ],
        [ "Dépendances", "lot-23.html#autotoc_md1224", null ],
        [ "Navigation des tâches", "lot-23.html#autotoc_md1225", null ],
        [ "TACHE-01 — Modèle de tuile et formule de courbe", "lot-23-tache-01-modele-tuile-arrondie.html", [
          [ "Contexte", "lot-23-tache-01-modele-tuile-arrondie.html#autotoc_md1226", null ],
          [ "Travail à réaliser", "lot-23-tache-01-modele-tuile-arrondie.html#autotoc_md1227", null ],
          [ "Fichiers impactés", "lot-23-tache-01-modele-tuile-arrondie.html#autotoc_md1228", null ],
          [ "Tests (obligatoires)", "lot-23-tache-01-modele-tuile-arrondie.html#autotoc_md1229", null ],
          [ "Points d'attention", "lot-23-tache-01-modele-tuile-arrondie.html#autotoc_md1230", null ],
          [ "Définition de fait (DoD)", "lot-23-tache-01-modele-tuile-arrondie.html#autotoc_md1231", null ],
          [ "Exigences", "lot-23-tache-01-modele-tuile-arrondie.html#autotoc_md1232", null ]
        ] ],
        [ "TACHE-02 — Éditeur et rendu", "lot-23-tache-02-editeur-rendu.html", [
          [ "Contexte", "lot-23-tache-02-editeur-rendu.html#autotoc_md1233", null ],
          [ "Travail à réaliser", "lot-23-tache-02-editeur-rendu.html#autotoc_md1234", null ],
          [ "Fichiers impactés", "lot-23-tache-02-editeur-rendu.html#autotoc_md1235", null ],
          [ "Tests (obligatoires)", "lot-23-tache-02-editeur-rendu.html#autotoc_md1236", null ],
          [ "Points d'attention", "lot-23-tache-02-editeur-rendu.html#autotoc_md1237", null ],
          [ "Définition de fait (DoD)", "lot-23-tache-02-editeur-rendu.html#autotoc_md1238", null ],
          [ "Exigences", "lot-23-tache-02-editeur-rendu.html#autotoc_md1239", null ]
        ] ],
        [ "TACHE-03 — Documentation et vérification", "lot-23-tache-03-documentation-verification.html", [
          [ "Contexte", "lot-23-tache-03-documentation-verification.html#autotoc_md1240", null ],
          [ "Travail à réaliser", "lot-23-tache-03-documentation-verification.html#autotoc_md1241", null ],
          [ "Fichiers impactés", "lot-23-tache-03-documentation-verification.html#autotoc_md1242", null ],
          [ "Tests (obligatoires)", "lot-23-tache-03-documentation-verification.html#autotoc_md1243", null ],
          [ "Points d'attention", "lot-23-tache-03-documentation-verification.html#autotoc_md1244", null ],
          [ "Définition de fait (DoD)", "lot-23-tache-03-documentation-verification.html#autotoc_md1245", null ],
          [ "Exigences", "lot-23-tache-03-documentation-verification.html#autotoc_md1246", null ]
        ] ]
      ] ],
      [ "LOT-24 — Blocs à taille fractionnaire", "lot-24.html", [
        [ "Objectif", "lot-24.html#autotoc_md1247", null ],
        [ "Périmètre", "lot-24.html#autotoc_md1248", [
          [ "Inclus", "lot-24.html#autotoc_md1249", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-24.html#autotoc_md1250", null ]
        ] ],
        [ "Décisions de cadrage", "lot-24.html#autotoc_md1251", null ],
        [ "Exigences couvertes", "lot-24.html#autotoc_md1252", null ],
        [ "Découpage", "lot-24.html#autotoc_md1253", null ],
        [ "Critères d'acceptation du lot", "lot-24.html#autotoc_md1254", null ],
        [ "Dépendances", "lot-24.html#autotoc_md1255", null ],
        [ "Navigation des tâches", "lot-24.html#autotoc_md1256", null ],
        [ "TACHE-01 — Modèle de bloc réduit", "lot-24-tache-01-modele-bloc-reduit.html", [
          [ "Contexte", "lot-24-tache-01-modele-bloc-reduit.html#autotoc_md1257", null ],
          [ "Travail à réaliser", "lot-24-tache-01-modele-bloc-reduit.html#autotoc_md1258", null ],
          [ "Fichiers impactés", "lot-24-tache-01-modele-bloc-reduit.html#autotoc_md1259", null ],
          [ "Tests (obligatoires)", "lot-24-tache-01-modele-bloc-reduit.html#autotoc_md1260", null ],
          [ "Points d'attention", "lot-24-tache-01-modele-bloc-reduit.html#autotoc_md1261", null ],
          [ "Définition de fait (DoD)", "lot-24-tache-01-modele-bloc-reduit.html#autotoc_md1262", null ],
          [ "Exigences", "lot-24-tache-01-modele-bloc-reduit.html#autotoc_md1263", null ]
        ] ],
        [ "TACHE-02 — Collision boîte-contre-boîte", "lot-24-tache-02-collision-boite-boite.html", [
          [ "Contexte", "lot-24-tache-02-collision-boite-boite.html#autotoc_md1264", null ],
          [ "Travail à réaliser", "lot-24-tache-02-collision-boite-boite.html#autotoc_md1265", null ],
          [ "Fichiers impactés", "lot-24-tache-02-collision-boite-boite.html#autotoc_md1266", null ],
          [ "Tests (obligatoires)", "lot-24-tache-02-collision-boite-boite.html#autotoc_md1267", null ],
          [ "Points d'attention", "lot-24-tache-02-collision-boite-boite.html#autotoc_md1268", [
            [ "Décision retenue : composition côté <tt>GameScreen</tt>, <tt>CharacterPhysicsSystem</tt> inchangé", "lot-24-tache-02-collision-boite-boite.html#autotoc_md1269", null ]
          ] ],
          [ "Définition de fait (DoD)", "lot-24-tache-02-collision-boite-boite.html#autotoc_md1270", null ],
          [ "Exigences", "lot-24-tache-02-collision-boite-boite.html#autotoc_md1271", null ]
        ] ],
        [ "TACHE-03 — Éditeur et rendu", "lot-24-tache-03-editeur-rendu.html", [
          [ "Contexte", "lot-24-tache-03-editeur-rendu.html#autotoc_md1272", null ],
          [ "Travail à réaliser", "lot-24-tache-03-editeur-rendu.html#autotoc_md1273", null ],
          [ "Fichiers impactés", "lot-24-tache-03-editeur-rendu.html#autotoc_md1274", null ],
          [ "Tests (obligatoires)", "lot-24-tache-03-editeur-rendu.html#autotoc_md1275", null ],
          [ "Points d'attention", "lot-24-tache-03-editeur-rendu.html#autotoc_md1276", null ],
          [ "Définition de fait (DoD)", "lot-24-tache-03-editeur-rendu.html#autotoc_md1277", null ],
          [ "Exigences", "lot-24-tache-03-editeur-rendu.html#autotoc_md1278", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-24-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-24-tache-04-documentation-verification.html#autotoc_md1279", null ],
          [ "Travail à réaliser", "lot-24-tache-04-documentation-verification.html#autotoc_md1280", null ],
          [ "Fichiers impactés", "lot-24-tache-04-documentation-verification.html#autotoc_md1281", null ],
          [ "Tests (obligatoires)", "lot-24-tache-04-documentation-verification.html#autotoc_md1282", null ],
          [ "Points d'attention", "lot-24-tache-04-documentation-verification.html#autotoc_md1283", null ],
          [ "Définition de fait (DoD)", "lot-24-tache-04-documentation-verification.html#autotoc_md1284", null ],
          [ "Exigences", "lot-24-tache-04-documentation-verification.html#autotoc_md1285", null ]
        ] ]
      ] ],
      [ "LOT-25 — Refactoring complet des niveaux démo", "lot-25.html", [
        [ "Objectif", "lot-25.html#autotoc_md1286", null ],
        [ "Périmètre", "lot-25.html#autotoc_md1287", [
          [ "Inclus", "lot-25.html#autotoc_md1288", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-25.html#autotoc_md1289", null ]
        ] ],
        [ "Décisions de cadrage", "lot-25.html#autotoc_md1290", null ],
        [ "Découpage", "lot-25.html#autotoc_md1291", null ],
        [ "Critères d'acceptation du lot", "lot-25.html#autotoc_md1292", null ],
        [ "Dépendances", "lot-25.html#autotoc_md1293", null ],
        [ "Navigation des tâches", "lot-25.html#autotoc_md1294", null ],
        [ "TACHE-01 — Inventaire des mécaniques et conception", "lot-25-tache-01-inventaire-conception.html", [
          [ "Contexte", "lot-25-tache-01-inventaire-conception.html#autotoc_md1295", null ],
          [ "Travail à réaliser", "lot-25-tache-01-inventaire-conception.html#autotoc_md1296", null ],
          [ "Fichiers impactés", "lot-25-tache-01-inventaire-conception.html#autotoc_md1297", null ],
          [ "Tests (obligatoires)", "lot-25-tache-01-inventaire-conception.html#autotoc_md1298", null ],
          [ "Points d'attention", "lot-25-tache-01-inventaire-conception.html#autotoc_md1299", null ],
          [ "Définition de fait (DoD)", "lot-25-tache-01-inventaire-conception.html#autotoc_md1300", null ],
          [ "Tableau mécanique → niveau (final)", "lot-25-tache-01-inventaire-conception.html#autotoc_md1301", null ],
          [ "Niveau final combiné (<tt>demo-final.json</tt>)", "lot-25-tache-01-inventaire-conception.html#autotoc_md1302", null ],
          [ "Exigences", "lot-25-tache-01-inventaire-conception.html#autotoc_md1303", null ]
        ] ],
        [ "TACHE-02 — Implémentation des niveaux", "lot-25-tache-02-implementation-niveaux.html", [
          [ "Contexte", "lot-25-tache-02-implementation-niveaux.html#autotoc_md1304", null ],
          [ "Travail à réaliser", "lot-25-tache-02-implementation-niveaux.html#autotoc_md1305", null ],
          [ "Fichiers impactés", "lot-25-tache-02-implementation-niveaux.html#autotoc_md1306", null ],
          [ "Tests (obligatoires)", "lot-25-tache-02-implementation-niveaux.html#autotoc_md1307", null ],
          [ "Points d'attention", "lot-25-tache-02-implementation-niveaux.html#autotoc_md1308", null ],
          [ "Définition de fait (DoD)", "lot-25-tache-02-implementation-niveaux.html#autotoc_md1309", null ],
          [ "Exigences", "lot-25-tache-02-implementation-niveaux.html#autotoc_md1310", null ]
        ] ],
        [ "TACHE-03 — Intégration séquence et tests système", "lot-25-tache-03-integration-sequence-tests.html", [
          [ "Contexte", "lot-25-tache-03-integration-sequence-tests.html#autotoc_md1311", null ],
          [ "Travail à réaliser", "lot-25-tache-03-integration-sequence-tests.html#autotoc_md1312", null ],
          [ "Fichiers impactés", "lot-25-tache-03-integration-sequence-tests.html#autotoc_md1313", null ],
          [ "Tests (obligatoires)", "lot-25-tache-03-integration-sequence-tests.html#autotoc_md1314", null ],
          [ "Points d'attention", "lot-25-tache-03-integration-sequence-tests.html#autotoc_md1315", null ],
          [ "Définition de fait (DoD)", "lot-25-tache-03-integration-sequence-tests.html#autotoc_md1316", null ],
          [ "Exigences", "lot-25-tache-03-integration-sequence-tests.html#autotoc_md1317", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-25-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-25-tache-04-documentation-verification.html#autotoc_md1318", null ],
          [ "Travail à réaliser", "lot-25-tache-04-documentation-verification.html#autotoc_md1319", null ],
          [ "Fichiers impactés", "lot-25-tache-04-documentation-verification.html#autotoc_md1320", null ],
          [ "Tests (obligatoires)", "lot-25-tache-04-documentation-verification.html#autotoc_md1321", null ],
          [ "Points d'attention", "lot-25-tache-04-documentation-verification.html#autotoc_md1322", null ],
          [ "Définition de fait (DoD)", "lot-25-tache-04-documentation-verification.html#autotoc_md1323", null ],
          [ "Exigences", "lot-25-tache-04-documentation-verification.html#autotoc_md1324", null ]
        ] ]
      ] ],
      [ "LOT-26 — Pentes et arrondis de plafond", "lot-26.html", [
        [ "Objectif", "lot-26.html#autotoc_md1325", null ],
        [ "Périmètre", "lot-26.html#autotoc_md1326", [
          [ "Inclus", "lot-26.html#autotoc_md1327", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-26.html#autotoc_md1328", null ]
        ] ],
        [ "Décisions de cadrage", "lot-26.html#autotoc_md1329", null ],
        [ "Exigences couvertes", "lot-26.html#autotoc_md1330", null ],
        [ "Découpage", "lot-26.html#autotoc_md1331", null ],
        [ "Critères d'acceptation du lot", "lot-26.html#autotoc_md1332", null ],
        [ "Dépendances", "lot-26.html#autotoc_md1333", null ],
        [ "Navigation des tâches", "lot-26.html#autotoc_md1334", null ],
        [ "TACHE-01 — Modèle de tuile et physique de suivi", "lot-26-tache-01-modele-physique-plafond.html", [
          [ "Contexte", "lot-26-tache-01-modele-physique-plafond.html#autotoc_md1335", null ],
          [ "Travail à réaliser", "lot-26-tache-01-modele-physique-plafond.html#autotoc_md1336", null ],
          [ "Fichiers impactés", "lot-26-tache-01-modele-physique-plafond.html#autotoc_md1337", null ],
          [ "Tests (obligatoires)", "lot-26-tache-01-modele-physique-plafond.html#autotoc_md1338", null ],
          [ "Points d'attention", "lot-26-tache-01-modele-physique-plafond.html#autotoc_md1339", null ],
          [ "Définition de fait (DoD)", "lot-26-tache-01-modele-physique-plafond.html#autotoc_md1340", null ],
          [ "Exigences", "lot-26-tache-01-modele-physique-plafond.html#autotoc_md1341", null ]
        ] ],
        [ "TACHE-02 — Éditeur et rendu", "lot-26-tache-02-editeur-rendu.html", [
          [ "Contexte", "lot-26-tache-02-editeur-rendu.html#autotoc_md1342", null ],
          [ "Travail à réaliser", "lot-26-tache-02-editeur-rendu.html#autotoc_md1343", null ],
          [ "Fichiers impactés", "lot-26-tache-02-editeur-rendu.html#autotoc_md1344", null ],
          [ "Tests (obligatoires)", "lot-26-tache-02-editeur-rendu.html#autotoc_md1345", null ],
          [ "Points d'attention", "lot-26-tache-02-editeur-rendu.html#autotoc_md1346", null ],
          [ "Définition de fait (DoD)", "lot-26-tache-02-editeur-rendu.html#autotoc_md1347", null ],
          [ "Exigences", "lot-26-tache-02-editeur-rendu.html#autotoc_md1348", null ]
        ] ],
        [ "TACHE-03 — Documentation et vérification", "lot-26-tache-03-documentation-verification.html", [
          [ "Contexte", "lot-26-tache-03-documentation-verification.html#autotoc_md1349", null ],
          [ "Travail à réaliser", "lot-26-tache-03-documentation-verification.html#autotoc_md1350", null ],
          [ "Fichiers impactés", "lot-26-tache-03-documentation-verification.html#autotoc_md1351", null ],
          [ "Tests (obligatoires)", "lot-26-tache-03-documentation-verification.html#autotoc_md1352", null ],
          [ "Points d'attention", "lot-26-tache-03-documentation-verification.html#autotoc_md1353", null ],
          [ "Définition de fait (DoD)", "lot-26-tache-03-documentation-verification.html#autotoc_md1354", null ],
          [ "Exigences", "lot-26-tache-03-documentation-verification.html#autotoc_md1355", null ]
        ] ]
      ] ],
      [ "LOT-27 — Palette de l'éditeur organisée par catégories", "lot-27.html", [
        [ "Objectif", "lot-27.html#autotoc_md1356", null ],
        [ "Périmètre", "lot-27.html#autotoc_md1357", [
          [ "Inclus", "lot-27.html#autotoc_md1358", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-27.html#autotoc_md1359", null ]
        ] ],
        [ "Décisions de cadrage", "lot-27.html#autotoc_md1360", null ],
        [ "Exigences couvertes", "lot-27.html#autotoc_md1361", null ],
        [ "Découpage", "lot-27.html#autotoc_md1362", null ],
        [ "Critères d'acceptation du lot", "lot-27.html#autotoc_md1363", null ],
        [ "Dépendances", "lot-27.html#autotoc_md1364", null ],
        [ "Navigation des tâches", "lot-27.html#autotoc_md1365", null ],
        [ "TACHE-01 — Palette en accordéon à trois niveaux", "lot-27-tache-01-palette-accordeon.html", [
          [ "Contexte", "lot-27-tache-01-palette-accordeon.html#autotoc_md1366", null ],
          [ "Travail à réaliser", "lot-27-tache-01-palette-accordeon.html#autotoc_md1367", null ],
          [ "Fichiers impactés", "lot-27-tache-01-palette-accordeon.html#autotoc_md1368", null ],
          [ "Tests (obligatoires)", "lot-27-tache-01-palette-accordeon.html#autotoc_md1369", null ],
          [ "Points d'attention", "lot-27-tache-01-palette-accordeon.html#autotoc_md1370", null ],
          [ "Définition de fait (DoD)", "lot-27-tache-01-palette-accordeon.html#autotoc_md1371", null ],
          [ "Exigences", "lot-27-tache-01-palette-accordeon.html#autotoc_md1372", null ]
        ] ],
        [ "TACHE-02 — Documentation et vérification", "lot-27-tache-02-documentation-verification.html", [
          [ "Contexte", "lot-27-tache-02-documentation-verification.html#autotoc_md1373", null ],
          [ "Travail à réaliser", "lot-27-tache-02-documentation-verification.html#autotoc_md1374", null ],
          [ "Fichiers impactés", "lot-27-tache-02-documentation-verification.html#autotoc_md1375", null ],
          [ "Tests (obligatoires)", "lot-27-tache-02-documentation-verification.html#autotoc_md1376", null ],
          [ "Définition de fait (DoD)", "lot-27-tache-02-documentation-verification.html#autotoc_md1377", null ],
          [ "Exigences", "lot-27-tache-02-documentation-verification.html#autotoc_md1378", null ]
        ] ]
      ] ],
      [ "LOT-28 — Arrondis concaves", "lot-28.html", [
        [ "Objectif", "lot-28.html#autotoc_md1379", null ],
        [ "Périmètre", "lot-28.html#autotoc_md1380", [
          [ "Inclus", "lot-28.html#autotoc_md1381", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-28.html#autotoc_md1382", null ]
        ] ],
        [ "Décisions de cadrage", "lot-28.html#autotoc_md1383", null ],
        [ "Exigences couvertes", "lot-28.html#autotoc_md1384", null ],
        [ "Découpage", "lot-28.html#autotoc_md1385", null ],
        [ "Critères d'acceptation du lot", "lot-28.html#autotoc_md1386", null ],
        [ "Dépendances", "lot-28.html#autotoc_md1387", null ],
        [ "Navigation des tâches", "lot-28.html#autotoc_md1388", null ],
        [ "TACHE-01 — Modèle de tuile et formule de courbe concave", "lot-28-tache-01-modele-physique-concave.html", [
          [ "Contexte", "lot-28-tache-01-modele-physique-concave.html#autotoc_md1389", null ],
          [ "Travail à réaliser", "lot-28-tache-01-modele-physique-concave.html#autotoc_md1390", null ],
          [ "Fichiers impactés", "lot-28-tache-01-modele-physique-concave.html#autotoc_md1391", null ],
          [ "Tests (obligatoires)", "lot-28-tache-01-modele-physique-concave.html#autotoc_md1392", null ],
          [ "Points d'attention", "lot-28-tache-01-modele-physique-concave.html#autotoc_md1393", null ],
          [ "Définition de fait (DoD)", "lot-28-tache-01-modele-physique-concave.html#autotoc_md1394", null ],
          [ "Exigences", "lot-28-tache-01-modele-physique-concave.html#autotoc_md1395", null ]
        ] ],
        [ "TACHE-02 — Éditeur et rendu", "lot-28-tache-02-editeur-rendu.html", [
          [ "Contexte", "lot-28-tache-02-editeur-rendu.html#autotoc_md1396", null ],
          [ "Travail à réaliser", "lot-28-tache-02-editeur-rendu.html#autotoc_md1397", null ],
          [ "Fichiers impactés", "lot-28-tache-02-editeur-rendu.html#autotoc_md1398", null ],
          [ "Tests (obligatoires)", "lot-28-tache-02-editeur-rendu.html#autotoc_md1399", null ],
          [ "Points d'attention", "lot-28-tache-02-editeur-rendu.html#autotoc_md1400", null ],
          [ "Définition de fait (DoD)", "lot-28-tache-02-editeur-rendu.html#autotoc_md1401", null ],
          [ "Exigences", "lot-28-tache-02-editeur-rendu.html#autotoc_md1402", null ]
        ] ],
        [ "TACHE-03 — Documentation et vérification", "lot-28-tache-03-documentation-verification.html", [
          [ "Contexte", "lot-28-tache-03-documentation-verification.html#autotoc_md1403", null ],
          [ "Travail à réaliser", "lot-28-tache-03-documentation-verification.html#autotoc_md1404", null ],
          [ "Fichiers impactés", "lot-28-tache-03-documentation-verification.html#autotoc_md1405", null ],
          [ "Tests (obligatoires)", "lot-28-tache-03-documentation-verification.html#autotoc_md1406", null ],
          [ "Points d'attention", "lot-28-tache-03-documentation-verification.html#autotoc_md1407", null ],
          [ "Définition de fait (DoD)", "lot-28-tache-03-documentation-verification.html#autotoc_md1408", null ],
          [ "Exigences", "lot-28-tache-03-documentation-verification.html#autotoc_md1409", null ]
        ] ]
      ] ],
      [ "LOT-29 — Remappage des touches (jeu + éditeur)", "lot-29.html", [
        [ "Objectif", "lot-29.html#autotoc_md1410", null ],
        [ "Périmètre", "lot-29.html#autotoc_md1411", [
          [ "Inclus", "lot-29.html#autotoc_md1412", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-29.html#autotoc_md1413", null ]
        ] ],
        [ "Décisions de cadrage", "lot-29.html#autotoc_md1414", null ],
        [ "Exigences couvertes", "lot-29.html#autotoc_md1415", null ],
        [ "Découpage", "lot-29.html#autotoc_md1416", null ],
        [ "Critères d'acceptation du lot", "lot-29.html#autotoc_md1417", null ],
        [ "Dépendances", "lot-29.html#autotoc_md1418", null ],
        [ "Navigation des tâches", "lot-29.html#autotoc_md1419", null ],
        [ "TACHE-01 — Modèle de bindings et persistance JSON", "lot-29-tache-01-modele-bindings.html", [
          [ "Contexte", "lot-29-tache-01-modele-bindings.html#autotoc_md1420", null ],
          [ "Travail à réaliser", "lot-29-tache-01-modele-bindings.html#autotoc_md1421", null ],
          [ "Fichiers impactés", "lot-29-tache-01-modele-bindings.html#autotoc_md1422", null ],
          [ "Tests (obligatoires)", "lot-29-tache-01-modele-bindings.html#autotoc_md1423", null ],
          [ "Points d'attention", "lot-29-tache-01-modele-bindings.html#autotoc_md1424", null ],
          [ "Définition de fait (DoD)", "lot-29-tache-01-modele-bindings.html#autotoc_md1425", null ],
          [ "Exigences", "lot-29-tache-01-modele-bindings.html#autotoc_md1426", null ]
        ] ],
        [ "TACHE-02 — Intégration jeu/éditeur", "lot-29-tache-02-integration-jeu-editeur.html", [
          [ "Contexte", "lot-29-tache-02-integration-jeu-editeur.html#autotoc_md1427", null ],
          [ "Travail à réaliser", "lot-29-tache-02-integration-jeu-editeur.html#autotoc_md1428", null ],
          [ "Fichiers impactés", "lot-29-tache-02-integration-jeu-editeur.html#autotoc_md1429", null ],
          [ "Tests (obligatoires)", "lot-29-tache-02-integration-jeu-editeur.html#autotoc_md1430", null ],
          [ "Points d'attention", "lot-29-tache-02-integration-jeu-editeur.html#autotoc_md1431", null ],
          [ "Définition de fait (DoD)", "lot-29-tache-02-integration-jeu-editeur.html#autotoc_md1432", null ],
          [ "Exigences", "lot-29-tache-02-integration-jeu-editeur.html#autotoc_md1433", null ]
        ] ],
        [ "TACHE-03 — UI de remappage et câblage", "lot-29-tache-03-ui-remappage.html", [
          [ "Contexte", "lot-29-tache-03-ui-remappage.html#autotoc_md1434", null ],
          [ "Travail à réaliser", "lot-29-tache-03-ui-remappage.html#autotoc_md1435", null ],
          [ "Fichiers impactés", "lot-29-tache-03-ui-remappage.html#autotoc_md1436", null ],
          [ "Tests (obligatoires)", "lot-29-tache-03-ui-remappage.html#autotoc_md1437", null ],
          [ "Points d'attention", "lot-29-tache-03-ui-remappage.html#autotoc_md1438", null ],
          [ "Définition de fait (DoD)", "lot-29-tache-03-ui-remappage.html#autotoc_md1439", null ],
          [ "Exigences", "lot-29-tache-03-ui-remappage.html#autotoc_md1440", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-29-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-29-tache-04-documentation-verification.html#autotoc_md1441", null ],
          [ "Travail à réaliser", "lot-29-tache-04-documentation-verification.html#autotoc_md1442", null ],
          [ "Fichiers impactés", "lot-29-tache-04-documentation-verification.html#autotoc_md1443", null ],
          [ "Tests (obligatoires)", "lot-29-tache-04-documentation-verification.html#autotoc_md1444", null ],
          [ "Points d'attention", "lot-29-tache-04-documentation-verification.html#autotoc_md1445", null ],
          [ "Définition de fait (DoD)", "lot-29-tache-04-documentation-verification.html#autotoc_md1446", null ],
          [ "Exigences", "lot-29-tache-04-documentation-verification.html#autotoc_md1447", null ]
        ] ]
      ] ],
      [ "LOT-30 — Remappage manette (jeu)", "lot-30.html", [
        [ "Objectif", "lot-30.html#autotoc_md1448", null ],
        [ "Périmètre", "lot-30.html#autotoc_md1449", [
          [ "Inclus", "lot-30.html#autotoc_md1450", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-30.html#autotoc_md1451", null ]
        ] ],
        [ "Décisions de cadrage", "lot-30.html#autotoc_md1452", null ],
        [ "Exigences couvertes", "lot-30.html#autotoc_md1453", null ],
        [ "Découpage", "lot-30.html#autotoc_md1454", null ],
        [ "Critères d'acceptation du lot", "lot-30.html#autotoc_md1455", null ],
        [ "Dépendances", "lot-30.html#autotoc_md1456", null ],
        [ "Navigation des tâches", "lot-30.html#autotoc_md1457", null ],
        [ "TACHE-01 — Modèle de bindings manette", "lot-30-tache-01-modele-bindings-manette.html", [
          [ "Contexte", "lot-30-tache-01-modele-bindings-manette.html#autotoc_md1458", null ],
          [ "Travail à réaliser", "lot-30-tache-01-modele-bindings-manette.html#autotoc_md1459", null ],
          [ "Fichiers impactés", "lot-30-tache-01-modele-bindings-manette.html#autotoc_md1460", null ],
          [ "Tests (obligatoires)", "lot-30-tache-01-modele-bindings-manette.html#autotoc_md1461", null ],
          [ "Définition de fait (DoD)", "lot-30-tache-01-modele-bindings-manette.html#autotoc_md1462", null ],
          [ "Exigences", "lot-30-tache-01-modele-bindings-manette.html#autotoc_md1463", null ]
        ] ],
        [ "TACHE-02 — Intégration jeu", "lot-30-tache-02-integration-jeu.html", [
          [ "Contexte", "lot-30-tache-02-integration-jeu.html#autotoc_md1464", null ],
          [ "Travail à réaliser", "lot-30-tache-02-integration-jeu.html#autotoc_md1465", null ],
          [ "Fichiers impactés", "lot-30-tache-02-integration-jeu.html#autotoc_md1466", null ],
          [ "Tests (obligatoires)", "lot-30-tache-02-integration-jeu.html#autotoc_md1467", null ],
          [ "Points d'attention", "lot-30-tache-02-integration-jeu.html#autotoc_md1468", null ],
          [ "Définition de fait (DoD)", "lot-30-tache-02-integration-jeu.html#autotoc_md1469", null ],
          [ "Exigences", "lot-30-tache-02-integration-jeu.html#autotoc_md1470", null ]
        ] ],
        [ "TACHE-03 — UI de remappage manette et câblage", "lot-30-tache-03-ui-remappage-manette.html", [
          [ "Contexte", "lot-30-tache-03-ui-remappage-manette.html#autotoc_md1471", null ],
          [ "Travail à réaliser", "lot-30-tache-03-ui-remappage-manette.html#autotoc_md1472", null ],
          [ "Fichiers impactés", "lot-30-tache-03-ui-remappage-manette.html#autotoc_md1473", null ],
          [ "Tests (obligatoires)", "lot-30-tache-03-ui-remappage-manette.html#autotoc_md1474", null ],
          [ "Points d'attention", "lot-30-tache-03-ui-remappage-manette.html#autotoc_md1475", null ],
          [ "Définition de fait (DoD)", "lot-30-tache-03-ui-remappage-manette.html#autotoc_md1476", null ],
          [ "Exigences", "lot-30-tache-03-ui-remappage-manette.html#autotoc_md1477", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-30-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-30-tache-04-documentation-verification.html#autotoc_md1478", null ],
          [ "Travail à réaliser", "lot-30-tache-04-documentation-verification.html#autotoc_md1479", null ],
          [ "Fichiers impactés", "lot-30-tache-04-documentation-verification.html#autotoc_md1480", null ],
          [ "Tests (obligatoires)", "lot-30-tache-04-documentation-verification.html#autotoc_md1481", null ],
          [ "Définition de fait (DoD)", "lot-30-tache-04-documentation-verification.html#autotoc_md1482", null ],
          [ "Exigences", "lot-30-tache-04-documentation-verification.html#autotoc_md1483", null ]
        ] ]
      ] ],
      [ "LOT-31 — Blocs de danger avancés", "lot-31.html", [
        [ "Objectif", "lot-31.html#autotoc_md1484", null ],
        [ "Périmètre", "lot-31.html#autotoc_md1485", [
          [ "Inclus", "lot-31.html#autotoc_md1486", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-31.html#autotoc_md1487", null ]
        ] ],
        [ "Décisions de cadrage", "lot-31.html#autotoc_md1488", null ],
        [ "Exigences couvertes", "lot-31.html#autotoc_md1489", null ],
        [ "Découpage", "lot-31.html#autotoc_md1490", null ],
        [ "Critères d'acceptation du lot", "lot-31.html#autotoc_md1491", null ],
        [ "Dépendances", "lot-31.html#autotoc_md1492", null ],
        [ "Navigation des tâches", "lot-31.html#autotoc_md1493", null ],
        [ "TACHE-01 — Modèle (types, format, généralisation des liaisons)", "lot-31-tache-01-modele-dangers-avances.html", [
          [ "Contexte", "lot-31-tache-01-modele-dangers-avances.html#autotoc_md1494", null ],
          [ "Travail à réaliser", "lot-31-tache-01-modele-dangers-avances.html#autotoc_md1495", null ],
          [ "Fichiers impactés", "lot-31-tache-01-modele-dangers-avances.html#autotoc_md1496", null ],
          [ "Tests (obligatoires)", "lot-31-tache-01-modele-dangers-avances.html#autotoc_md1497", null ],
          [ "Définition de fait (DoD)", "lot-31-tache-01-modele-dangers-avances.html#autotoc_md1498", null ],
          [ "Exigences", "lot-31-tache-01-modele-dangers-avances.html#autotoc_md1499", null ]
        ] ],
        [ "TACHE-02 — Intégration jeu (contrôleurs, résolution de fin de niveau)", "lot-31-tache-02-integration-jeu.html", [
          [ "Contexte", "lot-31-tache-02-integration-jeu.html#autotoc_md1500", null ],
          [ "Travail à réaliser", "lot-31-tache-02-integration-jeu.html#autotoc_md1501", null ],
          [ "Fichiers impactés", "lot-31-tache-02-integration-jeu.html#autotoc_md1502", null ],
          [ "Tests (obligatoires)", "lot-31-tache-02-integration-jeu.html#autotoc_md1503", null ],
          [ "Définition de fait (DoD)", "lot-31-tache-02-integration-jeu.html#autotoc_md1504", null ],
          [ "Exigences", "lot-31-tache-02-integration-jeu.html#autotoc_md1505", null ]
        ] ],
        [ "TACHE-03 — Intégration éditeur (palette, rendu, liaison)", "lot-31-tache-03-integration-editeur.html", [
          [ "Contexte", "lot-31-tache-03-integration-editeur.html#autotoc_md1506", null ],
          [ "Travail réalisé", "lot-31-tache-03-integration-editeur.html#autotoc_md1507", null ],
          [ "Fichiers impactés", "lot-31-tache-03-integration-editeur.html#autotoc_md1508", null ],
          [ "Tests (réalisés)", "lot-31-tache-03-integration-editeur.html#autotoc_md1509", null ],
          [ "Définition de fait (DoD)", "lot-31-tache-03-integration-editeur.html#autotoc_md1510", null ],
          [ "Exigences", "lot-31-tache-03-integration-editeur.html#autotoc_md1511", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-31-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-31-tache-04-documentation-verification.html#autotoc_md1512", null ],
          [ "Travail réalisé", "lot-31-tache-04-documentation-verification.html#autotoc_md1513", null ],
          [ "Fichiers impactés", "lot-31-tache-04-documentation-verification.html#autotoc_md1514", null ],
          [ "Tests (réalisés)", "lot-31-tache-04-documentation-verification.html#autotoc_md1515", null ],
          [ "Définition de fait (DoD)", "lot-31-tache-04-documentation-verification.html#autotoc_md1516", null ],
          [ "Exigences", "lot-31-tache-04-documentation-verification.html#autotoc_md1517", null ]
        ] ]
      ] ],
      [ "LOT-32 — Niveaux à salles (façon Celeste)", "lot-32.html", [
        [ "Objectif", "lot-32.html#autotoc_md1518", null ],
        [ "Périmètre", "lot-32.html#autotoc_md1519", [
          [ "Inclus", "lot-32.html#autotoc_md1520", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-32.html#autotoc_md1521", null ]
        ] ],
        [ "Décisions de cadrage", "lot-32.html#autotoc_md1522", null ],
        [ "Exigences couvertes", "lot-32.html#autotoc_md1523", null ],
        [ "Découpage", "lot-32.html#autotoc_md1524", null ],
        [ "Critères d'acceptation du lot", "lot-32.html#autotoc_md1525", null ],
        [ "Dépendances", "lot-32.html#autotoc_md1526", null ],
        [ "Navigation des tâches", "lot-32.html#autotoc_md1527", null ],
        [ "TACHE-01 — Partition en salles (`RoomGrid`, logique pure)", "lot-32-tache-01-partition-salles.html", [
          [ "Contexte", "lot-32-tache-01-partition-salles.html#autotoc_md1528", null ],
          [ "Travail à réaliser", "lot-32-tache-01-partition-salles.html#autotoc_md1529", null ],
          [ "Fichiers impactés", "lot-32-tache-01-partition-salles.html#autotoc_md1530", null ],
          [ "Tests (obligatoires)", "lot-32-tache-01-partition-salles.html#autotoc_md1531", null ],
          [ "Points d'attention", "lot-32-tache-01-partition-salles.html#autotoc_md1532", null ],
          [ "Définition de fait (DoD)", "lot-32-tache-01-partition-salles.html#autotoc_md1533", null ],
          [ "Exigences", "lot-32-tache-01-partition-salles.html#autotoc_md1534", null ]
        ] ],
        [ "TACHE-02 — Caméra par salle en jeu (coupure nette)", "lot-32-tache-02-camera-salle-jeu.html", [
          [ "Contexte", "lot-32-tache-02-camera-salle-jeu.html#autotoc_md1535", null ],
          [ "Travail à réaliser", "lot-32-tache-02-camera-salle-jeu.html#autotoc_md1536", null ],
          [ "Fichiers impactés", "lot-32-tache-02-camera-salle-jeu.html#autotoc_md1537", null ],
          [ "Tests (obligatoires)", "lot-32-tache-02-camera-salle-jeu.html#autotoc_md1538", null ],
          [ "Points d'attention", "lot-32-tache-02-camera-salle-jeu.html#autotoc_md1539", null ],
          [ "Définition de fait (DoD)", "lot-32-tache-02-camera-salle-jeu.html#autotoc_md1540", null ],
          [ "Exigences", "lot-32-tache-02-camera-salle-jeu.html#autotoc_md1541", null ]
        ] ],
        [ "TACHE-03 — Repère visuel de salles dans l'éditeur", "lot-32-tache-03-repere-editeur.html", [
          [ "Contexte", "lot-32-tache-03-repere-editeur.html#autotoc_md1542", null ],
          [ "Travail à réaliser", "lot-32-tache-03-repere-editeur.html#autotoc_md1543", null ],
          [ "Fichiers impactés", "lot-32-tache-03-repere-editeur.html#autotoc_md1544", null ],
          [ "Tests (obligatoires)", "lot-32-tache-03-repere-editeur.html#autotoc_md1545", null ],
          [ "Points d'attention", "lot-32-tache-03-repere-editeur.html#autotoc_md1546", null ],
          [ "Définition de fait (DoD)", "lot-32-tache-03-repere-editeur.html#autotoc_md1547", null ],
          [ "Exigences", "lot-32-tache-03-repere-editeur.html#autotoc_md1548", null ]
        ] ],
        [ "TACHE-04 — Niveau de démonstration, documentation et vérification", "lot-32-tache-04-demo-documentation-verification.html", [
          [ "Contexte", "lot-32-tache-04-demo-documentation-verification.html#autotoc_md1549", null ],
          [ "Travail à réaliser", "lot-32-tache-04-demo-documentation-verification.html#autotoc_md1550", null ],
          [ "Fichiers impactés", "lot-32-tache-04-demo-documentation-verification.html#autotoc_md1551", null ],
          [ "Tests (obligatoires)", "lot-32-tache-04-demo-documentation-verification.html#autotoc_md1552", null ],
          [ "Points d'attention", "lot-32-tache-04-demo-documentation-verification.html#autotoc_md1553", null ],
          [ "Définition de fait (DoD)", "lot-32-tache-04-demo-documentation-verification.html#autotoc_md1554", null ],
          [ "Exigences", "lot-32-tache-04-demo-documentation-verification.html#autotoc_md1555", null ]
        ] ]
      ] ],
      [ "LOT-33 — Fluidité du moteur (entrées nerveuses, présentation flip, interpolation)", "lot-33.html", [
        [ "Objectif", "lot-33.html#autotoc_md1556", null ],
        [ "Périmètre", "lot-33.html#autotoc_md1557", [
          [ "Inclus", "lot-33.html#autotoc_md1558", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-33.html#autotoc_md1559", null ]
        ] ],
        [ "Décisions de cadrage", "lot-33.html#autotoc_md1560", null ],
        [ "Exigences couvertes", "lot-33.html#autotoc_md1561", null ],
        [ "Découpage", "lot-33.html#autotoc_md1562", null ],
        [ "Critères d'acceptation du lot", "lot-33.html#autotoc_md1563", null ],
        [ "Dépendances", "lot-33.html#autotoc_md1564", null ],
        [ "Navigation des tâches", "lot-33.html#autotoc_md1565", null ],
        [ "TACHE-01 — Entrées nerveuses : fronts non perdus, focus, sondage manette", "lot-33-tache-01-entrees-nerveuses.html", [
          [ "Contexte", "lot-33-tache-01-entrees-nerveuses.html#autotoc_md1566", null ],
          [ "Travail à réaliser", "lot-33-tache-01-entrees-nerveuses.html#autotoc_md1567", null ],
          [ "Fichiers impactés", "lot-33-tache-01-entrees-nerveuses.html#autotoc_md1568", null ],
          [ "Tests (obligatoires)", "lot-33-tache-01-entrees-nerveuses.html#autotoc_md1569", null ],
          [ "Définition de fait (DoD)", "lot-33-tache-01-entrees-nerveuses.html#autotoc_md1570", null ],
          [ "Exigences", "lot-33-tache-01-entrees-nerveuses.html#autotoc_md1571", null ]
        ] ],
        [ "TACHE-02 — Présentation flip-model (latence réduite)", "lot-33-tache-02-presentation-flip.html", [
          [ "Contexte", "lot-33-tache-02-presentation-flip.html#autotoc_md1572", null ],
          [ "Travail à réaliser", "lot-33-tache-02-presentation-flip.html#autotoc_md1573", null ],
          [ "Fichiers impactés", "lot-33-tache-02-presentation-flip.html#autotoc_md1574", null ],
          [ "Tests (obligatoires)", "lot-33-tache-02-presentation-flip.html#autotoc_md1575", null ],
          [ "Définition de fait (DoD)", "lot-33-tache-02-presentation-flip.html#autotoc_md1576", null ],
          [ "Exigences", "lot-33-tache-02-presentation-flip.html#autotoc_md1577", null ]
        ] ],
        [ "TACHE-03 — Interpolation de rendu (EX-ARCH-031)", "lot-33-tache-03-interpolation-rendu.html", [
          [ "Contexte", "lot-33-tache-03-interpolation-rendu.html#autotoc_md1578", null ],
          [ "Travail à réaliser", "lot-33-tache-03-interpolation-rendu.html#autotoc_md1579", null ],
          [ "Fichiers impactés", "lot-33-tache-03-interpolation-rendu.html#autotoc_md1580", null ],
          [ "Tests (obligatoires)", "lot-33-tache-03-interpolation-rendu.html#autotoc_md1581", null ],
          [ "Définition de fait (DoD)", "lot-33-tache-03-interpolation-rendu.html#autotoc_md1582", null ],
          [ "Exigences", "lot-33-tache-03-interpolation-rendu.html#autotoc_md1583", null ]
        ] ],
        [ "TACHE-04 — Documentation, spécifications et vérification", "lot-33-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-33-tache-04-documentation-verification.html#autotoc_md1584", null ],
          [ "Travail à réaliser", "lot-33-tache-04-documentation-verification.html#autotoc_md1585", null ],
          [ "Fichiers impactés", "lot-33-tache-04-documentation-verification.html#autotoc_md1586", null ],
          [ "Définition de fait (DoD)", "lot-33-tache-04-documentation-verification.html#autotoc_md1587", null ],
          [ "Exigences", "lot-33-tache-04-documentation-verification.html#autotoc_md1588", null ]
        ] ]
      ] ]
    ] ],
    [ "Manuel utilisateur", "manuel.html", [
      [ "Pages", "manuel.html#autotoc_md1594", null ],
      [ "Télécharger et lancer le jeu", "manuel-telecharger.html", [
        [ "Prérequis", "manuel-telecharger.html#autotoc_md1601", null ],
        [ "Étapes", "manuel-telecharger.html#autotoc_md1602", null ],
        [ "Remarques", "manuel-telecharger.html#autotoc_md1603", null ]
      ] ],
      [ "Jouer", "manuel-jouer.html", [
        [ "Le menu principal", "manuel-jouer.html#autotoc_md1590", null ],
        [ "Contrôles en jeu", "manuel-jouer.html#autotoc_md1591", null ],
        [ "Objectif d'un niveau", "manuel-jouer.html#autotoc_md1592", null ],
        [ "Le menu d'options", "manuel-jouer.html#autotoc_md1593", null ]
      ] ],
      [ "Créer et partager un niveau (sans ligne de commande)", "manuel-partager-niveau.html", [
        [ "1. Récupérer le projet", "manuel-partager-niveau.html#autotoc_md1595", null ],
        [ "2. Lancer l'éditeur", "manuel-partager-niveau.html#autotoc_md1596", null ],
        [ "3. Créer un niveau", "manuel-partager-niveau.html#autotoc_md1597", null ],
        [ "4. Publier votre niveau", "manuel-partager-niveau.html#autotoc_md1598", null ],
        [ "5. Récupérer les niveaux des autres", "manuel-partager-niveau.html#autotoc_md1599", null ],
        [ "En cas de problème", "manuel-partager-niveau.html#autotoc_md1600", null ]
      ] ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"AabbVsAabb_8cpp.html",
"InputState_8h.html#a83c98e56e30eafa606048853f7962e48a013ec032d3460d4be4431c6ab1f8f224",
"TextInputField_8h.html",
"classcore_1_1ILogSink.html",
"classcore_1_1TileMap.html#a8034d1d1ddd90d5824269f9e0694272f",
"classhmi_1_1EditorScreen.html#a2b0a69c9c989ffbb4bccd4478c6588db",
"classhmi_1_1GameScreen.html#ab293c99d5d9398afcadf9e8f16f067ad",
"classhmi_1_1LevelPicker.html#a6cccc300525edeec26fa1485dcad3fde",
"classhmi_1_1ScreenManager.html#a59c57128d6943c33e04df24ba65798dd",
"classhmi_1_1Window.html#a8982fa83d7a924f987f33260b1bcb169",
"guide-editeur.html#autotoc_md76",
"lot-04-tache-01-reorganisation-arbo.html#autotoc_md394",
"lot-07-tache-05-niveau-demo.html#autotoc_md615",
"lot-12-tache-02-mecanismes.html#autotoc_md835",
"lot-18-tache-01-composant-systeme-animation.html",
"lot-24-tache-03-editeur-rendu.html#autotoc_md1278",
"lot-31-tache-01-modele-dangers-avances.html#autotoc_md1499",
"namespacehmi.html#a02048ad8ad69a87a10e8307ac2bd68dda6ff9dd0d34f65181173c1e4bc39939de",
"structcore_1_1Collider.html",
"structcore_1_1Vector2.html#a8c99961609599a2a08bf8db7aff830b9",
"test__component__pool_8cpp.html#a8028dc62abc97367a1097d4e37f57b13",
"test__level__size__validation_8cpp.html#a5897d1630ba9d028babe578246710d6d",
"test__text__input__field_8cpp.html#a310dad3cbfe6043da56b7a4d53959a8d"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
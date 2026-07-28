var guide =
[
    [ "Comment lire ce guide", "guide.html#autotoc_md167", null ],
    [ "Architecture en deux couches", "guide.html#autotoc_md168", null ],
    [ "Plan du guide", "guide.html#autotoc_md169", null ],
    [ "Boucle de jeu et pas de temps fixe", "guide-boucle.html", [
      [ "Qu'est-ce qu'une boucle de jeu ?", "guide-boucle.html#autotoc_md30", null ],
      [ "Le piège du framerate variable", "guide-boucle.html#autotoc_md31", null ],
      [ "Le principe du pas de temps fixe", "guide-boucle.html#autotoc_md32", null ],
      [ "L'accumulateur : <a class=\"el\" href=\"classcore_1_1FixedTimestep.html\" title=\"core::FixedTimestep\">core::FixedTimestep</a>", "guide-boucle.html#autotoc_md33", [
        [ "Exemple chiffré", "guide-boucle.html#autotoc_md34", null ],
        [ "La « spirale de la mort »", "guide-boucle.html#autotoc_md35", null ],
        [ "<a class=\"el\" href=\"classcore_1_1FixedTimestep.html#ab7f7f880eef11df91ab7bc647e19bf56\" title=\"core::FixedTimestep::interpolationAlpha\">interpolationAlpha</a>", "guide-boucle.html#autotoc_md36", null ],
        [ "Les frames sans pas de simulation et les entrées", "guide-boucle.html#autotoc_md37", null ]
      ] ],
      [ "Conséquence pratique pour tout le code de simulation", "guide-boucle.html#autotoc_md38", null ],
      [ "Voir aussi", "guide-boucle.html#autotoc_md39", null ]
    ] ],
    [ "ECS : entités, composants, systèmes", "guide-ecs.html", [
      [ "Le problème que l'ECS résout", "guide-ecs.html#autotoc_md46", null ],
      [ "L'entité : <a class=\"el\" href=\"structcore_1_1Entity.html\" title=\"core::Entity\">core::Entity</a>", "guide-ecs.html#autotoc_md47", null ],
      [ "Le <a class=\"el\" href=\"classcore_1_1World.html\" title=\"core::World\">World</a>", "guide-ecs.html#autotoc_md48", null ],
      [ "Le stockage : sparse set (core::ComponentPool&lt;T&gt;)", "guide-ecs.html#autotoc_md49", [
        [ "Ajout et suppression : <em>swap-and-pop</em>", "guide-ecs.html#autotoc_md50", null ],
        [ "Exemple pas à pas", "guide-ecs.html#autotoc_md51", null ]
      ] ],
      [ "Les vues : core::View&lt;Components...&gt;", "guide-ecs.html#autotoc_md52", null ],
      [ "Les systèmes et l'ordre d'exécution", "guide-ecs.html#autotoc_md53", null ],
      [ "Voir aussi", "guide-ecs.html#autotoc_md54", null ]
    ] ],
    [ "Mathématiques du moteur", "guide-maths.html", [
      [ "<a class=\"el\" href=\"structcore_1_1Vector2.html\" title=\"core::Vector2\">Vector2</a> : un point ou une direction dans le monde", "guide-maths.html#autotoc_md103", [
        [ "<a class=\"el\" href=\"structcore_1_1Vector2.html#a5f3517fe219407f73a3ed5542091c31c\" title=\"core::Vector2::lengthSquared\">lengthSquared</a> : éviter la racine carrée", "guide-maths.html#autotoc_md104", null ],
        [ "Égalité approchée", "guide-maths.html#autotoc_md105", null ]
      ] ],
      [ "<a class=\"el\" href=\"structcore_1_1Aabb.html\" title=\"core::Aabb\">Aabb</a> : la boîte englobante alignée aux axes", "guide-maths.html#autotoc_md106", null ],
      [ "Conventions d'unités et de repère", "guide-maths.html#autotoc_md107", null ],
      [ "Comparaison flottante : pourquoi l'égalité stricte est dangereuse", "guide-maths.html#autotoc_md108", null ],
      [ "Voir aussi", "guide-maths.html#autotoc_md109", null ]
    ] ],
    [ "Physique du personnage", "guide-physique.html", [
      [ "1. Collision par balayage continu (swept AABB)", "guide-physique.html#autotoc_md126", [
        [ "Le problème : le <em>tunneling</em>", "guide-physique.html#autotoc_md127", null ],
        [ "La solution : tester tout le trajet, pas seulement l'arrivée", "guide-physique.html#autotoc_md128", null ],
        [ "Méthode retenue : balayage <b>par axe</b> avec clamp direct", "guide-physique.html#autotoc_md129", null ],
        [ "Pourquoi caler directement plutôt que d'interpoler", "guide-physique.html#autotoc_md130", null ],
        [ "Lire le résultat : <a class=\"el\" href=\"structcore_1_1SweepResult.html\" title=\"core::SweepResult\">core::SweepResult</a>", "guide-physique.html#autotoc_md131", null ]
      ] ],
      [ "2. Suivi de pente et d'arrondi (EX-GP-003, EX-GP-004)", "guide-physique.html#autotoc_md132", [
        [ "Pourquoi une pente (ou un arrondi) n'est jamais solide", "guide-physique.html#autotoc_md133", null ],
        [ "<span class=\"tt\">core::slopeSurfaceHeight</span> et <span class=\"tt\">core::resolveSlopeFollow</span>", "guide-physique.html#autotoc_md134", null ],
        [ "Le piège du mur adjacent (correction du balayage horizontal)", "guide-physique.html#autotoc_md135", null ],
        [ "Particularité de l'arrondi : tangente verticale à une extrémité", "guide-physique.html#autotoc_md136", null ]
      ] ],
      [ "3. Gravité et intégration", "guide-physique.html#autotoc_md137", [
        [ "Vitesse terminale newtonienne (EX-GP-019)", "guide-physique.html#autotoc_md138", null ]
      ] ],
      [ "4. Saut et <em>game feel</em>", "guide-physique.html#autotoc_md139", null ],
      [ "5. Dash 8 directions", "guide-physique.html#autotoc_md140", null ],
      [ "6. Wall jump et wall slide", "guide-physique.html#autotoc_md141", null ],
      [ "Ordre d'un pas (résumé)", "guide-physique.html#autotoc_md142", null ],
      [ "Voir aussi", "guide-physique.html#autotoc_md143", null ]
    ] ],
    [ "Niveaux : modèle, chargement, mécanismes, budgets", "guide-niveaux.html", [
      [ "Le modèle en mémoire", "guide-niveaux.html#autotoc_md110", [
        [ "Deux systèmes de coordonnées à ne pas confondre", "guide-niveaux.html#autotoc_md111", null ],
        [ "<a class=\"el\" href=\"namespacecore.html#ace99a1be913e6294e42e9ebb145eb875\" title=\"core::TileType\">core::TileType</a> : le vocabulaire des cases", "guide-niveaux.html#autotoc_md112", null ],
        [ "<a class=\"el\" href=\"classcore_1_1TileMap.html\" title=\"core::TileMap\">core::TileMap</a> : la grille", "guide-niveaux.html#autotoc_md113", null ],
        [ "<a class=\"el\" href=\"classcore_1_1Level.html\" title=\"core::Level\">core::Level</a> : le niveau assemblé", "guide-niveaux.html#autotoc_md114", null ]
      ] ],
      [ "Chargement JSON", "guide-niveaux.html#autotoc_md115", [
        [ "Exemple concret", "guide-niveaux.html#autotoc_md116", null ],
        [ "Validation", "guide-niveaux.html#autotoc_md117", null ]
      ] ],
      [ "De la grille aux entités : <a class=\"el\" href=\"namespacecore.html#acb711314315ff134ed9c25c09b4cebe4\" title=\"core::buildLevelScene\">buildLevelScene</a>", "guide-niveaux.html#autotoc_md118", null ],
      [ "Mécanismes déclencheur ↔ porte", "guide-niveaux.html#autotoc_md119", null ],
      [ "Blocs poussables", "guide-niveaux.html#autotoc_md120", [
        [ "Blocs à taille réduite (<span class=\"tt\">×0.5</span>/<span class=\"tt\">×0.25</span>)", "guide-niveaux.html#autotoc_md121", null ]
      ] ],
      [ "Budget de mouvements", "guide-niveaux.html#autotoc_md122", null ],
      [ "Dangers avancés (<span class=\"tt\">LOT-31</span>)", "guide-niveaux.html#autotoc_md123", null ],
      [ "Issue et enchaînement", "guide-niveaux.html#autotoc_md124", null ],
      [ "Voir aussi", "guide-niveaux.html#autotoc_md125", null ]
    ] ],
    [ "Entrées et actions logiques", "guide-entrees.html", [
      [ "Le principe : ne jamais coder « en dur » une touche dans le gameplay", "guide-entrees.html#autotoc_md71", null ],
      [ "Échantillonner plutôt que réagir : <a class=\"el\" href=\"classhmi_1_1InputState.html\" title=\"hmi::InputState\">hmi::InputState</a>", "guide-entrees.html#autotoc_md72", [
        [ "Détecter les fronts, pas seulement l'état", "guide-entrees.html#autotoc_md73", null ],
        [ "Le cycle d'une frame", "guide-entrees.html#autotoc_md74", null ],
        [ "Le pont Qt → <a class=\"el\" href=\"namespacehmi.html#a83c98e56e30eafa606048853f7962e48\" title=\"hmi::Key\">Key</a> : <a class=\"el\" href=\"namespacehmi.html#a1b53de392afb0dfdf5cdef4543fb672d\" title=\"hmi::qtKeyToHmiKey\">qtKeyToHmiKey</a>", "guide-entrees.html#autotoc_md75", null ]
      ] ],
      [ "Traduire l'état en intention : <a class=\"el\" href=\"namespacehmi.html#a2c3341857c60f0bef0a01a6cd10ce5f7\" title=\"hmi::toPlayerInput\">hmi::toPlayerInput</a>", "guide-entrees.html#autotoc_md76", null ],
      [ "La manette : une seconde source, fusionnée en lecture (EX-CTRL-002, LOT-20)", "guide-entrees.html#autotoc_md77", null ],
      [ "Le menu d'options : <a class=\"el\" href=\"classhmi_1_1OptionsPage.html\" title=\"hmi::OptionsPage\">hmi::OptionsPage</a>", "guide-entrees.html#autotoc_md78", null ],
      [ "Remapper les touches et boutons : <a class=\"el\" href=\"classhmi_1_1GameKeyBindings.html\" title=\"hmi::GameKeyBindings\">GameKeyBindings</a>/<a class=\"el\" href=\"classhmi_1_1EditorKeyBindings.html\" title=\"hmi::EditorKeyBindings\">EditorKeyBindings</a> (LOT-29), <a class=\"el\" href=\"classhmi_1_1GamepadBindings.html\" title=\"hmi::GamepadBindings\">GamepadBindings</a> (LOT-30)", "guide-entrees.html#autotoc_md79", null ],
      [ "La langue de l'interface : <a class=\"el\" href=\"classhmi_1_1Localization.html\" title=\"hmi::Localization\">hmi::Localization</a>", "guide-entrees.html#autotoc_md80", null ],
      [ "Voir aussi", "guide-entrees.html#autotoc_md81", null ]
    ] ],
    [ "Rendu 2D : de l'ECS à l'écran", "guide-rendu.html", [
      [ "Vocabulaire de base : GPU, swap chain, back buffer", "guide-rendu.html#autotoc_md144", null ],
      [ "<a class=\"el\" href=\"classhmi_1_1GraphicsDevice.html\" title=\"hmi::GraphicsDevice\">hmi::GraphicsDevice</a> : initialiser Direct3D 11 et présenter l'image", "guide-rendu.html#autotoc_md145", null ],
      [ "La surface de dessin : le viewport Qt (<span class=\"tt\">hmi::GameViewport</span>)", "guide-rendu.html#autotoc_md146", null ],
      [ "Unités monde et pixels : <a class=\"el\" href=\"classhmi_1_1Camera2D.html\" title=\"hmi::Camera2D\">hmi::Camera2D</a>", "guide-rendu.html#autotoc_md147", [
        [ "Cadrer un contenu plus grand que la fenêtre : <span class=\"tt\">fitZoom</span> et <span class=\"tt\">hmi::RoomGrid</span>", "guide-rendu.html#autotoc_md148", null ]
      ] ],
      [ "Le pipeline de dessin de sprites : <a class=\"el\" href=\"classhmi_1_1SpriteBatch.html\" title=\"hmi::SpriteBatch\">hmi::SpriteBatch</a>", "guide-rendu.html#autotoc_md149", [
        [ "Pourquoi « batcher » plutôt que dessiner un sprite à la fois", "guide-rendu.html#autotoc_md150", null ],
        [ "<a class=\"el\" href=\"structhmi_1_1SpriteQuad.html\" title=\"hmi::SpriteQuad\">SpriteQuad</a> : un rectangle texturé", "guide-rendu.html#autotoc_md151", null ],
        [ "Sommets, shaders, et échantillonnage <em>nearest</em>", "guide-rendu.html#autotoc_md152", null ],
        [ "<a class=\"el\" href=\"structhmi_1_1LineQuad.html\" title=\"hmi::LineQuad\">LineQuad</a> : un segment orienté (liens de mécanismes, <span class=\"tt\">LOT-37</span>)", "guide-rendu.html#autotoc_md153", null ]
      ] ],
      [ "<a class=\"el\" href=\"classhmi_1_1TextureAtlas.html\" title=\"hmi::TextureAtlas\">hmi::TextureAtlas</a> : un spritesheet, chargé depuis un fichier", "guide-rendu.html#autotoc_md154", [
        [ "Le pipeline de textures depuis fichiers, et son repli procédural", "guide-rendu.html#autotoc_md155", null ],
        [ "<span class=\"tt\">Source/Elements/Assets/</span> : convention et régénération", "guide-rendu.html#autotoc_md156", null ],
        [ "Les images du personnage : pourquoi elles vivent dans le même atlas", "guide-rendu.html#autotoc_md157", null ],
        [ "L'animation : une projection de l'état physique, pas un état séparé", "guide-rendu.html#autotoc_md158", null ]
      ] ],
      [ "<a class=\"el\" href=\"classhmi_1_1SpriteRenderer.html\" title=\"hmi::SpriteRenderer\">hmi::SpriteRenderer</a> : le pont ECS → écran", "guide-rendu.html#autotoc_md159", [
        [ "Ne dessiner que ce qui se voit : le culling (<span class=\"tt\">LOT-40</span>)", "guide-rendu.html#autotoc_md160", null ],
        [ "Deux modes de rendu : Physique et Texture (<span class=\"tt\">LOT-41</span>)", "guide-rendu.html#autotoc_md161", null ],
        [ "Interpoler le mouvement : <span class=\"tt\">hmi::PreviousPosition</span> et le facteur d'interpolation", "guide-rendu.html#autotoc_md162", null ]
      ] ],
      [ "Le texte d'interface : côté Qt, plus dans le pipeline Direct3D", "guide-rendu.html#autotoc_md163", null ],
      [ "Assembler la frame complète", "guide-rendu.html#autotoc_md164", null ],
      [ "Ce qui vient ensuite : le programme d'habillage (<span class=\"tt\">LOT-40</span> → <span class=\"tt\">LOT-55</span>)", "guide-rendu.html#autotoc_md165", null ],
      [ "Voir aussi", "guide-rendu.html#autotoc_md166", null ]
    ] ],
    [ "Journalisation et assertions", "guide-journalisation.html", [
      [ "Pourquoi journaliser dans un jeu vidéo", "guide-journalisation.html#autotoc_md91", null ],
      [ "Les niveaux de gravité : <a class=\"el\" href=\"namespacecore.html#aa9b5a444ee11933c91d5e3235aa5b5e3\" title=\"core::LogLevel\">core::LogLevel</a>", "guide-journalisation.html#autotoc_md92", null ],
      [ "<a class=\"el\" href=\"classcore_1_1Logger.html\" title=\"core::Logger\">core::Logger</a> : filtrer puis diffuser", "guide-journalisation.html#autotoc_md93", null ],
      [ "Les sinks : où finissent les messages", "guide-journalisation.html#autotoc_md94", null ],
      [ "Les macros de journalisation, par catégorie", "guide-journalisation.html#autotoc_md95", [
        [ "Chaque module a sa propre catégorie", "guide-journalisation.html#autotoc_md96", null ],
        [ "Une règle de performance à respecter", "guide-journalisation.html#autotoc_md97", null ]
      ] ],
      [ "Le format d'une ligne : <a class=\"el\" href=\"namespacecore.html#aafa85d91ee91c84b123fff01f147615e\" title=\"core::formatLogLine\">core::formatLogLine</a>", "guide-journalisation.html#autotoc_md98", null ],
      [ "Configurer le niveau minimal au lancement", "guide-journalisation.html#autotoc_md99", [
        [ "Bootstrap réel : sinks différents en développement et en Release", "guide-journalisation.html#autotoc_md100", null ]
      ] ],
      [ "Assertions : <a class=\"el\" href=\"Assert_8h.html#a24ec0bff1ac1e68bdf8ac50376a41076\" title=\"PROJECTGAMING_ASSERT\">PROJECTGAMING_ASSERT</a>, un outil différent", "guide-journalisation.html#autotoc_md101", null ],
      [ "Voir aussi", "guide-journalisation.html#autotoc_md102", null ]
    ] ],
    [ "Éditeur de niveaux intégré", "guide-editeur.html", [
      [ "Le problème : éditer un niveau sans (re)coder le moteur", "guide-editeur.html#autotoc_md55", null ],
      [ "<a class=\"el\" href=\"classcore_1_1LevelDraft.html\" title=\"core::LevelDraft\">core::LevelDraft</a> : un niveau qu'on peut défaire", "guide-editeur.html#autotoc_md56", [
        [ "Mécanismes : qui a le droit de se lier à qui", "guide-editeur.html#autotoc_md57", null ],
        [ "Lier des mécanismes dans l'éditeur (<span class=\"tt\">LOT-37</span>, <span class=\"tt\">EX-IHM-030</span>/<span class=\"tt\">EX-IHM-031</span>)", "guide-editeur.html#autotoc_md58", null ]
      ] ],
      [ "<a class=\"el\" href=\"classcore_1_1LevelWriter.html\" title=\"core::LevelWriter\">core::LevelWriter</a> : l'inverse du chargement, avec un piège", "guide-editeur.html#autotoc_md59", null ],
      [ "Peindre, c'est convertir un pixel en case", "guide-editeur.html#autotoc_md60", [
        [ "La palette et les outils : des panneaux Qt séparés du canevas", "guide-editeur.html#autotoc_md61", null ],
        [ "Trois outils, une même grille : <a class=\"el\" href=\"namespacehmi.html#a02048ad8ad69a87a10e8307ac2bd68dd\" title=\"hmi::EditorTool\">EditorTool</a>", "guide-editeur.html#autotoc_md62", null ],
        [ "Peindre par lot sans dupliquer la logique de peinture : <a class=\"el\" href=\"classcore_1_1LevelDraft.html#a7fb667e044067f5e9bf16f747d3f5f8e\" title=\"core::LevelDraft::paintRegion\">LevelDraft::paintRegion</a>", "guide-editeur.html#autotoc_md63", null ]
      ] ],
      [ "Annuler/refaire : pourquoi des instantanés complets", "guide-editeur.html#autotoc_md64", null ],
      [ "Essai immédiat : jouer sans quitter l'éditeur", "guide-editeur.html#autotoc_md65", null ],
      [ "Enregistrer : valider avant d'écrire, jamais l'inverse", "guide-editeur.html#autotoc_md66", null ],
      [ "Garde-fous contre la perte de travail", "guide-editeur.html#autotoc_md67", null ],
      [ "Cadrer un niveau plus grand que la fenêtre", "guide-editeur.html#autotoc_md68", null ],
      [ "Gérer ses fichiers de niveaux", "guide-editeur.html#autotoc_md69", null ],
      [ "Voir aussi", "guide-editeur.html#autotoc_md70", null ]
    ] ],
    [ "Écrans et navigation", "guide-ecrans.html", [
      [ "Le principe : des pages empilées, pilotées par signaux", "guide-ecrans.html#autotoc_md40", null ],
      [ "Qui déclenche les transitions : les signaux", "guide-ecrans.html#autotoc_md41", null ],
      [ "Le viewport partagé : éditeur <b>et</b> jeu", "guide-ecrans.html#autotoc_md42", null ],
      [ "La séquence de niveaux démo", "guide-ecrans.html#autotoc_md43", null ],
      [ "Où ça s'insère dans la boucle", "guide-ecrans.html#autotoc_md44", null ],
      [ "Voir aussi", "guide-ecrans.html#autotoc_md45", null ]
    ] ],
    [ "IHM Qt (refonte) — socle applicatif &amp; viewport Direct3D 11", "guide-ihm-qt.html", [
      [ "Pourquoi Qt", "guide-ihm-qt.html#autotoc_md82", null ],
      [ "Une seule cible : l'application Qt", "guide-ihm-qt.html#autotoc_md83", null ],
      [ "Le viewport : Direct3D 11 dans une fenêtre Qt", "guide-ihm-qt.html#autotoc_md84", null ],
      [ "La boucle : Qt pilote, le pas fixe est préservé", "guide-ihm-qt.html#autotoc_md85", null ],
      [ "Les entrées : événements Qt vers l'état partagé", "guide-ihm-qt.html#autotoc_md86", null ],
      [ "Jouer un niveau : <span class=\"tt\">hmi::GameSession</span> réutilisée", "guide-ihm-qt.html#autotoc_md87", null ],
      [ "Éditeur : docks, palette, peinture (LOT-35)", "guide-ihm-qt.html#autotoc_md88", [
        [ "Gestion des niveaux (LOT-36)", "guide-ihm-qt.html#autotoc_md89", null ]
      ] ],
      [ "Voir aussi", "guide-ihm-qt.html#autotoc_md90", null ]
    ] ]
];
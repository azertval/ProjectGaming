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
      [ "Comment lire ce guide", "guide.html#autotoc_md108", null ],
      [ "Architecture en deux couches", "guide.html#autotoc_md109", null ],
      [ "Plan du guide", "guide.html#autotoc_md110", null ],
      [ "Boucle de jeu et pas de temps fixe", "guide-boucle.html", [
        [ "Qu'est-ce qu'une boucle de jeu ?", "guide-boucle.html#autotoc_md4", null ],
        [ "Le piège du framerate variable", "guide-boucle.html#autotoc_md5", null ],
        [ "Le principe du pas de temps fixe", "guide-boucle.html#autotoc_md6", null ],
        [ "L'accumulateur : <tt>core::FixedTimestep</tt>", "guide-boucle.html#autotoc_md7", [
          [ "Exemple chiffré", "guide-boucle.html#autotoc_md8", null ],
          [ "La « spirale de la mort »", "guide-boucle.html#autotoc_md9", null ],
          [ "<tt>interpolationAlpha</tt>", "guide-boucle.html#autotoc_md10", null ]
        ] ],
        [ "Conséquence pratique pour tout le code de simulation", "guide-boucle.html#autotoc_md11", null ],
        [ "Voir aussi", "guide-boucle.html#autotoc_md12", null ]
      ] ],
      [ "ECS : entités, composants, systèmes", "guide-ecs.html", [
        [ "Le problème que l'ECS résout", "guide-ecs.html#autotoc_md13", null ],
        [ "L'entité : <tt>core::Entity</tt>", "guide-ecs.html#autotoc_md14", null ],
        [ "Le <tt>World</tt>", "guide-ecs.html#autotoc_md15", null ],
        [ "Le stockage : sparse set (<tt>core::ComponentPool<T></tt>)", "guide-ecs.html#autotoc_md16", [
          [ "Ajout et suppression : <em>swap-and-pop</em>", "guide-ecs.html#autotoc_md17", null ],
          [ "Exemple pas à pas", "guide-ecs.html#autotoc_md18", null ]
        ] ],
        [ "Les vues : <tt>core::View<Components...></tt>", "guide-ecs.html#autotoc_md19", null ],
        [ "Les systèmes et l'ordre d'exécution", "guide-ecs.html#autotoc_md20", null ],
        [ "Voir aussi", "guide-ecs.html#autotoc_md21", null ]
      ] ],
      [ "Mathématiques du moteur", "guide-maths.html", [
        [ "<tt>Vector2</tt> : un point ou une direction dans le monde", "guide-maths.html#autotoc_md60", [
          [ "<tt>lengthSquared</tt> : éviter la racine carrée", "guide-maths.html#autotoc_md61", null ],
          [ "Égalité approchée", "guide-maths.html#autotoc_md62", null ]
        ] ],
        [ "<tt>Aabb</tt> : la boîte englobante alignée aux axes", "guide-maths.html#autotoc_md63", null ],
        [ "Conventions d'unités et de repère", "guide-maths.html#autotoc_md64", null ],
        [ "Comparaison flottante : pourquoi <tt>==</tt> est dangereux", "guide-maths.html#autotoc_md65", null ],
        [ "Voir aussi", "guide-maths.html#autotoc_md66", null ]
      ] ],
      [ "Physique du personnage", "guide-physique.html", [
        [ "1. Collision par balayage continu (swept AABB)", "guide-physique.html#autotoc_md80", [
          [ "Le problème : le <em>tunneling</em>", "guide-physique.html#autotoc_md81", null ],
          [ "La solution : tester tout le trajet, pas seulement l'arrivée", "guide-physique.html#autotoc_md82", null ],
          [ "Méthode retenue : balayage <strong>par axe</strong> avec clamp direct", "guide-physique.html#autotoc_md83", null ],
          [ "Pourquoi caler directement plutôt que d'interpoler", "guide-physique.html#autotoc_md84", null ],
          [ "Lire le résultat : <tt>core::SweepResult</tt>", "guide-physique.html#autotoc_md85", null ]
        ] ],
        [ "2. Gravité et intégration", "guide-physique.html#autotoc_md86", [
          [ "Vitesse terminale newtonienne (<tt>EX-GP-019</tt>)", "guide-physique.html#autotoc_md87", null ]
        ] ],
        [ "3. Saut et <em>game feel</em>", "guide-physique.html#autotoc_md88", null ],
        [ "4. Dash 8 directions", "guide-physique.html#autotoc_md89", null ],
        [ "5. Wall jump et wall slide", "guide-physique.html#autotoc_md90", null ],
        [ "Ordre d'un pas (résumé)", "guide-physique.html#autotoc_md91", null ],
        [ "Voir aussi", "guide-physique.html#autotoc_md92", null ]
      ] ],
      [ "Niveaux : modèle, chargement, mécanismes, budgets", "guide-niveaux.html", [
        [ "Le modèle en mémoire", "guide-niveaux.html#autotoc_md67", [
          [ "Deux systèmes de coordonnées à ne pas confondre", "guide-niveaux.html#autotoc_md68", null ],
          [ "<tt>core::TileType</tt> : le vocabulaire des cases", "guide-niveaux.html#autotoc_md69", null ],
          [ "<tt>core::TileMap</tt> : la grille", "guide-niveaux.html#autotoc_md70", null ],
          [ "<tt>core::Level</tt> : le niveau assemblé", "guide-niveaux.html#autotoc_md71", null ]
        ] ],
        [ "Chargement JSON", "guide-niveaux.html#autotoc_md72", [
          [ "Exemple concret", "guide-niveaux.html#autotoc_md73", null ],
          [ "Validation", "guide-niveaux.html#autotoc_md74", null ]
        ] ],
        [ "De la grille aux entités : <tt>buildLevelScene</tt>", "guide-niveaux.html#autotoc_md75", null ],
        [ "Mécanismes déclencheur ↔ porte", "guide-niveaux.html#autotoc_md76", null ],
        [ "Budget de mouvements", "guide-niveaux.html#autotoc_md77", null ],
        [ "Issue et enchaînement", "guide-niveaux.html#autotoc_md78", null ],
        [ "Voir aussi", "guide-niveaux.html#autotoc_md79", null ]
      ] ],
      [ "Entrées et actions logiques", "guide-entrees.html", [
        [ "Le principe : ne jamais coder « en dur » une touche dans le gameplay", "guide-entrees.html#autotoc_md41", null ],
        [ "Échantillonner plutôt que réagir : <tt>hmi::InputState</tt>", "guide-entrees.html#autotoc_md42", [
          [ "Détecter les fronts, pas seulement l'état", "guide-entrees.html#autotoc_md43", null ],
          [ "Le cycle d'une frame", "guide-entrees.html#autotoc_md44", null ],
          [ "Un détail d'implémentation qui simplifie tout : <tt>Key</tt> réutilise les codes Win32", "guide-entrees.html#autotoc_md45", null ]
        ] ],
        [ "Traduire l'état en intention : <tt>hmi::toPlayerInput</tt>", "guide-entrees.html#autotoc_md46", null ],
        [ "Voir aussi", "guide-entrees.html#autotoc_md47", null ]
      ] ],
      [ "Rendu 2D : de l'ECS à l'écran", "guide-rendu.html", [
        [ "Vocabulaire de base : GPU, swap chain, back buffer", "guide-rendu.html#autotoc_md93", null ],
        [ "<tt>hmi::GraphicsDevice</tt> : initialiser Direct3D 11 et présenter l'image", "guide-rendu.html#autotoc_md94", null ],
        [ "<tt>hmi::Window</tt> : la fenêtre, prérequis du rendu", "guide-rendu.html#autotoc_md95", null ],
        [ "Unités monde et pixels : <tt>hmi::Camera2D</tt>", "guide-rendu.html#autotoc_md96", null ],
        [ "Le pipeline de dessin de sprites : <tt>hmi::SpriteBatch</tt>", "guide-rendu.html#autotoc_md97", [
          [ "Pourquoi « batcher » plutôt que dessiner un sprite à la fois", "guide-rendu.html#autotoc_md98", null ],
          [ "<tt>SpriteQuad</tt> : un rectangle texturé", "guide-rendu.html#autotoc_md99", null ],
          [ "Sommets, shaders, et échantillonnage <em>nearest</em>", "guide-rendu.html#autotoc_md100", null ]
        ] ],
        [ "<tt>hmi::TextureAtlas</tt> : un spritesheet, généré en code", "guide-rendu.html#autotoc_md101", [
          [ "Les images du personnage : pourquoi elles vivent dans le même atlas", "guide-rendu.html#autotoc_md102", null ],
          [ "L'animation : une projection de l'état physique, pas un état séparé", "guide-rendu.html#autotoc_md103", null ]
        ] ],
        [ "<tt>hmi::SpriteRenderer</tt> : le pont ECS → écran", "guide-rendu.html#autotoc_md104", null ],
        [ "<tt>hmi::BitmapFont</tt> : dessiner du texte", "guide-rendu.html#autotoc_md105", null ],
        [ "Assembler la frame complète", "guide-rendu.html#autotoc_md106", null ],
        [ "Voir aussi", "guide-rendu.html#autotoc_md107", null ]
      ] ],
      [ "Journalisation et assertions", "guide-journalisation.html", [
        [ "Pourquoi journaliser dans un jeu vidéo", "guide-journalisation.html#autotoc_md48", null ],
        [ "Les niveaux de gravité : <tt>core::LogLevel</tt>", "guide-journalisation.html#autotoc_md49", null ],
        [ "<tt>core::Logger</tt> : filtrer puis diffuser", "guide-journalisation.html#autotoc_md50", null ],
        [ "Les sinks : où finissent les messages", "guide-journalisation.html#autotoc_md51", null ],
        [ "Les macros de journalisation, par catégorie", "guide-journalisation.html#autotoc_md52", [
          [ "Chaque module a sa propre catégorie", "guide-journalisation.html#autotoc_md53", null ],
          [ "Une règle de performance à respecter", "guide-journalisation.html#autotoc_md54", null ]
        ] ],
        [ "Le format d'une ligne : <tt>core::formatLogLine</tt>", "guide-journalisation.html#autotoc_md55", null ],
        [ "Configurer le niveau minimal au lancement", "guide-journalisation.html#autotoc_md56", [
          [ "Bootstrap réel : sinks différents en développement et en Release", "guide-journalisation.html#autotoc_md57", null ]
        ] ],
        [ "Assertions : <tt>PROJECTGAMING_ASSERT</tt>, un outil différent", "guide-journalisation.html#autotoc_md58", null ],
        [ "Voir aussi", "guide-journalisation.html#autotoc_md59", null ]
      ] ],
      [ "Éditeur de niveaux intégré", "guide-editeur.html", [
        [ "Le problème : éditer un niveau sans (re)coder le moteur", "guide-editeur.html#autotoc_md22", null ],
        [ "<tt>core::LevelDraft</tt> : un niveau qu'on peut défaire", "guide-editeur.html#autotoc_md23", [
          [ "Mécanismes : qui a le droit de se lier à qui", "guide-editeur.html#autotoc_md24", null ]
        ] ],
        [ "<tt>core::LevelWriter</tt> : l'inverse du chargement, avec un piège", "guide-editeur.html#autotoc_md25", null ],
        [ "<tt>EditorScreen</tt> : peindre, c'est convertir un pixel en case", "guide-editeur.html#autotoc_md26", [
          [ "La palette : une simple colonne de rectangles cliquables", "guide-editeur.html#autotoc_md27", null ],
          [ "Un clic, plusieurs significations possibles", "guide-editeur.html#autotoc_md28", null ],
          [ "Trois outils, une même grille : <tt>EditorTool</tt>", "guide-editeur.html#autotoc_md29", null ],
          [ "Peindre par lot sans dupliquer la logique de peinture : <tt>LevelDraft::paintRegion</tt>", "guide-editeur.html#autotoc_md30", null ],
          [ "Lier deux tuiles sans dessiner de trait", "guide-editeur.html#autotoc_md31", null ]
        ] ],
        [ "Annuler/refaire : pourquoi des instantanés complets", "guide-editeur.html#autotoc_md32", null ],
        [ "Essai immédiat : jouer sans quitter l'éditeur", "guide-editeur.html#autotoc_md33", null ],
        [ "Enregistrer : valider avant d'écrire, jamais l'inverse", "guide-editeur.html#autotoc_md34", null ],
        [ "Garde-fous contre la perte de travail", "guide-editeur.html#autotoc_md35", null ],
        [ "Un champ de saisie de texte générique : nommer, renommer, redimensionner", "guide-editeur.html#autotoc_md36", null ],
        [ "Cadrer un niveau plus grand que la fenêtre", "guide-editeur.html#autotoc_md37", null ],
        [ "Un panneau plutôt que des bandes empilées", "guide-editeur.html#autotoc_md38", null ],
        [ "Choisir un niveau à éditer : <tt>hmi::LevelPicker</tt>", "guide-editeur.html#autotoc_md39", null ],
        [ "Voir aussi", "guide-editeur.html#autotoc_md40", null ]
      ] ]
    ] ],
    [ "Spécifications", "specifications.html", [
      [ "Documents", "specifications.html#autotoc_md189", null ],
      [ "Vision & périmètre", "spec-vision.html", [
        [ "Concept", "spec-vision.html#autotoc_md190", [
          [ "Mécanique de jeu (décidée)", "spec-vision.html#autotoc_md191", null ]
        ] ],
        [ "Boucle de gameplay", "spec-vision.html#autotoc_md192", null ],
        [ "Objectifs (MVP)", "spec-vision.html#autotoc_md193", null ],
        [ "Objectifs produit (au-delà du moteur)", "spec-vision.html#autotoc_md194", null ],
        [ "Hors périmètre (MVP)", "spec-vision.html#autotoc_md195", null ],
        [ "Traçabilité", "spec-vision.html#autotoc_md196", null ]
      ] ],
      [ "Gameplay", "spec-gameplay.html", [
        [ "1. Monde en tuiles", "spec-gameplay.html#autotoc_md170", null ],
        [ "2. Personnage & déplacement", "spec-gameplay.html#autotoc_md171", [
          [ "Mécaniques aériennes avancées (au-delà du MVP)", "spec-gameplay.html#autotoc_md172", null ],
          [ "Ressenti (game feel) — ⚠️ à affiner par tests", "spec-gameplay.html#autotoc_md173", null ]
        ] ],
        [ "3. Mécanismes de puzzle", "spec-gameplay.html#autotoc_md174", null ],
        [ "4. Conditions de fin de niveau", "spec-gameplay.html#autotoc_md175", null ],
        [ "5. États de jeu", "spec-gameplay.html#autotoc_md176", null ],
        [ "Traçabilité", "spec-gameplay.html#autotoc_md177", null ]
      ] ],
      [ "Contrôles & entrées", "spec-controles.html", [
        [ "1. Périphériques", "spec-controles.html#autotoc_md123", null ],
        [ "2. Actions du jeu (mapping logique)", "spec-controles.html#autotoc_md124", null ],
        [ "3. Réactivité", "spec-controles.html#autotoc_md125", null ],
        [ "Traçabilité", "spec-controles.html#autotoc_md126", null ]
      ] ],
      [ "Rendu & cible technique", "spec-rendu-technique.html", [
        [ "1. Cible technique", "spec-rendu-technique.html#autotoc_md183", null ],
        [ "2. Rendu 2D", "spec-rendu-technique.html#autotoc_md184", null ],
        [ "3. Boucle & temps", "spec-rendu-technique.html#autotoc_md185", null ],
        [ "4. Interface (HMI)", "spec-rendu-technique.html#autotoc_md186", null ],
        [ "5. Audio (⚠️ minimal MVP)", "spec-rendu-technique.html#autotoc_md187", null ],
        [ "Traçabilité", "spec-rendu-technique.html#autotoc_md188", null ]
      ] ],
      [ "Niveaux & contenu", "spec-niveaux.html", [
        [ "1. Représentation des niveaux", "spec-niveaux.html#autotoc_md178", [
          [ "Format retenu (JSON, liste de tuiles-objets)", "spec-niveaux.html#autotoc_md179", null ]
        ] ],
        [ "2. Progression", "spec-niveaux.html#autotoc_md180", null ],
        [ "3. Conception (lignes directrices)", "spec-niveaux.html#autotoc_md181", null ],
        [ "Traçabilité", "spec-niveaux.html#autotoc_md182", null ]
      ] ],
      [ "Exigences non fonctionnelles", "spec-exigences.html", [
        [ "1. Performance", "spec-exigences.html#autotoc_md164", null ],
        [ "2. Architecture & maintenabilité", "spec-exigences.html#autotoc_md165", null ],
        [ "3. Qualité & vérification", "spec-exigences.html#autotoc_md166", null ],
        [ "4. Portabilité & reproductibilité", "spec-exigences.html#autotoc_md167", null ],
        [ "5. Robustesse", "spec-exigences.html#autotoc_md168", null ],
        [ "Traçabilité", "spec-exigences.html#autotoc_md169", null ]
      ] ],
      [ "Éditeur de niveaux", "spec-editeur.html", [
        [ "Objectif", "spec-editeur.html#autotoc_md154", null ],
        [ "1. Exigences fonctionnelles", "spec-editeur.html#autotoc_md155", null ],
        [ "2. Réutilisation & cohérence", "spec-editeur.html#autotoc_md156", null ],
        [ "3. Distribution & collaboration", "spec-editeur.html#autotoc_md157", null ],
        [ "4. Approche d'implémentation (décidée)", "spec-editeur.html#autotoc_md158", null ],
        [ "4bis. Décors & pixel art (post-MVP, intégré à l'éditeur)", "spec-editeur.html#autotoc_md159", null ],
        [ "5. Non-objectifs (éditeur, MVP)", "spec-editeur.html#autotoc_md160", null ],
        [ "6. Robustesse et confort d'édition (LOT-15)", "spec-editeur.html#autotoc_md161", null ],
        [ "7. Niveaux de grande taille (LOT-16)", "spec-editeur.html#autotoc_md162", null ],
        [ "Traçabilité", "spec-editeur.html#autotoc_md163", null ]
      ] ],
      [ "Architecture (décisions dimensionnantes)", "spec-architecture.html", [
        [ "1. Modules & dépendances", "spec-architecture.html#autotoc_md111", null ],
        [ "2. Modèle d'entités : ECS", "spec-architecture.html#autotoc_md112", null ],
        [ "3. Coordonnées & unités — trois espaces distincts", "spec-architecture.html#autotoc_md113", null ],
        [ "4. Frontière simulation ↔ rendu", "spec-architecture.html#autotoc_md114", null ],
        [ "5. Mathématiques dans <tt>Core</tt>", "spec-architecture.html#autotoc_md115", null ],
        [ "6. Abstraction de rendu", "spec-architecture.html#autotoc_md116", null ],
        [ "7. Modèle de threading", "spec-architecture.html#autotoc_md117", null ],
        [ "8. Communication inter-systèmes", "spec-architecture.html#autotoc_md118", null ],
        [ "9. Gestion des ressources", "spec-architecture.html#autotoc_md119", null ],
        [ "10. Contrainte « éditeur intégré »", "spec-architecture.html#autotoc_md120", null ],
        [ "11. Décors dynamiques (accommodation dimensionnante)", "spec-architecture.html#autotoc_md121", null ],
        [ "Traçabilité", "spec-architecture.html#autotoc_md122", null ]
      ] ],
      [ "Décors & pipeline pixel art", "spec-decors.html", [
        [ "Vision", "spec-decors.html#autotoc_md146", null ],
        [ "1. Système de décors", "spec-decors.html#autotoc_md147", null ],
        [ "2. Manipulation", "spec-decors.html#autotoc_md148", [
          [ "À la conception (éditeur)", "spec-decors.html#autotoc_md149", null ],
          [ "En jeu (mécanique, à terme)", "spec-decors.html#autotoc_md150", null ]
        ] ],
        [ "3. Pipeline photo → pixel art (intégré à l'éditeur)", "spec-decors.html#autotoc_md151", null ],
        [ "4. Périmètre & séquencement", "spec-decors.html#autotoc_md152", null ],
        [ "Traçabilité", "spec-decors.html#autotoc_md153", null ]
      ] ],
      [ "Conventions de code", "spec-conventions.html", [
        [ "1. Langage & standard", "spec-conventions.html#autotoc_md128", null ],
        [ "2. Nommage", "spec-conventions.html#autotoc_md129", null ],
        [ "3. Mise en forme", "spec-conventions.html#autotoc_md130", null ],
        [ "4. Inclusions (<tt>#include</tt>)", "spec-conventions.html#autotoc_md131", [
          [ "Chemins complets depuis <tt>Source/</tt>", "spec-conventions.html#autotoc_md132", null ],
          [ "Ordre des groupes", "spec-conventions.html#autotoc_md133", null ]
        ] ],
        [ "5. Architecture (dépendances entre modules)", "spec-conventions.html#autotoc_md134", [
          [ "Classes plutôt que fonctions libres", "spec-conventions.html#autotoc_md135", null ],
          [ "RAII obligatoire", "spec-conventions.html#autotoc_md136", null ]
        ] ],
        [ "6. Documentation Doxygen", "spec-conventions.html#autotoc_md137", [
          [ "Doxygen dans le header, commentaires simples <tt>//</tt> dans le <tt>.cpp</tt>", "spec-conventions.html#autotoc_md138", null ],
          [ "Documentation du corps (<tt>.cpp</tt>)", "spec-conventions.html#autotoc_md139", null ]
        ] ],
        [ "7. Bonnes pratiques", "spec-conventions.html#autotoc_md140", null ],
        [ "8. Tests", "spec-conventions.html#autotoc_md141", null ],
        [ "9. Gestion des erreurs", "spec-conventions.html#autotoc_md142", null ],
        [ "10. Assertions & journalisation", "spec-conventions.html#autotoc_md143", null ],
        [ "11. Outillage qualité (automatisé)", "spec-conventions.html#autotoc_md144", null ],
        [ "12. Identifiants d'exigences (<tt>EX-…</tt>)", "spec-conventions.html#autotoc_md145", null ]
      ] ]
    ] ],
    [ "Lots", "lots.html", [
      [ "Lots", "lots.html#autotoc_md1062", null ],
      [ "LOT-01 — Fenêtre & boucle de jeu (Direct3D 11)", "lot-01.html", [
        [ "Objectif", "lot-01.html#autotoc_md197", null ],
        [ "Périmètre", "lot-01.html#autotoc_md198", [
          [ "Inclus", "lot-01.html#autotoc_md199", null ],
          [ "Exclus (lots ultérieurs)", "lot-01.html#autotoc_md200", null ]
        ] ],
        [ "Exigences couvertes", "lot-01.html#autotoc_md201", null ],
        [ "Découpage", "lot-01.html#autotoc_md202", null ],
        [ "Critères d'acceptation du lot", "lot-01.html#autotoc_md203", null ],
        [ "Navigation des tâches", "lot-01.html#autotoc_md204", null ],
        [ "TACHE-01 — Fenêtre Win32 & pompe de messages", "lot-01-tache-01-fenetre-win32.html", [
          [ "Contexte", "lot-01-tache-01-fenetre-win32.html#autotoc_md205", null ],
          [ "Travail à réaliser", "lot-01-tache-01-fenetre-win32.html#autotoc_md206", null ],
          [ "Fichiers impactés", "lot-01-tache-01-fenetre-win32.html#autotoc_md207", null ],
          [ "Points d'attention", "lot-01-tache-01-fenetre-win32.html#autotoc_md208", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-01-fenetre-win32.html#autotoc_md209", null ],
          [ "Exigences", "lot-01-tache-01-fenetre-win32.html#autotoc_md210", null ]
        ] ],
        [ "TACHE-02 — Initialisation Direct3D 11 (RAII)", "lot-01-tache-02-init-direct3d11.html", [
          [ "Contexte", "lot-01-tache-02-init-direct3d11.html#autotoc_md211", null ],
          [ "Travail à réaliser", "lot-01-tache-02-init-direct3d11.html#autotoc_md212", null ],
          [ "Fichiers impactés", "lot-01-tache-02-init-direct3d11.html#autotoc_md213", null ],
          [ "Points d'attention", "lot-01-tache-02-init-direct3d11.html#autotoc_md214", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-02-init-direct3d11.html#autotoc_md215", null ],
          [ "Exigences", "lot-01-tache-02-init-direct3d11.html#autotoc_md216", null ]
        ] ],
        [ "TACHE-03 — Boucle à pas de temps fixe (testable)", "lot-01-tache-03-boucle-pas-fixe.html", [
          [ "Contexte", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md217", null ],
          [ "Travail à réaliser", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md218", null ],
          [ "Fichiers impactés", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md219", null ],
          [ "Tests (obligatoires)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md220", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md221", null ],
          [ "Exigences", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md222", null ]
        ] ],
        [ "TACHE-04 — Effacement écran, présentation & redimensionnement", "lot-01-tache-04-effacement-presentation.html", [
          [ "Contexte", "lot-01-tache-04-effacement-presentation.html#autotoc_md223", null ],
          [ "Travail à réaliser", "lot-01-tache-04-effacement-presentation.html#autotoc_md224", null ],
          [ "Fichiers impactés", "lot-01-tache-04-effacement-presentation.html#autotoc_md225", null ],
          [ "Points d'attention", "lot-01-tache-04-effacement-presentation.html#autotoc_md226", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-04-effacement-presentation.html#autotoc_md227", null ],
          [ "Exigences", "lot-01-tache-04-effacement-presentation.html#autotoc_md228", null ]
        ] ],
        [ "TACHE-05 — Intégration `main` & vérification", "lot-01-tache-05-integration.html", [
          [ "Contexte", "lot-01-tache-05-integration.html#autotoc_md229", null ],
          [ "Travail à réaliser", "lot-01-tache-05-integration.html#autotoc_md230", null ],
          [ "Fichiers impactés", "lot-01-tache-05-integration.html#autotoc_md231", null ],
          [ "Vérification (manuelle + automatique)", "lot-01-tache-05-integration.html#autotoc_md232", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-05-integration.html#autotoc_md233", null ],
          [ "Exigences", "lot-01-tache-05-integration.html#autotoc_md234", null ]
        ] ]
      ] ],
      [ "LOT-02 — Journalisation & diagnostics", "lot-02.html", [
        [ "Objectif", "lot-02.html#autotoc_md235", null ],
        [ "Périmètre", "lot-02.html#autotoc_md236", [
          [ "Inclus", "lot-02.html#autotoc_md237", null ],
          [ "Exclus (plus tard)", "lot-02.html#autotoc_md238", null ]
        ] ],
        [ "Exigences couvertes", "lot-02.html#autotoc_md239", null ],
        [ "Découpage", "lot-02.html#autotoc_md240", null ],
        [ "Critères d'acceptation du lot", "lot-02.html#autotoc_md241", null ],
        [ "Dépendances", "lot-02.html#autotoc_md242", null ],
        [ "Navigation des tâches", "lot-02.html#autotoc_md243", null ],
        [ "TACHE-01 — Niveaux de log & interface `Logger`", "lot-02-tache-01-niveaux-logger.html", [
          [ "Contexte", "lot-02-tache-01-niveaux-logger.html#autotoc_md244", null ],
          [ "Travail à réaliser", "lot-02-tache-01-niveaux-logger.html#autotoc_md245", null ],
          [ "Fichiers impactés", "lot-02-tache-01-niveaux-logger.html#autotoc_md246", null ],
          [ "Tests (obligatoires)", "lot-02-tache-01-niveaux-logger.html#autotoc_md247", null ],
          [ "Points d'attention", "lot-02-tache-01-niveaux-logger.html#autotoc_md248", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-01-niveaux-logger.html#autotoc_md249", null ],
          [ "Exigences", "lot-02-tache-01-niveaux-logger.html#autotoc_md250", null ]
        ] ],
        [ "TACHE-02 — Sinks enfichables", "lot-02-tache-02-sinks.html", [
          [ "Contexte", "lot-02-tache-02-sinks.html#autotoc_md251", null ],
          [ "Travail à réaliser", "lot-02-tache-02-sinks.html#autotoc_md252", null ],
          [ "Fichiers impactés", "lot-02-tache-02-sinks.html#autotoc_md253", null ],
          [ "Tests (obligatoires)", "lot-02-tache-02-sinks.html#autotoc_md254", null ],
          [ "Points d'attention", "lot-02-tache-02-sinks.html#autotoc_md255", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-02-sinks.html#autotoc_md256", null ],
          [ "Exigences", "lot-02-tache-02-sinks.html#autotoc_md257", null ]
        ] ],
        [ "TACHE-03 — Macros de log (fichier/ligne, horodatage)", "lot-02-tache-03-macros-log.html", [
          [ "Contexte", "lot-02-tache-03-macros-log.html#autotoc_md258", null ],
          [ "Travail à réaliser", "lot-02-tache-03-macros-log.html#autotoc_md259", null ],
          [ "Fichiers impactés", "lot-02-tache-03-macros-log.html#autotoc_md260", null ],
          [ "Tests (obligatoires)", "lot-02-tache-03-macros-log.html#autotoc_md261", null ],
          [ "Points d'attention", "lot-02-tache-03-macros-log.html#autotoc_md262", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-03-macros-log.html#autotoc_md263", null ],
          [ "Exigences", "lot-02-tache-03-macros-log.html#autotoc_md264", null ]
        ] ],
        [ "TACHE-04 — Assertions `PROJECTGAMING_ASSERT`", "lot-02-tache-04-assertions.html", [
          [ "Contexte", "lot-02-tache-04-assertions.html#autotoc_md265", null ],
          [ "Travail à réaliser", "lot-02-tache-04-assertions.html#autotoc_md266", null ],
          [ "Fichiers impactés", "lot-02-tache-04-assertions.html#autotoc_md267", null ],
          [ "Tests (obligatoires)", "lot-02-tache-04-assertions.html#autotoc_md268", null ],
          [ "Points d'attention", "lot-02-tache-04-assertions.html#autotoc_md269", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-04-assertions.html#autotoc_md270", null ],
          [ "Exigences", "lot-02-tache-04-assertions.html#autotoc_md271", null ]
        ] ],
        [ "TACHE-05 — Intégration dans `main` & documentation", "lot-02-tache-05-integration.html", [
          [ "Contexte", "lot-02-tache-05-integration.html#autotoc_md272", null ],
          [ "Travail à réaliser", "lot-02-tache-05-integration.html#autotoc_md273", null ],
          [ "Fichiers impactés", "lot-02-tache-05-integration.html#autotoc_md274", null ],
          [ "Vérification", "lot-02-tache-05-integration.html#autotoc_md275", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-05-integration.html#autotoc_md276", null ],
          [ "Exigences", "lot-02-tache-05-integration.html#autotoc_md277", null ]
        ] ]
      ] ],
      [ "LOT-03 — Fondation ECS & mathématiques `Core`", "lot-03.html", [
        [ "Objectif", "lot-03.html#autotoc_md278", null ],
        [ "⚠️ Décision préalable : ECS maison vs bibliothèque", "lot-03.html#autotoc_md279", null ],
        [ "Périmètre", "lot-03.html#autotoc_md280", [
          [ "Inclus", "lot-03.html#autotoc_md281", null ],
          [ "Exclus (lots ultérieurs)", "lot-03.html#autotoc_md282", null ]
        ] ],
        [ "Exigences couvertes", "lot-03.html#autotoc_md283", null ],
        [ "Découpage", "lot-03.html#autotoc_md284", null ],
        [ "Critères d'acceptation du lot", "lot-03.html#autotoc_md285", null ],
        [ "Dépendances", "lot-03.html#autotoc_md286", null ],
        [ "Navigation des tâches", "lot-03.html#autotoc_md287", null ],
        [ "TACHE-01 — Types mathématiques de `Core`", "lot-03-tache-01-math-core.html", [
          [ "Contexte", "lot-03-tache-01-math-core.html#autotoc_md288", null ],
          [ "Travail à réaliser", "lot-03-tache-01-math-core.html#autotoc_md289", null ],
          [ "Fichiers impactés", "lot-03-tache-01-math-core.html#autotoc_md290", null ],
          [ "Tests (obligatoires)", "lot-03-tache-01-math-core.html#autotoc_md291", null ],
          [ "Points d'attention", "lot-03-tache-01-math-core.html#autotoc_md292", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-01-math-core.html#autotoc_md293", null ],
          [ "Exigences", "lot-03-tache-01-math-core.html#autotoc_md294", null ]
        ] ],
        [ "TACHE-02 — Entités : handles générationnels & cycle de vie", "lot-03-tache-02-entites.html", [
          [ "Contexte", "lot-03-tache-02-entites.html#autotoc_md295", null ],
          [ "Travail à réaliser", "lot-03-tache-02-entites.html#autotoc_md296", null ],
          [ "Fichiers impactés", "lot-03-tache-02-entites.html#autotoc_md297", null ],
          [ "Tests (obligatoires)", "lot-03-tache-02-entites.html#autotoc_md298", null ],
          [ "Points d'attention", "lot-03-tache-02-entites.html#autotoc_md299", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-02-entites.html#autotoc_md300", null ],
          [ "Exigences", "lot-03-tache-02-entites.html#autotoc_md301", null ]
        ] ],
        [ "TACHE-03 — Stockage de composants (sparse set typé)", "lot-03-tache-03-stockage-composants.html", [
          [ "Contexte", "lot-03-tache-03-stockage-composants.html#autotoc_md302", null ],
          [ "Travail à réaliser", "lot-03-tache-03-stockage-composants.html#autotoc_md303", null ],
          [ "Fichiers impactés", "lot-03-tache-03-stockage-composants.html#autotoc_md304", null ],
          [ "Tests (obligatoires)", "lot-03-tache-03-stockage-composants.html#autotoc_md305", null ],
          [ "Points d'attention", "lot-03-tache-03-stockage-composants.html#autotoc_md306", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-03-stockage-composants.html#autotoc_md307", null ],
          [ "Exigences", "lot-03-tache-03-stockage-composants.html#autotoc_md308", null ]
        ] ],
        [ "TACHE-04 — Requêtes / vues multi-composants", "lot-03-tache-04-vues-requetes.html", [
          [ "Contexte", "lot-03-tache-04-vues-requetes.html#autotoc_md309", null ],
          [ "Travail à réaliser", "lot-03-tache-04-vues-requetes.html#autotoc_md310", null ],
          [ "Fichiers impactés", "lot-03-tache-04-vues-requetes.html#autotoc_md311", null ],
          [ "Tests (obligatoires)", "lot-03-tache-04-vues-requetes.html#autotoc_md312", null ],
          [ "Points d'attention", "lot-03-tache-04-vues-requetes.html#autotoc_md313", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-04-vues-requetes.html#autotoc_md314", null ],
          [ "Exigences", "lot-03-tache-04-vues-requetes.html#autotoc_md315", null ]
        ] ],
        [ "TACHE-05 — Systèmes & `World` (orchestration au pas fixe)", "lot-03-tache-05-systemes-world.html", [
          [ "Contexte", "lot-03-tache-05-systemes-world.html#autotoc_md316", null ],
          [ "Travail à réaliser", "lot-03-tache-05-systemes-world.html#autotoc_md317", null ],
          [ "Fichiers impactés", "lot-03-tache-05-systemes-world.html#autotoc_md318", null ],
          [ "Tests (obligatoires)", "lot-03-tache-05-systemes-world.html#autotoc_md319", null ],
          [ "Points d'attention", "lot-03-tache-05-systemes-world.html#autotoc_md320", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-05-systemes-world.html#autotoc_md321", null ],
          [ "Exigences", "lot-03-tache-05-systemes-world.html#autotoc_md322", null ]
        ] ],
        [ "TACHE-06 — Composant `Transform` + système de mouvement (démo)", "lot-03-tache-06-transform-demo.html", [
          [ "Contexte", "lot-03-tache-06-transform-demo.html#autotoc_md323", null ],
          [ "Travail à réaliser", "lot-03-tache-06-transform-demo.html#autotoc_md324", null ],
          [ "Fichiers impactés", "lot-03-tache-06-transform-demo.html#autotoc_md325", null ],
          [ "Tests (obligatoires)", "lot-03-tache-06-transform-demo.html#autotoc_md326", null ],
          [ "Points d'attention", "lot-03-tache-06-transform-demo.html#autotoc_md327", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-06-transform-demo.html#autotoc_md328", null ],
          [ "Exigences", "lot-03-tache-06-transform-demo.html#autotoc_md329", null ]
        ] ]
      ] ],
      [ "LOT-04 — Documentation Doxygen & réorganisation de l'arborescence documentaire", "lot-04.html", [
        [ "Objectif", "lot-04.html#autotoc_md330", null ],
        [ "Périmètre", "lot-04.html#autotoc_md331", [
          [ "Inclus", "lot-04.html#autotoc_md332", null ],
          [ "Exclus (lots ultérieurs)", "lot-04.html#autotoc_md333", null ]
        ] ],
        [ "Décisions de cadrage", "lot-04.html#autotoc_md334", null ],
        [ "Exigences couvertes", "lot-04.html#autotoc_md335", null ],
        [ "Découpage", "lot-04.html#autotoc_md336", null ],
        [ "Critères d'acceptation du lot", "lot-04.html#autotoc_md337", null ],
        [ "Dépendances", "lot-04.html#autotoc_md338", null ],
        [ "Navigation des tâches", "lot-04.html#autotoc_md339", null ],
        [ "TACHE-01 — Réorganisation de l'arborescence documentaire", "lot-04-tache-01-reorganisation-arbo.html", [
          [ "Contexte", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md340", null ],
          [ "Travail à réaliser", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md341", null ],
          [ "Fichiers impactés", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md342", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md343", null ],
          [ "Points d'attention", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md344", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md345", null ],
          [ "Exigences", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md346", null ]
        ] ],
        [ "TACHE-02 — Configuration Doxygen pour le Markdown", "lot-04-tache-02-config-doxygen-markdown.html", [
          [ "Contexte", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md347", null ],
          [ "Travail à réaliser", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md348", null ],
          [ "Fichiers impactés", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md349", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md350", null ],
          [ "Points d'attention", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md351", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md352", null ],
          [ "Exigences", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md353", null ]
        ] ],
        [ "TACHE-03 — Page d'accueil du projet (mainpage)", "lot-04-tache-03-page-accueil.html", [
          [ "Contexte", "lot-04-tache-03-page-accueil.html#autotoc_md354", null ],
          [ "Travail à réaliser", "lot-04-tache-03-page-accueil.html#autotoc_md355", null ],
          [ "Fichiers impactés", "lot-04-tache-03-page-accueil.html#autotoc_md356", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-03-page-accueil.html#autotoc_md357", null ],
          [ "Points d'attention", "lot-04-tache-03-page-accueil.html#autotoc_md358", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-03-page-accueil.html#autotoc_md359", null ],
          [ "Exigences", "lot-04-tache-03-page-accueil.html#autotoc_md360", null ]
        ] ],
        [ "TACHE-04 — Pages de spécification navigables (conventions incluses)", "lot-04-tache-04-pages-specification.html", [
          [ "Contexte", "lot-04-tache-04-pages-specification.html#autotoc_md361", null ],
          [ "Travail à réaliser", "lot-04-tache-04-pages-specification.html#autotoc_md362", null ],
          [ "Convention d'insertion d'une nouvelle spec (à documenter dans l'index)", "lot-04-tache-04-pages-specification.html#autotoc_md363", null ],
          [ "Fichiers impactés", "lot-04-tache-04-pages-specification.html#autotoc_md364", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-04-pages-specification.html#autotoc_md365", null ],
          [ "Points d'attention", "lot-04-tache-04-pages-specification.html#autotoc_md366", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-04-pages-specification.html#autotoc_md367", null ],
          [ "Exigences", "lot-04-tache-04-pages-specification.html#autotoc_md368", null ]
        ] ],
        [ "TACHE-05 — Pages de lots navigables", "lot-04-tache-05-pages-lots.html", [
          [ "Contexte", "lot-04-tache-05-pages-lots.html#autotoc_md369", null ],
          [ "Travail à réaliser", "lot-04-tache-05-pages-lots.html#autotoc_md370", null ],
          [ "Fichiers impactés", "lot-04-tache-05-pages-lots.html#autotoc_md371", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-05-pages-lots.html#autotoc_md372", null ],
          [ "Points d'attention", "lot-04-tache-05-pages-lots.html#autotoc_md373", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-05-pages-lots.html#autotoc_md374", null ],
          [ "Exigences", "lot-04-tache-05-pages-lots.html#autotoc_md375", null ]
        ] ],
        [ "TACHE-06 — Manuel utilisateur (squelette + première page)", "lot-04-tache-06-manuel-utilisateur.html", [
          [ "Contexte", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md376", null ],
          [ "Travail à réaliser", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md377", null ],
          [ "Fichiers impactés", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md378", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md379", null ],
          [ "Points d'attention", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md380", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md381", null ],
          [ "Exigences", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md382", null ]
        ] ],
        [ "TACHE-07 — CI documentation (WARN_AS_ERROR & déploiement)", "lot-04-tache-07-ci-docs.html", [
          [ "Contexte", "lot-04-tache-07-ci-docs.html#autotoc_md383", null ],
          [ "Travail à réaliser", "lot-04-tache-07-ci-docs.html#autotoc_md384", null ],
          [ "Fichiers impactés", "lot-04-tache-07-ci-docs.html#autotoc_md385", null ],
          [ "Avertissements connus à corriger avant <tt>WARN_AS_ERROR</tt> (relevés en TACHE-02)", "lot-04-tache-07-ci-docs.html#autotoc_md386", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-07-ci-docs.html#autotoc_md387", null ],
          [ "Points d'attention", "lot-04-tache-07-ci-docs.html#autotoc_md388", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-07-ci-docs.html#autotoc_md389", null ],
          [ "Exigences", "lot-04-tache-07-ci-docs.html#autotoc_md390", null ]
        ] ],
        [ "TACHE-08 — Traçabilité des exigences (IDs stables, ancres Doxygen, lint CI)", "lot-04-tache-08-tracabilite-exigences.html", [
          [ "Contexte", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md391", null ],
          [ "Règle à formaliser (dans <tt>conventions.md</tt>)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md392", null ],
          [ "Travail à réaliser", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md393", null ],
          [ "Fichiers impactés", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md394", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md395", null ],
          [ "Points d'attention", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md396", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md397", null ],
          [ "Exigences", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md398", null ]
        ] ]
      ] ],
      [ "LOT-05 — Rendu 2D : atlas, sprites & caméra", "lot-05.html", [
        [ "Objectif", "lot-05.html#autotoc_md399", null ],
        [ "Périmètre", "lot-05.html#autotoc_md400", [
          [ "Inclus", "lot-05.html#autotoc_md401", null ],
          [ "Exclus (lots ultérieurs)", "lot-05.html#autotoc_md402", null ]
        ] ],
        [ "Décisions de cadrage", "lot-05.html#autotoc_md403", null ],
        [ "Exigences couvertes", "lot-05.html#autotoc_md404", null ],
        [ "Découpage", "lot-05.html#autotoc_md405", null ],
        [ "Critères d'acceptation du lot", "lot-05.html#autotoc_md406", null ],
        [ "Dépendances", "lot-05.html#autotoc_md407", null ],
        [ "Navigation des tâches", "lot-05.html#autotoc_md408", null ],
        [ "TACHE-01 — Composant `Sprite` (données pures)", "lot-05-tache-01-composant-sprite.html", [
          [ "Contexte", "lot-05-tache-01-composant-sprite.html#autotoc_md409", null ],
          [ "Travail à réaliser", "lot-05-tache-01-composant-sprite.html#autotoc_md410", null ],
          [ "Fichiers impactés", "lot-05-tache-01-composant-sprite.html#autotoc_md411", null ],
          [ "Tests (obligatoires si logique)", "lot-05-tache-01-composant-sprite.html#autotoc_md412", null ],
          [ "Points d'attention", "lot-05-tache-01-composant-sprite.html#autotoc_md413", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-01-composant-sprite.html#autotoc_md414", null ],
          [ "Exigences", "lot-05-tache-01-composant-sprite.html#autotoc_md415", null ]
        ] ],
        [ "TACHE-02 — Pipeline de quads texturés (HLSL, blend, nearest)", "lot-05-tache-02-pipeline-quads-textures.html", [
          [ "Contexte", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md416", null ],
          [ "Travail à réaliser", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md417", null ],
          [ "Fichiers impactés", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md418", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md419", null ],
          [ "Points d'attention", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md420", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md421", null ],
          [ "Exigences", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md422", null ]
        ] ],
        [ "TACHE-03 — Atlas de textures procédural", "lot-05-tache-03-atlas-procedural.html", [
          [ "Contexte", "lot-05-tache-03-atlas-procedural.html#autotoc_md423", null ],
          [ "Travail à réaliser", "lot-05-tache-03-atlas-procedural.html#autotoc_md424", null ],
          [ "Fichiers impactés", "lot-05-tache-03-atlas-procedural.html#autotoc_md425", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-03-atlas-procedural.html#autotoc_md426", null ],
          [ "Points d'attention", "lot-05-tache-03-atlas-procedural.html#autotoc_md427", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-03-atlas-procedural.html#autotoc_md428", null ],
          [ "Exigences", "lot-05-tache-03-atlas-procedural.html#autotoc_md429", null ]
        ] ],
        [ "TACHE-04 — Caméra 2D (monde → écran)", "lot-05-tache-04-camera-2d.html", [
          [ "Contexte", "lot-05-tache-04-camera-2d.html#autotoc_md430", null ],
          [ "Travail à réaliser", "lot-05-tache-04-camera-2d.html#autotoc_md431", null ],
          [ "Fichiers impactés", "lot-05-tache-04-camera-2d.html#autotoc_md432", null ],
          [ "Tests (obligatoires)", "lot-05-tache-04-camera-2d.html#autotoc_md433", null ],
          [ "Points d'attention", "lot-05-tache-04-camera-2d.html#autotoc_md434", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-04-camera-2d.html#autotoc_md435", null ],
          [ "Exigences", "lot-05-tache-04-camera-2d.html#autotoc_md436", null ]
        ] ],
        [ "TACHE-05 — Système de rendu des sprites (ECS → écran)", "lot-05-tache-05-systeme-rendu-sprites.html", [
          [ "Contexte", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md437", null ],
          [ "Travail à réaliser", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md438", null ],
          [ "Fichiers impactés", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md439", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md440", null ],
          [ "Points d'attention", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md441", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md442", null ],
          [ "Exigences", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md443", null ]
        ] ],
        [ "TACHE-06 — Câblage du `World` dans la boucle + scène de démo", "lot-05-tache-06-cablage-world-demo.html", [
          [ "Contexte", "lot-05-tache-06-cablage-world-demo.html#autotoc_md444", null ],
          [ "Travail à réaliser", "lot-05-tache-06-cablage-world-demo.html#autotoc_md445", null ],
          [ "Fichiers impactés", "lot-05-tache-06-cablage-world-demo.html#autotoc_md446", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md447", null ],
          [ "Points d'attention", "lot-05-tache-06-cablage-world-demo.html#autotoc_md448", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md449", null ],
          [ "Exigences", "lot-05-tache-06-cablage-world-demo.html#autotoc_md450", null ]
        ] ]
      ] ],
      [ "LOT-06 — Menu principal", "lot-06.html", [
        [ "Objectif", "lot-06.html#autotoc_md451", null ],
        [ "Périmètre", "lot-06.html#autotoc_md452", [
          [ "Inclus", "lot-06.html#autotoc_md453", null ],
          [ "Exclus (lots ultérieurs)", "lot-06.html#autotoc_md454", null ]
        ] ],
        [ "Décisions de cadrage", "lot-06.html#autotoc_md455", null ],
        [ "Exigences couvertes", "lot-06.html#autotoc_md456", null ],
        [ "Découpage", "lot-06.html#autotoc_md457", null ],
        [ "Critères d'acceptation du lot", "lot-06.html#autotoc_md458", null ],
        [ "Dépendances", "lot-06.html#autotoc_md459", null ],
        [ "Navigation des tâches", "lot-06.html#autotoc_md460", null ],
        [ "TACHE-01 — Entrées clavier & souris", "lot-06-tache-01-entrees-clavier-souris.html", [
          [ "Contexte", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md461", null ],
          [ "Travail à réaliser", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md462", null ],
          [ "Fichiers impactés", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md463", null ],
          [ "Tests (obligatoires)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md464", null ],
          [ "Points d'attention", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md465", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md466", null ],
          [ "Exigences", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md467", null ]
        ] ],
        [ "TACHE-02 — Rendu de texte (police bitmap)", "lot-06-tache-02-rendu-texte-bitmap.html", [
          [ "Contexte", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md468", null ],
          [ "Travail à réaliser", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md469", null ],
          [ "Fichiers impactés", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md470", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md471", null ],
          [ "Points d'attention", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md472", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md473", null ],
          [ "Exigences", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md474", null ]
        ] ],
        [ "TACHE-03 — Catalogue de traduction (i18n)", "lot-06-tache-03-catalogue-traduction.html", [
          [ "Contexte", "lot-06-tache-03-catalogue-traduction.html#autotoc_md475", null ],
          [ "Travail à réaliser", "lot-06-tache-03-catalogue-traduction.html#autotoc_md476", null ],
          [ "Fichiers impactés", "lot-06-tache-03-catalogue-traduction.html#autotoc_md477", null ],
          [ "Tests (obligatoires)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md478", null ],
          [ "Points d'attention", "lot-06-tache-03-catalogue-traduction.html#autotoc_md479", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md480", null ],
          [ "Exigences", "lot-06-tache-03-catalogue-traduction.html#autotoc_md481", null ]
        ] ],
        [ "TACHE-04 — États d'application (écrans)", "lot-06-tache-04-etats-application.html", [
          [ "Contexte", "lot-06-tache-04-etats-application.html#autotoc_md482", null ],
          [ "Travail à réaliser", "lot-06-tache-04-etats-application.html#autotoc_md483", null ],
          [ "Fichiers impactés", "lot-06-tache-04-etats-application.html#autotoc_md484", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-04-etats-application.html#autotoc_md485", null ],
          [ "Points d'attention", "lot-06-tache-04-etats-application.html#autotoc_md486", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-04-etats-application.html#autotoc_md487", null ],
          [ "Exigences", "lot-06-tache-04-etats-application.html#autotoc_md488", null ]
        ] ],
        [ "TACHE-05 — Écran de menu principal", "lot-06-tache-05-ecran-menu-principal.html", [
          [ "Contexte", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md489", null ],
          [ "Travail à réaliser", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md490", null ],
          [ "Fichiers impactés", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md491", null ],
          [ "Tests (obligatoires)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md492", null ],
          [ "Points d'attention", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md493", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md494", null ],
          [ "Exigences", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md495", null ]
        ] ],
        [ "TACHE-06 — Écrans cibles (jeu démo + éditeur placeholder)", "lot-06-tache-06-ecrans-cibles.html", [
          [ "Contexte", "lot-06-tache-06-ecrans-cibles.html#autotoc_md496", null ],
          [ "Travail à réaliser", "lot-06-tache-06-ecrans-cibles.html#autotoc_md497", null ],
          [ "Fichiers impactés", "lot-06-tache-06-ecrans-cibles.html#autotoc_md498", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md499", null ],
          [ "Points d'attention", "lot-06-tache-06-ecrans-cibles.html#autotoc_md500", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md501", null ],
          [ "Exigences", "lot-06-tache-06-ecrans-cibles.html#autotoc_md502", null ]
        ] ],
        [ "TACHE-07 — Intégration `main` (boucle pilotée par l'écran)", "lot-06-tache-07-integration-main.html", [
          [ "Contexte", "lot-06-tache-07-integration-main.html#autotoc_md503", null ],
          [ "Travail à réaliser", "lot-06-tache-07-integration-main.html#autotoc_md504", null ],
          [ "Fichiers impactés", "lot-06-tache-07-integration-main.html#autotoc_md505", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-07-integration-main.html#autotoc_md506", null ],
          [ "Points d'attention", "lot-06-tache-07-integration-main.html#autotoc_md507", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-07-integration-main.html#autotoc_md508", null ],
          [ "Exigences", "lot-06-tache-07-integration-main.html#autotoc_md509", null ]
        ] ],
        [ "TACHE-08 — Sélecteur de langue", "lot-06-tache-08-selecteur-langue.html", [
          [ "Contexte", "lot-06-tache-08-selecteur-langue.html#autotoc_md510", null ],
          [ "Travail à réaliser", "lot-06-tache-08-selecteur-langue.html#autotoc_md511", null ],
          [ "Fichiers impactés", "lot-06-tache-08-selecteur-langue.html#autotoc_md512", null ],
          [ "Tests (obligatoires)", "lot-06-tache-08-selecteur-langue.html#autotoc_md513", null ],
          [ "Points d'attention", "lot-06-tache-08-selecteur-langue.html#autotoc_md514", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-08-selecteur-langue.html#autotoc_md515", null ],
          [ "Exigences", "lot-06-tache-08-selecteur-langue.html#autotoc_md516", null ]
        ] ]
      ] ],
      [ "LOT-07 — Niveaux : modèle et chargement", "lot-07.html", [
        [ "Objectif", "lot-07.html#autotoc_md517", null ],
        [ "Périmètre", "lot-07.html#autotoc_md518", [
          [ "Inclus", "lot-07.html#autotoc_md519", null ],
          [ "Exclus (lots ultérieurs)", "lot-07.html#autotoc_md520", null ]
        ] ],
        [ "Décisions de cadrage", "lot-07.html#autotoc_md521", null ],
        [ "Exigences couvertes", "lot-07.html#autotoc_md522", null ],
        [ "Découpage", "lot-07.html#autotoc_md523", null ],
        [ "Critères d'acceptation du lot", "lot-07.html#autotoc_md524", null ],
        [ "Dépendances", "lot-07.html#autotoc_md525", null ],
        [ "Navigation des tâches", "lot-07.html#autotoc_md526", null ],
        [ "TACHE-01 — Dépendance JSON (nlohmann/json épinglé)", "lot-07-tache-01-dependance-json.html", [
          [ "Contexte", "lot-07-tache-01-dependance-json.html#autotoc_md527", null ],
          [ "Travail à réaliser", "lot-07-tache-01-dependance-json.html#autotoc_md528", null ],
          [ "Fichiers impactés", "lot-07-tache-01-dependance-json.html#autotoc_md529", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-01-dependance-json.html#autotoc_md530", null ],
          [ "Points d'attention", "lot-07-tache-01-dependance-json.html#autotoc_md531", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-01-dependance-json.html#autotoc_md532", null ],
          [ "Exigences", "lot-07-tache-01-dependance-json.html#autotoc_md533", null ]
        ] ],
        [ "TACHE-02 — Modèle de tuiles et de niveau", "lot-07-tache-02-modele-niveau.html", [
          [ "Contexte", "lot-07-tache-02-modele-niveau.html#autotoc_md534", null ],
          [ "Travail à réaliser", "lot-07-tache-02-modele-niveau.html#autotoc_md535", null ],
          [ "Fichiers impactés", "lot-07-tache-02-modele-niveau.html#autotoc_md536", null ],
          [ "Tests (obligatoires)", "lot-07-tache-02-modele-niveau.html#autotoc_md537", null ],
          [ "Points d'attention", "lot-07-tache-02-modele-niveau.html#autotoc_md538", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-02-modele-niveau.html#autotoc_md539", null ],
          [ "Exigences", "lot-07-tache-02-modele-niveau.html#autotoc_md540", null ]
        ] ],
        [ "TACHE-03 — Chargement du niveau (JSON)", "lot-07-tache-03-chargement-json.html", [
          [ "Contexte", "lot-07-tache-03-chargement-json.html#autotoc_md541", null ],
          [ "Travail à réaliser", "lot-07-tache-03-chargement-json.html#autotoc_md542", null ],
          [ "Fichiers impactés", "lot-07-tache-03-chargement-json.html#autotoc_md543", null ],
          [ "Tests (obligatoires)", "lot-07-tache-03-chargement-json.html#autotoc_md544", null ],
          [ "Points d'attention", "lot-07-tache-03-chargement-json.html#autotoc_md545", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-03-chargement-json.html#autotoc_md546", null ],
          [ "Exigences", "lot-07-tache-03-chargement-json.html#autotoc_md547", null ]
        ] ],
        [ "TACHE-04 — Validation du niveau", "lot-07-tache-04-validation.html", [
          [ "Contexte", "lot-07-tache-04-validation.html#autotoc_md548", null ],
          [ "Travail à réaliser", "lot-07-tache-04-validation.html#autotoc_md549", null ],
          [ "Fichiers impactés", "lot-07-tache-04-validation.html#autotoc_md550", null ],
          [ "Tests (obligatoires)", "lot-07-tache-04-validation.html#autotoc_md551", null ],
          [ "Points d'attention", "lot-07-tache-04-validation.html#autotoc_md552", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-04-validation.html#autotoc_md553", null ],
          [ "Exigences", "lot-07-tache-04-validation.html#autotoc_md554", null ]
        ] ],
        [ "TACHE-05 — Niveau de démonstration", "lot-07-tache-05-niveau-demo.html", [
          [ "Contexte", "lot-07-tache-05-niveau-demo.html#autotoc_md555", null ],
          [ "Travail à réaliser", "lot-07-tache-05-niveau-demo.html#autotoc_md556", null ],
          [ "Fichiers impactés", "lot-07-tache-05-niveau-demo.html#autotoc_md557", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-05-niveau-demo.html#autotoc_md558", null ],
          [ "Points d'attention", "lot-07-tache-05-niveau-demo.html#autotoc_md559", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-05-niveau-demo.html#autotoc_md560", null ],
          [ "Exigences", "lot-07-tache-05-niveau-demo.html#autotoc_md561", null ]
        ] ],
        [ "TACHE-06 — Rendu du niveau + intégration « Charger niveau »", "lot-07-tache-06-rendu-integration.html", [
          [ "Contexte", "lot-07-tache-06-rendu-integration.html#autotoc_md562", null ],
          [ "Travail à réaliser", "lot-07-tache-06-rendu-integration.html#autotoc_md563", null ],
          [ "Fichiers impactés", "lot-07-tache-06-rendu-integration.html#autotoc_md564", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-06-rendu-integration.html#autotoc_md565", null ],
          [ "Points d'attention", "lot-07-tache-06-rendu-integration.html#autotoc_md566", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-06-rendu-integration.html#autotoc_md567", null ],
          [ "Exigences", "lot-07-tache-06-rendu-integration.html#autotoc_md568", null ]
        ] ]
      ] ],
      [ "LOT-08 — Gameplay personnage : déplacement, gravité et collisions", "lot-08.html", [
        [ "Objectif", "lot-08.html#autotoc_md569", null ],
        [ "Périmètre", "lot-08.html#autotoc_md570", [
          [ "Inclus", "lot-08.html#autotoc_md571", null ],
          [ "Exclus (lots ultérieurs)", "lot-08.html#autotoc_md572", null ]
        ] ],
        [ "Décisions de cadrage", "lot-08.html#autotoc_md573", null ],
        [ "Exigences couvertes", "lot-08.html#autotoc_md574", null ],
        [ "Découpage", "lot-08.html#autotoc_md575", null ],
        [ "Critères d'acceptation du lot", "lot-08.html#autotoc_md576", null ],
        [ "Dépendances", "lot-08.html#autotoc_md577", null ],
        [ "Navigation des tâches", "lot-08.html#autotoc_md578", null ],
        [ "TACHE-01 — Composants du personnage & intention d'entrée", "lot-08-tache-01-composants-personnage.html", [
          [ "Contexte", "lot-08-tache-01-composants-personnage.html#autotoc_md579", null ],
          [ "Travail à réaliser", "lot-08-tache-01-composants-personnage.html#autotoc_md580", null ],
          [ "Fichiers impactés", "lot-08-tache-01-composants-personnage.html#autotoc_md581", null ],
          [ "Tests (obligatoires)", "lot-08-tache-01-composants-personnage.html#autotoc_md582", null ],
          [ "Points d'attention", "lot-08-tache-01-composants-personnage.html#autotoc_md583", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-01-composants-personnage.html#autotoc_md584", null ],
          [ "Exigences", "lot-08-tache-01-composants-personnage.html#autotoc_md585", null ]
        ] ],
        [ "TACHE-02 — Balayage AABB contre la grille (géométrie pure)", "lot-08-tache-02-balayage-aabb.html", [
          [ "Contexte", "lot-08-tache-02-balayage-aabb.html#autotoc_md586", null ],
          [ "Travail à réaliser", "lot-08-tache-02-balayage-aabb.html#autotoc_md587", null ],
          [ "Fichiers impactés", "lot-08-tache-02-balayage-aabb.html#autotoc_md588", null ],
          [ "Tests (obligatoires)", "lot-08-tache-02-balayage-aabb.html#autotoc_md589", null ],
          [ "Points d'attention", "lot-08-tache-02-balayage-aabb.html#autotoc_md590", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-02-balayage-aabb.html#autotoc_md591", null ],
          [ "Exigences", "lot-08-tache-02-balayage-aabb.html#autotoc_md592", null ]
        ] ],
        [ "TACHE-03 — Physique du personnage (gravité + déplacement + collisions)", "lot-08-tache-03-physique-personnage.html", [
          [ "Contexte", "lot-08-tache-03-physique-personnage.html#autotoc_md593", null ],
          [ "Travail à réaliser", "lot-08-tache-03-physique-personnage.html#autotoc_md594", null ],
          [ "Fichiers impactés", "lot-08-tache-03-physique-personnage.html#autotoc_md595", null ],
          [ "Tests (obligatoires)", "lot-08-tache-03-physique-personnage.html#autotoc_md596", null ],
          [ "Points d'attention", "lot-08-tache-03-physique-personnage.html#autotoc_md597", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-03-physique-personnage.html#autotoc_md598", null ],
          [ "Exigences", "lot-08-tache-03-physique-personnage.html#autotoc_md599", null ]
        ] ],
        [ "TACHE-04 — Règles de fin de niveau (succès / échec)", "lot-08-tache-04-regles-fin-niveau.html", [
          [ "Contexte", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md600", null ],
          [ "Travail à réaliser", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md601", null ],
          [ "Fichiers impactés", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md602", null ],
          [ "Tests (obligatoires)", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md603", null ],
          [ "Points d'attention", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md604", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md605", null ],
          [ "Exigences", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md606", null ]
        ] ],
        [ "TACHE-05 — Actions logiques d'entrée (mapping touches → intention)", "lot-08-tache-05-actions-logiques.html", [
          [ "Contexte", "lot-08-tache-05-actions-logiques.html#autotoc_md607", null ],
          [ "Travail à réaliser", "lot-08-tache-05-actions-logiques.html#autotoc_md608", null ],
          [ "Fichiers impactés", "lot-08-tache-05-actions-logiques.html#autotoc_md609", null ],
          [ "Tests (obligatoires)", "lot-08-tache-05-actions-logiques.html#autotoc_md610", null ],
          [ "Points d'attention", "lot-08-tache-05-actions-logiques.html#autotoc_md611", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-05-actions-logiques.html#autotoc_md612", null ],
          [ "Exigences", "lot-08-tache-05-actions-logiques.html#autotoc_md613", null ]
        ] ],
        [ "TACHE-06 — Intégration jouable dans `GameScreen` (cadrage fixe, succès / échec)", "lot-08-tache-06-integration-jouable.html", [
          [ "Contexte", "lot-08-tache-06-integration-jouable.html#autotoc_md614", null ],
          [ "Travail à réaliser", "lot-08-tache-06-integration-jouable.html#autotoc_md615", null ],
          [ "Fichiers impactés", "lot-08-tache-06-integration-jouable.html#autotoc_md616", null ],
          [ "Vérification (visuelle, pas de test unitaire)", "lot-08-tache-06-integration-jouable.html#autotoc_md617", null ],
          [ "Points d'attention", "lot-08-tache-06-integration-jouable.html#autotoc_md618", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-06-integration-jouable.html#autotoc_md619", null ],
          [ "Exigences", "lot-08-tache-06-integration-jouable.html#autotoc_md620", null ]
        ] ]
      ] ],
      [ "LOT-09 — Saut, game feel et enchaînement de niveaux", "lot-09.html", [
        [ "Objectif", "lot-09.html#autotoc_md621", null ],
        [ "Périmètre", "lot-09.html#autotoc_md622", [
          [ "Inclus", "lot-09.html#autotoc_md623", null ],
          [ "Exclus (lots ultérieurs)", "lot-09.html#autotoc_md624", null ]
        ] ],
        [ "Décisions de cadrage", "lot-09.html#autotoc_md625", null ],
        [ "Exigences couvertes", "lot-09.html#autotoc_md626", null ],
        [ "Découpage", "lot-09.html#autotoc_md627", null ],
        [ "Critères d'acceptation du lot", "lot-09.html#autotoc_md628", null ],
        [ "Dépendances", "lot-09.html#autotoc_md629", null ],
        [ "Navigation des tâches", "lot-09.html#autotoc_md630", null ],
        [ "TACHE-01 — Données du saut : `PlayerInput`, `Player`, `PhysicsConfig`", "lot-09-tache-01-donnees-saut.html", [
          [ "Contexte", "lot-09-tache-01-donnees-saut.html#autotoc_md631", null ],
          [ "Travail à réaliser", "lot-09-tache-01-donnees-saut.html#autotoc_md632", null ],
          [ "Fichiers impactés", "lot-09-tache-01-donnees-saut.html#autotoc_md633", null ],
          [ "Tests (obligatoires)", "lot-09-tache-01-donnees-saut.html#autotoc_md634", null ],
          [ "Points d'attention", "lot-09-tache-01-donnees-saut.html#autotoc_md635", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-01-donnees-saut.html#autotoc_md636", null ],
          [ "Exigences", "lot-09-tache-01-donnees-saut.html#autotoc_md637", null ]
        ] ],
        [ "TACHE-02 — Mapping du saut (`Espace`/`W` → intention)", "lot-09-tache-02-mapping-saut.html", [
          [ "Contexte", "lot-09-tache-02-mapping-saut.html#autotoc_md638", null ],
          [ "Travail à réaliser", "lot-09-tache-02-mapping-saut.html#autotoc_md639", null ],
          [ "Fichiers impactés", "lot-09-tache-02-mapping-saut.html#autotoc_md640", null ],
          [ "Tests (obligatoires)", "lot-09-tache-02-mapping-saut.html#autotoc_md641", null ],
          [ "Points d'attention", "lot-09-tache-02-mapping-saut.html#autotoc_md642", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-02-mapping-saut.html#autotoc_md643", null ],
          [ "Exigences", "lot-09-tache-02-mapping-saut.html#autotoc_md644", null ]
        ] ],
        [ "TACHE-03 — Saut au sol + hauteur variable", "lot-09-tache-03-saut-hauteur-variable.html", [
          [ "Contexte", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md645", null ],
          [ "Travail à réaliser", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md646", null ],
          [ "Fichiers impactés", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md647", null ],
          [ "Tests (obligatoires)", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md648", null ],
          [ "Points d'attention", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md649", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md650", null ],
          [ "Exigences", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md651", null ]
        ] ],
        [ "TACHE-04 — Coyote time + jump buffering", "lot-09-tache-04-coyote-buffering.html", [
          [ "Contexte", "lot-09-tache-04-coyote-buffering.html#autotoc_md652", null ],
          [ "Travail à réaliser", "lot-09-tache-04-coyote-buffering.html#autotoc_md653", null ],
          [ "Fichiers impactés", "lot-09-tache-04-coyote-buffering.html#autotoc_md654", null ],
          [ "Tests (obligatoires)", "lot-09-tache-04-coyote-buffering.html#autotoc_md655", null ],
          [ "Points d'attention", "lot-09-tache-04-coyote-buffering.html#autotoc_md656", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-04-coyote-buffering.html#autotoc_md657", null ],
          [ "Exigences", "lot-09-tache-04-coyote-buffering.html#autotoc_md658", null ]
        ] ],
        [ "TACHE-05 — Enchaînement de niveaux (séquence, auto-avance, retour titre)", "lot-09-tache-05-enchainement-niveaux.html", [
          [ "Contexte", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md659", null ],
          [ "Travail à réaliser", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md660", null ],
          [ "Fichiers impactés", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md661", null ],
          [ "Vérification / tests", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md662", null ],
          [ "Points d'attention", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md663", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md664", null ],
          [ "Exigences", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md665", null ]
        ] ],
        [ "TACHE-06 — Niveaux de démo (séquence, dont saut requis) + preuve", "lot-09-tache-06-niveaux-demo.html", [
          [ "Contexte", "lot-09-tache-06-niveaux-demo.html#autotoc_md666", null ],
          [ "Travail à réaliser", "lot-09-tache-06-niveaux-demo.html#autotoc_md667", null ],
          [ "Fichiers impactés", "lot-09-tache-06-niveaux-demo.html#autotoc_md668", null ],
          [ "Tests (obligatoires)", "lot-09-tache-06-niveaux-demo.html#autotoc_md669", null ],
          [ "Points d'attention", "lot-09-tache-06-niveaux-demo.html#autotoc_md670", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-06-niveaux-demo.html#autotoc_md671", null ],
          [ "Exigences", "lot-09-tache-06-niveaux-demo.html#autotoc_md672", null ]
        ] ]
      ] ],
      [ "LOT-10 — Mécaniques aériennes avancées : double saut, wall jump, dash", "lot-10.html", [
        [ "Objectif", "lot-10.html#autotoc_md673", null ],
        [ "Périmètre", "lot-10.html#autotoc_md674", [
          [ "Inclus", "lot-10.html#autotoc_md675", null ],
          [ "Exclus (lots ultérieurs)", "lot-10.html#autotoc_md676", null ]
        ] ],
        [ "Décisions de cadrage", "lot-10.html#autotoc_md677", null ],
        [ "Exigences couvertes", "lot-10.html#autotoc_md678", null ],
        [ "Découpage", "lot-10.html#autotoc_md679", null ],
        [ "Critères d'acceptation du lot", "lot-10.html#autotoc_md680", null ],
        [ "Dépendances", "lot-10.html#autotoc_md681", null ],
        [ "Navigation des tâches", "lot-10.html#autotoc_md682", null ],
        [ "TACHE-01 — Données des mécaniques (`PlayerInput`, `Player`, `PhysicsConfig`)", "lot-10-tache-01-donnees.html", [
          [ "Contexte", "lot-10-tache-01-donnees.html#autotoc_md683", null ],
          [ "Travail à réaliser", "lot-10-tache-01-donnees.html#autotoc_md684", null ],
          [ "Fichiers impactés", "lot-10-tache-01-donnees.html#autotoc_md685", null ],
          [ "Tests (obligatoires)", "lot-10-tache-01-donnees.html#autotoc_md686", null ],
          [ "Points d'attention", "lot-10-tache-01-donnees.html#autotoc_md687", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-01-donnees.html#autotoc_md688", null ],
          [ "Exigences", "lot-10-tache-01-donnees.html#autotoc_md689", null ]
        ] ],
        [ "TACHE-02 — Mapping du dash + direction de visée / orientation", "lot-10-tache-02-mapping-dash.html", [
          [ "Contexte", "lot-10-tache-02-mapping-dash.html#autotoc_md690", null ],
          [ "Travail à réaliser", "lot-10-tache-02-mapping-dash.html#autotoc_md691", null ],
          [ "Fichiers impactés", "lot-10-tache-02-mapping-dash.html#autotoc_md692", null ],
          [ "Tests (obligatoires)", "lot-10-tache-02-mapping-dash.html#autotoc_md693", null ],
          [ "Points d'attention", "lot-10-tache-02-mapping-dash.html#autotoc_md694", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-02-mapping-dash.html#autotoc_md695", null ],
          [ "Exigences", "lot-10-tache-02-mapping-dash.html#autotoc_md696", null ]
        ] ],
        [ "TACHE-03 — Double saut (sauts aériens rechargés au sol)", "lot-10-tache-03-double-saut.html", [
          [ "Contexte", "lot-10-tache-03-double-saut.html#autotoc_md697", null ],
          [ "Travail à réaliser", "lot-10-tache-03-double-saut.html#autotoc_md698", null ],
          [ "Fichiers impactés", "lot-10-tache-03-double-saut.html#autotoc_md699", null ],
          [ "Tests (obligatoires)", "lot-10-tache-03-double-saut.html#autotoc_md700", null ],
          [ "Points d'attention", "lot-10-tache-03-double-saut.html#autotoc_md701", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-03-double-saut.html#autotoc_md702", null ],
          [ "Exigences", "lot-10-tache-03-double-saut.html#autotoc_md703", null ]
        ] ],
        [ "TACHE-04 — Wall jump + wall slide", "lot-10-tache-04-wall-jump.html", [
          [ "Contexte", "lot-10-tache-04-wall-jump.html#autotoc_md704", null ],
          [ "Travail à réaliser", "lot-10-tache-04-wall-jump.html#autotoc_md705", null ],
          [ "Fichiers impactés", "lot-10-tache-04-wall-jump.html#autotoc_md706", null ],
          [ "Tests (obligatoires)", "lot-10-tache-04-wall-jump.html#autotoc_md707", null ],
          [ "Points d'attention", "lot-10-tache-04-wall-jump.html#autotoc_md708", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-04-wall-jump.html#autotoc_md709", null ],
          [ "Exigences", "lot-10-tache-04-wall-jump.html#autotoc_md710", null ]
        ] ],
        [ "TACHE-05 — Dash 8 directions (burst, durée, recharge au sol)", "lot-10-tache-05-dash.html", [
          [ "Contexte", "lot-10-tache-05-dash.html#autotoc_md711", null ],
          [ "Travail à réaliser", "lot-10-tache-05-dash.html#autotoc_md712", null ],
          [ "Fichiers impactés", "lot-10-tache-05-dash.html#autotoc_md713", null ],
          [ "Tests (obligatoires)", "lot-10-tache-05-dash.html#autotoc_md714", null ],
          [ "Points d'attention", "lot-10-tache-05-dash.html#autotoc_md715", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-05-dash.html#autotoc_md716", null ],
          [ "Exigences", "lot-10-tache-05-dash.html#autotoc_md717", null ]
        ] ],
        [ "TACHE-06 — Niveau de démo « parkour » + preuve système", "lot-10-tache-06-niveau-parkour.html", [
          [ "Contexte", "lot-10-tache-06-niveau-parkour.html#autotoc_md718", null ],
          [ "Travail à réaliser", "lot-10-tache-06-niveau-parkour.html#autotoc_md719", null ],
          [ "Fichiers impactés", "lot-10-tache-06-niveau-parkour.html#autotoc_md720", null ],
          [ "Tests (obligatoires)", "lot-10-tache-06-niveau-parkour.html#autotoc_md721", null ],
          [ "Points d'attention", "lot-10-tache-06-niveau-parkour.html#autotoc_md722", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-06-niveau-parkour.html#autotoc_md723", null ],
          [ "Exigences", "lot-10-tache-06-niveau-parkour.html#autotoc_md724", null ]
        ] ]
      ] ],
      [ "LOT-11 — Ressenti avancé : personnage humanoïde, gravité asymétrique, finitions", "lot-11.html", [
        [ "Objectif", "lot-11.html#autotoc_md725", null ],
        [ "Périmètre", "lot-11.html#autotoc_md726", [
          [ "Inclus", "lot-11.html#autotoc_md727", null ],
          [ "Exclus (lots ultérieurs)", "lot-11.html#autotoc_md728", null ]
        ] ],
        [ "Décisions de cadrage", "lot-11.html#autotoc_md729", null ],
        [ "Exigences couvertes", "lot-11.html#autotoc_md730", null ],
        [ "Découpage", "lot-11.html#autotoc_md731", null ],
        [ "Critères d'acceptation du lot", "lot-11.html#autotoc_md732", null ],
        [ "Dépendances", "lot-11.html#autotoc_md733", null ],
        [ "Navigation des tâches", "lot-11.html#autotoc_md734", null ],
        [ "TACHE-01 — Données : réglages de *feel* + taille/placement du personnage", "lot-11-tache-01-donnees.html", [
          [ "Contexte", "lot-11-tache-01-donnees.html#autotoc_md735", null ],
          [ "Travail à réaliser", "lot-11-tache-01-donnees.html#autotoc_md736", null ],
          [ "Fichiers impactés", "lot-11-tache-01-donnees.html#autotoc_md737", null ],
          [ "Tests (obligatoires)", "lot-11-tache-01-donnees.html#autotoc_md738", null ],
          [ "Points d'attention", "lot-11-tache-01-donnees.html#autotoc_md739", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-01-donnees.html#autotoc_md740", null ],
          [ "Exigences", "lot-11-tache-01-donnees.html#autotoc_md741", null ]
        ] ],
        [ "TACHE-02 — Gravité asymétrique + apex hang + fast-fall", "lot-11-tache-02-gravite-asymetrique.html", [
          [ "Contexte", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md742", null ],
          [ "Travail à réaliser", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md743", null ],
          [ "Fichiers impactés", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md744", null ],
          [ "Tests (obligatoires)", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md745", null ],
          [ "Points d'attention", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md746", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md747", null ],
          [ "Exigences", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md748", null ]
        ] ],
        [ "TACHE-03 — Personnage humanoïde (spawn 0,4×0,8, sprite)", "lot-11-tache-03-personnage-humanoide.html", [
          [ "Contexte", "lot-11-tache-03-personnage-humanoide.html#autotoc_md749", null ],
          [ "Travail à réaliser", "lot-11-tache-03-personnage-humanoide.html#autotoc_md750", null ],
          [ "Fichiers impactés", "lot-11-tache-03-personnage-humanoide.html#autotoc_md751", null ],
          [ "Vérification (visuelle, pas de test unitaire — brique GPU)", "lot-11-tache-03-personnage-humanoide.html#autotoc_md752", null ],
          [ "Points d'attention", "lot-11-tache-03-personnage-humanoide.html#autotoc_md753", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-03-personnage-humanoide.html#autotoc_md754", null ],
          [ "Exigences", "lot-11-tache-03-personnage-humanoide.html#autotoc_md755", null ]
        ] ],
        [ "TACHE-04 — Rééquilibrage des niveaux + preuves à la vraie taille", "lot-11-tache-04-reequilibrage.html", [
          [ "Contexte", "lot-11-tache-04-reequilibrage.html#autotoc_md756", null ],
          [ "Travail à réaliser", "lot-11-tache-04-reequilibrage.html#autotoc_md757", null ],
          [ "Fichiers impactés", "lot-11-tache-04-reequilibrage.html#autotoc_md758", null ],
          [ "Tests (obligatoires)", "lot-11-tache-04-reequilibrage.html#autotoc_md759", null ],
          [ "Points d'attention", "lot-11-tache-04-reequilibrage.html#autotoc_md760", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-04-reequilibrage.html#autotoc_md761", null ],
          [ "Exigences", "lot-11-tache-04-reequilibrage.html#autotoc_md762", null ]
        ] ]
      ] ],
      [ "LOT-12 — Niveau puzzle : mécanismes interrupteur/porte + budget de mouvements", "lot-12.html", [
        [ "Objectif", "lot-12.html#autotoc_md763", null ],
        [ "Périmètre", "lot-12.html#autotoc_md764", [
          [ "Inclus", "lot-12.html#autotoc_md765", null ],
          [ "Exclus (lots ultérieurs)", "lot-12.html#autotoc_md766", null ]
        ] ],
        [ "Décisions de cadrage", "lot-12.html#autotoc_md767", null ],
        [ "Exigences couvertes", "lot-12.html#autotoc_md768", null ],
        [ "Découpage", "lot-12.html#autotoc_md769", null ],
        [ "Critères d'acceptation du lot", "lot-12.html#autotoc_md770", null ],
        [ "Dépendances", "lot-12.html#autotoc_md771", null ],
        [ "Navigation des tâches", "lot-12.html#autotoc_md772", null ],
        [ "TACHE-01 — Données : budget (`Player`, `Level`, `LevelLoader`)", "lot-12-tache-01-donnees.html", [
          [ "Contexte", "lot-12-tache-01-donnees.html#autotoc_md773", null ],
          [ "Travail à réaliser", "lot-12-tache-01-donnees.html#autotoc_md774", null ],
          [ "Fichiers impactés", "lot-12-tache-01-donnees.html#autotoc_md775", null ],
          [ "Tests (obligatoires)", "lot-12-tache-01-donnees.html#autotoc_md776", null ],
          [ "Points d'attention", "lot-12-tache-01-donnees.html#autotoc_md777", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-01-donnees.html#autotoc_md778", null ],
          [ "Exigences", "lot-12-tache-01-donnees.html#autotoc_md779", null ]
        ] ],
        [ "TACHE-02 — Mécanismes interrupteur/porte (`MechanismController`)", "lot-12-tache-02-mecanismes.html", [
          [ "Contexte", "lot-12-tache-02-mecanismes.html#autotoc_md780", null ],
          [ "Travail à réaliser", "lot-12-tache-02-mecanismes.html#autotoc_md781", null ],
          [ "Fichiers impactés", "lot-12-tache-02-mecanismes.html#autotoc_md782", null ],
          [ "Tests (obligatoires)", "lot-12-tache-02-mecanismes.html#autotoc_md783", null ],
          [ "Points d'attention", "lot-12-tache-02-mecanismes.html#autotoc_md784", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-02-mecanismes.html#autotoc_md785", null ],
          [ "Exigences", "lot-12-tache-02-mecanismes.html#autotoc_md786", null ]
        ] ],
        [ "TACHE-03 — Budget de sauts/dashs dans la physique", "lot-12-tache-03-budget.html", [
          [ "Contexte", "lot-12-tache-03-budget.html#autotoc_md787", null ],
          [ "Travail à réaliser", "lot-12-tache-03-budget.html#autotoc_md788", null ],
          [ "Fichiers impactés", "lot-12-tache-03-budget.html#autotoc_md789", null ],
          [ "Tests (obligatoires)", "lot-12-tache-03-budget.html#autotoc_md790", null ],
          [ "Points d'attention", "lot-12-tache-03-budget.html#autotoc_md791", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-03-budget.html#autotoc_md792", null ],
          [ "Exigences", "lot-12-tache-03-budget.html#autotoc_md793", null ]
        ] ],
        [ "TACHE-04 — Intégration `GameScreen` + niveau `demo4` + preuves", "lot-12-tache-04-integration-puzzle.html", [
          [ "Contexte", "lot-12-tache-04-integration-puzzle.html#autotoc_md794", null ],
          [ "Travail à réaliser", "lot-12-tache-04-integration-puzzle.html#autotoc_md795", null ],
          [ "Fichiers impactés", "lot-12-tache-04-integration-puzzle.html#autotoc_md796", null ],
          [ "Tests (obligatoires)", "lot-12-tache-04-integration-puzzle.html#autotoc_md797", null ],
          [ "Points d'attention", "lot-12-tache-04-integration-puzzle.html#autotoc_md798", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-04-integration-puzzle.html#autotoc_md799", null ],
          [ "Exigences", "lot-12-tache-04-integration-puzzle.html#autotoc_md800", null ]
        ] ]
      ] ],
      [ "LOT-13 — Consolidation de la documentation", "lot-13.html", [
        [ "Objectif", "lot-13.html#autotoc_md801", null ],
        [ "Périmètre", "lot-13.html#autotoc_md802", [
          [ "Inclus", "lot-13.html#autotoc_md803", null ],
          [ "Exclus", "lot-13.html#autotoc_md804", null ]
        ] ],
        [ "Décisions de cadrage", "lot-13.html#autotoc_md805", null ],
        [ "Exigences couvertes", "lot-13.html#autotoc_md806", null ],
        [ "Découpage", "lot-13.html#autotoc_md807", null ],
        [ "Critères d'acceptation du lot", "lot-13.html#autotoc_md808", null ],
        [ "Dépendances", "lot-13.html#autotoc_md809", null ]
      ] ],
      [ "LOT-14 — Éditeur de niveaux intégré : édition de tuiles, mécanismes, essai immédiat", "lot-14.html", [
        [ "Objectif", "lot-14.html#autotoc_md810", null ],
        [ "Périmètre", "lot-14.html#autotoc_md811", [
          [ "Inclus", "lot-14.html#autotoc_md812", null ],
          [ "Exclus (lots ultérieurs)", "lot-14.html#autotoc_md813", null ]
        ] ],
        [ "Décisions de cadrage", "lot-14.html#autotoc_md814", null ],
        [ "Exigences couvertes", "lot-14.html#autotoc_md815", null ],
        [ "Découpage", "lot-14.html#autotoc_md816", null ],
        [ "Critères d'acceptation du lot", "lot-14.html#autotoc_md817", null ],
        [ "Dépendances", "lot-14.html#autotoc_md818", null ],
        [ "Navigation des tâches", "lot-14.html#autotoc_md819", null ],
        [ "TACHE-01 — Sérialisation JSON + modèle d'édition mutable", "lot-14-tache-01-serialisation-modele-edition.html", [
          [ "Contexte", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md820", null ],
          [ "Travail à réaliser", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md821", null ],
          [ "Fichiers impactés", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md822", null ],
          [ "Tests (obligatoires)", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md823", null ],
          [ "Points d'attention", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md824", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md825", null ],
          [ "Exigences", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md826", null ]
        ] ],
        [ "TACHE-02 — Écran éditeur : grille cliquable + palette de tuiles", "lot-14-tache-02-ecran-editeur-palette.html", [
          [ "Contexte", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md827", null ],
          [ "Travail à réaliser", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md828", null ],
          [ "Fichiers impactés", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md829", null ],
          [ "Tests (obligatoires)", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md830", null ],
          [ "Points d'attention", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md831", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md832", null ],
          [ "Exigences", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md833", null ]
        ] ],
        [ "TACHE-03 — Entrée/sortie, liaison de mécanismes, redimensionnement", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html", [
          [ "Contexte", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md834", null ],
          [ "Travail à réaliser", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md835", null ],
          [ "Fichiers impactés", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md836", null ],
          [ "Tests (obligatoires)", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md837", null ],
          [ "Points d'attention", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md838", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md839", null ],
          [ "Exigences", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md840", null ]
        ] ],
        [ "TACHE-04 — Historique annuler/refaire", "lot-14-tache-04-undo-redo.html", [
          [ "Contexte", "lot-14-tache-04-undo-redo.html#autotoc_md841", null ],
          [ "Travail à réaliser", "lot-14-tache-04-undo-redo.html#autotoc_md842", null ],
          [ "Fichiers impactés", "lot-14-tache-04-undo-redo.html#autotoc_md843", null ],
          [ "Tests (obligatoires)", "lot-14-tache-04-undo-redo.html#autotoc_md844", null ],
          [ "Points d'attention", "lot-14-tache-04-undo-redo.html#autotoc_md845", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-04-undo-redo.html#autotoc_md846", null ],
          [ "Exigences", "lot-14-tache-04-undo-redo.html#autotoc_md847", null ]
        ] ],
        [ "TACHE-05 — Enregistrement, validation, essai immédiat", "lot-14-tache-05-enregistrement-validation-essai.html", [
          [ "Contexte", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md848", null ],
          [ "Travail à réaliser", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md849", null ],
          [ "Fichiers impactés", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md850", null ],
          [ "Tests (obligatoires)", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md851", null ],
          [ "Points d'attention", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md852", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md853", null ],
          [ "Exigences", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md854", null ]
        ] ],
        [ "TACHE-06 — Intégration menu, tests système, guide non-codeur Git", "lot-14-tache-06-integration-guide-non-codeur.html", [
          [ "Contexte", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md855", null ],
          [ "Travail à réaliser", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md856", null ],
          [ "Fichiers impactés", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md857", null ],
          [ "Tests (obligatoires)", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md858", null ],
          [ "Points d'attention", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md859", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md860", null ],
          [ "Exigences", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md861", null ]
        ] ]
      ] ],
      [ "LOT-15 — Éditeur de niveaux : robustesse et confort d'édition", "lot-15.html", [
        [ "Objectif", "lot-15.html#autotoc_md862", null ],
        [ "Périmètre", "lot-15.html#autotoc_md863", [
          [ "Inclus", "lot-15.html#autotoc_md864", null ],
          [ "Exclus (lots ultérieurs ou non retenus)", "lot-15.html#autotoc_md865", null ]
        ] ],
        [ "Décisions de cadrage", "lot-15.html#autotoc_md866", null ],
        [ "Exigences couvertes", "lot-15.html#autotoc_md867", null ],
        [ "Découpage", "lot-15.html#autotoc_md868", null ],
        [ "Critères d'acceptation du lot", "lot-15.html#autotoc_md869", null ],
        [ "Dépendances", "lot-15.html#autotoc_md870", null ],
        [ "Navigation des tâches", "lot-15.html#autotoc_md871", null ],
        [ "TACHE-01 — Entrées bas niveau : molette et texte tapé", "lot-15-tache-01-entrees-molette-texte.html", [
          [ "Contexte", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md872", null ],
          [ "Travail à réaliser", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md873", null ],
          [ "Fichiers impactés", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md874", null ],
          [ "Tests (obligatoires)", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md875", null ],
          [ "Points d'attention", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md876", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md877", null ],
          [ "Exigences", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md878", null ],
          [ "Ajustement post-livraison (essai utilisateur)", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md879", null ]
        ] ],
        [ "TACHE-02 — Garde-fous : redimensionnement destructeur, quitter sans enregistrer", "lot-15-tache-02-garde-fous-perte-donnees.html", [
          [ "Contexte", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md880", null ],
          [ "Travail à réaliser", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md881", null ],
          [ "Fichiers impactés", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md882", null ],
          [ "Tests (obligatoires)", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md883", null ],
          [ "Points d'attention", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md884", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md885", null ],
          [ "Exigences", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md886", null ]
        ] ],
        [ "TACHE-03 — Nommage, renommage, avertissement d'écrasement", "lot-15-tache-03-nommage-renommage.html", [
          [ "Contexte", "lot-15-tache-03-nommage-renommage.html#autotoc_md887", null ],
          [ "Travail à réaliser", "lot-15-tache-03-nommage-renommage.html#autotoc_md888", null ],
          [ "Fichiers impactés", "lot-15-tache-03-nommage-renommage.html#autotoc_md889", null ],
          [ "Tests (obligatoires)", "lot-15-tache-03-nommage-renommage.html#autotoc_md890", null ],
          [ "Points d'attention", "lot-15-tache-03-nommage-renommage.html#autotoc_md891", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-03-nommage-renommage.html#autotoc_md892", null ],
          [ "Exigences", "lot-15-tache-03-nommage-renommage.html#autotoc_md893", null ]
        ] ],
        [ "TACHE-04 — Caméra : pan et zoom manuels", "lot-15-tache-04-camera-pan-zoom.html", [
          [ "Contexte", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md894", null ],
          [ "Travail à réaliser", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md895", null ],
          [ "Fichiers impactés", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md896", null ],
          [ "Tests (obligatoires)", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md897", null ],
          [ "Points d'attention", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md898", null ],
          [ "Ajustement post-livraison (essai utilisateur)", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md899", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md900", null ],
          [ "Exigences", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md901", null ]
        ] ],
        [ "TACHE-05 — Outils de zone : remplissage rectangulaire, sélection, copier/coller", "lot-15-tache-05-outils-rectangle-selection.html", [
          [ "Contexte", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md902", null ],
          [ "Travail à réaliser", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md903", null ],
          [ "Fichiers impactés", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md904", null ],
          [ "Tests (obligatoires)", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md905", null ],
          [ "Points d'attention", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md906", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md907", null ],
          [ "Exigences", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md908", null ]
        ] ],
        [ "TACHE-06 — Découvrabilité : barre d'outils, aide, libellés, liaisons lisibles", "lot-15-tache-06-decouvrabilite.html", [
          [ "Contexte", "lot-15-tache-06-decouvrabilite.html#autotoc_md909", null ],
          [ "Travail à réaliser", "lot-15-tache-06-decouvrabilite.html#autotoc_md910", null ],
          [ "Fichiers impactés", "lot-15-tache-06-decouvrabilite.html#autotoc_md911", null ],
          [ "Tests (obligatoires)", "lot-15-tache-06-decouvrabilite.html#autotoc_md912", null ],
          [ "Points d'attention", "lot-15-tache-06-decouvrabilite.html#autotoc_md913", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-06-decouvrabilite.html#autotoc_md914", null ],
          [ "Exigences", "lot-15-tache-06-decouvrabilite.html#autotoc_md915", null ],
          [ "Ajustement post-livraison (essai utilisateur)", "lot-15-tache-06-decouvrabilite.html#autotoc_md916", null ]
        ] ],
        [ "TACHE-07 — Essai immédiat en mémoire, erreurs de validation structurées", "lot-15-tache-07-essai-memoire-erreurs-structurees.html", [
          [ "Contexte", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md917", null ],
          [ "Travail à réaliser", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md918", null ],
          [ "Fichiers impactés", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md919", null ],
          [ "Tests (obligatoires)", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md920", null ],
          [ "Points d'attention", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md921", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md922", null ],
          [ "Exigences", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md923", null ]
        ] ],
        [ "TACHE-08 — Nettoyage documentaire", "lot-15-tache-08-nettoyage-documentation.html", [
          [ "Contexte", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md924", null ],
          [ "Travail à réaliser", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md925", null ],
          [ "Fichiers impactés", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md926", null ],
          [ "Tests (obligatoires)", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md927", null ],
          [ "Points d'attention", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md928", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md929", null ],
          [ "Exigences", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md930", null ]
        ] ]
      ] ],
      [ "LOT-16 — Niveaux de grande taille", "lot-16.html", [
        [ "Objectif", "lot-16.html#autotoc_md931", null ],
        [ "Périmètre", "lot-16.html#autotoc_md932", [
          [ "Inclus", "lot-16.html#autotoc_md933", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-16.html#autotoc_md934", null ]
        ] ],
        [ "Décisions de cadrage", "lot-16.html#autotoc_md935", null ],
        [ "Exigences couvertes", "lot-16.html#autotoc_md936", null ],
        [ "Découpage", "lot-16.html#autotoc_md937", null ],
        [ "Critères d'acceptation du lot", "lot-16.html#autotoc_md938", null ],
        [ "Dépendances", "lot-16.html#autotoc_md939", null ],
        [ "Navigation des tâches", "lot-16.html#autotoc_md940", null ],
        [ "TACHE-01 — Plafond de taille et validation « largeur x hauteur »", "lot-16-tache-01-plafond-validation-taille.html", [
          [ "Contexte", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md941", null ],
          [ "Travail à réaliser", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md942", null ],
          [ "Fichiers impactés", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md943", null ],
          [ "Tests (obligatoires)", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md944", null ],
          [ "Points d'attention", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md945", null ],
          [ "Définition de fait (DoD)", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md946", null ],
          [ "Exigences", "lot-16-tache-01-plafond-validation-taille.html#autotoc_md947", null ]
        ] ],
        [ "TACHE-02 — Boîte de dialogue de redimensionnement (`Ctrl+R`)", "lot-16-tache-02-boite-dialogue-redimensionnement.html", [
          [ "Contexte", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md948", null ],
          [ "Travail à réaliser", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md949", null ],
          [ "Fichiers impactés", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md950", null ],
          [ "Tests (obligatoires)", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md951", null ],
          [ "Points d'attention", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md952", null ],
          [ "Définition de fait (DoD)", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md953", null ],
          [ "Exigences", "lot-16-tache-02-boite-dialogue-redimensionnement.html#autotoc_md954", null ]
        ] ],
        [ "TACHE-03 — Caméra : englober tout le niveau (éditeur et jeu)", "lot-16-tache-03-camera-niveau-entier.html", [
          [ "Contexte", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md955", null ],
          [ "Travail à réaliser", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md956", null ],
          [ "Fichiers impactés", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md957", null ],
          [ "Tests (obligatoires)", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md958", null ],
          [ "Points d'attention", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md959", null ],
          [ "Définition de fait (DoD)", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md960", null ],
          [ "Exigences", "lot-16-tache-03-camera-niveau-entier.html#autotoc_md961", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-16-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-16-tache-04-documentation-verification.html#autotoc_md962", null ],
          [ "Travail à réaliser", "lot-16-tache-04-documentation-verification.html#autotoc_md963", null ],
          [ "Fichiers impactés", "lot-16-tache-04-documentation-verification.html#autotoc_md964", null ],
          [ "Tests (obligatoires)", "lot-16-tache-04-documentation-verification.html#autotoc_md965", null ],
          [ "Points d'attention", "lot-16-tache-04-documentation-verification.html#autotoc_md966", null ],
          [ "Définition de fait (DoD)", "lot-16-tache-04-documentation-verification.html#autotoc_md967", null ],
          [ "Exigences", "lot-16-tache-04-documentation-verification.html#autotoc_md968", null ]
        ] ]
      ] ],
      [ "LOT-17 — Sprite du personnage (statique)", "lot-17.html", [
        [ "Objectif", "lot-17.html#autotoc_md969", null ],
        [ "Périmètre", "lot-17.html#autotoc_md970", [
          [ "Inclus", "lot-17.html#autotoc_md971", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-17.html#autotoc_md972", null ]
        ] ],
        [ "Décisions de cadrage", "lot-17.html#autotoc_md973", null ],
        [ "Exigences couvertes", "lot-17.html#autotoc_md974", null ],
        [ "Découpage", "lot-17.html#autotoc_md975", null ],
        [ "Critères d'acceptation du lot", "lot-17.html#autotoc_md976", null ],
        [ "Dépendances", "lot-17.html#autotoc_md977", null ],
        [ "Navigation des tâches", "lot-17.html#autotoc_md978", null ],
        [ "TACHE-01 — Silhouette du personnage dans l'atlas", "lot-17-tache-01-silhouette-personnage.html", [
          [ "Contexte", "lot-17-tache-01-silhouette-personnage.html#autotoc_md979", null ],
          [ "Travail à réaliser", "lot-17-tache-01-silhouette-personnage.html#autotoc_md980", null ],
          [ "Fichiers impactés", "lot-17-tache-01-silhouette-personnage.html#autotoc_md981", null ],
          [ "Tests (obligatoires)", "lot-17-tache-01-silhouette-personnage.html#autotoc_md982", null ],
          [ "Points d'attention", "lot-17-tache-01-silhouette-personnage.html#autotoc_md983", null ],
          [ "Définition de fait (DoD)", "lot-17-tache-01-silhouette-personnage.html#autotoc_md984", null ],
          [ "Exigences", "lot-17-tache-01-silhouette-personnage.html#autotoc_md985", null ]
        ] ],
        [ "TACHE-02 — Documentation et vérification", "lot-17-tache-02-documentation-verification.html", [
          [ "Contexte", "lot-17-tache-02-documentation-verification.html#autotoc_md986", null ],
          [ "Travail à réaliser", "lot-17-tache-02-documentation-verification.html#autotoc_md987", null ],
          [ "Fichiers impactés", "lot-17-tache-02-documentation-verification.html#autotoc_md988", null ],
          [ "Tests (obligatoires)", "lot-17-tache-02-documentation-verification.html#autotoc_md989", null ],
          [ "Points d'attention", "lot-17-tache-02-documentation-verification.html#autotoc_md990", null ],
          [ "Définition de fait (DoD)", "lot-17-tache-02-documentation-verification.html#autotoc_md991", null ],
          [ "Exigences", "lot-17-tache-02-documentation-verification.html#autotoc_md992", null ]
        ] ]
      ] ],
      [ "LOT-18 — Animation du personnage (repos, course, saut)", "lot-18.html", [
        [ "Objectif", "lot-18.html#autotoc_md993", null ],
        [ "Périmètre", "lot-18.html#autotoc_md994", [
          [ "Inclus", "lot-18.html#autotoc_md995", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-18.html#autotoc_md996", null ]
        ] ],
        [ "Décisions de cadrage", "lot-18.html#autotoc_md997", null ],
        [ "Exigences couvertes", "lot-18.html#autotoc_md998", null ],
        [ "Découpage", "lot-18.html#autotoc_md999", null ],
        [ "Critères d'acceptation du lot", "lot-18.html#autotoc_md1000", null ],
        [ "Dépendances", "lot-18.html#autotoc_md1001", null ],
        [ "Navigation des tâches", "lot-18.html#autotoc_md1002", null ],
        [ "TACHE-01 — Composant et système d'animation", "lot-18-tache-01-composant-systeme-animation.html", [
          [ "Contexte", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1003", null ],
          [ "Travail à réaliser", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1004", null ],
          [ "Fichiers impactés", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1005", null ],
          [ "Tests (obligatoires)", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1006", null ],
          [ "Points d'attention", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1007", null ],
          [ "Définition de fait (DoD)", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1008", null ],
          [ "Exigences", "lot-18-tache-01-composant-systeme-animation.html#autotoc_md1009", null ]
        ] ],
        [ "TACHE-02 — Images dans l'atlas et intégration au rendu", "lot-18-tache-02-frames-atlas-integration.html", [
          [ "Contexte", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1010", null ],
          [ "Travail à réaliser", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1011", null ],
          [ "Fichiers impactés", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1012", null ],
          [ "Tests (obligatoires)", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1013", null ],
          [ "Points d'attention", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1014", null ],
          [ "Définition de fait (DoD)", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1015", null ],
          [ "Exigences", "lot-18-tache-02-frames-atlas-integration.html#autotoc_md1016", null ]
        ] ],
        [ "TACHE-03 — Documentation et vérification", "lot-18-tache-03-documentation-verification.html", [
          [ "Contexte", "lot-18-tache-03-documentation-verification.html#autotoc_md1017", null ],
          [ "Travail à réaliser", "lot-18-tache-03-documentation-verification.html#autotoc_md1018", null ],
          [ "Fichiers impactés", "lot-18-tache-03-documentation-verification.html#autotoc_md1019", null ],
          [ "Tests (obligatoires)", "lot-18-tache-03-documentation-verification.html#autotoc_md1020", null ],
          [ "Points d'attention", "lot-18-tache-03-documentation-verification.html#autotoc_md1021", null ],
          [ "Définition de fait (DoD)", "lot-18-tache-03-documentation-verification.html#autotoc_md1022", null ],
          [ "Exigences", "lot-18-tache-03-documentation-verification.html#autotoc_md1023", null ]
        ] ]
      ] ],
      [ "LOT-19 — Physique newtonienne et plaque de pression", "lot-19.html", [
        [ "Objectif", "lot-19.html#autotoc_md1024", null ],
        [ "Périmètre", "lot-19.html#autotoc_md1025", [
          [ "Inclus", "lot-19.html#autotoc_md1026", null ],
          [ "Exclus (hors périmètre de ce lot)", "lot-19.html#autotoc_md1027", null ]
        ] ],
        [ "Décisions de cadrage", "lot-19.html#autotoc_md1028", null ],
        [ "Exigences couvertes", "lot-19.html#autotoc_md1029", null ],
        [ "Découpage", "lot-19.html#autotoc_md1030", null ],
        [ "Critères d'acceptation du lot", "lot-19.html#autotoc_md1031", null ],
        [ "Dépendances", "lot-19.html#autotoc_md1032", null ],
        [ "Navigation des tâches", "lot-19.html#autotoc_md1033", null ],
        [ "TACHE-01 — Masse et chute newtonienne", "lot-19-tache-01-masse-chute-newtonienne.html", [
          [ "Contexte", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1034", null ],
          [ "Travail à réaliser", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1035", null ],
          [ "Fichiers impactés", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1036", null ],
          [ "Tests (obligatoires)", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1037", null ],
          [ "Points d'attention", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1038", null ],
          [ "Définition de fait (DoD)", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1039", null ],
          [ "Exigences", "lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1040", null ]
        ] ],
        [ "TACHE-02 — Plaque de pression", "lot-19-tache-02-plaque-de-pression.html", [
          [ "Contexte", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1041", null ],
          [ "Travail à réaliser", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1042", null ],
          [ "Fichiers impactés", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1043", null ],
          [ "Tests (obligatoires)", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1044", null ],
          [ "Points d'attention", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1045", null ],
          [ "Définition de fait (DoD)", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1046", null ],
          [ "Exigences", "lot-19-tache-02-plaque-de-pression.html#autotoc_md1047", null ]
        ] ],
        [ "TACHE-03 — Intégration éditeur et niveau de démonstration", "lot-19-tache-03-editeur-niveau-demo.html", [
          [ "Contexte", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1048", null ],
          [ "Travail à réaliser", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1049", null ],
          [ "Fichiers impactés", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1050", null ],
          [ "Tests (obligatoires)", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1051", null ],
          [ "Points d'attention", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1052", null ],
          [ "Définition de fait (DoD)", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1053", null ],
          [ "Exigences", "lot-19-tache-03-editeur-niveau-demo.html#autotoc_md1054", null ]
        ] ],
        [ "TACHE-04 — Documentation et vérification", "lot-19-tache-04-documentation-verification.html", [
          [ "Contexte", "lot-19-tache-04-documentation-verification.html#autotoc_md1055", null ],
          [ "Travail à réaliser", "lot-19-tache-04-documentation-verification.html#autotoc_md1056", null ],
          [ "Fichiers impactés", "lot-19-tache-04-documentation-verification.html#autotoc_md1057", null ],
          [ "Tests (obligatoires)", "lot-19-tache-04-documentation-verification.html#autotoc_md1058", null ],
          [ "Points d'attention", "lot-19-tache-04-documentation-verification.html#autotoc_md1059", null ],
          [ "Définition de fait (DoD)", "lot-19-tache-04-documentation-verification.html#autotoc_md1060", null ],
          [ "Exigences", "lot-19-tache-04-documentation-verification.html#autotoc_md1061", null ]
        ] ]
      ] ]
    ] ],
    [ "Manuel utilisateur", "manuel.html", [
      [ "Pages", "manuel.html#autotoc_md1063", null ],
      [ "À venir", "manuel.html#autotoc_md1064", null ],
      [ "Télécharger et lancer le jeu", "manuel-telecharger.html", [
        [ "Prérequis", "manuel-telecharger.html#autotoc_md1071", null ],
        [ "Étapes", "manuel-telecharger.html#autotoc_md1072", null ],
        [ "Remarques", "manuel-telecharger.html#autotoc_md1073", null ]
      ] ],
      [ "Créer et partager un niveau (sans ligne de commande)", "manuel-partager-niveau.html", [
        [ "1. Récupérer le projet", "manuel-partager-niveau.html#autotoc_md1065", null ],
        [ "2. Lancer l'éditeur", "manuel-partager-niveau.html#autotoc_md1066", null ],
        [ "3. Créer un niveau", "manuel-partager-niveau.html#autotoc_md1067", null ],
        [ "4. Publier votre niveau", "manuel-partager-niveau.html#autotoc_md1068", null ],
        [ "5. Récupérer les niveaux des autres", "manuel-partager-niveau.html#autotoc_md1069", null ],
        [ "En cas de problème", "manuel-partager-niveau.html#autotoc_md1070", null ]
      ] ]
    ] ],
    [ "Cahier de test", "cahiertest.html", null ],
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
"Aabb_8h.html",
"Log_8h.html#acb91f3beabd6cadfedd9029f0ca76017",
"classcore_1_1ISystem.html",
"classcore_1_1View_1_1Iterator.html#ad90ca7dc587c9ef248e9df6f420c1cf0",
"classhmi_1_1GameScreen.html#a493f5509d5086709cd3e9084f2777909",
"classhmi_1_1MenuModel.html#a7006a37cb54fd087c01dd4a52a94c801",
"classhmi_1_1Window.html#a84edce17a0339adcb5aa1fb6473fe4c0",
"guide-maths.html#autotoc_md64",
"lot-04-tache-06-manuel-utilisateur.html#autotoc_md376",
"lot-08-tache-03-physique-personnage.html#autotoc_md597",
"lot-13.html#autotoc_md808",
"lot-19-tache-01-masse-chute-newtonienne.html#autotoc_md1039",
"spec-exigences.html",
"structcore_1_1SweepResult.html#a9f874ecc7effce266633af49e4da07f0",
"test__input__state_8cpp.html#aa8e6d3ad41aa204883ebcd46b5406b41",
"test__text__input__field_8cpp.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
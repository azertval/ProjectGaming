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
      [ "Comment lire ce guide", "guide.html#autotoc_md104", null ],
      [ "Architecture en deux couches", "guide.html#autotoc_md105", null ],
      [ "Plan du guide", "guide.html#autotoc_md106", null ],
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
        [ "<tt>Vector2</tt> : un point ou une direction dans le monde", "guide-maths.html#autotoc_md59", [
          [ "<tt>lengthSquared</tt> : éviter la racine carrée", "guide-maths.html#autotoc_md60", null ],
          [ "Égalité approchée", "guide-maths.html#autotoc_md61", null ]
        ] ],
        [ "<tt>Aabb</tt> : la boîte englobante alignée aux axes", "guide-maths.html#autotoc_md62", null ],
        [ "Conventions d'unités et de repère", "guide-maths.html#autotoc_md63", null ],
        [ "Comparaison flottante : pourquoi <tt>==</tt> est dangereux", "guide-maths.html#autotoc_md64", null ],
        [ "Voir aussi", "guide-maths.html#autotoc_md65", null ]
      ] ],
      [ "Physique du personnage", "guide-physique.html", [
        [ "1. Collision par balayage continu (swept AABB)", "guide-physique.html#autotoc_md79", [
          [ "Le problème : le <em>tunneling</em>", "guide-physique.html#autotoc_md80", null ],
          [ "La solution : tester tout le trajet, pas seulement l'arrivée", "guide-physique.html#autotoc_md81", null ],
          [ "Méthode retenue : balayage <strong>par axe</strong> avec clamp direct", "guide-physique.html#autotoc_md82", null ],
          [ "Pourquoi caler directement plutôt que d'interpoler", "guide-physique.html#autotoc_md83", null ],
          [ "Lire le résultat : <tt>core::SweepResult</tt>", "guide-physique.html#autotoc_md84", null ]
        ] ],
        [ "2. Gravité et intégration", "guide-physique.html#autotoc_md85", null ],
        [ "3. Saut et <em>game feel</em>", "guide-physique.html#autotoc_md86", null ],
        [ "4. Dash 8 directions", "guide-physique.html#autotoc_md87", null ],
        [ "5. Wall jump et wall slide", "guide-physique.html#autotoc_md88", null ],
        [ "Ordre d'un pas (résumé)", "guide-physique.html#autotoc_md89", null ],
        [ "Voir aussi", "guide-physique.html#autotoc_md90", null ]
      ] ],
      [ "Niveaux : modèle, chargement, mécanismes, budgets", "guide-niveaux.html", [
        [ "Le modèle en mémoire", "guide-niveaux.html#autotoc_md66", [
          [ "Deux systèmes de coordonnées à ne pas confondre", "guide-niveaux.html#autotoc_md67", null ],
          [ "<tt>core::TileType</tt> : le vocabulaire des cases", "guide-niveaux.html#autotoc_md68", null ],
          [ "<tt>core::TileMap</tt> : la grille", "guide-niveaux.html#autotoc_md69", null ],
          [ "<tt>core::Level</tt> : le niveau assemblé", "guide-niveaux.html#autotoc_md70", null ]
        ] ],
        [ "Chargement JSON", "guide-niveaux.html#autotoc_md71", [
          [ "Exemple concret", "guide-niveaux.html#autotoc_md72", null ],
          [ "Validation", "guide-niveaux.html#autotoc_md73", null ]
        ] ],
        [ "De la grille aux entités : <tt>buildLevelScene</tt>", "guide-niveaux.html#autotoc_md74", null ],
        [ "Mécanismes interrupteur ↔ porte", "guide-niveaux.html#autotoc_md75", null ],
        [ "Budget de mouvements", "guide-niveaux.html#autotoc_md76", null ],
        [ "Issue et enchaînement", "guide-niveaux.html#autotoc_md77", null ],
        [ "Voir aussi", "guide-niveaux.html#autotoc_md78", null ]
      ] ],
      [ "Entrées et actions logiques", "guide-entrees.html", [
        [ "Le principe : ne jamais coder « en dur » une touche dans le gameplay", "guide-entrees.html#autotoc_md40", null ],
        [ "Échantillonner plutôt que réagir : <tt>hmi::InputState</tt>", "guide-entrees.html#autotoc_md41", [
          [ "Détecter les fronts, pas seulement l'état", "guide-entrees.html#autotoc_md42", null ],
          [ "Le cycle d'une frame", "guide-entrees.html#autotoc_md43", null ],
          [ "Un détail d'implémentation qui simplifie tout : <tt>Key</tt> réutilise les codes Win32", "guide-entrees.html#autotoc_md44", null ]
        ] ],
        [ "Traduire l'état en intention : <tt>hmi::toPlayerInput</tt>", "guide-entrees.html#autotoc_md45", null ],
        [ "Voir aussi", "guide-entrees.html#autotoc_md46", null ]
      ] ],
      [ "Rendu 2D : de l'ECS à l'écran", "guide-rendu.html", [
        [ "Vocabulaire de base : GPU, swap chain, back buffer", "guide-rendu.html#autotoc_md91", null ],
        [ "<tt>hmi::GraphicsDevice</tt> : initialiser Direct3D 11 et présenter l'image", "guide-rendu.html#autotoc_md92", null ],
        [ "<tt>hmi::Window</tt> : la fenêtre, prérequis du rendu", "guide-rendu.html#autotoc_md93", null ],
        [ "Unités monde et pixels : <tt>hmi::Camera2D</tt>", "guide-rendu.html#autotoc_md94", null ],
        [ "Le pipeline de dessin de sprites : <tt>hmi::SpriteBatch</tt>", "guide-rendu.html#autotoc_md95", [
          [ "Pourquoi « batcher » plutôt que dessiner un sprite à la fois", "guide-rendu.html#autotoc_md96", null ],
          [ "<tt>SpriteQuad</tt> : un rectangle texturé", "guide-rendu.html#autotoc_md97", null ],
          [ "Sommets, shaders, et échantillonnage <em>nearest</em>", "guide-rendu.html#autotoc_md98", null ]
        ] ],
        [ "<tt>hmi::TextureAtlas</tt> : un spritesheet, généré en code", "guide-rendu.html#autotoc_md99", null ],
        [ "<tt>hmi::SpriteRenderer</tt> : le pont ECS → écran", "guide-rendu.html#autotoc_md100", null ],
        [ "<tt>hmi::BitmapFont</tt> : dessiner du texte", "guide-rendu.html#autotoc_md101", null ],
        [ "Assembler la frame complète", "guide-rendu.html#autotoc_md102", null ],
        [ "Voir aussi", "guide-rendu.html#autotoc_md103", null ]
      ] ],
      [ "Journalisation et assertions", "guide-journalisation.html", [
        [ "Pourquoi journaliser dans un jeu vidéo", "guide-journalisation.html#autotoc_md47", null ],
        [ "Les niveaux de gravité : <tt>core::LogLevel</tt>", "guide-journalisation.html#autotoc_md48", null ],
        [ "<tt>core::Logger</tt> : filtrer puis diffuser", "guide-journalisation.html#autotoc_md49", null ],
        [ "Les sinks : où finissent les messages", "guide-journalisation.html#autotoc_md50", null ],
        [ "Les macros de journalisation, par catégorie", "guide-journalisation.html#autotoc_md51", [
          [ "Chaque module a sa propre catégorie", "guide-journalisation.html#autotoc_md52", null ],
          [ "Une règle de performance à respecter", "guide-journalisation.html#autotoc_md53", null ]
        ] ],
        [ "Le format d'une ligne : <tt>core::formatLogLine</tt>", "guide-journalisation.html#autotoc_md54", null ],
        [ "Configurer le niveau minimal au lancement", "guide-journalisation.html#autotoc_md55", [
          [ "Bootstrap réel : sinks différents en développement et en Release", "guide-journalisation.html#autotoc_md56", null ]
        ] ],
        [ "Assertions : <tt>PROJECTGAMING_ASSERT</tt>, un outil différent", "guide-journalisation.html#autotoc_md57", null ],
        [ "Voir aussi", "guide-journalisation.html#autotoc_md58", null ]
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
        [ "Nommer un niveau : un champ de saisie minimal", "guide-editeur.html#autotoc_md36", null ],
        [ "Un panneau plutôt que des bandes empilées", "guide-editeur.html#autotoc_md37", null ],
        [ "Choisir un niveau à éditer : <tt>hmi::LevelPicker</tt>", "guide-editeur.html#autotoc_md38", null ],
        [ "Voir aussi", "guide-editeur.html#autotoc_md39", null ]
      ] ]
    ] ],
    [ "Spécifications", "specifications.html", [
      [ "Documents", "specifications.html#autotoc_md184", null ],
      [ "Vision & périmètre", "spec-vision.html", [
        [ "Concept", "spec-vision.html#autotoc_md185", [
          [ "Mécanique de jeu (décidée)", "spec-vision.html#autotoc_md186", null ]
        ] ],
        [ "Boucle de gameplay", "spec-vision.html#autotoc_md187", null ],
        [ "Objectifs (MVP)", "spec-vision.html#autotoc_md188", null ],
        [ "Objectifs produit (au-delà du moteur)", "spec-vision.html#autotoc_md189", null ],
        [ "Hors périmètre (MVP)", "spec-vision.html#autotoc_md190", null ],
        [ "Traçabilité", "spec-vision.html#autotoc_md191", null ]
      ] ],
      [ "Gameplay", "spec-gameplay.html", [
        [ "1. Monde en tuiles", "spec-gameplay.html#autotoc_md165", null ],
        [ "2. Personnage & déplacement", "spec-gameplay.html#autotoc_md166", [
          [ "Mécaniques aériennes avancées (au-delà du MVP)", "spec-gameplay.html#autotoc_md167", null ],
          [ "Ressenti (game feel) — ⚠️ à affiner par tests", "spec-gameplay.html#autotoc_md168", null ]
        ] ],
        [ "3. Mécanismes de puzzle", "spec-gameplay.html#autotoc_md169", null ],
        [ "4. Conditions de fin de niveau", "spec-gameplay.html#autotoc_md170", null ],
        [ "5. États de jeu", "spec-gameplay.html#autotoc_md171", null ],
        [ "Traçabilité", "spec-gameplay.html#autotoc_md172", null ]
      ] ],
      [ "Contrôles & entrées", "spec-controles.html", [
        [ "1. Périphériques", "spec-controles.html#autotoc_md119", null ],
        [ "2. Actions du jeu (mapping logique)", "spec-controles.html#autotoc_md120", null ],
        [ "3. Réactivité", "spec-controles.html#autotoc_md121", null ],
        [ "Traçabilité", "spec-controles.html#autotoc_md122", null ]
      ] ],
      [ "Rendu & cible technique", "spec-rendu-technique.html", [
        [ "1. Cible technique", "spec-rendu-technique.html#autotoc_md178", null ],
        [ "2. Rendu 2D", "spec-rendu-technique.html#autotoc_md179", null ],
        [ "3. Boucle & temps", "spec-rendu-technique.html#autotoc_md180", null ],
        [ "4. Interface (HMI)", "spec-rendu-technique.html#autotoc_md181", null ],
        [ "5. Audio (⚠️ minimal MVP)", "spec-rendu-technique.html#autotoc_md182", null ],
        [ "Traçabilité", "spec-rendu-technique.html#autotoc_md183", null ]
      ] ],
      [ "Niveaux & contenu", "spec-niveaux.html", [
        [ "1. Représentation des niveaux", "spec-niveaux.html#autotoc_md173", [
          [ "Format retenu (JSON, liste de tuiles-objets)", "spec-niveaux.html#autotoc_md174", null ]
        ] ],
        [ "2. Progression", "spec-niveaux.html#autotoc_md175", null ],
        [ "3. Conception (lignes directrices)", "spec-niveaux.html#autotoc_md176", null ],
        [ "Traçabilité", "spec-niveaux.html#autotoc_md177", null ]
      ] ],
      [ "Exigences non fonctionnelles", "spec-exigences.html", [
        [ "1. Performance", "spec-exigences.html#autotoc_md159", null ],
        [ "2. Architecture & maintenabilité", "spec-exigences.html#autotoc_md160", null ],
        [ "3. Qualité & vérification", "spec-exigences.html#autotoc_md161", null ],
        [ "4. Portabilité & reproductibilité", "spec-exigences.html#autotoc_md162", null ],
        [ "5. Robustesse", "spec-exigences.html#autotoc_md163", null ],
        [ "Traçabilité", "spec-exigences.html#autotoc_md164", null ]
      ] ],
      [ "Éditeur de niveaux", "spec-editeur.html", [
        [ "Objectif", "spec-editeur.html#autotoc_md150", null ],
        [ "1. Exigences fonctionnelles", "spec-editeur.html#autotoc_md151", null ],
        [ "2. Réutilisation & cohérence", "spec-editeur.html#autotoc_md152", null ],
        [ "3. Distribution & collaboration", "spec-editeur.html#autotoc_md153", null ],
        [ "4. Approche d'implémentation (décidée)", "spec-editeur.html#autotoc_md154", null ],
        [ "4bis. Décors & pixel art (post-MVP, intégré à l'éditeur)", "spec-editeur.html#autotoc_md155", null ],
        [ "5. Non-objectifs (éditeur, MVP)", "spec-editeur.html#autotoc_md156", null ],
        [ "6. Robustesse et confort d'édition (LOT-15)", "spec-editeur.html#autotoc_md157", null ],
        [ "Traçabilité", "spec-editeur.html#autotoc_md158", null ]
      ] ],
      [ "Architecture (décisions dimensionnantes)", "spec-architecture.html", [
        [ "1. Modules & dépendances", "spec-architecture.html#autotoc_md107", null ],
        [ "2. Modèle d'entités : ECS", "spec-architecture.html#autotoc_md108", null ],
        [ "3. Coordonnées & unités — trois espaces distincts", "spec-architecture.html#autotoc_md109", null ],
        [ "4. Frontière simulation ↔ rendu", "spec-architecture.html#autotoc_md110", null ],
        [ "5. Mathématiques dans <tt>Core</tt>", "spec-architecture.html#autotoc_md111", null ],
        [ "6. Abstraction de rendu", "spec-architecture.html#autotoc_md112", null ],
        [ "7. Modèle de threading", "spec-architecture.html#autotoc_md113", null ],
        [ "8. Communication inter-systèmes", "spec-architecture.html#autotoc_md114", null ],
        [ "9. Gestion des ressources", "spec-architecture.html#autotoc_md115", null ],
        [ "10. Contrainte « éditeur intégré »", "spec-architecture.html#autotoc_md116", null ],
        [ "11. Décors dynamiques (accommodation dimensionnante)", "spec-architecture.html#autotoc_md117", null ],
        [ "Traçabilité", "spec-architecture.html#autotoc_md118", null ]
      ] ],
      [ "Décors & pipeline pixel art", "spec-decors.html", [
        [ "Vision", "spec-decors.html#autotoc_md142", null ],
        [ "1. Système de décors", "spec-decors.html#autotoc_md143", null ],
        [ "2. Manipulation", "spec-decors.html#autotoc_md144", [
          [ "À la conception (éditeur)", "spec-decors.html#autotoc_md145", null ],
          [ "En jeu (mécanique, à terme)", "spec-decors.html#autotoc_md146", null ]
        ] ],
        [ "3. Pipeline photo → pixel art (intégré à l'éditeur)", "spec-decors.html#autotoc_md147", null ],
        [ "4. Périmètre & séquencement", "spec-decors.html#autotoc_md148", null ],
        [ "Traçabilité", "spec-decors.html#autotoc_md149", null ]
      ] ],
      [ "Conventions de code", "spec-conventions.html", [
        [ "1. Langage & standard", "spec-conventions.html#autotoc_md124", null ],
        [ "2. Nommage", "spec-conventions.html#autotoc_md125", null ],
        [ "3. Mise en forme", "spec-conventions.html#autotoc_md126", null ],
        [ "4. Inclusions (<tt>#include</tt>)", "spec-conventions.html#autotoc_md127", [
          [ "Chemins complets depuis <tt>Source/</tt>", "spec-conventions.html#autotoc_md128", null ],
          [ "Ordre des groupes", "spec-conventions.html#autotoc_md129", null ]
        ] ],
        [ "5. Architecture (dépendances entre modules)", "spec-conventions.html#autotoc_md130", [
          [ "Classes plutôt que fonctions libres", "spec-conventions.html#autotoc_md131", null ],
          [ "RAII obligatoire", "spec-conventions.html#autotoc_md132", null ]
        ] ],
        [ "6. Documentation Doxygen", "spec-conventions.html#autotoc_md133", [
          [ "Doxygen dans le header, commentaires simples <tt>//</tt> dans le <tt>.cpp</tt>", "spec-conventions.html#autotoc_md134", null ],
          [ "Documentation du corps (<tt>.cpp</tt>)", "spec-conventions.html#autotoc_md135", null ]
        ] ],
        [ "7. Bonnes pratiques", "spec-conventions.html#autotoc_md136", null ],
        [ "8. Tests", "spec-conventions.html#autotoc_md137", null ],
        [ "9. Gestion des erreurs", "spec-conventions.html#autotoc_md138", null ],
        [ "10. Assertions & journalisation", "spec-conventions.html#autotoc_md139", null ],
        [ "11. Outillage qualité (automatisé)", "spec-conventions.html#autotoc_md140", null ],
        [ "12. Identifiants d'exigences (<tt>EX-…</tt>)", "spec-conventions.html#autotoc_md141", null ]
      ] ]
    ] ],
    [ "Lots", "lots.html", [
      [ "Lots", "lots.html#autotoc_md926", null ],
      [ "LOT-01 — Fenêtre & boucle de jeu (Direct3D 11)", "lot-01.html", [
        [ "Objectif", "lot-01.html#autotoc_md192", null ],
        [ "Périmètre", "lot-01.html#autotoc_md193", [
          [ "Inclus", "lot-01.html#autotoc_md194", null ],
          [ "Exclus (lots ultérieurs)", "lot-01.html#autotoc_md195", null ]
        ] ],
        [ "Exigences couvertes", "lot-01.html#autotoc_md196", null ],
        [ "Découpage", "lot-01.html#autotoc_md197", null ],
        [ "Critères d'acceptation du lot", "lot-01.html#autotoc_md198", null ],
        [ "Navigation des tâches", "lot-01.html#autotoc_md199", null ],
        [ "TACHE-01 — Fenêtre Win32 & pompe de messages", "lot-01-tache-01-fenetre-win32.html", [
          [ "Contexte", "lot-01-tache-01-fenetre-win32.html#autotoc_md200", null ],
          [ "Travail à réaliser", "lot-01-tache-01-fenetre-win32.html#autotoc_md201", null ],
          [ "Fichiers impactés", "lot-01-tache-01-fenetre-win32.html#autotoc_md202", null ],
          [ "Points d'attention", "lot-01-tache-01-fenetre-win32.html#autotoc_md203", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-01-fenetre-win32.html#autotoc_md204", null ],
          [ "Exigences", "lot-01-tache-01-fenetre-win32.html#autotoc_md205", null ]
        ] ],
        [ "TACHE-02 — Initialisation Direct3D 11 (RAII)", "lot-01-tache-02-init-direct3d11.html", [
          [ "Contexte", "lot-01-tache-02-init-direct3d11.html#autotoc_md206", null ],
          [ "Travail à réaliser", "lot-01-tache-02-init-direct3d11.html#autotoc_md207", null ],
          [ "Fichiers impactés", "lot-01-tache-02-init-direct3d11.html#autotoc_md208", null ],
          [ "Points d'attention", "lot-01-tache-02-init-direct3d11.html#autotoc_md209", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-02-init-direct3d11.html#autotoc_md210", null ],
          [ "Exigences", "lot-01-tache-02-init-direct3d11.html#autotoc_md211", null ]
        ] ],
        [ "TACHE-03 — Boucle à pas de temps fixe (testable)", "lot-01-tache-03-boucle-pas-fixe.html", [
          [ "Contexte", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md212", null ],
          [ "Travail à réaliser", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md213", null ],
          [ "Fichiers impactés", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md214", null ],
          [ "Tests (obligatoires)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md215", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md216", null ],
          [ "Exigences", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md217", null ]
        ] ],
        [ "TACHE-04 — Effacement écran, présentation & redimensionnement", "lot-01-tache-04-effacement-presentation.html", [
          [ "Contexte", "lot-01-tache-04-effacement-presentation.html#autotoc_md218", null ],
          [ "Travail à réaliser", "lot-01-tache-04-effacement-presentation.html#autotoc_md219", null ],
          [ "Fichiers impactés", "lot-01-tache-04-effacement-presentation.html#autotoc_md220", null ],
          [ "Points d'attention", "lot-01-tache-04-effacement-presentation.html#autotoc_md221", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-04-effacement-presentation.html#autotoc_md222", null ],
          [ "Exigences", "lot-01-tache-04-effacement-presentation.html#autotoc_md223", null ]
        ] ],
        [ "TACHE-05 — Intégration `main` & vérification", "lot-01-tache-05-integration.html", [
          [ "Contexte", "lot-01-tache-05-integration.html#autotoc_md224", null ],
          [ "Travail à réaliser", "lot-01-tache-05-integration.html#autotoc_md225", null ],
          [ "Fichiers impactés", "lot-01-tache-05-integration.html#autotoc_md226", null ],
          [ "Vérification (manuelle + automatique)", "lot-01-tache-05-integration.html#autotoc_md227", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-05-integration.html#autotoc_md228", null ],
          [ "Exigences", "lot-01-tache-05-integration.html#autotoc_md229", null ]
        ] ]
      ] ],
      [ "LOT-02 — Journalisation & diagnostics", "lot-02.html", [
        [ "Objectif", "lot-02.html#autotoc_md230", null ],
        [ "Périmètre", "lot-02.html#autotoc_md231", [
          [ "Inclus", "lot-02.html#autotoc_md232", null ],
          [ "Exclus (plus tard)", "lot-02.html#autotoc_md233", null ]
        ] ],
        [ "Exigences couvertes", "lot-02.html#autotoc_md234", null ],
        [ "Découpage", "lot-02.html#autotoc_md235", null ],
        [ "Critères d'acceptation du lot", "lot-02.html#autotoc_md236", null ],
        [ "Dépendances", "lot-02.html#autotoc_md237", null ],
        [ "Navigation des tâches", "lot-02.html#autotoc_md238", null ],
        [ "TACHE-01 — Niveaux de log & interface `Logger`", "lot-02-tache-01-niveaux-logger.html", [
          [ "Contexte", "lot-02-tache-01-niveaux-logger.html#autotoc_md239", null ],
          [ "Travail à réaliser", "lot-02-tache-01-niveaux-logger.html#autotoc_md240", null ],
          [ "Fichiers impactés", "lot-02-tache-01-niveaux-logger.html#autotoc_md241", null ],
          [ "Tests (obligatoires)", "lot-02-tache-01-niveaux-logger.html#autotoc_md242", null ],
          [ "Points d'attention", "lot-02-tache-01-niveaux-logger.html#autotoc_md243", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-01-niveaux-logger.html#autotoc_md244", null ],
          [ "Exigences", "lot-02-tache-01-niveaux-logger.html#autotoc_md245", null ]
        ] ],
        [ "TACHE-02 — Sinks enfichables", "lot-02-tache-02-sinks.html", [
          [ "Contexte", "lot-02-tache-02-sinks.html#autotoc_md246", null ],
          [ "Travail à réaliser", "lot-02-tache-02-sinks.html#autotoc_md247", null ],
          [ "Fichiers impactés", "lot-02-tache-02-sinks.html#autotoc_md248", null ],
          [ "Tests (obligatoires)", "lot-02-tache-02-sinks.html#autotoc_md249", null ],
          [ "Points d'attention", "lot-02-tache-02-sinks.html#autotoc_md250", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-02-sinks.html#autotoc_md251", null ],
          [ "Exigences", "lot-02-tache-02-sinks.html#autotoc_md252", null ]
        ] ],
        [ "TACHE-03 — Macros de log (fichier/ligne, horodatage)", "lot-02-tache-03-macros-log.html", [
          [ "Contexte", "lot-02-tache-03-macros-log.html#autotoc_md253", null ],
          [ "Travail à réaliser", "lot-02-tache-03-macros-log.html#autotoc_md254", null ],
          [ "Fichiers impactés", "lot-02-tache-03-macros-log.html#autotoc_md255", null ],
          [ "Tests (obligatoires)", "lot-02-tache-03-macros-log.html#autotoc_md256", null ],
          [ "Points d'attention", "lot-02-tache-03-macros-log.html#autotoc_md257", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-03-macros-log.html#autotoc_md258", null ],
          [ "Exigences", "lot-02-tache-03-macros-log.html#autotoc_md259", null ]
        ] ],
        [ "TACHE-04 — Assertions `PROJECTGAMING_ASSERT`", "lot-02-tache-04-assertions.html", [
          [ "Contexte", "lot-02-tache-04-assertions.html#autotoc_md260", null ],
          [ "Travail à réaliser", "lot-02-tache-04-assertions.html#autotoc_md261", null ],
          [ "Fichiers impactés", "lot-02-tache-04-assertions.html#autotoc_md262", null ],
          [ "Tests (obligatoires)", "lot-02-tache-04-assertions.html#autotoc_md263", null ],
          [ "Points d'attention", "lot-02-tache-04-assertions.html#autotoc_md264", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-04-assertions.html#autotoc_md265", null ],
          [ "Exigences", "lot-02-tache-04-assertions.html#autotoc_md266", null ]
        ] ],
        [ "TACHE-05 — Intégration dans `main` & documentation", "lot-02-tache-05-integration.html", [
          [ "Contexte", "lot-02-tache-05-integration.html#autotoc_md267", null ],
          [ "Travail à réaliser", "lot-02-tache-05-integration.html#autotoc_md268", null ],
          [ "Fichiers impactés", "lot-02-tache-05-integration.html#autotoc_md269", null ],
          [ "Vérification", "lot-02-tache-05-integration.html#autotoc_md270", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-05-integration.html#autotoc_md271", null ],
          [ "Exigences", "lot-02-tache-05-integration.html#autotoc_md272", null ]
        ] ]
      ] ],
      [ "LOT-03 — Fondation ECS & mathématiques `Core`", "lot-03.html", [
        [ "Objectif", "lot-03.html#autotoc_md273", null ],
        [ "⚠️ Décision préalable : ECS maison vs bibliothèque", "lot-03.html#autotoc_md274", null ],
        [ "Périmètre", "lot-03.html#autotoc_md275", [
          [ "Inclus", "lot-03.html#autotoc_md276", null ],
          [ "Exclus (lots ultérieurs)", "lot-03.html#autotoc_md277", null ]
        ] ],
        [ "Exigences couvertes", "lot-03.html#autotoc_md278", null ],
        [ "Découpage", "lot-03.html#autotoc_md279", null ],
        [ "Critères d'acceptation du lot", "lot-03.html#autotoc_md280", null ],
        [ "Dépendances", "lot-03.html#autotoc_md281", null ],
        [ "Navigation des tâches", "lot-03.html#autotoc_md282", null ],
        [ "TACHE-01 — Types mathématiques de `Core`", "lot-03-tache-01-math-core.html", [
          [ "Contexte", "lot-03-tache-01-math-core.html#autotoc_md283", null ],
          [ "Travail à réaliser", "lot-03-tache-01-math-core.html#autotoc_md284", null ],
          [ "Fichiers impactés", "lot-03-tache-01-math-core.html#autotoc_md285", null ],
          [ "Tests (obligatoires)", "lot-03-tache-01-math-core.html#autotoc_md286", null ],
          [ "Points d'attention", "lot-03-tache-01-math-core.html#autotoc_md287", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-01-math-core.html#autotoc_md288", null ],
          [ "Exigences", "lot-03-tache-01-math-core.html#autotoc_md289", null ]
        ] ],
        [ "TACHE-02 — Entités : handles générationnels & cycle de vie", "lot-03-tache-02-entites.html", [
          [ "Contexte", "lot-03-tache-02-entites.html#autotoc_md290", null ],
          [ "Travail à réaliser", "lot-03-tache-02-entites.html#autotoc_md291", null ],
          [ "Fichiers impactés", "lot-03-tache-02-entites.html#autotoc_md292", null ],
          [ "Tests (obligatoires)", "lot-03-tache-02-entites.html#autotoc_md293", null ],
          [ "Points d'attention", "lot-03-tache-02-entites.html#autotoc_md294", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-02-entites.html#autotoc_md295", null ],
          [ "Exigences", "lot-03-tache-02-entites.html#autotoc_md296", null ]
        ] ],
        [ "TACHE-03 — Stockage de composants (sparse set typé)", "lot-03-tache-03-stockage-composants.html", [
          [ "Contexte", "lot-03-tache-03-stockage-composants.html#autotoc_md297", null ],
          [ "Travail à réaliser", "lot-03-tache-03-stockage-composants.html#autotoc_md298", null ],
          [ "Fichiers impactés", "lot-03-tache-03-stockage-composants.html#autotoc_md299", null ],
          [ "Tests (obligatoires)", "lot-03-tache-03-stockage-composants.html#autotoc_md300", null ],
          [ "Points d'attention", "lot-03-tache-03-stockage-composants.html#autotoc_md301", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-03-stockage-composants.html#autotoc_md302", null ],
          [ "Exigences", "lot-03-tache-03-stockage-composants.html#autotoc_md303", null ]
        ] ],
        [ "TACHE-04 — Requêtes / vues multi-composants", "lot-03-tache-04-vues-requetes.html", [
          [ "Contexte", "lot-03-tache-04-vues-requetes.html#autotoc_md304", null ],
          [ "Travail à réaliser", "lot-03-tache-04-vues-requetes.html#autotoc_md305", null ],
          [ "Fichiers impactés", "lot-03-tache-04-vues-requetes.html#autotoc_md306", null ],
          [ "Tests (obligatoires)", "lot-03-tache-04-vues-requetes.html#autotoc_md307", null ],
          [ "Points d'attention", "lot-03-tache-04-vues-requetes.html#autotoc_md308", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-04-vues-requetes.html#autotoc_md309", null ],
          [ "Exigences", "lot-03-tache-04-vues-requetes.html#autotoc_md310", null ]
        ] ],
        [ "TACHE-05 — Systèmes & `World` (orchestration au pas fixe)", "lot-03-tache-05-systemes-world.html", [
          [ "Contexte", "lot-03-tache-05-systemes-world.html#autotoc_md311", null ],
          [ "Travail à réaliser", "lot-03-tache-05-systemes-world.html#autotoc_md312", null ],
          [ "Fichiers impactés", "lot-03-tache-05-systemes-world.html#autotoc_md313", null ],
          [ "Tests (obligatoires)", "lot-03-tache-05-systemes-world.html#autotoc_md314", null ],
          [ "Points d'attention", "lot-03-tache-05-systemes-world.html#autotoc_md315", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-05-systemes-world.html#autotoc_md316", null ],
          [ "Exigences", "lot-03-tache-05-systemes-world.html#autotoc_md317", null ]
        ] ],
        [ "TACHE-06 — Composant `Transform` + système de mouvement (démo)", "lot-03-tache-06-transform-demo.html", [
          [ "Contexte", "lot-03-tache-06-transform-demo.html#autotoc_md318", null ],
          [ "Travail à réaliser", "lot-03-tache-06-transform-demo.html#autotoc_md319", null ],
          [ "Fichiers impactés", "lot-03-tache-06-transform-demo.html#autotoc_md320", null ],
          [ "Tests (obligatoires)", "lot-03-tache-06-transform-demo.html#autotoc_md321", null ],
          [ "Points d'attention", "lot-03-tache-06-transform-demo.html#autotoc_md322", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-06-transform-demo.html#autotoc_md323", null ],
          [ "Exigences", "lot-03-tache-06-transform-demo.html#autotoc_md324", null ]
        ] ]
      ] ],
      [ "LOT-04 — Documentation Doxygen & réorganisation de l'arborescence documentaire", "lot-04.html", [
        [ "Objectif", "lot-04.html#autotoc_md325", null ],
        [ "Périmètre", "lot-04.html#autotoc_md326", [
          [ "Inclus", "lot-04.html#autotoc_md327", null ],
          [ "Exclus (lots ultérieurs)", "lot-04.html#autotoc_md328", null ]
        ] ],
        [ "Décisions de cadrage", "lot-04.html#autotoc_md329", null ],
        [ "Exigences couvertes", "lot-04.html#autotoc_md330", null ],
        [ "Découpage", "lot-04.html#autotoc_md331", null ],
        [ "Critères d'acceptation du lot", "lot-04.html#autotoc_md332", null ],
        [ "Dépendances", "lot-04.html#autotoc_md333", null ],
        [ "Navigation des tâches", "lot-04.html#autotoc_md334", null ],
        [ "TACHE-01 — Réorganisation de l'arborescence documentaire", "lot-04-tache-01-reorganisation-arbo.html", [
          [ "Contexte", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md335", null ],
          [ "Travail à réaliser", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md336", null ],
          [ "Fichiers impactés", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md337", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md338", null ],
          [ "Points d'attention", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md339", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md340", null ],
          [ "Exigences", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md341", null ]
        ] ],
        [ "TACHE-02 — Configuration Doxygen pour le Markdown", "lot-04-tache-02-config-doxygen-markdown.html", [
          [ "Contexte", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md342", null ],
          [ "Travail à réaliser", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md343", null ],
          [ "Fichiers impactés", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md344", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md345", null ],
          [ "Points d'attention", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md346", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md347", null ],
          [ "Exigences", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md348", null ]
        ] ],
        [ "TACHE-03 — Page d'accueil du projet (mainpage)", "lot-04-tache-03-page-accueil.html", [
          [ "Contexte", "lot-04-tache-03-page-accueil.html#autotoc_md349", null ],
          [ "Travail à réaliser", "lot-04-tache-03-page-accueil.html#autotoc_md350", null ],
          [ "Fichiers impactés", "lot-04-tache-03-page-accueil.html#autotoc_md351", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-03-page-accueil.html#autotoc_md352", null ],
          [ "Points d'attention", "lot-04-tache-03-page-accueil.html#autotoc_md353", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-03-page-accueil.html#autotoc_md354", null ],
          [ "Exigences", "lot-04-tache-03-page-accueil.html#autotoc_md355", null ]
        ] ],
        [ "TACHE-04 — Pages de spécification navigables (conventions incluses)", "lot-04-tache-04-pages-specification.html", [
          [ "Contexte", "lot-04-tache-04-pages-specification.html#autotoc_md356", null ],
          [ "Travail à réaliser", "lot-04-tache-04-pages-specification.html#autotoc_md357", null ],
          [ "Convention d'insertion d'une nouvelle spec (à documenter dans l'index)", "lot-04-tache-04-pages-specification.html#autotoc_md358", null ],
          [ "Fichiers impactés", "lot-04-tache-04-pages-specification.html#autotoc_md359", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-04-pages-specification.html#autotoc_md360", null ],
          [ "Points d'attention", "lot-04-tache-04-pages-specification.html#autotoc_md361", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-04-pages-specification.html#autotoc_md362", null ],
          [ "Exigences", "lot-04-tache-04-pages-specification.html#autotoc_md363", null ]
        ] ],
        [ "TACHE-05 — Pages de lots navigables", "lot-04-tache-05-pages-lots.html", [
          [ "Contexte", "lot-04-tache-05-pages-lots.html#autotoc_md364", null ],
          [ "Travail à réaliser", "lot-04-tache-05-pages-lots.html#autotoc_md365", null ],
          [ "Fichiers impactés", "lot-04-tache-05-pages-lots.html#autotoc_md366", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-05-pages-lots.html#autotoc_md367", null ],
          [ "Points d'attention", "lot-04-tache-05-pages-lots.html#autotoc_md368", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-05-pages-lots.html#autotoc_md369", null ],
          [ "Exigences", "lot-04-tache-05-pages-lots.html#autotoc_md370", null ]
        ] ],
        [ "TACHE-06 — Manuel utilisateur (squelette + première page)", "lot-04-tache-06-manuel-utilisateur.html", [
          [ "Contexte", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md371", null ],
          [ "Travail à réaliser", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md372", null ],
          [ "Fichiers impactés", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md373", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md374", null ],
          [ "Points d'attention", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md375", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md376", null ],
          [ "Exigences", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md377", null ]
        ] ],
        [ "TACHE-07 — CI documentation (WARN_AS_ERROR & déploiement)", "lot-04-tache-07-ci-docs.html", [
          [ "Contexte", "lot-04-tache-07-ci-docs.html#autotoc_md378", null ],
          [ "Travail à réaliser", "lot-04-tache-07-ci-docs.html#autotoc_md379", null ],
          [ "Fichiers impactés", "lot-04-tache-07-ci-docs.html#autotoc_md380", null ],
          [ "Avertissements connus à corriger avant <tt>WARN_AS_ERROR</tt> (relevés en TACHE-02)", "lot-04-tache-07-ci-docs.html#autotoc_md381", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-07-ci-docs.html#autotoc_md382", null ],
          [ "Points d'attention", "lot-04-tache-07-ci-docs.html#autotoc_md383", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-07-ci-docs.html#autotoc_md384", null ],
          [ "Exigences", "lot-04-tache-07-ci-docs.html#autotoc_md385", null ]
        ] ],
        [ "TACHE-08 — Traçabilité des exigences (IDs stables, ancres Doxygen, lint CI)", "lot-04-tache-08-tracabilite-exigences.html", [
          [ "Contexte", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md386", null ],
          [ "Règle à formaliser (dans <tt>conventions.md</tt>)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md387", null ],
          [ "Travail à réaliser", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md388", null ],
          [ "Fichiers impactés", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md389", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md390", null ],
          [ "Points d'attention", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md391", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md392", null ],
          [ "Exigences", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md393", null ]
        ] ]
      ] ],
      [ "LOT-05 — Rendu 2D : atlas, sprites & caméra", "lot-05.html", [
        [ "Objectif", "lot-05.html#autotoc_md394", null ],
        [ "Périmètre", "lot-05.html#autotoc_md395", [
          [ "Inclus", "lot-05.html#autotoc_md396", null ],
          [ "Exclus (lots ultérieurs)", "lot-05.html#autotoc_md397", null ]
        ] ],
        [ "Décisions de cadrage", "lot-05.html#autotoc_md398", null ],
        [ "Exigences couvertes", "lot-05.html#autotoc_md399", null ],
        [ "Découpage", "lot-05.html#autotoc_md400", null ],
        [ "Critères d'acceptation du lot", "lot-05.html#autotoc_md401", null ],
        [ "Dépendances", "lot-05.html#autotoc_md402", null ],
        [ "Navigation des tâches", "lot-05.html#autotoc_md403", null ],
        [ "TACHE-01 — Composant `Sprite` (données pures)", "lot-05-tache-01-composant-sprite.html", [
          [ "Contexte", "lot-05-tache-01-composant-sprite.html#autotoc_md404", null ],
          [ "Travail à réaliser", "lot-05-tache-01-composant-sprite.html#autotoc_md405", null ],
          [ "Fichiers impactés", "lot-05-tache-01-composant-sprite.html#autotoc_md406", null ],
          [ "Tests (obligatoires si logique)", "lot-05-tache-01-composant-sprite.html#autotoc_md407", null ],
          [ "Points d'attention", "lot-05-tache-01-composant-sprite.html#autotoc_md408", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-01-composant-sprite.html#autotoc_md409", null ],
          [ "Exigences", "lot-05-tache-01-composant-sprite.html#autotoc_md410", null ]
        ] ],
        [ "TACHE-02 — Pipeline de quads texturés (HLSL, blend, nearest)", "lot-05-tache-02-pipeline-quads-textures.html", [
          [ "Contexte", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md411", null ],
          [ "Travail à réaliser", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md412", null ],
          [ "Fichiers impactés", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md413", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md414", null ],
          [ "Points d'attention", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md415", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md416", null ],
          [ "Exigences", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md417", null ]
        ] ],
        [ "TACHE-03 — Atlas de textures procédural", "lot-05-tache-03-atlas-procedural.html", [
          [ "Contexte", "lot-05-tache-03-atlas-procedural.html#autotoc_md418", null ],
          [ "Travail à réaliser", "lot-05-tache-03-atlas-procedural.html#autotoc_md419", null ],
          [ "Fichiers impactés", "lot-05-tache-03-atlas-procedural.html#autotoc_md420", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-03-atlas-procedural.html#autotoc_md421", null ],
          [ "Points d'attention", "lot-05-tache-03-atlas-procedural.html#autotoc_md422", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-03-atlas-procedural.html#autotoc_md423", null ],
          [ "Exigences", "lot-05-tache-03-atlas-procedural.html#autotoc_md424", null ]
        ] ],
        [ "TACHE-04 — Caméra 2D (monde → écran)", "lot-05-tache-04-camera-2d.html", [
          [ "Contexte", "lot-05-tache-04-camera-2d.html#autotoc_md425", null ],
          [ "Travail à réaliser", "lot-05-tache-04-camera-2d.html#autotoc_md426", null ],
          [ "Fichiers impactés", "lot-05-tache-04-camera-2d.html#autotoc_md427", null ],
          [ "Tests (obligatoires)", "lot-05-tache-04-camera-2d.html#autotoc_md428", null ],
          [ "Points d'attention", "lot-05-tache-04-camera-2d.html#autotoc_md429", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-04-camera-2d.html#autotoc_md430", null ],
          [ "Exigences", "lot-05-tache-04-camera-2d.html#autotoc_md431", null ]
        ] ],
        [ "TACHE-05 — Système de rendu des sprites (ECS → écran)", "lot-05-tache-05-systeme-rendu-sprites.html", [
          [ "Contexte", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md432", null ],
          [ "Travail à réaliser", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md433", null ],
          [ "Fichiers impactés", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md434", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md435", null ],
          [ "Points d'attention", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md436", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md437", null ],
          [ "Exigences", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md438", null ]
        ] ],
        [ "TACHE-06 — Câblage du `World` dans la boucle + scène de démo", "lot-05-tache-06-cablage-world-demo.html", [
          [ "Contexte", "lot-05-tache-06-cablage-world-demo.html#autotoc_md439", null ],
          [ "Travail à réaliser", "lot-05-tache-06-cablage-world-demo.html#autotoc_md440", null ],
          [ "Fichiers impactés", "lot-05-tache-06-cablage-world-demo.html#autotoc_md441", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md442", null ],
          [ "Points d'attention", "lot-05-tache-06-cablage-world-demo.html#autotoc_md443", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md444", null ],
          [ "Exigences", "lot-05-tache-06-cablage-world-demo.html#autotoc_md445", null ]
        ] ]
      ] ],
      [ "LOT-06 — Menu principal", "lot-06.html", [
        [ "Objectif", "lot-06.html#autotoc_md446", null ],
        [ "Périmètre", "lot-06.html#autotoc_md447", [
          [ "Inclus", "lot-06.html#autotoc_md448", null ],
          [ "Exclus (lots ultérieurs)", "lot-06.html#autotoc_md449", null ]
        ] ],
        [ "Décisions de cadrage", "lot-06.html#autotoc_md450", null ],
        [ "Exigences couvertes", "lot-06.html#autotoc_md451", null ],
        [ "Découpage", "lot-06.html#autotoc_md452", null ],
        [ "Critères d'acceptation du lot", "lot-06.html#autotoc_md453", null ],
        [ "Dépendances", "lot-06.html#autotoc_md454", null ],
        [ "Navigation des tâches", "lot-06.html#autotoc_md455", null ],
        [ "TACHE-01 — Entrées clavier & souris", "lot-06-tache-01-entrees-clavier-souris.html", [
          [ "Contexte", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md456", null ],
          [ "Travail à réaliser", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md457", null ],
          [ "Fichiers impactés", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md458", null ],
          [ "Tests (obligatoires)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md459", null ],
          [ "Points d'attention", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md460", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md461", null ],
          [ "Exigences", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md462", null ]
        ] ],
        [ "TACHE-02 — Rendu de texte (police bitmap)", "lot-06-tache-02-rendu-texte-bitmap.html", [
          [ "Contexte", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md463", null ],
          [ "Travail à réaliser", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md464", null ],
          [ "Fichiers impactés", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md465", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md466", null ],
          [ "Points d'attention", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md467", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md468", null ],
          [ "Exigences", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md469", null ]
        ] ],
        [ "TACHE-03 — Catalogue de traduction (i18n)", "lot-06-tache-03-catalogue-traduction.html", [
          [ "Contexte", "lot-06-tache-03-catalogue-traduction.html#autotoc_md470", null ],
          [ "Travail à réaliser", "lot-06-tache-03-catalogue-traduction.html#autotoc_md471", null ],
          [ "Fichiers impactés", "lot-06-tache-03-catalogue-traduction.html#autotoc_md472", null ],
          [ "Tests (obligatoires)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md473", null ],
          [ "Points d'attention", "lot-06-tache-03-catalogue-traduction.html#autotoc_md474", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md475", null ],
          [ "Exigences", "lot-06-tache-03-catalogue-traduction.html#autotoc_md476", null ]
        ] ],
        [ "TACHE-04 — États d'application (écrans)", "lot-06-tache-04-etats-application.html", [
          [ "Contexte", "lot-06-tache-04-etats-application.html#autotoc_md477", null ],
          [ "Travail à réaliser", "lot-06-tache-04-etats-application.html#autotoc_md478", null ],
          [ "Fichiers impactés", "lot-06-tache-04-etats-application.html#autotoc_md479", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-04-etats-application.html#autotoc_md480", null ],
          [ "Points d'attention", "lot-06-tache-04-etats-application.html#autotoc_md481", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-04-etats-application.html#autotoc_md482", null ],
          [ "Exigences", "lot-06-tache-04-etats-application.html#autotoc_md483", null ]
        ] ],
        [ "TACHE-05 — Écran de menu principal", "lot-06-tache-05-ecran-menu-principal.html", [
          [ "Contexte", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md484", null ],
          [ "Travail à réaliser", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md485", null ],
          [ "Fichiers impactés", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md486", null ],
          [ "Tests (obligatoires)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md487", null ],
          [ "Points d'attention", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md488", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md489", null ],
          [ "Exigences", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md490", null ]
        ] ],
        [ "TACHE-06 — Écrans cibles (jeu démo + éditeur placeholder)", "lot-06-tache-06-ecrans-cibles.html", [
          [ "Contexte", "lot-06-tache-06-ecrans-cibles.html#autotoc_md491", null ],
          [ "Travail à réaliser", "lot-06-tache-06-ecrans-cibles.html#autotoc_md492", null ],
          [ "Fichiers impactés", "lot-06-tache-06-ecrans-cibles.html#autotoc_md493", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md494", null ],
          [ "Points d'attention", "lot-06-tache-06-ecrans-cibles.html#autotoc_md495", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md496", null ],
          [ "Exigences", "lot-06-tache-06-ecrans-cibles.html#autotoc_md497", null ]
        ] ],
        [ "TACHE-07 — Intégration `main` (boucle pilotée par l'écran)", "lot-06-tache-07-integration-main.html", [
          [ "Contexte", "lot-06-tache-07-integration-main.html#autotoc_md498", null ],
          [ "Travail à réaliser", "lot-06-tache-07-integration-main.html#autotoc_md499", null ],
          [ "Fichiers impactés", "lot-06-tache-07-integration-main.html#autotoc_md500", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-07-integration-main.html#autotoc_md501", null ],
          [ "Points d'attention", "lot-06-tache-07-integration-main.html#autotoc_md502", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-07-integration-main.html#autotoc_md503", null ],
          [ "Exigences", "lot-06-tache-07-integration-main.html#autotoc_md504", null ]
        ] ],
        [ "TACHE-08 — Sélecteur de langue", "lot-06-tache-08-selecteur-langue.html", [
          [ "Contexte", "lot-06-tache-08-selecteur-langue.html#autotoc_md505", null ],
          [ "Travail à réaliser", "lot-06-tache-08-selecteur-langue.html#autotoc_md506", null ],
          [ "Fichiers impactés", "lot-06-tache-08-selecteur-langue.html#autotoc_md507", null ],
          [ "Tests (obligatoires)", "lot-06-tache-08-selecteur-langue.html#autotoc_md508", null ],
          [ "Points d'attention", "lot-06-tache-08-selecteur-langue.html#autotoc_md509", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-08-selecteur-langue.html#autotoc_md510", null ],
          [ "Exigences", "lot-06-tache-08-selecteur-langue.html#autotoc_md511", null ]
        ] ]
      ] ],
      [ "LOT-07 — Niveaux : modèle et chargement", "lot-07.html", [
        [ "Objectif", "lot-07.html#autotoc_md512", null ],
        [ "Périmètre", "lot-07.html#autotoc_md513", [
          [ "Inclus", "lot-07.html#autotoc_md514", null ],
          [ "Exclus (lots ultérieurs)", "lot-07.html#autotoc_md515", null ]
        ] ],
        [ "Décisions de cadrage", "lot-07.html#autotoc_md516", null ],
        [ "Exigences couvertes", "lot-07.html#autotoc_md517", null ],
        [ "Découpage", "lot-07.html#autotoc_md518", null ],
        [ "Critères d'acceptation du lot", "lot-07.html#autotoc_md519", null ],
        [ "Dépendances", "lot-07.html#autotoc_md520", null ],
        [ "Navigation des tâches", "lot-07.html#autotoc_md521", null ],
        [ "TACHE-01 — Dépendance JSON (nlohmann/json épinglé)", "lot-07-tache-01-dependance-json.html", [
          [ "Contexte", "lot-07-tache-01-dependance-json.html#autotoc_md522", null ],
          [ "Travail à réaliser", "lot-07-tache-01-dependance-json.html#autotoc_md523", null ],
          [ "Fichiers impactés", "lot-07-tache-01-dependance-json.html#autotoc_md524", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-01-dependance-json.html#autotoc_md525", null ],
          [ "Points d'attention", "lot-07-tache-01-dependance-json.html#autotoc_md526", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-01-dependance-json.html#autotoc_md527", null ],
          [ "Exigences", "lot-07-tache-01-dependance-json.html#autotoc_md528", null ]
        ] ],
        [ "TACHE-02 — Modèle de tuiles et de niveau", "lot-07-tache-02-modele-niveau.html", [
          [ "Contexte", "lot-07-tache-02-modele-niveau.html#autotoc_md529", null ],
          [ "Travail à réaliser", "lot-07-tache-02-modele-niveau.html#autotoc_md530", null ],
          [ "Fichiers impactés", "lot-07-tache-02-modele-niveau.html#autotoc_md531", null ],
          [ "Tests (obligatoires)", "lot-07-tache-02-modele-niveau.html#autotoc_md532", null ],
          [ "Points d'attention", "lot-07-tache-02-modele-niveau.html#autotoc_md533", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-02-modele-niveau.html#autotoc_md534", null ],
          [ "Exigences", "lot-07-tache-02-modele-niveau.html#autotoc_md535", null ]
        ] ],
        [ "TACHE-03 — Chargement du niveau (JSON)", "lot-07-tache-03-chargement-json.html", [
          [ "Contexte", "lot-07-tache-03-chargement-json.html#autotoc_md536", null ],
          [ "Travail à réaliser", "lot-07-tache-03-chargement-json.html#autotoc_md537", null ],
          [ "Fichiers impactés", "lot-07-tache-03-chargement-json.html#autotoc_md538", null ],
          [ "Tests (obligatoires)", "lot-07-tache-03-chargement-json.html#autotoc_md539", null ],
          [ "Points d'attention", "lot-07-tache-03-chargement-json.html#autotoc_md540", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-03-chargement-json.html#autotoc_md541", null ],
          [ "Exigences", "lot-07-tache-03-chargement-json.html#autotoc_md542", null ]
        ] ],
        [ "TACHE-04 — Validation du niveau", "lot-07-tache-04-validation.html", [
          [ "Contexte", "lot-07-tache-04-validation.html#autotoc_md543", null ],
          [ "Travail à réaliser", "lot-07-tache-04-validation.html#autotoc_md544", null ],
          [ "Fichiers impactés", "lot-07-tache-04-validation.html#autotoc_md545", null ],
          [ "Tests (obligatoires)", "lot-07-tache-04-validation.html#autotoc_md546", null ],
          [ "Points d'attention", "lot-07-tache-04-validation.html#autotoc_md547", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-04-validation.html#autotoc_md548", null ],
          [ "Exigences", "lot-07-tache-04-validation.html#autotoc_md549", null ]
        ] ],
        [ "TACHE-05 — Niveau de démonstration", "lot-07-tache-05-niveau-demo.html", [
          [ "Contexte", "lot-07-tache-05-niveau-demo.html#autotoc_md550", null ],
          [ "Travail à réaliser", "lot-07-tache-05-niveau-demo.html#autotoc_md551", null ],
          [ "Fichiers impactés", "lot-07-tache-05-niveau-demo.html#autotoc_md552", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-05-niveau-demo.html#autotoc_md553", null ],
          [ "Points d'attention", "lot-07-tache-05-niveau-demo.html#autotoc_md554", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-05-niveau-demo.html#autotoc_md555", null ],
          [ "Exigences", "lot-07-tache-05-niveau-demo.html#autotoc_md556", null ]
        ] ],
        [ "TACHE-06 — Rendu du niveau + intégration « Charger niveau »", "lot-07-tache-06-rendu-integration.html", [
          [ "Contexte", "lot-07-tache-06-rendu-integration.html#autotoc_md557", null ],
          [ "Travail à réaliser", "lot-07-tache-06-rendu-integration.html#autotoc_md558", null ],
          [ "Fichiers impactés", "lot-07-tache-06-rendu-integration.html#autotoc_md559", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-06-rendu-integration.html#autotoc_md560", null ],
          [ "Points d'attention", "lot-07-tache-06-rendu-integration.html#autotoc_md561", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-06-rendu-integration.html#autotoc_md562", null ],
          [ "Exigences", "lot-07-tache-06-rendu-integration.html#autotoc_md563", null ]
        ] ]
      ] ],
      [ "LOT-08 — Gameplay personnage : déplacement, gravité et collisions", "lot-08.html", [
        [ "Objectif", "lot-08.html#autotoc_md564", null ],
        [ "Périmètre", "lot-08.html#autotoc_md565", [
          [ "Inclus", "lot-08.html#autotoc_md566", null ],
          [ "Exclus (lots ultérieurs)", "lot-08.html#autotoc_md567", null ]
        ] ],
        [ "Décisions de cadrage", "lot-08.html#autotoc_md568", null ],
        [ "Exigences couvertes", "lot-08.html#autotoc_md569", null ],
        [ "Découpage", "lot-08.html#autotoc_md570", null ],
        [ "Critères d'acceptation du lot", "lot-08.html#autotoc_md571", null ],
        [ "Dépendances", "lot-08.html#autotoc_md572", null ],
        [ "Navigation des tâches", "lot-08.html#autotoc_md573", null ],
        [ "TACHE-01 — Composants du personnage & intention d'entrée", "lot-08-tache-01-composants-personnage.html", [
          [ "Contexte", "lot-08-tache-01-composants-personnage.html#autotoc_md574", null ],
          [ "Travail à réaliser", "lot-08-tache-01-composants-personnage.html#autotoc_md575", null ],
          [ "Fichiers impactés", "lot-08-tache-01-composants-personnage.html#autotoc_md576", null ],
          [ "Tests (obligatoires)", "lot-08-tache-01-composants-personnage.html#autotoc_md577", null ],
          [ "Points d'attention", "lot-08-tache-01-composants-personnage.html#autotoc_md578", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-01-composants-personnage.html#autotoc_md579", null ],
          [ "Exigences", "lot-08-tache-01-composants-personnage.html#autotoc_md580", null ]
        ] ],
        [ "TACHE-02 — Balayage AABB contre la grille (géométrie pure)", "lot-08-tache-02-balayage-aabb.html", [
          [ "Contexte", "lot-08-tache-02-balayage-aabb.html#autotoc_md581", null ],
          [ "Travail à réaliser", "lot-08-tache-02-balayage-aabb.html#autotoc_md582", null ],
          [ "Fichiers impactés", "lot-08-tache-02-balayage-aabb.html#autotoc_md583", null ],
          [ "Tests (obligatoires)", "lot-08-tache-02-balayage-aabb.html#autotoc_md584", null ],
          [ "Points d'attention", "lot-08-tache-02-balayage-aabb.html#autotoc_md585", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-02-balayage-aabb.html#autotoc_md586", null ],
          [ "Exigences", "lot-08-tache-02-balayage-aabb.html#autotoc_md587", null ]
        ] ],
        [ "TACHE-03 — Physique du personnage (gravité + déplacement + collisions)", "lot-08-tache-03-physique-personnage.html", [
          [ "Contexte", "lot-08-tache-03-physique-personnage.html#autotoc_md588", null ],
          [ "Travail à réaliser", "lot-08-tache-03-physique-personnage.html#autotoc_md589", null ],
          [ "Fichiers impactés", "lot-08-tache-03-physique-personnage.html#autotoc_md590", null ],
          [ "Tests (obligatoires)", "lot-08-tache-03-physique-personnage.html#autotoc_md591", null ],
          [ "Points d'attention", "lot-08-tache-03-physique-personnage.html#autotoc_md592", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-03-physique-personnage.html#autotoc_md593", null ],
          [ "Exigences", "lot-08-tache-03-physique-personnage.html#autotoc_md594", null ]
        ] ],
        [ "TACHE-04 — Règles de fin de niveau (succès / échec)", "lot-08-tache-04-regles-fin-niveau.html", [
          [ "Contexte", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md595", null ],
          [ "Travail à réaliser", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md596", null ],
          [ "Fichiers impactés", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md597", null ],
          [ "Tests (obligatoires)", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md598", null ],
          [ "Points d'attention", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md599", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md600", null ],
          [ "Exigences", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md601", null ]
        ] ],
        [ "TACHE-05 — Actions logiques d'entrée (mapping touches → intention)", "lot-08-tache-05-actions-logiques.html", [
          [ "Contexte", "lot-08-tache-05-actions-logiques.html#autotoc_md602", null ],
          [ "Travail à réaliser", "lot-08-tache-05-actions-logiques.html#autotoc_md603", null ],
          [ "Fichiers impactés", "lot-08-tache-05-actions-logiques.html#autotoc_md604", null ],
          [ "Tests (obligatoires)", "lot-08-tache-05-actions-logiques.html#autotoc_md605", null ],
          [ "Points d'attention", "lot-08-tache-05-actions-logiques.html#autotoc_md606", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-05-actions-logiques.html#autotoc_md607", null ],
          [ "Exigences", "lot-08-tache-05-actions-logiques.html#autotoc_md608", null ]
        ] ],
        [ "TACHE-06 — Intégration jouable dans `GameScreen` (cadrage fixe, succès / échec)", "lot-08-tache-06-integration-jouable.html", [
          [ "Contexte", "lot-08-tache-06-integration-jouable.html#autotoc_md609", null ],
          [ "Travail à réaliser", "lot-08-tache-06-integration-jouable.html#autotoc_md610", null ],
          [ "Fichiers impactés", "lot-08-tache-06-integration-jouable.html#autotoc_md611", null ],
          [ "Vérification (visuelle, pas de test unitaire)", "lot-08-tache-06-integration-jouable.html#autotoc_md612", null ],
          [ "Points d'attention", "lot-08-tache-06-integration-jouable.html#autotoc_md613", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-06-integration-jouable.html#autotoc_md614", null ],
          [ "Exigences", "lot-08-tache-06-integration-jouable.html#autotoc_md615", null ]
        ] ]
      ] ],
      [ "LOT-09 — Saut, game feel et enchaînement de niveaux", "lot-09.html", [
        [ "Objectif", "lot-09.html#autotoc_md616", null ],
        [ "Périmètre", "lot-09.html#autotoc_md617", [
          [ "Inclus", "lot-09.html#autotoc_md618", null ],
          [ "Exclus (lots ultérieurs)", "lot-09.html#autotoc_md619", null ]
        ] ],
        [ "Décisions de cadrage", "lot-09.html#autotoc_md620", null ],
        [ "Exigences couvertes", "lot-09.html#autotoc_md621", null ],
        [ "Découpage", "lot-09.html#autotoc_md622", null ],
        [ "Critères d'acceptation du lot", "lot-09.html#autotoc_md623", null ],
        [ "Dépendances", "lot-09.html#autotoc_md624", null ],
        [ "Navigation des tâches", "lot-09.html#autotoc_md625", null ],
        [ "TACHE-01 — Données du saut : `PlayerInput`, `Player`, `PhysicsConfig`", "lot-09-tache-01-donnees-saut.html", [
          [ "Contexte", "lot-09-tache-01-donnees-saut.html#autotoc_md626", null ],
          [ "Travail à réaliser", "lot-09-tache-01-donnees-saut.html#autotoc_md627", null ],
          [ "Fichiers impactés", "lot-09-tache-01-donnees-saut.html#autotoc_md628", null ],
          [ "Tests (obligatoires)", "lot-09-tache-01-donnees-saut.html#autotoc_md629", null ],
          [ "Points d'attention", "lot-09-tache-01-donnees-saut.html#autotoc_md630", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-01-donnees-saut.html#autotoc_md631", null ],
          [ "Exigences", "lot-09-tache-01-donnees-saut.html#autotoc_md632", null ]
        ] ],
        [ "TACHE-02 — Mapping du saut (`Espace`/`W` → intention)", "lot-09-tache-02-mapping-saut.html", [
          [ "Contexte", "lot-09-tache-02-mapping-saut.html#autotoc_md633", null ],
          [ "Travail à réaliser", "lot-09-tache-02-mapping-saut.html#autotoc_md634", null ],
          [ "Fichiers impactés", "lot-09-tache-02-mapping-saut.html#autotoc_md635", null ],
          [ "Tests (obligatoires)", "lot-09-tache-02-mapping-saut.html#autotoc_md636", null ],
          [ "Points d'attention", "lot-09-tache-02-mapping-saut.html#autotoc_md637", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-02-mapping-saut.html#autotoc_md638", null ],
          [ "Exigences", "lot-09-tache-02-mapping-saut.html#autotoc_md639", null ]
        ] ],
        [ "TACHE-03 — Saut au sol + hauteur variable", "lot-09-tache-03-saut-hauteur-variable.html", [
          [ "Contexte", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md640", null ],
          [ "Travail à réaliser", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md641", null ],
          [ "Fichiers impactés", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md642", null ],
          [ "Tests (obligatoires)", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md643", null ],
          [ "Points d'attention", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md644", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md645", null ],
          [ "Exigences", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md646", null ]
        ] ],
        [ "TACHE-04 — Coyote time + jump buffering", "lot-09-tache-04-coyote-buffering.html", [
          [ "Contexte", "lot-09-tache-04-coyote-buffering.html#autotoc_md647", null ],
          [ "Travail à réaliser", "lot-09-tache-04-coyote-buffering.html#autotoc_md648", null ],
          [ "Fichiers impactés", "lot-09-tache-04-coyote-buffering.html#autotoc_md649", null ],
          [ "Tests (obligatoires)", "lot-09-tache-04-coyote-buffering.html#autotoc_md650", null ],
          [ "Points d'attention", "lot-09-tache-04-coyote-buffering.html#autotoc_md651", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-04-coyote-buffering.html#autotoc_md652", null ],
          [ "Exigences", "lot-09-tache-04-coyote-buffering.html#autotoc_md653", null ]
        ] ],
        [ "TACHE-05 — Enchaînement de niveaux (séquence, auto-avance, retour titre)", "lot-09-tache-05-enchainement-niveaux.html", [
          [ "Contexte", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md654", null ],
          [ "Travail à réaliser", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md655", null ],
          [ "Fichiers impactés", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md656", null ],
          [ "Vérification / tests", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md657", null ],
          [ "Points d'attention", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md658", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md659", null ],
          [ "Exigences", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md660", null ]
        ] ],
        [ "TACHE-06 — Niveaux de démo (séquence, dont saut requis) + preuve", "lot-09-tache-06-niveaux-demo.html", [
          [ "Contexte", "lot-09-tache-06-niveaux-demo.html#autotoc_md661", null ],
          [ "Travail à réaliser", "lot-09-tache-06-niveaux-demo.html#autotoc_md662", null ],
          [ "Fichiers impactés", "lot-09-tache-06-niveaux-demo.html#autotoc_md663", null ],
          [ "Tests (obligatoires)", "lot-09-tache-06-niveaux-demo.html#autotoc_md664", null ],
          [ "Points d'attention", "lot-09-tache-06-niveaux-demo.html#autotoc_md665", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-06-niveaux-demo.html#autotoc_md666", null ],
          [ "Exigences", "lot-09-tache-06-niveaux-demo.html#autotoc_md667", null ]
        ] ]
      ] ],
      [ "LOT-10 — Mécaniques aériennes avancées : double saut, wall jump, dash", "lot-10.html", [
        [ "Objectif", "lot-10.html#autotoc_md668", null ],
        [ "Périmètre", "lot-10.html#autotoc_md669", [
          [ "Inclus", "lot-10.html#autotoc_md670", null ],
          [ "Exclus (lots ultérieurs)", "lot-10.html#autotoc_md671", null ]
        ] ],
        [ "Décisions de cadrage", "lot-10.html#autotoc_md672", null ],
        [ "Exigences couvertes", "lot-10.html#autotoc_md673", null ],
        [ "Découpage", "lot-10.html#autotoc_md674", null ],
        [ "Critères d'acceptation du lot", "lot-10.html#autotoc_md675", null ],
        [ "Dépendances", "lot-10.html#autotoc_md676", null ],
        [ "Navigation des tâches", "lot-10.html#autotoc_md677", null ],
        [ "TACHE-01 — Données des mécaniques (`PlayerInput`, `Player`, `PhysicsConfig`)", "lot-10-tache-01-donnees.html", [
          [ "Contexte", "lot-10-tache-01-donnees.html#autotoc_md678", null ],
          [ "Travail à réaliser", "lot-10-tache-01-donnees.html#autotoc_md679", null ],
          [ "Fichiers impactés", "lot-10-tache-01-donnees.html#autotoc_md680", null ],
          [ "Tests (obligatoires)", "lot-10-tache-01-donnees.html#autotoc_md681", null ],
          [ "Points d'attention", "lot-10-tache-01-donnees.html#autotoc_md682", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-01-donnees.html#autotoc_md683", null ],
          [ "Exigences", "lot-10-tache-01-donnees.html#autotoc_md684", null ]
        ] ],
        [ "TACHE-02 — Mapping du dash + direction de visée / orientation", "lot-10-tache-02-mapping-dash.html", [
          [ "Contexte", "lot-10-tache-02-mapping-dash.html#autotoc_md685", null ],
          [ "Travail à réaliser", "lot-10-tache-02-mapping-dash.html#autotoc_md686", null ],
          [ "Fichiers impactés", "lot-10-tache-02-mapping-dash.html#autotoc_md687", null ],
          [ "Tests (obligatoires)", "lot-10-tache-02-mapping-dash.html#autotoc_md688", null ],
          [ "Points d'attention", "lot-10-tache-02-mapping-dash.html#autotoc_md689", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-02-mapping-dash.html#autotoc_md690", null ],
          [ "Exigences", "lot-10-tache-02-mapping-dash.html#autotoc_md691", null ]
        ] ],
        [ "TACHE-03 — Double saut (sauts aériens rechargés au sol)", "lot-10-tache-03-double-saut.html", [
          [ "Contexte", "lot-10-tache-03-double-saut.html#autotoc_md692", null ],
          [ "Travail à réaliser", "lot-10-tache-03-double-saut.html#autotoc_md693", null ],
          [ "Fichiers impactés", "lot-10-tache-03-double-saut.html#autotoc_md694", null ],
          [ "Tests (obligatoires)", "lot-10-tache-03-double-saut.html#autotoc_md695", null ],
          [ "Points d'attention", "lot-10-tache-03-double-saut.html#autotoc_md696", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-03-double-saut.html#autotoc_md697", null ],
          [ "Exigences", "lot-10-tache-03-double-saut.html#autotoc_md698", null ]
        ] ],
        [ "TACHE-04 — Wall jump + wall slide", "lot-10-tache-04-wall-jump.html", [
          [ "Contexte", "lot-10-tache-04-wall-jump.html#autotoc_md699", null ],
          [ "Travail à réaliser", "lot-10-tache-04-wall-jump.html#autotoc_md700", null ],
          [ "Fichiers impactés", "lot-10-tache-04-wall-jump.html#autotoc_md701", null ],
          [ "Tests (obligatoires)", "lot-10-tache-04-wall-jump.html#autotoc_md702", null ],
          [ "Points d'attention", "lot-10-tache-04-wall-jump.html#autotoc_md703", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-04-wall-jump.html#autotoc_md704", null ],
          [ "Exigences", "lot-10-tache-04-wall-jump.html#autotoc_md705", null ]
        ] ],
        [ "TACHE-05 — Dash 8 directions (burst, durée, recharge au sol)", "lot-10-tache-05-dash.html", [
          [ "Contexte", "lot-10-tache-05-dash.html#autotoc_md706", null ],
          [ "Travail à réaliser", "lot-10-tache-05-dash.html#autotoc_md707", null ],
          [ "Fichiers impactés", "lot-10-tache-05-dash.html#autotoc_md708", null ],
          [ "Tests (obligatoires)", "lot-10-tache-05-dash.html#autotoc_md709", null ],
          [ "Points d'attention", "lot-10-tache-05-dash.html#autotoc_md710", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-05-dash.html#autotoc_md711", null ],
          [ "Exigences", "lot-10-tache-05-dash.html#autotoc_md712", null ]
        ] ],
        [ "TACHE-06 — Niveau de démo « parkour » + preuve système", "lot-10-tache-06-niveau-parkour.html", [
          [ "Contexte", "lot-10-tache-06-niveau-parkour.html#autotoc_md713", null ],
          [ "Travail à réaliser", "lot-10-tache-06-niveau-parkour.html#autotoc_md714", null ],
          [ "Fichiers impactés", "lot-10-tache-06-niveau-parkour.html#autotoc_md715", null ],
          [ "Tests (obligatoires)", "lot-10-tache-06-niveau-parkour.html#autotoc_md716", null ],
          [ "Points d'attention", "lot-10-tache-06-niveau-parkour.html#autotoc_md717", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-06-niveau-parkour.html#autotoc_md718", null ],
          [ "Exigences", "lot-10-tache-06-niveau-parkour.html#autotoc_md719", null ]
        ] ]
      ] ],
      [ "LOT-11 — Ressenti avancé : personnage humanoïde, gravité asymétrique, finitions", "lot-11.html", [
        [ "Objectif", "lot-11.html#autotoc_md720", null ],
        [ "Périmètre", "lot-11.html#autotoc_md721", [
          [ "Inclus", "lot-11.html#autotoc_md722", null ],
          [ "Exclus (lots ultérieurs)", "lot-11.html#autotoc_md723", null ]
        ] ],
        [ "Décisions de cadrage", "lot-11.html#autotoc_md724", null ],
        [ "Exigences couvertes", "lot-11.html#autotoc_md725", null ],
        [ "Découpage", "lot-11.html#autotoc_md726", null ],
        [ "Critères d'acceptation du lot", "lot-11.html#autotoc_md727", null ],
        [ "Dépendances", "lot-11.html#autotoc_md728", null ],
        [ "Navigation des tâches", "lot-11.html#autotoc_md729", null ],
        [ "TACHE-01 — Données : réglages de *feel* + taille/placement du personnage", "lot-11-tache-01-donnees.html", [
          [ "Contexte", "lot-11-tache-01-donnees.html#autotoc_md730", null ],
          [ "Travail à réaliser", "lot-11-tache-01-donnees.html#autotoc_md731", null ],
          [ "Fichiers impactés", "lot-11-tache-01-donnees.html#autotoc_md732", null ],
          [ "Tests (obligatoires)", "lot-11-tache-01-donnees.html#autotoc_md733", null ],
          [ "Points d'attention", "lot-11-tache-01-donnees.html#autotoc_md734", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-01-donnees.html#autotoc_md735", null ],
          [ "Exigences", "lot-11-tache-01-donnees.html#autotoc_md736", null ]
        ] ],
        [ "TACHE-02 — Gravité asymétrique + apex hang + fast-fall", "lot-11-tache-02-gravite-asymetrique.html", [
          [ "Contexte", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md737", null ],
          [ "Travail à réaliser", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md738", null ],
          [ "Fichiers impactés", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md739", null ],
          [ "Tests (obligatoires)", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md740", null ],
          [ "Points d'attention", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md741", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md742", null ],
          [ "Exigences", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md743", null ]
        ] ],
        [ "TACHE-03 — Personnage humanoïde (spawn 0,4×0,8, sprite)", "lot-11-tache-03-personnage-humanoide.html", [
          [ "Contexte", "lot-11-tache-03-personnage-humanoide.html#autotoc_md744", null ],
          [ "Travail à réaliser", "lot-11-tache-03-personnage-humanoide.html#autotoc_md745", null ],
          [ "Fichiers impactés", "lot-11-tache-03-personnage-humanoide.html#autotoc_md746", null ],
          [ "Vérification (visuelle, pas de test unitaire — brique GPU)", "lot-11-tache-03-personnage-humanoide.html#autotoc_md747", null ],
          [ "Points d'attention", "lot-11-tache-03-personnage-humanoide.html#autotoc_md748", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-03-personnage-humanoide.html#autotoc_md749", null ],
          [ "Exigences", "lot-11-tache-03-personnage-humanoide.html#autotoc_md750", null ]
        ] ],
        [ "TACHE-04 — Rééquilibrage des niveaux + preuves à la vraie taille", "lot-11-tache-04-reequilibrage.html", [
          [ "Contexte", "lot-11-tache-04-reequilibrage.html#autotoc_md751", null ],
          [ "Travail à réaliser", "lot-11-tache-04-reequilibrage.html#autotoc_md752", null ],
          [ "Fichiers impactés", "lot-11-tache-04-reequilibrage.html#autotoc_md753", null ],
          [ "Tests (obligatoires)", "lot-11-tache-04-reequilibrage.html#autotoc_md754", null ],
          [ "Points d'attention", "lot-11-tache-04-reequilibrage.html#autotoc_md755", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-04-reequilibrage.html#autotoc_md756", null ],
          [ "Exigences", "lot-11-tache-04-reequilibrage.html#autotoc_md757", null ]
        ] ]
      ] ],
      [ "LOT-12 — Niveau puzzle : mécanismes interrupteur/porte + budget de mouvements", "lot-12.html", [
        [ "Objectif", "lot-12.html#autotoc_md758", null ],
        [ "Périmètre", "lot-12.html#autotoc_md759", [
          [ "Inclus", "lot-12.html#autotoc_md760", null ],
          [ "Exclus (lots ultérieurs)", "lot-12.html#autotoc_md761", null ]
        ] ],
        [ "Décisions de cadrage", "lot-12.html#autotoc_md762", null ],
        [ "Exigences couvertes", "lot-12.html#autotoc_md763", null ],
        [ "Découpage", "lot-12.html#autotoc_md764", null ],
        [ "Critères d'acceptation du lot", "lot-12.html#autotoc_md765", null ],
        [ "Dépendances", "lot-12.html#autotoc_md766", null ],
        [ "Navigation des tâches", "lot-12.html#autotoc_md767", null ],
        [ "TACHE-01 — Données : budget (`Player`, `Level`, `LevelLoader`)", "lot-12-tache-01-donnees.html", [
          [ "Contexte", "lot-12-tache-01-donnees.html#autotoc_md768", null ],
          [ "Travail à réaliser", "lot-12-tache-01-donnees.html#autotoc_md769", null ],
          [ "Fichiers impactés", "lot-12-tache-01-donnees.html#autotoc_md770", null ],
          [ "Tests (obligatoires)", "lot-12-tache-01-donnees.html#autotoc_md771", null ],
          [ "Points d'attention", "lot-12-tache-01-donnees.html#autotoc_md772", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-01-donnees.html#autotoc_md773", null ],
          [ "Exigences", "lot-12-tache-01-donnees.html#autotoc_md774", null ]
        ] ],
        [ "TACHE-02 — Mécanismes interrupteur/porte (`MechanismController`)", "lot-12-tache-02-mecanismes.html", [
          [ "Contexte", "lot-12-tache-02-mecanismes.html#autotoc_md775", null ],
          [ "Travail à réaliser", "lot-12-tache-02-mecanismes.html#autotoc_md776", null ],
          [ "Fichiers impactés", "lot-12-tache-02-mecanismes.html#autotoc_md777", null ],
          [ "Tests (obligatoires)", "lot-12-tache-02-mecanismes.html#autotoc_md778", null ],
          [ "Points d'attention", "lot-12-tache-02-mecanismes.html#autotoc_md779", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-02-mecanismes.html#autotoc_md780", null ],
          [ "Exigences", "lot-12-tache-02-mecanismes.html#autotoc_md781", null ]
        ] ],
        [ "TACHE-03 — Budget de sauts/dashs dans la physique", "lot-12-tache-03-budget.html", [
          [ "Contexte", "lot-12-tache-03-budget.html#autotoc_md782", null ],
          [ "Travail à réaliser", "lot-12-tache-03-budget.html#autotoc_md783", null ],
          [ "Fichiers impactés", "lot-12-tache-03-budget.html#autotoc_md784", null ],
          [ "Tests (obligatoires)", "lot-12-tache-03-budget.html#autotoc_md785", null ],
          [ "Points d'attention", "lot-12-tache-03-budget.html#autotoc_md786", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-03-budget.html#autotoc_md787", null ],
          [ "Exigences", "lot-12-tache-03-budget.html#autotoc_md788", null ]
        ] ],
        [ "TACHE-04 — Intégration `GameScreen` + niveau `demo4` + preuves", "lot-12-tache-04-integration-puzzle.html", [
          [ "Contexte", "lot-12-tache-04-integration-puzzle.html#autotoc_md789", null ],
          [ "Travail à réaliser", "lot-12-tache-04-integration-puzzle.html#autotoc_md790", null ],
          [ "Fichiers impactés", "lot-12-tache-04-integration-puzzle.html#autotoc_md791", null ],
          [ "Tests (obligatoires)", "lot-12-tache-04-integration-puzzle.html#autotoc_md792", null ],
          [ "Points d'attention", "lot-12-tache-04-integration-puzzle.html#autotoc_md793", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-04-integration-puzzle.html#autotoc_md794", null ],
          [ "Exigences", "lot-12-tache-04-integration-puzzle.html#autotoc_md795", null ]
        ] ]
      ] ],
      [ "LOT-13 — Consolidation de la documentation", "lot-13.html", [
        [ "Objectif", "lot-13.html#autotoc_md796", null ],
        [ "Périmètre", "lot-13.html#autotoc_md797", [
          [ "Inclus", "lot-13.html#autotoc_md798", null ],
          [ "Exclus", "lot-13.html#autotoc_md799", null ]
        ] ],
        [ "Décisions de cadrage", "lot-13.html#autotoc_md800", null ],
        [ "Exigences couvertes", "lot-13.html#autotoc_md801", null ],
        [ "Découpage", "lot-13.html#autotoc_md802", null ],
        [ "Critères d'acceptation du lot", "lot-13.html#autotoc_md803", null ],
        [ "Dépendances", "lot-13.html#autotoc_md804", null ]
      ] ],
      [ "LOT-14 — Éditeur de niveaux intégré : édition de tuiles, mécanismes, essai immédiat", "lot-14.html", [
        [ "Objectif", "lot-14.html#autotoc_md805", null ],
        [ "Périmètre", "lot-14.html#autotoc_md806", [
          [ "Inclus", "lot-14.html#autotoc_md807", null ],
          [ "Exclus (lots ultérieurs)", "lot-14.html#autotoc_md808", null ]
        ] ],
        [ "Décisions de cadrage", "lot-14.html#autotoc_md809", null ],
        [ "Exigences couvertes", "lot-14.html#autotoc_md810", null ],
        [ "Découpage", "lot-14.html#autotoc_md811", null ],
        [ "Critères d'acceptation du lot", "lot-14.html#autotoc_md812", null ],
        [ "Dépendances", "lot-14.html#autotoc_md813", null ],
        [ "Navigation des tâches", "lot-14.html#autotoc_md814", null ],
        [ "TACHE-01 — Sérialisation JSON + modèle d'édition mutable", "lot-14-tache-01-serialisation-modele-edition.html", [
          [ "Contexte", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md815", null ],
          [ "Travail à réaliser", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md816", null ],
          [ "Fichiers impactés", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md817", null ],
          [ "Tests (obligatoires)", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md818", null ],
          [ "Points d'attention", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md819", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md820", null ],
          [ "Exigences", "lot-14-tache-01-serialisation-modele-edition.html#autotoc_md821", null ]
        ] ],
        [ "TACHE-02 — Écran éditeur : grille cliquable + palette de tuiles", "lot-14-tache-02-ecran-editeur-palette.html", [
          [ "Contexte", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md822", null ],
          [ "Travail à réaliser", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md823", null ],
          [ "Fichiers impactés", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md824", null ],
          [ "Tests (obligatoires)", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md825", null ],
          [ "Points d'attention", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md826", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md827", null ],
          [ "Exigences", "lot-14-tache-02-ecran-editeur-palette.html#autotoc_md828", null ]
        ] ],
        [ "TACHE-03 — Entrée/sortie, liaison de mécanismes, redimensionnement", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html", [
          [ "Contexte", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md829", null ],
          [ "Travail à réaliser", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md830", null ],
          [ "Fichiers impactés", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md831", null ],
          [ "Tests (obligatoires)", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md832", null ],
          [ "Points d'attention", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md833", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md834", null ],
          [ "Exigences", "lot-14-tache-03-entree-sortie-mecanismes-redimension.html#autotoc_md835", null ]
        ] ],
        [ "TACHE-04 — Historique annuler/refaire", "lot-14-tache-04-undo-redo.html", [
          [ "Contexte", "lot-14-tache-04-undo-redo.html#autotoc_md836", null ],
          [ "Travail à réaliser", "lot-14-tache-04-undo-redo.html#autotoc_md837", null ],
          [ "Fichiers impactés", "lot-14-tache-04-undo-redo.html#autotoc_md838", null ],
          [ "Tests (obligatoires)", "lot-14-tache-04-undo-redo.html#autotoc_md839", null ],
          [ "Points d'attention", "lot-14-tache-04-undo-redo.html#autotoc_md840", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-04-undo-redo.html#autotoc_md841", null ],
          [ "Exigences", "lot-14-tache-04-undo-redo.html#autotoc_md842", null ]
        ] ],
        [ "TACHE-05 — Enregistrement, validation, essai immédiat", "lot-14-tache-05-enregistrement-validation-essai.html", [
          [ "Contexte", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md843", null ],
          [ "Travail à réaliser", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md844", null ],
          [ "Fichiers impactés", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md845", null ],
          [ "Tests (obligatoires)", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md846", null ],
          [ "Points d'attention", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md847", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md848", null ],
          [ "Exigences", "lot-14-tache-05-enregistrement-validation-essai.html#autotoc_md849", null ]
        ] ],
        [ "TACHE-06 — Intégration menu, tests système, guide non-codeur Git", "lot-14-tache-06-integration-guide-non-codeur.html", [
          [ "Contexte", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md850", null ],
          [ "Travail à réaliser", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md851", null ],
          [ "Fichiers impactés", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md852", null ],
          [ "Tests (obligatoires)", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md853", null ],
          [ "Points d'attention", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md854", null ],
          [ "Définition de fait (DoD)", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md855", null ],
          [ "Exigences", "lot-14-tache-06-integration-guide-non-codeur.html#autotoc_md856", null ]
        ] ]
      ] ],
      [ "LOT-15 — Éditeur de niveaux : robustesse et confort d'édition", "lot-15.html", [
        [ "Objectif", "lot-15.html#autotoc_md857", null ],
        [ "Périmètre", "lot-15.html#autotoc_md858", [
          [ "Inclus", "lot-15.html#autotoc_md859", null ],
          [ "Exclus (lots ultérieurs ou non retenus)", "lot-15.html#autotoc_md860", null ]
        ] ],
        [ "Décisions de cadrage", "lot-15.html#autotoc_md861", null ],
        [ "Exigences couvertes", "lot-15.html#autotoc_md862", null ],
        [ "Découpage", "lot-15.html#autotoc_md863", null ],
        [ "Critères d'acceptation du lot", "lot-15.html#autotoc_md864", null ],
        [ "Dépendances", "lot-15.html#autotoc_md865", null ],
        [ "Navigation des tâches", "lot-15.html#autotoc_md866", null ],
        [ "TACHE-01 — Entrées bas niveau : molette et texte tapé", "lot-15-tache-01-entrees-molette-texte.html", [
          [ "Contexte", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md867", null ],
          [ "Travail à réaliser", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md868", null ],
          [ "Fichiers impactés", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md869", null ],
          [ "Tests (obligatoires)", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md870", null ],
          [ "Points d'attention", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md871", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md872", null ],
          [ "Exigences", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md873", null ],
          [ "Ajustement post-livraison (essai utilisateur)", "lot-15-tache-01-entrees-molette-texte.html#autotoc_md874", null ]
        ] ],
        [ "TACHE-02 — Garde-fous : redimensionnement destructeur, quitter sans enregistrer", "lot-15-tache-02-garde-fous-perte-donnees.html", [
          [ "Contexte", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md875", null ],
          [ "Travail à réaliser", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md876", null ],
          [ "Fichiers impactés", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md877", null ],
          [ "Tests (obligatoires)", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md878", null ],
          [ "Points d'attention", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md879", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md880", null ],
          [ "Exigences", "lot-15-tache-02-garde-fous-perte-donnees.html#autotoc_md881", null ]
        ] ],
        [ "TACHE-03 — Nommage, renommage, avertissement d'écrasement", "lot-15-tache-03-nommage-renommage.html", [
          [ "Contexte", "lot-15-tache-03-nommage-renommage.html#autotoc_md882", null ],
          [ "Travail à réaliser", "lot-15-tache-03-nommage-renommage.html#autotoc_md883", null ],
          [ "Fichiers impactés", "lot-15-tache-03-nommage-renommage.html#autotoc_md884", null ],
          [ "Tests (obligatoires)", "lot-15-tache-03-nommage-renommage.html#autotoc_md885", null ],
          [ "Points d'attention", "lot-15-tache-03-nommage-renommage.html#autotoc_md886", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-03-nommage-renommage.html#autotoc_md887", null ],
          [ "Exigences", "lot-15-tache-03-nommage-renommage.html#autotoc_md888", null ]
        ] ],
        [ "TACHE-04 — Caméra : pan et zoom manuels", "lot-15-tache-04-camera-pan-zoom.html", [
          [ "Contexte", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md889", null ],
          [ "Travail à réaliser", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md890", null ],
          [ "Fichiers impactés", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md891", null ],
          [ "Tests (obligatoires)", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md892", null ],
          [ "Points d'attention", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md893", null ],
          [ "Ajustement post-livraison (essai utilisateur)", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md894", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md895", null ],
          [ "Exigences", "lot-15-tache-04-camera-pan-zoom.html#autotoc_md896", null ]
        ] ],
        [ "TACHE-05 — Outils de zone : remplissage rectangulaire, sélection, copier/coller", "lot-15-tache-05-outils-rectangle-selection.html", [
          [ "Contexte", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md897", null ],
          [ "Travail à réaliser", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md898", null ],
          [ "Fichiers impactés", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md899", null ],
          [ "Tests (obligatoires)", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md900", null ],
          [ "Points d'attention", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md901", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md902", null ],
          [ "Exigences", "lot-15-tache-05-outils-rectangle-selection.html#autotoc_md903", null ]
        ] ],
        [ "TACHE-06 — Découvrabilité : barre d'outils, aide, libellés, liaisons lisibles", "lot-15-tache-06-decouvrabilite.html", [
          [ "Contexte", "lot-15-tache-06-decouvrabilite.html#autotoc_md904", null ],
          [ "Travail à réaliser", "lot-15-tache-06-decouvrabilite.html#autotoc_md905", null ],
          [ "Fichiers impactés", "lot-15-tache-06-decouvrabilite.html#autotoc_md906", null ],
          [ "Tests (obligatoires)", "lot-15-tache-06-decouvrabilite.html#autotoc_md907", null ],
          [ "Points d'attention", "lot-15-tache-06-decouvrabilite.html#autotoc_md908", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-06-decouvrabilite.html#autotoc_md909", null ],
          [ "Exigences", "lot-15-tache-06-decouvrabilite.html#autotoc_md910", null ],
          [ "Ajustement post-livraison (essai utilisateur)", "lot-15-tache-06-decouvrabilite.html#autotoc_md911", null ]
        ] ],
        [ "TACHE-07 — Essai immédiat en mémoire, erreurs de validation structurées", "lot-15-tache-07-essai-memoire-erreurs-structurees.html", [
          [ "Contexte", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md912", null ],
          [ "Travail à réaliser", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md913", null ],
          [ "Fichiers impactés", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md914", null ],
          [ "Tests (obligatoires)", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md915", null ],
          [ "Points d'attention", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md916", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md917", null ],
          [ "Exigences", "lot-15-tache-07-essai-memoire-erreurs-structurees.html#autotoc_md918", null ]
        ] ],
        [ "TACHE-08 — Nettoyage documentaire", "lot-15-tache-08-nettoyage-documentation.html", [
          [ "Contexte", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md919", null ],
          [ "Travail à réaliser", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md920", null ],
          [ "Fichiers impactés", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md921", null ],
          [ "Tests (obligatoires)", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md922", null ],
          [ "Points d'attention", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md923", null ],
          [ "Définition de fait (DoD)", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md924", null ],
          [ "Exigences", "lot-15-tache-08-nettoyage-documentation.html#autotoc_md925", null ]
        ] ]
      ] ]
    ] ],
    [ "Manuel utilisateur", "manuel.html", [
      [ "Pages", "manuel.html#autotoc_md927", null ],
      [ "À venir", "manuel.html#autotoc_md928", null ],
      [ "Télécharger et lancer le jeu", "manuel-telecharger.html", [
        [ "Prérequis", "manuel-telecharger.html#autotoc_md935", null ],
        [ "Étapes", "manuel-telecharger.html#autotoc_md936", null ],
        [ "Remarques", "manuel-telecharger.html#autotoc_md937", null ]
      ] ],
      [ "Créer et partager un niveau (sans ligne de commande)", "manuel-partager-niveau.html", [
        [ "1. Récupérer le projet", "manuel-partager-niveau.html#autotoc_md929", null ],
        [ "2. Lancer l'éditeur", "manuel-partager-niveau.html#autotoc_md930", null ],
        [ "3. Créer un niveau", "manuel-partager-niveau.html#autotoc_md931", null ],
        [ "4. Publier votre niveau", "manuel-partager-niveau.html#autotoc_md932", null ],
        [ "5. Récupérer les niveaux des autres", "manuel-partager-niveau.html#autotoc_md933", null ],
        [ "En cas de problème", "manuel-partager-niveau.html#autotoc_md934", null ]
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
"MenuScreen_8h.html",
"classcore_1_1Level.html#a6222b34ab7079f9ce8044892436e5313",
"classcore_1_1World.html#acee09d7b00169a19e1bd2d9247b6bce5",
"classhmi_1_1GraphicsDevice.html#a41a7bece5d4ff3bf2e1df97c669346ef",
"classhmi_1_1SaveIcon.html#a19ae609a8e30fcd6fef1ec4e1ffeb955",
"dir_7c7356e692772728ab91d19f773c60c4.html",
"lot-01-tache-02-init-direct3d11.html#autotoc_md206",
"lot-05-tache-04-camera-2d.html#autotoc_md428",
"lot-09-tache-04-coyote-buffering.html#autotoc_md648",
"lot-15-tache-01-entrees-molette-texte.html#autotoc_md869",
"spec-conventions.html#autotoc_md137",
"structcore_1_1Rect.html#af4677180f48fd136bc50d3e98cd99169",
"test__input__state_8cpp.html#a45cf3dc465b7f8ead7c72522f7ac8198",
"test__tile__palette_8cpp.html#a177a49a311fc252f8c49e0530e2c475b"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
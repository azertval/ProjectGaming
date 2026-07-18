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
      [ "Comment lire ce guide", "guide.html#autotoc_md34", null ],
      [ "Architecture en deux couches", "guide.html#autotoc_md35", null ],
      [ "Plan du guide", "guide.html#autotoc_md36", null ],
      [ "Boucle de jeu et pas de temps fixe", "guide-boucle.html", [
        [ "Pourquoi un pas de temps fixe", "guide-boucle.html#autotoc_md4", null ],
        [ "Fonctionnement", "guide-boucle.html#autotoc_md5", null ],
        [ "Voir aussi", "guide-boucle.html#autotoc_md6", null ]
      ] ],
      [ "ECS : entités, composants, systèmes", "guide-ecs.html", [
        [ "Le <tt>World</tt>", "guide-ecs.html#autotoc_md7", null ],
        [ "Sparse sets et vues", "guide-ecs.html#autotoc_md8", null ],
        [ "Voir aussi", "guide-ecs.html#autotoc_md9", null ]
      ] ],
      [ "Mathématiques du moteur", "guide-maths.html", [
        [ "<tt>Vector2</tt>", "guide-maths.html#autotoc_md13", null ],
        [ "Boîtes englobantes : <tt>Aabb</tt>", "guide-maths.html#autotoc_md14", null ],
        [ "Conventions d'unités et de repère", "guide-maths.html#autotoc_md15", null ],
        [ "Comparaison flottante", "guide-maths.html#autotoc_md16", null ],
        [ "Voir aussi", "guide-maths.html#autotoc_md17", null ]
      ] ],
      [ "Physique du personnage", "guide-physique.html", [
        [ "1. Collision par balayage continu (swept AABB)", "guide-physique.html#autotoc_md25", [
          [ "Pourquoi « continu »", "guide-physique.html#autotoc_md26", null ],
          [ "Méthode retenue : balayage <strong>par axe</strong> avec clamp direct", "guide-physique.html#autotoc_md27", null ]
        ] ],
        [ "2. Gravité et intégration", "guide-physique.html#autotoc_md28", null ],
        [ "3. Saut et <em>game feel</em>", "guide-physique.html#autotoc_md29", null ],
        [ "4. Dash 8 directions", "guide-physique.html#autotoc_md30", null ],
        [ "5. Wall jump et wall slide", "guide-physique.html#autotoc_md31", null ],
        [ "Ordre d'un pas (résumé)", "guide-physique.html#autotoc_md32", null ],
        [ "Voir aussi", "guide-physique.html#autotoc_md33", null ]
      ] ],
      [ "Niveaux : modèle, chargement, mécanismes, budgets", "guide-niveaux.html", [
        [ "Modèle", "guide-niveaux.html#autotoc_md18", null ],
        [ "Chargement JSON", "guide-niveaux.html#autotoc_md19", null ],
        [ "De la grille aux entités : <tt>buildLevelScene</tt>", "guide-niveaux.html#autotoc_md20", null ],
        [ "Mécanismes interrupteur ↔ porte", "guide-niveaux.html#autotoc_md21", null ],
        [ "Budget de mouvements", "guide-niveaux.html#autotoc_md22", null ],
        [ "Issue et enchaînement", "guide-niveaux.html#autotoc_md23", null ],
        [ "Voir aussi", "guide-niveaux.html#autotoc_md24", null ]
      ] ],
      [ "Entrées et actions logiques", "guide-entrees.html", [
        [ "Échantillonnage : <tt>InputState</tt>", "guide-entrees.html#autotoc_md10", null ],
        [ "Traduction : <tt>toPlayerInput</tt>", "guide-entrees.html#autotoc_md11", null ],
        [ "Voir aussi", "guide-entrees.html#autotoc_md12", null ]
      ] ]
    ] ],
    [ "Spécifications", "specifications.html", [
      [ "Documents", "specifications.html#autotoc_md113", null ],
      [ "Vision & périmètre", "spec-vision.html", [
        [ "Concept", "spec-vision.html#autotoc_md114", [
          [ "Mécanique de jeu (décidée)", "spec-vision.html#autotoc_md115", null ]
        ] ],
        [ "Boucle de gameplay", "spec-vision.html#autotoc_md116", null ],
        [ "Objectifs (MVP)", "spec-vision.html#autotoc_md117", null ],
        [ "Objectifs produit (au-delà du moteur)", "spec-vision.html#autotoc_md118", null ],
        [ "Hors périmètre (MVP)", "spec-vision.html#autotoc_md119", null ],
        [ "Traçabilité", "spec-vision.html#autotoc_md120", null ]
      ] ],
      [ "Gameplay", "spec-gameplay.html", [
        [ "1. Monde en tuiles", "spec-gameplay.html#autotoc_md94", null ],
        [ "2. Personnage & déplacement", "spec-gameplay.html#autotoc_md95", [
          [ "Mécaniques aériennes avancées (au-delà du MVP)", "spec-gameplay.html#autotoc_md96", null ],
          [ "Ressenti (game feel) — ⚠️ à affiner par tests", "spec-gameplay.html#autotoc_md97", null ]
        ] ],
        [ "3. Mécanismes de puzzle", "spec-gameplay.html#autotoc_md98", null ],
        [ "4. Conditions de fin de niveau", "spec-gameplay.html#autotoc_md99", null ],
        [ "5. États de jeu", "spec-gameplay.html#autotoc_md100", null ],
        [ "Traçabilité", "spec-gameplay.html#autotoc_md101", null ]
      ] ],
      [ "Contrôles & entrées", "spec-controles.html", [
        [ "1. Périphériques", "spec-controles.html#autotoc_md49", null ],
        [ "2. Actions du jeu (mapping logique)", "spec-controles.html#autotoc_md50", null ],
        [ "3. Réactivité", "spec-controles.html#autotoc_md51", null ],
        [ "Traçabilité", "spec-controles.html#autotoc_md52", null ]
      ] ],
      [ "Rendu & cible technique", "spec-rendu-technique.html", [
        [ "1. Cible technique", "spec-rendu-technique.html#autotoc_md107", null ],
        [ "2. Rendu 2D", "spec-rendu-technique.html#autotoc_md108", null ],
        [ "3. Boucle & temps", "spec-rendu-technique.html#autotoc_md109", null ],
        [ "4. Interface (HMI)", "spec-rendu-technique.html#autotoc_md110", null ],
        [ "5. Audio (⚠️ minimal MVP)", "spec-rendu-technique.html#autotoc_md111", null ],
        [ "Traçabilité", "spec-rendu-technique.html#autotoc_md112", null ]
      ] ],
      [ "Niveaux & contenu", "spec-niveaux.html", [
        [ "1. Représentation des niveaux", "spec-niveaux.html#autotoc_md102", [
          [ "Format retenu (JSON, liste de tuiles-objets)", "spec-niveaux.html#autotoc_md103", null ]
        ] ],
        [ "2. Progression", "spec-niveaux.html#autotoc_md104", null ],
        [ "3. Conception (lignes directrices)", "spec-niveaux.html#autotoc_md105", null ],
        [ "Traçabilité", "spec-niveaux.html#autotoc_md106", null ]
      ] ],
      [ "Exigences non fonctionnelles", "spec-exigences.html", [
        [ "1. Performance", "spec-exigences.html#autotoc_md88", null ],
        [ "2. Architecture & maintenabilité", "spec-exigences.html#autotoc_md89", null ],
        [ "3. Qualité & vérification", "spec-exigences.html#autotoc_md90", null ],
        [ "4. Portabilité & reproductibilité", "spec-exigences.html#autotoc_md91", null ],
        [ "5. Robustesse", "spec-exigences.html#autotoc_md92", null ],
        [ "Traçabilité", "spec-exigences.html#autotoc_md93", null ]
      ] ],
      [ "Éditeur de niveaux", "spec-editeur.html", [
        [ "Objectif", "spec-editeur.html#autotoc_md80", null ],
        [ "1. Exigences fonctionnelles", "spec-editeur.html#autotoc_md81", null ],
        [ "2. Réutilisation & cohérence", "spec-editeur.html#autotoc_md82", null ],
        [ "3. Distribution & collaboration", "spec-editeur.html#autotoc_md83", null ],
        [ "4. Approche d'implémentation (décidée)", "spec-editeur.html#autotoc_md84", null ],
        [ "4bis. Décors & pixel art (post-MVP, intégré à l'éditeur)", "spec-editeur.html#autotoc_md85", null ],
        [ "5. Non-objectifs (éditeur, MVP)", "spec-editeur.html#autotoc_md86", null ],
        [ "Traçabilité", "spec-editeur.html#autotoc_md87", null ]
      ] ],
      [ "Architecture (décisions dimensionnantes)", "spec-architecture.html", [
        [ "1. Modules & dépendances", "spec-architecture.html#autotoc_md37", null ],
        [ "2. Modèle d'entités : ECS", "spec-architecture.html#autotoc_md38", null ],
        [ "3. Coordonnées & unités — trois espaces distincts", "spec-architecture.html#autotoc_md39", null ],
        [ "4. Frontière simulation ↔ rendu", "spec-architecture.html#autotoc_md40", null ],
        [ "5. Mathématiques dans <tt>Core</tt>", "spec-architecture.html#autotoc_md41", null ],
        [ "6. Abstraction de rendu", "spec-architecture.html#autotoc_md42", null ],
        [ "7. Modèle de threading", "spec-architecture.html#autotoc_md43", null ],
        [ "8. Communication inter-systèmes", "spec-architecture.html#autotoc_md44", null ],
        [ "9. Gestion des ressources", "spec-architecture.html#autotoc_md45", null ],
        [ "10. Contrainte « éditeur intégré »", "spec-architecture.html#autotoc_md46", null ],
        [ "11. Décors dynamiques (accommodation dimensionnante)", "spec-architecture.html#autotoc_md47", null ],
        [ "Traçabilité", "spec-architecture.html#autotoc_md48", null ]
      ] ],
      [ "Décors & pipeline pixel art", "spec-decors.html", [
        [ "Vision", "spec-decors.html#autotoc_md72", null ],
        [ "1. Système de décors", "spec-decors.html#autotoc_md73", null ],
        [ "2. Manipulation", "spec-decors.html#autotoc_md74", [
          [ "À la conception (éditeur)", "spec-decors.html#autotoc_md75", null ],
          [ "En jeu (mécanique, à terme)", "spec-decors.html#autotoc_md76", null ]
        ] ],
        [ "3. Pipeline photo → pixel art (intégré à l'éditeur)", "spec-decors.html#autotoc_md77", null ],
        [ "4. Périmètre & séquencement", "spec-decors.html#autotoc_md78", null ],
        [ "Traçabilité", "spec-decors.html#autotoc_md79", null ]
      ] ],
      [ "Conventions de code", "spec-conventions.html", [
        [ "1. Langage & standard", "spec-conventions.html#autotoc_md54", null ],
        [ "2. Nommage", "spec-conventions.html#autotoc_md55", null ],
        [ "3. Mise en forme", "spec-conventions.html#autotoc_md56", null ],
        [ "4. Inclusions (<tt>#include</tt>)", "spec-conventions.html#autotoc_md57", [
          [ "Chemins complets depuis <tt>Source/</tt>", "spec-conventions.html#autotoc_md58", null ],
          [ "Ordre des groupes", "spec-conventions.html#autotoc_md59", null ]
        ] ],
        [ "5. Architecture (dépendances entre modules)", "spec-conventions.html#autotoc_md60", [
          [ "Classes plutôt que fonctions libres", "spec-conventions.html#autotoc_md61", null ],
          [ "RAII obligatoire", "spec-conventions.html#autotoc_md62", null ]
        ] ],
        [ "6. Documentation Doxygen", "spec-conventions.html#autotoc_md63", [
          [ "Doxygen dans le header, commentaires simples <tt>//</tt> dans le <tt>.cpp</tt>", "spec-conventions.html#autotoc_md64", null ],
          [ "Documentation du corps (<tt>.cpp</tt>)", "spec-conventions.html#autotoc_md65", null ]
        ] ],
        [ "7. Bonnes pratiques", "spec-conventions.html#autotoc_md66", null ],
        [ "8. Tests", "spec-conventions.html#autotoc_md67", null ],
        [ "9. Gestion des erreurs", "spec-conventions.html#autotoc_md68", null ],
        [ "10. Assertions & journalisation", "spec-conventions.html#autotoc_md69", null ],
        [ "11. Outillage qualité (automatisé)", "spec-conventions.html#autotoc_md70", null ],
        [ "12. Identifiants d'exigences (<tt>EX-…</tt>)", "spec-conventions.html#autotoc_md71", null ]
      ] ]
    ] ],
    [ "Lots", "lots.html", [
      [ "Lots", "lots.html#autotoc_md734", null ],
      [ "LOT-01 — Fenêtre & boucle de jeu (Direct3D 11)", "lot-01.html", [
        [ "Objectif", "lot-01.html#autotoc_md121", null ],
        [ "Périmètre", "lot-01.html#autotoc_md122", [
          [ "Inclus", "lot-01.html#autotoc_md123", null ],
          [ "Exclus (lots ultérieurs)", "lot-01.html#autotoc_md124", null ]
        ] ],
        [ "Exigences couvertes", "lot-01.html#autotoc_md125", null ],
        [ "Découpage", "lot-01.html#autotoc_md126", null ],
        [ "Critères d'acceptation du lot", "lot-01.html#autotoc_md127", null ],
        [ "Navigation des tâches", "lot-01.html#autotoc_md128", null ],
        [ "TACHE-01 — Fenêtre Win32 & pompe de messages", "lot-01-tache-01-fenetre-win32.html", [
          [ "Contexte", "lot-01-tache-01-fenetre-win32.html#autotoc_md129", null ],
          [ "Travail à réaliser", "lot-01-tache-01-fenetre-win32.html#autotoc_md130", null ],
          [ "Fichiers impactés", "lot-01-tache-01-fenetre-win32.html#autotoc_md131", null ],
          [ "Points d'attention", "lot-01-tache-01-fenetre-win32.html#autotoc_md132", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-01-fenetre-win32.html#autotoc_md133", null ],
          [ "Exigences", "lot-01-tache-01-fenetre-win32.html#autotoc_md134", null ]
        ] ],
        [ "TACHE-02 — Initialisation Direct3D 11 (RAII)", "lot-01-tache-02-init-direct3d11.html", [
          [ "Contexte", "lot-01-tache-02-init-direct3d11.html#autotoc_md135", null ],
          [ "Travail à réaliser", "lot-01-tache-02-init-direct3d11.html#autotoc_md136", null ],
          [ "Fichiers impactés", "lot-01-tache-02-init-direct3d11.html#autotoc_md137", null ],
          [ "Points d'attention", "lot-01-tache-02-init-direct3d11.html#autotoc_md138", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-02-init-direct3d11.html#autotoc_md139", null ],
          [ "Exigences", "lot-01-tache-02-init-direct3d11.html#autotoc_md140", null ]
        ] ],
        [ "TACHE-03 — Boucle à pas de temps fixe (testable)", "lot-01-tache-03-boucle-pas-fixe.html", [
          [ "Contexte", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md141", null ],
          [ "Travail à réaliser", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md142", null ],
          [ "Fichiers impactés", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md143", null ],
          [ "Tests (obligatoires)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md144", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md145", null ],
          [ "Exigences", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md146", null ]
        ] ],
        [ "TACHE-04 — Effacement écran, présentation & redimensionnement", "lot-01-tache-04-effacement-presentation.html", [
          [ "Contexte", "lot-01-tache-04-effacement-presentation.html#autotoc_md147", null ],
          [ "Travail à réaliser", "lot-01-tache-04-effacement-presentation.html#autotoc_md148", null ],
          [ "Fichiers impactés", "lot-01-tache-04-effacement-presentation.html#autotoc_md149", null ],
          [ "Points d'attention", "lot-01-tache-04-effacement-presentation.html#autotoc_md150", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-04-effacement-presentation.html#autotoc_md151", null ],
          [ "Exigences", "lot-01-tache-04-effacement-presentation.html#autotoc_md152", null ]
        ] ],
        [ "TACHE-05 — Intégration `main` & vérification", "lot-01-tache-05-integration.html", [
          [ "Contexte", "lot-01-tache-05-integration.html#autotoc_md153", null ],
          [ "Travail à réaliser", "lot-01-tache-05-integration.html#autotoc_md154", null ],
          [ "Fichiers impactés", "lot-01-tache-05-integration.html#autotoc_md155", null ],
          [ "Vérification (manuelle + automatique)", "lot-01-tache-05-integration.html#autotoc_md156", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-05-integration.html#autotoc_md157", null ],
          [ "Exigences", "lot-01-tache-05-integration.html#autotoc_md158", null ]
        ] ]
      ] ],
      [ "LOT-02 — Journalisation & diagnostics", "lot-02.html", [
        [ "Objectif", "lot-02.html#autotoc_md159", null ],
        [ "Périmètre", "lot-02.html#autotoc_md160", [
          [ "Inclus", "lot-02.html#autotoc_md161", null ],
          [ "Exclus (plus tard)", "lot-02.html#autotoc_md162", null ]
        ] ],
        [ "Exigences couvertes", "lot-02.html#autotoc_md163", null ],
        [ "Découpage", "lot-02.html#autotoc_md164", null ],
        [ "Critères d'acceptation du lot", "lot-02.html#autotoc_md165", null ],
        [ "Dépendances", "lot-02.html#autotoc_md166", null ],
        [ "Navigation des tâches", "lot-02.html#autotoc_md167", null ],
        [ "TACHE-01 — Niveaux de log & interface `Logger`", "lot-02-tache-01-niveaux-logger.html", [
          [ "Contexte", "lot-02-tache-01-niveaux-logger.html#autotoc_md168", null ],
          [ "Travail à réaliser", "lot-02-tache-01-niveaux-logger.html#autotoc_md169", null ],
          [ "Fichiers impactés", "lot-02-tache-01-niveaux-logger.html#autotoc_md170", null ],
          [ "Tests (obligatoires)", "lot-02-tache-01-niveaux-logger.html#autotoc_md171", null ],
          [ "Points d'attention", "lot-02-tache-01-niveaux-logger.html#autotoc_md172", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-01-niveaux-logger.html#autotoc_md173", null ],
          [ "Exigences", "lot-02-tache-01-niveaux-logger.html#autotoc_md174", null ]
        ] ],
        [ "TACHE-02 — Sinks enfichables", "lot-02-tache-02-sinks.html", [
          [ "Contexte", "lot-02-tache-02-sinks.html#autotoc_md175", null ],
          [ "Travail à réaliser", "lot-02-tache-02-sinks.html#autotoc_md176", null ],
          [ "Fichiers impactés", "lot-02-tache-02-sinks.html#autotoc_md177", null ],
          [ "Tests (obligatoires)", "lot-02-tache-02-sinks.html#autotoc_md178", null ],
          [ "Points d'attention", "lot-02-tache-02-sinks.html#autotoc_md179", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-02-sinks.html#autotoc_md180", null ],
          [ "Exigences", "lot-02-tache-02-sinks.html#autotoc_md181", null ]
        ] ],
        [ "TACHE-03 — Macros de log (fichier/ligne, horodatage)", "lot-02-tache-03-macros-log.html", [
          [ "Contexte", "lot-02-tache-03-macros-log.html#autotoc_md182", null ],
          [ "Travail à réaliser", "lot-02-tache-03-macros-log.html#autotoc_md183", null ],
          [ "Fichiers impactés", "lot-02-tache-03-macros-log.html#autotoc_md184", null ],
          [ "Tests (obligatoires)", "lot-02-tache-03-macros-log.html#autotoc_md185", null ],
          [ "Points d'attention", "lot-02-tache-03-macros-log.html#autotoc_md186", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-03-macros-log.html#autotoc_md187", null ],
          [ "Exigences", "lot-02-tache-03-macros-log.html#autotoc_md188", null ]
        ] ],
        [ "TACHE-04 — Assertions `PROJECTGAMING_ASSERT`", "lot-02-tache-04-assertions.html", [
          [ "Contexte", "lot-02-tache-04-assertions.html#autotoc_md189", null ],
          [ "Travail à réaliser", "lot-02-tache-04-assertions.html#autotoc_md190", null ],
          [ "Fichiers impactés", "lot-02-tache-04-assertions.html#autotoc_md191", null ],
          [ "Tests (obligatoires)", "lot-02-tache-04-assertions.html#autotoc_md192", null ],
          [ "Points d'attention", "lot-02-tache-04-assertions.html#autotoc_md193", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-04-assertions.html#autotoc_md194", null ],
          [ "Exigences", "lot-02-tache-04-assertions.html#autotoc_md195", null ]
        ] ],
        [ "TACHE-05 — Intégration dans `main` & documentation", "lot-02-tache-05-integration.html", [
          [ "Contexte", "lot-02-tache-05-integration.html#autotoc_md196", null ],
          [ "Travail à réaliser", "lot-02-tache-05-integration.html#autotoc_md197", null ],
          [ "Fichiers impactés", "lot-02-tache-05-integration.html#autotoc_md198", null ],
          [ "Vérification", "lot-02-tache-05-integration.html#autotoc_md199", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-05-integration.html#autotoc_md200", null ],
          [ "Exigences", "lot-02-tache-05-integration.html#autotoc_md201", null ]
        ] ]
      ] ],
      [ "LOT-03 — Fondation ECS & mathématiques `Core`", "lot-03.html", [
        [ "Objectif", "lot-03.html#autotoc_md202", null ],
        [ "⚠️ Décision préalable : ECS maison vs bibliothèque", "lot-03.html#autotoc_md203", null ],
        [ "Périmètre", "lot-03.html#autotoc_md204", [
          [ "Inclus", "lot-03.html#autotoc_md205", null ],
          [ "Exclus (lots ultérieurs)", "lot-03.html#autotoc_md206", null ]
        ] ],
        [ "Exigences couvertes", "lot-03.html#autotoc_md207", null ],
        [ "Découpage", "lot-03.html#autotoc_md208", null ],
        [ "Critères d'acceptation du lot", "lot-03.html#autotoc_md209", null ],
        [ "Dépendances", "lot-03.html#autotoc_md210", null ],
        [ "Navigation des tâches", "lot-03.html#autotoc_md211", null ],
        [ "TACHE-01 — Types mathématiques de `Core`", "lot-03-tache-01-math-core.html", [
          [ "Contexte", "lot-03-tache-01-math-core.html#autotoc_md212", null ],
          [ "Travail à réaliser", "lot-03-tache-01-math-core.html#autotoc_md213", null ],
          [ "Fichiers impactés", "lot-03-tache-01-math-core.html#autotoc_md214", null ],
          [ "Tests (obligatoires)", "lot-03-tache-01-math-core.html#autotoc_md215", null ],
          [ "Points d'attention", "lot-03-tache-01-math-core.html#autotoc_md216", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-01-math-core.html#autotoc_md217", null ],
          [ "Exigences", "lot-03-tache-01-math-core.html#autotoc_md218", null ]
        ] ],
        [ "TACHE-02 — Entités : handles générationnels & cycle de vie", "lot-03-tache-02-entites.html", [
          [ "Contexte", "lot-03-tache-02-entites.html#autotoc_md219", null ],
          [ "Travail à réaliser", "lot-03-tache-02-entites.html#autotoc_md220", null ],
          [ "Fichiers impactés", "lot-03-tache-02-entites.html#autotoc_md221", null ],
          [ "Tests (obligatoires)", "lot-03-tache-02-entites.html#autotoc_md222", null ],
          [ "Points d'attention", "lot-03-tache-02-entites.html#autotoc_md223", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-02-entites.html#autotoc_md224", null ],
          [ "Exigences", "lot-03-tache-02-entites.html#autotoc_md225", null ]
        ] ],
        [ "TACHE-03 — Stockage de composants (sparse set typé)", "lot-03-tache-03-stockage-composants.html", [
          [ "Contexte", "lot-03-tache-03-stockage-composants.html#autotoc_md226", null ],
          [ "Travail à réaliser", "lot-03-tache-03-stockage-composants.html#autotoc_md227", null ],
          [ "Fichiers impactés", "lot-03-tache-03-stockage-composants.html#autotoc_md228", null ],
          [ "Tests (obligatoires)", "lot-03-tache-03-stockage-composants.html#autotoc_md229", null ],
          [ "Points d'attention", "lot-03-tache-03-stockage-composants.html#autotoc_md230", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-03-stockage-composants.html#autotoc_md231", null ],
          [ "Exigences", "lot-03-tache-03-stockage-composants.html#autotoc_md232", null ]
        ] ],
        [ "TACHE-04 — Requêtes / vues multi-composants", "lot-03-tache-04-vues-requetes.html", [
          [ "Contexte", "lot-03-tache-04-vues-requetes.html#autotoc_md233", null ],
          [ "Travail à réaliser", "lot-03-tache-04-vues-requetes.html#autotoc_md234", null ],
          [ "Fichiers impactés", "lot-03-tache-04-vues-requetes.html#autotoc_md235", null ],
          [ "Tests (obligatoires)", "lot-03-tache-04-vues-requetes.html#autotoc_md236", null ],
          [ "Points d'attention", "lot-03-tache-04-vues-requetes.html#autotoc_md237", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-04-vues-requetes.html#autotoc_md238", null ],
          [ "Exigences", "lot-03-tache-04-vues-requetes.html#autotoc_md239", null ]
        ] ],
        [ "TACHE-05 — Systèmes & `World` (orchestration au pas fixe)", "lot-03-tache-05-systemes-world.html", [
          [ "Contexte", "lot-03-tache-05-systemes-world.html#autotoc_md240", null ],
          [ "Travail à réaliser", "lot-03-tache-05-systemes-world.html#autotoc_md241", null ],
          [ "Fichiers impactés", "lot-03-tache-05-systemes-world.html#autotoc_md242", null ],
          [ "Tests (obligatoires)", "lot-03-tache-05-systemes-world.html#autotoc_md243", null ],
          [ "Points d'attention", "lot-03-tache-05-systemes-world.html#autotoc_md244", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-05-systemes-world.html#autotoc_md245", null ],
          [ "Exigences", "lot-03-tache-05-systemes-world.html#autotoc_md246", null ]
        ] ],
        [ "TACHE-06 — Composant `Transform` + système de mouvement (démo)", "lot-03-tache-06-transform-demo.html", [
          [ "Contexte", "lot-03-tache-06-transform-demo.html#autotoc_md247", null ],
          [ "Travail à réaliser", "lot-03-tache-06-transform-demo.html#autotoc_md248", null ],
          [ "Fichiers impactés", "lot-03-tache-06-transform-demo.html#autotoc_md249", null ],
          [ "Tests (obligatoires)", "lot-03-tache-06-transform-demo.html#autotoc_md250", null ],
          [ "Points d'attention", "lot-03-tache-06-transform-demo.html#autotoc_md251", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-06-transform-demo.html#autotoc_md252", null ],
          [ "Exigences", "lot-03-tache-06-transform-demo.html#autotoc_md253", null ]
        ] ]
      ] ],
      [ "LOT-04 — Documentation Doxygen & réorganisation de l'arborescence documentaire", "lot-04.html", [
        [ "Objectif", "lot-04.html#autotoc_md254", null ],
        [ "Périmètre", "lot-04.html#autotoc_md255", [
          [ "Inclus", "lot-04.html#autotoc_md256", null ],
          [ "Exclus (lots ultérieurs)", "lot-04.html#autotoc_md257", null ]
        ] ],
        [ "Décisions de cadrage", "lot-04.html#autotoc_md258", null ],
        [ "Exigences couvertes", "lot-04.html#autotoc_md259", null ],
        [ "Découpage", "lot-04.html#autotoc_md260", null ],
        [ "Critères d'acceptation du lot", "lot-04.html#autotoc_md261", null ],
        [ "Dépendances", "lot-04.html#autotoc_md262", null ],
        [ "Navigation des tâches", "lot-04.html#autotoc_md263", null ],
        [ "TACHE-01 — Réorganisation de l'arborescence documentaire", "lot-04-tache-01-reorganisation-arbo.html", [
          [ "Contexte", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md264", null ],
          [ "Travail à réaliser", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md265", null ],
          [ "Fichiers impactés", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md266", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md267", null ],
          [ "Points d'attention", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md268", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md269", null ],
          [ "Exigences", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md270", null ]
        ] ],
        [ "TACHE-02 — Configuration Doxygen pour le Markdown", "lot-04-tache-02-config-doxygen-markdown.html", [
          [ "Contexte", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md271", null ],
          [ "Travail à réaliser", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md272", null ],
          [ "Fichiers impactés", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md273", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md274", null ],
          [ "Points d'attention", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md275", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md276", null ],
          [ "Exigences", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md277", null ]
        ] ],
        [ "TACHE-03 — Page d'accueil du projet (mainpage)", "lot-04-tache-03-page-accueil.html", [
          [ "Contexte", "lot-04-tache-03-page-accueil.html#autotoc_md278", null ],
          [ "Travail à réaliser", "lot-04-tache-03-page-accueil.html#autotoc_md279", null ],
          [ "Fichiers impactés", "lot-04-tache-03-page-accueil.html#autotoc_md280", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-03-page-accueil.html#autotoc_md281", null ],
          [ "Points d'attention", "lot-04-tache-03-page-accueil.html#autotoc_md282", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-03-page-accueil.html#autotoc_md283", null ],
          [ "Exigences", "lot-04-tache-03-page-accueil.html#autotoc_md284", null ]
        ] ],
        [ "TACHE-04 — Pages de spécification navigables (conventions incluses)", "lot-04-tache-04-pages-specification.html", [
          [ "Contexte", "lot-04-tache-04-pages-specification.html#autotoc_md285", null ],
          [ "Travail à réaliser", "lot-04-tache-04-pages-specification.html#autotoc_md286", null ],
          [ "Convention d'insertion d'une nouvelle spec (à documenter dans l'index)", "lot-04-tache-04-pages-specification.html#autotoc_md287", null ],
          [ "Fichiers impactés", "lot-04-tache-04-pages-specification.html#autotoc_md288", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-04-pages-specification.html#autotoc_md289", null ],
          [ "Points d'attention", "lot-04-tache-04-pages-specification.html#autotoc_md290", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-04-pages-specification.html#autotoc_md291", null ],
          [ "Exigences", "lot-04-tache-04-pages-specification.html#autotoc_md292", null ]
        ] ],
        [ "TACHE-05 — Pages de lots navigables", "lot-04-tache-05-pages-lots.html", [
          [ "Contexte", "lot-04-tache-05-pages-lots.html#autotoc_md293", null ],
          [ "Travail à réaliser", "lot-04-tache-05-pages-lots.html#autotoc_md294", null ],
          [ "Fichiers impactés", "lot-04-tache-05-pages-lots.html#autotoc_md295", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-05-pages-lots.html#autotoc_md296", null ],
          [ "Points d'attention", "lot-04-tache-05-pages-lots.html#autotoc_md297", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-05-pages-lots.html#autotoc_md298", null ],
          [ "Exigences", "lot-04-tache-05-pages-lots.html#autotoc_md299", null ]
        ] ],
        [ "TACHE-06 — Manuel utilisateur (squelette + première page)", "lot-04-tache-06-manuel-utilisateur.html", [
          [ "Contexte", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md300", null ],
          [ "Travail à réaliser", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md301", null ],
          [ "Fichiers impactés", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md302", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md303", null ],
          [ "Points d'attention", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md304", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md305", null ],
          [ "Exigences", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md306", null ]
        ] ],
        [ "TACHE-07 — CI documentation (WARN_AS_ERROR & déploiement)", "lot-04-tache-07-ci-docs.html", [
          [ "Contexte", "lot-04-tache-07-ci-docs.html#autotoc_md307", null ],
          [ "Travail à réaliser", "lot-04-tache-07-ci-docs.html#autotoc_md308", null ],
          [ "Fichiers impactés", "lot-04-tache-07-ci-docs.html#autotoc_md309", null ],
          [ "Avertissements connus à corriger avant <tt>WARN_AS_ERROR</tt> (relevés en TACHE-02)", "lot-04-tache-07-ci-docs.html#autotoc_md310", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-07-ci-docs.html#autotoc_md311", null ],
          [ "Points d'attention", "lot-04-tache-07-ci-docs.html#autotoc_md312", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-07-ci-docs.html#autotoc_md313", null ],
          [ "Exigences", "lot-04-tache-07-ci-docs.html#autotoc_md314", null ]
        ] ],
        [ "TACHE-08 — Traçabilité des exigences (IDs stables, ancres Doxygen, lint CI)", "lot-04-tache-08-tracabilite-exigences.html", [
          [ "Contexte", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md315", null ],
          [ "Règle à formaliser (dans <tt>conventions.md</tt>)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md316", null ],
          [ "Travail à réaliser", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md317", null ],
          [ "Fichiers impactés", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md318", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md319", null ],
          [ "Points d'attention", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md320", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md321", null ],
          [ "Exigences", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md322", null ]
        ] ]
      ] ],
      [ "LOT-05 — Rendu 2D : atlas, sprites & caméra", "lot-05.html", [
        [ "Objectif", "lot-05.html#autotoc_md323", null ],
        [ "Périmètre", "lot-05.html#autotoc_md324", [
          [ "Inclus", "lot-05.html#autotoc_md325", null ],
          [ "Exclus (lots ultérieurs)", "lot-05.html#autotoc_md326", null ]
        ] ],
        [ "Décisions de cadrage", "lot-05.html#autotoc_md327", null ],
        [ "Exigences couvertes", "lot-05.html#autotoc_md328", null ],
        [ "Découpage", "lot-05.html#autotoc_md329", null ],
        [ "Critères d'acceptation du lot", "lot-05.html#autotoc_md330", null ],
        [ "Dépendances", "lot-05.html#autotoc_md331", null ],
        [ "Navigation des tâches", "lot-05.html#autotoc_md332", null ],
        [ "TACHE-01 — Composant `Sprite` (données pures)", "lot-05-tache-01-composant-sprite.html", [
          [ "Contexte", "lot-05-tache-01-composant-sprite.html#autotoc_md333", null ],
          [ "Travail à réaliser", "lot-05-tache-01-composant-sprite.html#autotoc_md334", null ],
          [ "Fichiers impactés", "lot-05-tache-01-composant-sprite.html#autotoc_md335", null ],
          [ "Tests (obligatoires si logique)", "lot-05-tache-01-composant-sprite.html#autotoc_md336", null ],
          [ "Points d'attention", "lot-05-tache-01-composant-sprite.html#autotoc_md337", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-01-composant-sprite.html#autotoc_md338", null ],
          [ "Exigences", "lot-05-tache-01-composant-sprite.html#autotoc_md339", null ]
        ] ],
        [ "TACHE-02 — Pipeline de quads texturés (HLSL, blend, nearest)", "lot-05-tache-02-pipeline-quads-textures.html", [
          [ "Contexte", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md340", null ],
          [ "Travail à réaliser", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md341", null ],
          [ "Fichiers impactés", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md342", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md343", null ],
          [ "Points d'attention", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md344", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md345", null ],
          [ "Exigences", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md346", null ]
        ] ],
        [ "TACHE-03 — Atlas de textures procédural", "lot-05-tache-03-atlas-procedural.html", [
          [ "Contexte", "lot-05-tache-03-atlas-procedural.html#autotoc_md347", null ],
          [ "Travail à réaliser", "lot-05-tache-03-atlas-procedural.html#autotoc_md348", null ],
          [ "Fichiers impactés", "lot-05-tache-03-atlas-procedural.html#autotoc_md349", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-03-atlas-procedural.html#autotoc_md350", null ],
          [ "Points d'attention", "lot-05-tache-03-atlas-procedural.html#autotoc_md351", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-03-atlas-procedural.html#autotoc_md352", null ],
          [ "Exigences", "lot-05-tache-03-atlas-procedural.html#autotoc_md353", null ]
        ] ],
        [ "TACHE-04 — Caméra 2D (monde → écran)", "lot-05-tache-04-camera-2d.html", [
          [ "Contexte", "lot-05-tache-04-camera-2d.html#autotoc_md354", null ],
          [ "Travail à réaliser", "lot-05-tache-04-camera-2d.html#autotoc_md355", null ],
          [ "Fichiers impactés", "lot-05-tache-04-camera-2d.html#autotoc_md356", null ],
          [ "Tests (obligatoires)", "lot-05-tache-04-camera-2d.html#autotoc_md357", null ],
          [ "Points d'attention", "lot-05-tache-04-camera-2d.html#autotoc_md358", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-04-camera-2d.html#autotoc_md359", null ],
          [ "Exigences", "lot-05-tache-04-camera-2d.html#autotoc_md360", null ]
        ] ],
        [ "TACHE-05 — Système de rendu des sprites (ECS → écran)", "lot-05-tache-05-systeme-rendu-sprites.html", [
          [ "Contexte", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md361", null ],
          [ "Travail à réaliser", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md362", null ],
          [ "Fichiers impactés", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md363", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md364", null ],
          [ "Points d'attention", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md365", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md366", null ],
          [ "Exigences", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md367", null ]
        ] ],
        [ "TACHE-06 — Câblage du `World` dans la boucle + scène de démo", "lot-05-tache-06-cablage-world-demo.html", [
          [ "Contexte", "lot-05-tache-06-cablage-world-demo.html#autotoc_md368", null ],
          [ "Travail à réaliser", "lot-05-tache-06-cablage-world-demo.html#autotoc_md369", null ],
          [ "Fichiers impactés", "lot-05-tache-06-cablage-world-demo.html#autotoc_md370", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md371", null ],
          [ "Points d'attention", "lot-05-tache-06-cablage-world-demo.html#autotoc_md372", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md373", null ],
          [ "Exigences", "lot-05-tache-06-cablage-world-demo.html#autotoc_md374", null ]
        ] ]
      ] ],
      [ "LOT-06 — Menu principal", "lot-06.html", [
        [ "Objectif", "lot-06.html#autotoc_md375", null ],
        [ "Périmètre", "lot-06.html#autotoc_md376", [
          [ "Inclus", "lot-06.html#autotoc_md377", null ],
          [ "Exclus (lots ultérieurs)", "lot-06.html#autotoc_md378", null ]
        ] ],
        [ "Décisions de cadrage", "lot-06.html#autotoc_md379", null ],
        [ "Exigences couvertes", "lot-06.html#autotoc_md380", null ],
        [ "Découpage", "lot-06.html#autotoc_md381", null ],
        [ "Critères d'acceptation du lot", "lot-06.html#autotoc_md382", null ],
        [ "Dépendances", "lot-06.html#autotoc_md383", null ],
        [ "Navigation des tâches", "lot-06.html#autotoc_md384", null ],
        [ "TACHE-01 — Entrées clavier & souris", "lot-06-tache-01-entrees-clavier-souris.html", [
          [ "Contexte", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md385", null ],
          [ "Travail à réaliser", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md386", null ],
          [ "Fichiers impactés", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md387", null ],
          [ "Tests (obligatoires)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md388", null ],
          [ "Points d'attention", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md389", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md390", null ],
          [ "Exigences", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md391", null ]
        ] ],
        [ "TACHE-02 — Rendu de texte (police bitmap)", "lot-06-tache-02-rendu-texte-bitmap.html", [
          [ "Contexte", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md392", null ],
          [ "Travail à réaliser", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md393", null ],
          [ "Fichiers impactés", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md394", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md395", null ],
          [ "Points d'attention", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md396", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md397", null ],
          [ "Exigences", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md398", null ]
        ] ],
        [ "TACHE-03 — Catalogue de traduction (i18n)", "lot-06-tache-03-catalogue-traduction.html", [
          [ "Contexte", "lot-06-tache-03-catalogue-traduction.html#autotoc_md399", null ],
          [ "Travail à réaliser", "lot-06-tache-03-catalogue-traduction.html#autotoc_md400", null ],
          [ "Fichiers impactés", "lot-06-tache-03-catalogue-traduction.html#autotoc_md401", null ],
          [ "Tests (obligatoires)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md402", null ],
          [ "Points d'attention", "lot-06-tache-03-catalogue-traduction.html#autotoc_md403", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md404", null ],
          [ "Exigences", "lot-06-tache-03-catalogue-traduction.html#autotoc_md405", null ]
        ] ],
        [ "TACHE-04 — États d'application (écrans)", "lot-06-tache-04-etats-application.html", [
          [ "Contexte", "lot-06-tache-04-etats-application.html#autotoc_md406", null ],
          [ "Travail à réaliser", "lot-06-tache-04-etats-application.html#autotoc_md407", null ],
          [ "Fichiers impactés", "lot-06-tache-04-etats-application.html#autotoc_md408", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-04-etats-application.html#autotoc_md409", null ],
          [ "Points d'attention", "lot-06-tache-04-etats-application.html#autotoc_md410", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-04-etats-application.html#autotoc_md411", null ],
          [ "Exigences", "lot-06-tache-04-etats-application.html#autotoc_md412", null ]
        ] ],
        [ "TACHE-05 — Écran de menu principal", "lot-06-tache-05-ecran-menu-principal.html", [
          [ "Contexte", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md413", null ],
          [ "Travail à réaliser", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md414", null ],
          [ "Fichiers impactés", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md415", null ],
          [ "Tests (obligatoires)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md416", null ],
          [ "Points d'attention", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md417", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md418", null ],
          [ "Exigences", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md419", null ]
        ] ],
        [ "TACHE-06 — Écrans cibles (jeu démo + éditeur placeholder)", "lot-06-tache-06-ecrans-cibles.html", [
          [ "Contexte", "lot-06-tache-06-ecrans-cibles.html#autotoc_md420", null ],
          [ "Travail à réaliser", "lot-06-tache-06-ecrans-cibles.html#autotoc_md421", null ],
          [ "Fichiers impactés", "lot-06-tache-06-ecrans-cibles.html#autotoc_md422", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md423", null ],
          [ "Points d'attention", "lot-06-tache-06-ecrans-cibles.html#autotoc_md424", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md425", null ],
          [ "Exigences", "lot-06-tache-06-ecrans-cibles.html#autotoc_md426", null ]
        ] ],
        [ "TACHE-07 — Intégration `main` (boucle pilotée par l'écran)", "lot-06-tache-07-integration-main.html", [
          [ "Contexte", "lot-06-tache-07-integration-main.html#autotoc_md427", null ],
          [ "Travail à réaliser", "lot-06-tache-07-integration-main.html#autotoc_md428", null ],
          [ "Fichiers impactés", "lot-06-tache-07-integration-main.html#autotoc_md429", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-07-integration-main.html#autotoc_md430", null ],
          [ "Points d'attention", "lot-06-tache-07-integration-main.html#autotoc_md431", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-07-integration-main.html#autotoc_md432", null ],
          [ "Exigences", "lot-06-tache-07-integration-main.html#autotoc_md433", null ]
        ] ],
        [ "TACHE-08 — Sélecteur de langue", "lot-06-tache-08-selecteur-langue.html", [
          [ "Contexte", "lot-06-tache-08-selecteur-langue.html#autotoc_md434", null ],
          [ "Travail à réaliser", "lot-06-tache-08-selecteur-langue.html#autotoc_md435", null ],
          [ "Fichiers impactés", "lot-06-tache-08-selecteur-langue.html#autotoc_md436", null ],
          [ "Tests (obligatoires)", "lot-06-tache-08-selecteur-langue.html#autotoc_md437", null ],
          [ "Points d'attention", "lot-06-tache-08-selecteur-langue.html#autotoc_md438", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-08-selecteur-langue.html#autotoc_md439", null ],
          [ "Exigences", "lot-06-tache-08-selecteur-langue.html#autotoc_md440", null ]
        ] ]
      ] ],
      [ "LOT-07 — Niveaux : modèle et chargement", "lot-07.html", [
        [ "Objectif", "lot-07.html#autotoc_md441", null ],
        [ "Périmètre", "lot-07.html#autotoc_md442", [
          [ "Inclus", "lot-07.html#autotoc_md443", null ],
          [ "Exclus (lots ultérieurs)", "lot-07.html#autotoc_md444", null ]
        ] ],
        [ "Décisions de cadrage", "lot-07.html#autotoc_md445", null ],
        [ "Exigences couvertes", "lot-07.html#autotoc_md446", null ],
        [ "Découpage", "lot-07.html#autotoc_md447", null ],
        [ "Critères d'acceptation du lot", "lot-07.html#autotoc_md448", null ],
        [ "Dépendances", "lot-07.html#autotoc_md449", null ],
        [ "Navigation des tâches", "lot-07.html#autotoc_md450", null ],
        [ "TACHE-01 — Dépendance JSON (nlohmann/json épinglé)", "lot-07-tache-01-dependance-json.html", [
          [ "Contexte", "lot-07-tache-01-dependance-json.html#autotoc_md451", null ],
          [ "Travail à réaliser", "lot-07-tache-01-dependance-json.html#autotoc_md452", null ],
          [ "Fichiers impactés", "lot-07-tache-01-dependance-json.html#autotoc_md453", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-01-dependance-json.html#autotoc_md454", null ],
          [ "Points d'attention", "lot-07-tache-01-dependance-json.html#autotoc_md455", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-01-dependance-json.html#autotoc_md456", null ],
          [ "Exigences", "lot-07-tache-01-dependance-json.html#autotoc_md457", null ]
        ] ],
        [ "TACHE-02 — Modèle de tuiles et de niveau", "lot-07-tache-02-modele-niveau.html", [
          [ "Contexte", "lot-07-tache-02-modele-niveau.html#autotoc_md458", null ],
          [ "Travail à réaliser", "lot-07-tache-02-modele-niveau.html#autotoc_md459", null ],
          [ "Fichiers impactés", "lot-07-tache-02-modele-niveau.html#autotoc_md460", null ],
          [ "Tests (obligatoires)", "lot-07-tache-02-modele-niveau.html#autotoc_md461", null ],
          [ "Points d'attention", "lot-07-tache-02-modele-niveau.html#autotoc_md462", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-02-modele-niveau.html#autotoc_md463", null ],
          [ "Exigences", "lot-07-tache-02-modele-niveau.html#autotoc_md464", null ]
        ] ],
        [ "TACHE-03 — Chargement du niveau (JSON)", "lot-07-tache-03-chargement-json.html", [
          [ "Contexte", "lot-07-tache-03-chargement-json.html#autotoc_md465", null ],
          [ "Travail à réaliser", "lot-07-tache-03-chargement-json.html#autotoc_md466", null ],
          [ "Fichiers impactés", "lot-07-tache-03-chargement-json.html#autotoc_md467", null ],
          [ "Tests (obligatoires)", "lot-07-tache-03-chargement-json.html#autotoc_md468", null ],
          [ "Points d'attention", "lot-07-tache-03-chargement-json.html#autotoc_md469", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-03-chargement-json.html#autotoc_md470", null ],
          [ "Exigences", "lot-07-tache-03-chargement-json.html#autotoc_md471", null ]
        ] ],
        [ "TACHE-04 — Validation du niveau", "lot-07-tache-04-validation.html", [
          [ "Contexte", "lot-07-tache-04-validation.html#autotoc_md472", null ],
          [ "Travail à réaliser", "lot-07-tache-04-validation.html#autotoc_md473", null ],
          [ "Fichiers impactés", "lot-07-tache-04-validation.html#autotoc_md474", null ],
          [ "Tests (obligatoires)", "lot-07-tache-04-validation.html#autotoc_md475", null ],
          [ "Points d'attention", "lot-07-tache-04-validation.html#autotoc_md476", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-04-validation.html#autotoc_md477", null ],
          [ "Exigences", "lot-07-tache-04-validation.html#autotoc_md478", null ]
        ] ],
        [ "TACHE-05 — Niveau de démonstration", "lot-07-tache-05-niveau-demo.html", [
          [ "Contexte", "lot-07-tache-05-niveau-demo.html#autotoc_md479", null ],
          [ "Travail à réaliser", "lot-07-tache-05-niveau-demo.html#autotoc_md480", null ],
          [ "Fichiers impactés", "lot-07-tache-05-niveau-demo.html#autotoc_md481", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-05-niveau-demo.html#autotoc_md482", null ],
          [ "Points d'attention", "lot-07-tache-05-niveau-demo.html#autotoc_md483", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-05-niveau-demo.html#autotoc_md484", null ],
          [ "Exigences", "lot-07-tache-05-niveau-demo.html#autotoc_md485", null ]
        ] ],
        [ "TACHE-06 — Rendu du niveau + intégration « Charger niveau »", "lot-07-tache-06-rendu-integration.html", [
          [ "Contexte", "lot-07-tache-06-rendu-integration.html#autotoc_md486", null ],
          [ "Travail à réaliser", "lot-07-tache-06-rendu-integration.html#autotoc_md487", null ],
          [ "Fichiers impactés", "lot-07-tache-06-rendu-integration.html#autotoc_md488", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-06-rendu-integration.html#autotoc_md489", null ],
          [ "Points d'attention", "lot-07-tache-06-rendu-integration.html#autotoc_md490", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-06-rendu-integration.html#autotoc_md491", null ],
          [ "Exigences", "lot-07-tache-06-rendu-integration.html#autotoc_md492", null ]
        ] ]
      ] ],
      [ "LOT-08 — Gameplay personnage : déplacement, gravité et collisions", "lot-08.html", [
        [ "Objectif", "lot-08.html#autotoc_md493", null ],
        [ "Périmètre", "lot-08.html#autotoc_md494", [
          [ "Inclus", "lot-08.html#autotoc_md495", null ],
          [ "Exclus (lots ultérieurs)", "lot-08.html#autotoc_md496", null ]
        ] ],
        [ "Décisions de cadrage", "lot-08.html#autotoc_md497", null ],
        [ "Exigences couvertes", "lot-08.html#autotoc_md498", null ],
        [ "Découpage", "lot-08.html#autotoc_md499", null ],
        [ "Critères d'acceptation du lot", "lot-08.html#autotoc_md500", null ],
        [ "Dépendances", "lot-08.html#autotoc_md501", null ],
        [ "Navigation des tâches", "lot-08.html#autotoc_md502", null ],
        [ "TACHE-01 — Composants du personnage & intention d'entrée", "lot-08-tache-01-composants-personnage.html", [
          [ "Contexte", "lot-08-tache-01-composants-personnage.html#autotoc_md503", null ],
          [ "Travail à réaliser", "lot-08-tache-01-composants-personnage.html#autotoc_md504", null ],
          [ "Fichiers impactés", "lot-08-tache-01-composants-personnage.html#autotoc_md505", null ],
          [ "Tests (obligatoires)", "lot-08-tache-01-composants-personnage.html#autotoc_md506", null ],
          [ "Points d'attention", "lot-08-tache-01-composants-personnage.html#autotoc_md507", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-01-composants-personnage.html#autotoc_md508", null ],
          [ "Exigences", "lot-08-tache-01-composants-personnage.html#autotoc_md509", null ]
        ] ],
        [ "TACHE-02 — Balayage AABB contre la grille (géométrie pure)", "lot-08-tache-02-balayage-aabb.html", [
          [ "Contexte", "lot-08-tache-02-balayage-aabb.html#autotoc_md510", null ],
          [ "Travail à réaliser", "lot-08-tache-02-balayage-aabb.html#autotoc_md511", null ],
          [ "Fichiers impactés", "lot-08-tache-02-balayage-aabb.html#autotoc_md512", null ],
          [ "Tests (obligatoires)", "lot-08-tache-02-balayage-aabb.html#autotoc_md513", null ],
          [ "Points d'attention", "lot-08-tache-02-balayage-aabb.html#autotoc_md514", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-02-balayage-aabb.html#autotoc_md515", null ],
          [ "Exigences", "lot-08-tache-02-balayage-aabb.html#autotoc_md516", null ]
        ] ],
        [ "TACHE-03 — Physique du personnage (gravité + déplacement + collisions)", "lot-08-tache-03-physique-personnage.html", [
          [ "Contexte", "lot-08-tache-03-physique-personnage.html#autotoc_md517", null ],
          [ "Travail à réaliser", "lot-08-tache-03-physique-personnage.html#autotoc_md518", null ],
          [ "Fichiers impactés", "lot-08-tache-03-physique-personnage.html#autotoc_md519", null ],
          [ "Tests (obligatoires)", "lot-08-tache-03-physique-personnage.html#autotoc_md520", null ],
          [ "Points d'attention", "lot-08-tache-03-physique-personnage.html#autotoc_md521", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-03-physique-personnage.html#autotoc_md522", null ],
          [ "Exigences", "lot-08-tache-03-physique-personnage.html#autotoc_md523", null ]
        ] ],
        [ "TACHE-04 — Règles de fin de niveau (succès / échec)", "lot-08-tache-04-regles-fin-niveau.html", [
          [ "Contexte", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md524", null ],
          [ "Travail à réaliser", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md525", null ],
          [ "Fichiers impactés", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md526", null ],
          [ "Tests (obligatoires)", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md527", null ],
          [ "Points d'attention", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md528", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md529", null ],
          [ "Exigences", "lot-08-tache-04-regles-fin-niveau.html#autotoc_md530", null ]
        ] ],
        [ "TACHE-05 — Actions logiques d'entrée (mapping touches → intention)", "lot-08-tache-05-actions-logiques.html", [
          [ "Contexte", "lot-08-tache-05-actions-logiques.html#autotoc_md531", null ],
          [ "Travail à réaliser", "lot-08-tache-05-actions-logiques.html#autotoc_md532", null ],
          [ "Fichiers impactés", "lot-08-tache-05-actions-logiques.html#autotoc_md533", null ],
          [ "Tests (obligatoires)", "lot-08-tache-05-actions-logiques.html#autotoc_md534", null ],
          [ "Points d'attention", "lot-08-tache-05-actions-logiques.html#autotoc_md535", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-05-actions-logiques.html#autotoc_md536", null ],
          [ "Exigences", "lot-08-tache-05-actions-logiques.html#autotoc_md537", null ]
        ] ],
        [ "TACHE-06 — Intégration jouable dans `GameScreen` (cadrage fixe, succès / échec)", "lot-08-tache-06-integration-jouable.html", [
          [ "Contexte", "lot-08-tache-06-integration-jouable.html#autotoc_md538", null ],
          [ "Travail à réaliser", "lot-08-tache-06-integration-jouable.html#autotoc_md539", null ],
          [ "Fichiers impactés", "lot-08-tache-06-integration-jouable.html#autotoc_md540", null ],
          [ "Vérification (visuelle, pas de test unitaire)", "lot-08-tache-06-integration-jouable.html#autotoc_md541", null ],
          [ "Points d'attention", "lot-08-tache-06-integration-jouable.html#autotoc_md542", null ],
          [ "Définition de fait (DoD)", "lot-08-tache-06-integration-jouable.html#autotoc_md543", null ],
          [ "Exigences", "lot-08-tache-06-integration-jouable.html#autotoc_md544", null ]
        ] ]
      ] ],
      [ "LOT-09 — Saut, game feel et enchaînement de niveaux", "lot-09.html", [
        [ "Objectif", "lot-09.html#autotoc_md545", null ],
        [ "Périmètre", "lot-09.html#autotoc_md546", [
          [ "Inclus", "lot-09.html#autotoc_md547", null ],
          [ "Exclus (lots ultérieurs)", "lot-09.html#autotoc_md548", null ]
        ] ],
        [ "Décisions de cadrage", "lot-09.html#autotoc_md549", null ],
        [ "Exigences couvertes", "lot-09.html#autotoc_md550", null ],
        [ "Découpage", "lot-09.html#autotoc_md551", null ],
        [ "Critères d'acceptation du lot", "lot-09.html#autotoc_md552", null ],
        [ "Dépendances", "lot-09.html#autotoc_md553", null ],
        [ "Navigation des tâches", "lot-09.html#autotoc_md554", null ],
        [ "TACHE-01 — Données du saut : `PlayerInput`, `Player`, `PhysicsConfig`", "lot-09-tache-01-donnees-saut.html", [
          [ "Contexte", "lot-09-tache-01-donnees-saut.html#autotoc_md555", null ],
          [ "Travail à réaliser", "lot-09-tache-01-donnees-saut.html#autotoc_md556", null ],
          [ "Fichiers impactés", "lot-09-tache-01-donnees-saut.html#autotoc_md557", null ],
          [ "Tests (obligatoires)", "lot-09-tache-01-donnees-saut.html#autotoc_md558", null ],
          [ "Points d'attention", "lot-09-tache-01-donnees-saut.html#autotoc_md559", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-01-donnees-saut.html#autotoc_md560", null ],
          [ "Exigences", "lot-09-tache-01-donnees-saut.html#autotoc_md561", null ]
        ] ],
        [ "TACHE-02 — Mapping du saut (`Espace`/`W` → intention)", "lot-09-tache-02-mapping-saut.html", [
          [ "Contexte", "lot-09-tache-02-mapping-saut.html#autotoc_md562", null ],
          [ "Travail à réaliser", "lot-09-tache-02-mapping-saut.html#autotoc_md563", null ],
          [ "Fichiers impactés", "lot-09-tache-02-mapping-saut.html#autotoc_md564", null ],
          [ "Tests (obligatoires)", "lot-09-tache-02-mapping-saut.html#autotoc_md565", null ],
          [ "Points d'attention", "lot-09-tache-02-mapping-saut.html#autotoc_md566", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-02-mapping-saut.html#autotoc_md567", null ],
          [ "Exigences", "lot-09-tache-02-mapping-saut.html#autotoc_md568", null ]
        ] ],
        [ "TACHE-03 — Saut au sol + hauteur variable", "lot-09-tache-03-saut-hauteur-variable.html", [
          [ "Contexte", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md569", null ],
          [ "Travail à réaliser", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md570", null ],
          [ "Fichiers impactés", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md571", null ],
          [ "Tests (obligatoires)", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md572", null ],
          [ "Points d'attention", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md573", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md574", null ],
          [ "Exigences", "lot-09-tache-03-saut-hauteur-variable.html#autotoc_md575", null ]
        ] ],
        [ "TACHE-04 — Coyote time + jump buffering", "lot-09-tache-04-coyote-buffering.html", [
          [ "Contexte", "lot-09-tache-04-coyote-buffering.html#autotoc_md576", null ],
          [ "Travail à réaliser", "lot-09-tache-04-coyote-buffering.html#autotoc_md577", null ],
          [ "Fichiers impactés", "lot-09-tache-04-coyote-buffering.html#autotoc_md578", null ],
          [ "Tests (obligatoires)", "lot-09-tache-04-coyote-buffering.html#autotoc_md579", null ],
          [ "Points d'attention", "lot-09-tache-04-coyote-buffering.html#autotoc_md580", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-04-coyote-buffering.html#autotoc_md581", null ],
          [ "Exigences", "lot-09-tache-04-coyote-buffering.html#autotoc_md582", null ]
        ] ],
        [ "TACHE-05 — Enchaînement de niveaux (séquence, auto-avance, retour titre)", "lot-09-tache-05-enchainement-niveaux.html", [
          [ "Contexte", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md583", null ],
          [ "Travail à réaliser", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md584", null ],
          [ "Fichiers impactés", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md585", null ],
          [ "Vérification / tests", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md586", null ],
          [ "Points d'attention", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md587", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md588", null ],
          [ "Exigences", "lot-09-tache-05-enchainement-niveaux.html#autotoc_md589", null ]
        ] ],
        [ "TACHE-06 — Niveaux de démo (séquence, dont saut requis) + preuve", "lot-09-tache-06-niveaux-demo.html", [
          [ "Contexte", "lot-09-tache-06-niveaux-demo.html#autotoc_md590", null ],
          [ "Travail à réaliser", "lot-09-tache-06-niveaux-demo.html#autotoc_md591", null ],
          [ "Fichiers impactés", "lot-09-tache-06-niveaux-demo.html#autotoc_md592", null ],
          [ "Tests (obligatoires)", "lot-09-tache-06-niveaux-demo.html#autotoc_md593", null ],
          [ "Points d'attention", "lot-09-tache-06-niveaux-demo.html#autotoc_md594", null ],
          [ "Définition de fait (DoD)", "lot-09-tache-06-niveaux-demo.html#autotoc_md595", null ],
          [ "Exigences", "lot-09-tache-06-niveaux-demo.html#autotoc_md596", null ]
        ] ]
      ] ],
      [ "LOT-10 — Mécaniques aériennes avancées : double saut, wall jump, dash", "lot-10.html", [
        [ "Objectif", "lot-10.html#autotoc_md597", null ],
        [ "Périmètre", "lot-10.html#autotoc_md598", [
          [ "Inclus", "lot-10.html#autotoc_md599", null ],
          [ "Exclus (lots ultérieurs)", "lot-10.html#autotoc_md600", null ]
        ] ],
        [ "Décisions de cadrage", "lot-10.html#autotoc_md601", null ],
        [ "Exigences couvertes", "lot-10.html#autotoc_md602", null ],
        [ "Découpage", "lot-10.html#autotoc_md603", null ],
        [ "Critères d'acceptation du lot", "lot-10.html#autotoc_md604", null ],
        [ "Dépendances", "lot-10.html#autotoc_md605", null ],
        [ "Navigation des tâches", "lot-10.html#autotoc_md606", null ],
        [ "TACHE-01 — Données des mécaniques (`PlayerInput`, `Player`, `PhysicsConfig`)", "lot-10-tache-01-donnees.html", [
          [ "Contexte", "lot-10-tache-01-donnees.html#autotoc_md607", null ],
          [ "Travail à réaliser", "lot-10-tache-01-donnees.html#autotoc_md608", null ],
          [ "Fichiers impactés", "lot-10-tache-01-donnees.html#autotoc_md609", null ],
          [ "Tests (obligatoires)", "lot-10-tache-01-donnees.html#autotoc_md610", null ],
          [ "Points d'attention", "lot-10-tache-01-donnees.html#autotoc_md611", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-01-donnees.html#autotoc_md612", null ],
          [ "Exigences", "lot-10-tache-01-donnees.html#autotoc_md613", null ]
        ] ],
        [ "TACHE-02 — Mapping du dash + direction de visée / orientation", "lot-10-tache-02-mapping-dash.html", [
          [ "Contexte", "lot-10-tache-02-mapping-dash.html#autotoc_md614", null ],
          [ "Travail à réaliser", "lot-10-tache-02-mapping-dash.html#autotoc_md615", null ],
          [ "Fichiers impactés", "lot-10-tache-02-mapping-dash.html#autotoc_md616", null ],
          [ "Tests (obligatoires)", "lot-10-tache-02-mapping-dash.html#autotoc_md617", null ],
          [ "Points d'attention", "lot-10-tache-02-mapping-dash.html#autotoc_md618", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-02-mapping-dash.html#autotoc_md619", null ],
          [ "Exigences", "lot-10-tache-02-mapping-dash.html#autotoc_md620", null ]
        ] ],
        [ "TACHE-03 — Double saut (sauts aériens rechargés au sol)", "lot-10-tache-03-double-saut.html", [
          [ "Contexte", "lot-10-tache-03-double-saut.html#autotoc_md621", null ],
          [ "Travail à réaliser", "lot-10-tache-03-double-saut.html#autotoc_md622", null ],
          [ "Fichiers impactés", "lot-10-tache-03-double-saut.html#autotoc_md623", null ],
          [ "Tests (obligatoires)", "lot-10-tache-03-double-saut.html#autotoc_md624", null ],
          [ "Points d'attention", "lot-10-tache-03-double-saut.html#autotoc_md625", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-03-double-saut.html#autotoc_md626", null ],
          [ "Exigences", "lot-10-tache-03-double-saut.html#autotoc_md627", null ]
        ] ],
        [ "TACHE-04 — Wall jump + wall slide", "lot-10-tache-04-wall-jump.html", [
          [ "Contexte", "lot-10-tache-04-wall-jump.html#autotoc_md628", null ],
          [ "Travail à réaliser", "lot-10-tache-04-wall-jump.html#autotoc_md629", null ],
          [ "Fichiers impactés", "lot-10-tache-04-wall-jump.html#autotoc_md630", null ],
          [ "Tests (obligatoires)", "lot-10-tache-04-wall-jump.html#autotoc_md631", null ],
          [ "Points d'attention", "lot-10-tache-04-wall-jump.html#autotoc_md632", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-04-wall-jump.html#autotoc_md633", null ],
          [ "Exigences", "lot-10-tache-04-wall-jump.html#autotoc_md634", null ]
        ] ],
        [ "TACHE-05 — Dash 8 directions (burst, durée, recharge au sol)", "lot-10-tache-05-dash.html", [
          [ "Contexte", "lot-10-tache-05-dash.html#autotoc_md635", null ],
          [ "Travail à réaliser", "lot-10-tache-05-dash.html#autotoc_md636", null ],
          [ "Fichiers impactés", "lot-10-tache-05-dash.html#autotoc_md637", null ],
          [ "Tests (obligatoires)", "lot-10-tache-05-dash.html#autotoc_md638", null ],
          [ "Points d'attention", "lot-10-tache-05-dash.html#autotoc_md639", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-05-dash.html#autotoc_md640", null ],
          [ "Exigences", "lot-10-tache-05-dash.html#autotoc_md641", null ]
        ] ],
        [ "TACHE-06 — Niveau de démo « parkour » + preuve système", "lot-10-tache-06-niveau-parkour.html", [
          [ "Contexte", "lot-10-tache-06-niveau-parkour.html#autotoc_md642", null ],
          [ "Travail à réaliser", "lot-10-tache-06-niveau-parkour.html#autotoc_md643", null ],
          [ "Fichiers impactés", "lot-10-tache-06-niveau-parkour.html#autotoc_md644", null ],
          [ "Tests (obligatoires)", "lot-10-tache-06-niveau-parkour.html#autotoc_md645", null ],
          [ "Points d'attention", "lot-10-tache-06-niveau-parkour.html#autotoc_md646", null ],
          [ "Définition de fait (DoD)", "lot-10-tache-06-niveau-parkour.html#autotoc_md647", null ],
          [ "Exigences", "lot-10-tache-06-niveau-parkour.html#autotoc_md648", null ]
        ] ]
      ] ],
      [ "LOT-11 — Ressenti avancé : personnage humanoïde, gravité asymétrique, finitions", "lot-11.html", [
        [ "Objectif", "lot-11.html#autotoc_md649", null ],
        [ "Périmètre", "lot-11.html#autotoc_md650", [
          [ "Inclus", "lot-11.html#autotoc_md651", null ],
          [ "Exclus (lots ultérieurs)", "lot-11.html#autotoc_md652", null ]
        ] ],
        [ "Décisions de cadrage", "lot-11.html#autotoc_md653", null ],
        [ "Exigences couvertes", "lot-11.html#autotoc_md654", null ],
        [ "Découpage", "lot-11.html#autotoc_md655", null ],
        [ "Critères d'acceptation du lot", "lot-11.html#autotoc_md656", null ],
        [ "Dépendances", "lot-11.html#autotoc_md657", null ],
        [ "Navigation des tâches", "lot-11.html#autotoc_md658", null ],
        [ "TACHE-01 — Données : réglages de *feel* + taille/placement du personnage", "lot-11-tache-01-donnees.html", [
          [ "Contexte", "lot-11-tache-01-donnees.html#autotoc_md659", null ],
          [ "Travail à réaliser", "lot-11-tache-01-donnees.html#autotoc_md660", null ],
          [ "Fichiers impactés", "lot-11-tache-01-donnees.html#autotoc_md661", null ],
          [ "Tests (obligatoires)", "lot-11-tache-01-donnees.html#autotoc_md662", null ],
          [ "Points d'attention", "lot-11-tache-01-donnees.html#autotoc_md663", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-01-donnees.html#autotoc_md664", null ],
          [ "Exigences", "lot-11-tache-01-donnees.html#autotoc_md665", null ]
        ] ],
        [ "TACHE-02 — Gravité asymétrique + apex hang + fast-fall", "lot-11-tache-02-gravite-asymetrique.html", [
          [ "Contexte", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md666", null ],
          [ "Travail à réaliser", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md667", null ],
          [ "Fichiers impactés", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md668", null ],
          [ "Tests (obligatoires)", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md669", null ],
          [ "Points d'attention", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md670", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md671", null ],
          [ "Exigences", "lot-11-tache-02-gravite-asymetrique.html#autotoc_md672", null ]
        ] ],
        [ "TACHE-03 — Personnage humanoïde (spawn 0,4×0,8, sprite)", "lot-11-tache-03-personnage-humanoide.html", [
          [ "Contexte", "lot-11-tache-03-personnage-humanoide.html#autotoc_md673", null ],
          [ "Travail à réaliser", "lot-11-tache-03-personnage-humanoide.html#autotoc_md674", null ],
          [ "Fichiers impactés", "lot-11-tache-03-personnage-humanoide.html#autotoc_md675", null ],
          [ "Vérification (visuelle, pas de test unitaire — brique GPU)", "lot-11-tache-03-personnage-humanoide.html#autotoc_md676", null ],
          [ "Points d'attention", "lot-11-tache-03-personnage-humanoide.html#autotoc_md677", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-03-personnage-humanoide.html#autotoc_md678", null ],
          [ "Exigences", "lot-11-tache-03-personnage-humanoide.html#autotoc_md679", null ]
        ] ],
        [ "TACHE-04 — Rééquilibrage des niveaux + preuves à la vraie taille", "lot-11-tache-04-reequilibrage.html", [
          [ "Contexte", "lot-11-tache-04-reequilibrage.html#autotoc_md680", null ],
          [ "Travail à réaliser", "lot-11-tache-04-reequilibrage.html#autotoc_md681", null ],
          [ "Fichiers impactés", "lot-11-tache-04-reequilibrage.html#autotoc_md682", null ],
          [ "Tests (obligatoires)", "lot-11-tache-04-reequilibrage.html#autotoc_md683", null ],
          [ "Points d'attention", "lot-11-tache-04-reequilibrage.html#autotoc_md684", null ],
          [ "Définition de fait (DoD)", "lot-11-tache-04-reequilibrage.html#autotoc_md685", null ],
          [ "Exigences", "lot-11-tache-04-reequilibrage.html#autotoc_md686", null ]
        ] ]
      ] ],
      [ "LOT-12 — Niveau puzzle : mécanismes interrupteur/porte + budget de mouvements", "lot-12.html", [
        [ "Objectif", "lot-12.html#autotoc_md687", null ],
        [ "Périmètre", "lot-12.html#autotoc_md688", [
          [ "Inclus", "lot-12.html#autotoc_md689", null ],
          [ "Exclus (lots ultérieurs)", "lot-12.html#autotoc_md690", null ]
        ] ],
        [ "Décisions de cadrage", "lot-12.html#autotoc_md691", null ],
        [ "Exigences couvertes", "lot-12.html#autotoc_md692", null ],
        [ "Découpage", "lot-12.html#autotoc_md693", null ],
        [ "Critères d'acceptation du lot", "lot-12.html#autotoc_md694", null ],
        [ "Dépendances", "lot-12.html#autotoc_md695", null ],
        [ "Navigation des tâches", "lot-12.html#autotoc_md696", null ],
        [ "TACHE-01 — Données : budget (`Player`, `Level`, `LevelLoader`)", "lot-12-tache-01-donnees.html", [
          [ "Contexte", "lot-12-tache-01-donnees.html#autotoc_md697", null ],
          [ "Travail à réaliser", "lot-12-tache-01-donnees.html#autotoc_md698", null ],
          [ "Fichiers impactés", "lot-12-tache-01-donnees.html#autotoc_md699", null ],
          [ "Tests (obligatoires)", "lot-12-tache-01-donnees.html#autotoc_md700", null ],
          [ "Points d'attention", "lot-12-tache-01-donnees.html#autotoc_md701", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-01-donnees.html#autotoc_md702", null ],
          [ "Exigences", "lot-12-tache-01-donnees.html#autotoc_md703", null ]
        ] ],
        [ "TACHE-02 — Mécanismes interrupteur/porte (`MechanismController`)", "lot-12-tache-02-mecanismes.html", [
          [ "Contexte", "lot-12-tache-02-mecanismes.html#autotoc_md704", null ],
          [ "Travail à réaliser", "lot-12-tache-02-mecanismes.html#autotoc_md705", null ],
          [ "Fichiers impactés", "lot-12-tache-02-mecanismes.html#autotoc_md706", null ],
          [ "Tests (obligatoires)", "lot-12-tache-02-mecanismes.html#autotoc_md707", null ],
          [ "Points d'attention", "lot-12-tache-02-mecanismes.html#autotoc_md708", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-02-mecanismes.html#autotoc_md709", null ],
          [ "Exigences", "lot-12-tache-02-mecanismes.html#autotoc_md710", null ]
        ] ],
        [ "TACHE-03 — Budget de sauts/dashs dans la physique", "lot-12-tache-03-budget.html", [
          [ "Contexte", "lot-12-tache-03-budget.html#autotoc_md711", null ],
          [ "Travail à réaliser", "lot-12-tache-03-budget.html#autotoc_md712", null ],
          [ "Fichiers impactés", "lot-12-tache-03-budget.html#autotoc_md713", null ],
          [ "Tests (obligatoires)", "lot-12-tache-03-budget.html#autotoc_md714", null ],
          [ "Points d'attention", "lot-12-tache-03-budget.html#autotoc_md715", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-03-budget.html#autotoc_md716", null ],
          [ "Exigences", "lot-12-tache-03-budget.html#autotoc_md717", null ]
        ] ],
        [ "TACHE-04 — Intégration `GameScreen` + niveau `demo4` + preuves", "lot-12-tache-04-integration-puzzle.html", [
          [ "Contexte", "lot-12-tache-04-integration-puzzle.html#autotoc_md718", null ],
          [ "Travail à réaliser", "lot-12-tache-04-integration-puzzle.html#autotoc_md719", null ],
          [ "Fichiers impactés", "lot-12-tache-04-integration-puzzle.html#autotoc_md720", null ],
          [ "Tests (obligatoires)", "lot-12-tache-04-integration-puzzle.html#autotoc_md721", null ],
          [ "Points d'attention", "lot-12-tache-04-integration-puzzle.html#autotoc_md722", null ],
          [ "Définition de fait (DoD)", "lot-12-tache-04-integration-puzzle.html#autotoc_md723", null ],
          [ "Exigences", "lot-12-tache-04-integration-puzzle.html#autotoc_md724", null ]
        ] ]
      ] ],
      [ "LOT-13 — Consolidation de la documentation", "lot-13.html", [
        [ "Objectif", "lot-13.html#autotoc_md725", null ],
        [ "Périmètre", "lot-13.html#autotoc_md726", [
          [ "Inclus", "lot-13.html#autotoc_md727", null ],
          [ "Exclus", "lot-13.html#autotoc_md728", null ]
        ] ],
        [ "Décisions de cadrage", "lot-13.html#autotoc_md729", null ],
        [ "Exigences couvertes", "lot-13.html#autotoc_md730", null ],
        [ "Découpage", "lot-13.html#autotoc_md731", null ],
        [ "Critères d'acceptation du lot", "lot-13.html#autotoc_md732", null ],
        [ "Dépendances", "lot-13.html#autotoc_md733", null ]
      ] ]
    ] ],
    [ "Manuel utilisateur", "manuel.html", [
      [ "Pages", "manuel.html#autotoc_md735", null ],
      [ "À venir", "manuel.html#autotoc_md736", null ],
      [ "Télécharger et lancer le jeu", "manuel-telecharger.html", [
        [ "Prérequis", "manuel-telecharger.html#autotoc_md737", null ],
        [ "Étapes", "manuel-telecharger.html#autotoc_md738", null ],
        [ "Remarques", "manuel-telecharger.html#autotoc_md739", null ]
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
"Sprite_8h.html",
"classcore_1_1MovementSystem.html#af4476267f1301a583126866e01567b1f",
"classhmi_1_1GraphicsDevice.html#a8f0ce520f7a676109374adafc4bd6669",
"classhmi_1_1SpriteBatch.html",
"functions_x.html",
"lot-04-tache-04-pages-specification.html#autotoc_md292",
"lot-08-tache-02-balayage-aabb.html#autotoc_md512",
"lot-12.html#autotoc_md695",
"structcore_1_1Entity.html#a9e7053bba3d45fb33819fc95e9a82989",
"structhmi_1_1SpriteBatch_1_1Vertex.html#a9a81b3d446ca4b8deda6cfec5b7376fe",
"test__session__log_8cpp.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
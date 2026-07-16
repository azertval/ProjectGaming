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
    [ "Spécifications", "specifications.html", [
      [ "Documents", "specifications.html#autotoc_md79", null ],
      [ "Vision & périmètre", "spec-vision.html", [
        [ "Concept", "spec-vision.html#autotoc_md80", [
          [ "Mécanique de jeu (décidée)", "spec-vision.html#autotoc_md81", null ]
        ] ],
        [ "Boucle de gameplay", "spec-vision.html#autotoc_md82", null ],
        [ "Objectifs (MVP)", "spec-vision.html#autotoc_md83", null ],
        [ "Objectifs produit (au-delà du moteur)", "spec-vision.html#autotoc_md84", null ],
        [ "Hors périmètre (MVP)", "spec-vision.html#autotoc_md85", null ],
        [ "Traçabilité", "spec-vision.html#autotoc_md86", null ]
      ] ],
      [ "Gameplay", "spec-gameplay.html", [
        [ "1. Monde en tuiles", "spec-gameplay.html#autotoc_md61", null ],
        [ "2. Personnage & déplacement", "spec-gameplay.html#autotoc_md62", [
          [ "Ressenti (game feel) — ⚠️ à affiner par tests", "spec-gameplay.html#autotoc_md63", null ]
        ] ],
        [ "3. Mécanismes de puzzle", "spec-gameplay.html#autotoc_md64", null ],
        [ "4. Conditions de fin de niveau", "spec-gameplay.html#autotoc_md65", null ],
        [ "5. États de jeu", "spec-gameplay.html#autotoc_md66", null ],
        [ "Traçabilité", "spec-gameplay.html#autotoc_md67", null ]
      ] ],
      [ "Contrôles & entrées", "spec-controles.html", [
        [ "1. Périphériques", "spec-controles.html#autotoc_md16", null ],
        [ "2. Actions du jeu (mapping logique)", "spec-controles.html#autotoc_md17", null ],
        [ "3. Réactivité", "spec-controles.html#autotoc_md18", null ],
        [ "Traçabilité", "spec-controles.html#autotoc_md19", null ]
      ] ],
      [ "Rendu & cible technique", "spec-rendu-technique.html", [
        [ "1. Cible technique", "spec-rendu-technique.html#autotoc_md73", null ],
        [ "2. Rendu 2D", "spec-rendu-technique.html#autotoc_md74", null ],
        [ "3. Boucle & temps", "spec-rendu-technique.html#autotoc_md75", null ],
        [ "4. Interface (HMI)", "spec-rendu-technique.html#autotoc_md76", null ],
        [ "5. Audio (⚠️ minimal MVP)", "spec-rendu-technique.html#autotoc_md77", null ],
        [ "Traçabilité", "spec-rendu-technique.html#autotoc_md78", null ]
      ] ],
      [ "Niveaux & contenu", "spec-niveaux.html", [
        [ "1. Représentation des niveaux", "spec-niveaux.html#autotoc_md68", [
          [ "Format retenu (JSON, liste de tuiles-objets)", "spec-niveaux.html#autotoc_md69", null ]
        ] ],
        [ "2. Progression", "spec-niveaux.html#autotoc_md70", null ],
        [ "3. Conception (lignes directrices)", "spec-niveaux.html#autotoc_md71", null ],
        [ "Traçabilité", "spec-niveaux.html#autotoc_md72", null ]
      ] ],
      [ "Exigences non fonctionnelles", "spec-exigences.html", [
        [ "1. Performance", "spec-exigences.html#autotoc_md55", null ],
        [ "2. Architecture & maintenabilité", "spec-exigences.html#autotoc_md56", null ],
        [ "3. Qualité & vérification", "spec-exigences.html#autotoc_md57", null ],
        [ "4. Portabilité & reproductibilité", "spec-exigences.html#autotoc_md58", null ],
        [ "5. Robustesse", "spec-exigences.html#autotoc_md59", null ],
        [ "Traçabilité", "spec-exigences.html#autotoc_md60", null ]
      ] ],
      [ "Éditeur de niveaux", "spec-editeur.html", [
        [ "Objectif", "spec-editeur.html#autotoc_md47", null ],
        [ "1. Exigences fonctionnelles", "spec-editeur.html#autotoc_md48", null ],
        [ "2. Réutilisation & cohérence", "spec-editeur.html#autotoc_md49", null ],
        [ "3. Distribution & collaboration", "spec-editeur.html#autotoc_md50", null ],
        [ "4. Approche d'implémentation (décidée)", "spec-editeur.html#autotoc_md51", null ],
        [ "4bis. Décors & pixel art (post-MVP, intégré à l'éditeur)", "spec-editeur.html#autotoc_md52", null ],
        [ "5. Non-objectifs (éditeur, MVP)", "spec-editeur.html#autotoc_md53", null ],
        [ "Traçabilité", "spec-editeur.html#autotoc_md54", null ]
      ] ],
      [ "Architecture (décisions dimensionnantes)", "spec-architecture.html", [
        [ "1. Modules & dépendances", "spec-architecture.html#autotoc_md4", null ],
        [ "2. Modèle d'entités : ECS", "spec-architecture.html#autotoc_md5", null ],
        [ "3. Coordonnées & unités — trois espaces distincts", "spec-architecture.html#autotoc_md6", null ],
        [ "4. Frontière simulation ↔ rendu", "spec-architecture.html#autotoc_md7", null ],
        [ "5. Mathématiques dans <tt>Core</tt>", "spec-architecture.html#autotoc_md8", null ],
        [ "6. Abstraction de rendu", "spec-architecture.html#autotoc_md9", null ],
        [ "7. Modèle de threading", "spec-architecture.html#autotoc_md10", null ],
        [ "8. Communication inter-systèmes", "spec-architecture.html#autotoc_md11", null ],
        [ "9. Gestion des ressources", "spec-architecture.html#autotoc_md12", null ],
        [ "10. Contrainte « éditeur intégré »", "spec-architecture.html#autotoc_md13", null ],
        [ "11. Décors dynamiques (accommodation dimensionnante)", "spec-architecture.html#autotoc_md14", null ],
        [ "Traçabilité", "spec-architecture.html#autotoc_md15", null ]
      ] ],
      [ "Décors & pipeline pixel art", "spec-decors.html", [
        [ "Vision", "spec-decors.html#autotoc_md39", null ],
        [ "1. Système de décors", "spec-decors.html#autotoc_md40", null ],
        [ "2. Manipulation", "spec-decors.html#autotoc_md41", [
          [ "À la conception (éditeur)", "spec-decors.html#autotoc_md42", null ],
          [ "En jeu (mécanique, à terme)", "spec-decors.html#autotoc_md43", null ]
        ] ],
        [ "3. Pipeline photo → pixel art (intégré à l'éditeur)", "spec-decors.html#autotoc_md44", null ],
        [ "4. Périmètre & séquencement", "spec-decors.html#autotoc_md45", null ],
        [ "Traçabilité", "spec-decors.html#autotoc_md46", null ]
      ] ],
      [ "Conventions de code", "spec-conventions.html", [
        [ "1. Langage & standard", "spec-conventions.html#autotoc_md21", null ],
        [ "2. Nommage", "spec-conventions.html#autotoc_md22", null ],
        [ "3. Mise en forme", "spec-conventions.html#autotoc_md23", null ],
        [ "4. Inclusions (<tt>#include</tt>)", "spec-conventions.html#autotoc_md24", [
          [ "Chemins complets depuis <tt>Source/</tt>", "spec-conventions.html#autotoc_md25", null ],
          [ "Ordre des groupes", "spec-conventions.html#autotoc_md26", null ]
        ] ],
        [ "5. Architecture (dépendances entre modules)", "spec-conventions.html#autotoc_md27", [
          [ "Classes plutôt que fonctions libres", "spec-conventions.html#autotoc_md28", null ],
          [ "RAII obligatoire", "spec-conventions.html#autotoc_md29", null ]
        ] ],
        [ "6. Documentation Doxygen", "spec-conventions.html#autotoc_md30", [
          [ "Doxygen dans le header, commentaires simples <tt>//</tt> dans le <tt>.cpp</tt>", "spec-conventions.html#autotoc_md31", null ],
          [ "Documentation du corps (<tt>.cpp</tt>)", "spec-conventions.html#autotoc_md32", null ]
        ] ],
        [ "7. Bonnes pratiques", "spec-conventions.html#autotoc_md33", null ],
        [ "8. Tests", "spec-conventions.html#autotoc_md34", null ],
        [ "9. Gestion des erreurs", "spec-conventions.html#autotoc_md35", null ],
        [ "10. Assertions & journalisation", "spec-conventions.html#autotoc_md36", null ],
        [ "11. Outillage qualité (automatisé)", "spec-conventions.html#autotoc_md37", null ],
        [ "12. Identifiants d'exigences (<tt>EX-…</tt>)", "spec-conventions.html#autotoc_md38", null ]
      ] ]
    ] ],
    [ "Lots", "lots.html", [
      [ "Lots", "lots.html#autotoc_md459", null ],
      [ "LOT-01 — Fenêtre & boucle de jeu (Direct3D 11)", "lot-01.html", [
        [ "Objectif", "lot-01.html#autotoc_md87", null ],
        [ "Périmètre", "lot-01.html#autotoc_md88", [
          [ "Inclus", "lot-01.html#autotoc_md89", null ],
          [ "Exclus (lots ultérieurs)", "lot-01.html#autotoc_md90", null ]
        ] ],
        [ "Exigences couvertes", "lot-01.html#autotoc_md91", null ],
        [ "Découpage", "lot-01.html#autotoc_md92", null ],
        [ "Critères d'acceptation du lot", "lot-01.html#autotoc_md93", null ],
        [ "Navigation des tâches", "lot-01.html#autotoc_md94", null ],
        [ "TACHE-01 — Fenêtre Win32 & pompe de messages", "lot-01-tache-01-fenetre-win32.html", [
          [ "Contexte", "lot-01-tache-01-fenetre-win32.html#autotoc_md95", null ],
          [ "Travail à réaliser", "lot-01-tache-01-fenetre-win32.html#autotoc_md96", null ],
          [ "Fichiers impactés", "lot-01-tache-01-fenetre-win32.html#autotoc_md97", null ],
          [ "Points d'attention", "lot-01-tache-01-fenetre-win32.html#autotoc_md98", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-01-fenetre-win32.html#autotoc_md99", null ],
          [ "Exigences", "lot-01-tache-01-fenetre-win32.html#autotoc_md100", null ]
        ] ],
        [ "TACHE-02 — Initialisation Direct3D 11 (RAII)", "lot-01-tache-02-init-direct3d11.html", [
          [ "Contexte", "lot-01-tache-02-init-direct3d11.html#autotoc_md101", null ],
          [ "Travail à réaliser", "lot-01-tache-02-init-direct3d11.html#autotoc_md102", null ],
          [ "Fichiers impactés", "lot-01-tache-02-init-direct3d11.html#autotoc_md103", null ],
          [ "Points d'attention", "lot-01-tache-02-init-direct3d11.html#autotoc_md104", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-02-init-direct3d11.html#autotoc_md105", null ],
          [ "Exigences", "lot-01-tache-02-init-direct3d11.html#autotoc_md106", null ]
        ] ],
        [ "TACHE-03 — Boucle à pas de temps fixe (testable)", "lot-01-tache-03-boucle-pas-fixe.html", [
          [ "Contexte", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md107", null ],
          [ "Travail à réaliser", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md108", null ],
          [ "Fichiers impactés", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md109", null ],
          [ "Tests (obligatoires)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md110", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md111", null ],
          [ "Exigences", "lot-01-tache-03-boucle-pas-fixe.html#autotoc_md112", null ]
        ] ],
        [ "TACHE-04 — Effacement écran, présentation & redimensionnement", "lot-01-tache-04-effacement-presentation.html", [
          [ "Contexte", "lot-01-tache-04-effacement-presentation.html#autotoc_md113", null ],
          [ "Travail à réaliser", "lot-01-tache-04-effacement-presentation.html#autotoc_md114", null ],
          [ "Fichiers impactés", "lot-01-tache-04-effacement-presentation.html#autotoc_md115", null ],
          [ "Points d'attention", "lot-01-tache-04-effacement-presentation.html#autotoc_md116", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-04-effacement-presentation.html#autotoc_md117", null ],
          [ "Exigences", "lot-01-tache-04-effacement-presentation.html#autotoc_md118", null ]
        ] ],
        [ "TACHE-05 — Intégration `main` & vérification", "lot-01-tache-05-integration.html", [
          [ "Contexte", "lot-01-tache-05-integration.html#autotoc_md119", null ],
          [ "Travail à réaliser", "lot-01-tache-05-integration.html#autotoc_md120", null ],
          [ "Fichiers impactés", "lot-01-tache-05-integration.html#autotoc_md121", null ],
          [ "Vérification (manuelle + automatique)", "lot-01-tache-05-integration.html#autotoc_md122", null ],
          [ "Définition de fait (DoD)", "lot-01-tache-05-integration.html#autotoc_md123", null ],
          [ "Exigences", "lot-01-tache-05-integration.html#autotoc_md124", null ]
        ] ]
      ] ],
      [ "LOT-02 — Journalisation & diagnostics", "lot-02.html", [
        [ "Objectif", "lot-02.html#autotoc_md125", null ],
        [ "Périmètre", "lot-02.html#autotoc_md126", [
          [ "Inclus", "lot-02.html#autotoc_md127", null ],
          [ "Exclus (plus tard)", "lot-02.html#autotoc_md128", null ]
        ] ],
        [ "Exigences couvertes", "lot-02.html#autotoc_md129", null ],
        [ "Découpage", "lot-02.html#autotoc_md130", null ],
        [ "Critères d'acceptation du lot", "lot-02.html#autotoc_md131", null ],
        [ "Dépendances", "lot-02.html#autotoc_md132", null ],
        [ "Navigation des tâches", "lot-02.html#autotoc_md133", null ],
        [ "TACHE-01 — Niveaux de log & interface `Logger`", "lot-02-tache-01-niveaux-logger.html", [
          [ "Contexte", "lot-02-tache-01-niveaux-logger.html#autotoc_md134", null ],
          [ "Travail à réaliser", "lot-02-tache-01-niveaux-logger.html#autotoc_md135", null ],
          [ "Fichiers impactés", "lot-02-tache-01-niveaux-logger.html#autotoc_md136", null ],
          [ "Tests (obligatoires)", "lot-02-tache-01-niveaux-logger.html#autotoc_md137", null ],
          [ "Points d'attention", "lot-02-tache-01-niveaux-logger.html#autotoc_md138", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-01-niveaux-logger.html#autotoc_md139", null ],
          [ "Exigences", "lot-02-tache-01-niveaux-logger.html#autotoc_md140", null ]
        ] ],
        [ "TACHE-02 — Sinks enfichables", "lot-02-tache-02-sinks.html", [
          [ "Contexte", "lot-02-tache-02-sinks.html#autotoc_md141", null ],
          [ "Travail à réaliser", "lot-02-tache-02-sinks.html#autotoc_md142", null ],
          [ "Fichiers impactés", "lot-02-tache-02-sinks.html#autotoc_md143", null ],
          [ "Tests (obligatoires)", "lot-02-tache-02-sinks.html#autotoc_md144", null ],
          [ "Points d'attention", "lot-02-tache-02-sinks.html#autotoc_md145", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-02-sinks.html#autotoc_md146", null ],
          [ "Exigences", "lot-02-tache-02-sinks.html#autotoc_md147", null ]
        ] ],
        [ "TACHE-03 — Macros de log (fichier/ligne, horodatage)", "lot-02-tache-03-macros-log.html", [
          [ "Contexte", "lot-02-tache-03-macros-log.html#autotoc_md148", null ],
          [ "Travail à réaliser", "lot-02-tache-03-macros-log.html#autotoc_md149", null ],
          [ "Fichiers impactés", "lot-02-tache-03-macros-log.html#autotoc_md150", null ],
          [ "Tests (obligatoires)", "lot-02-tache-03-macros-log.html#autotoc_md151", null ],
          [ "Points d'attention", "lot-02-tache-03-macros-log.html#autotoc_md152", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-03-macros-log.html#autotoc_md153", null ],
          [ "Exigences", "lot-02-tache-03-macros-log.html#autotoc_md154", null ]
        ] ],
        [ "TACHE-04 — Assertions `PROJECTGAMING_ASSERT`", "lot-02-tache-04-assertions.html", [
          [ "Contexte", "lot-02-tache-04-assertions.html#autotoc_md155", null ],
          [ "Travail à réaliser", "lot-02-tache-04-assertions.html#autotoc_md156", null ],
          [ "Fichiers impactés", "lot-02-tache-04-assertions.html#autotoc_md157", null ],
          [ "Tests (obligatoires)", "lot-02-tache-04-assertions.html#autotoc_md158", null ],
          [ "Points d'attention", "lot-02-tache-04-assertions.html#autotoc_md159", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-04-assertions.html#autotoc_md160", null ],
          [ "Exigences", "lot-02-tache-04-assertions.html#autotoc_md161", null ]
        ] ],
        [ "TACHE-05 — Intégration dans `main` & documentation", "lot-02-tache-05-integration.html", [
          [ "Contexte", "lot-02-tache-05-integration.html#autotoc_md162", null ],
          [ "Travail à réaliser", "lot-02-tache-05-integration.html#autotoc_md163", null ],
          [ "Fichiers impactés", "lot-02-tache-05-integration.html#autotoc_md164", null ],
          [ "Vérification", "lot-02-tache-05-integration.html#autotoc_md165", null ],
          [ "Définition de fait (DoD)", "lot-02-tache-05-integration.html#autotoc_md166", null ],
          [ "Exigences", "lot-02-tache-05-integration.html#autotoc_md167", null ]
        ] ]
      ] ],
      [ "LOT-03 — Fondation ECS & mathématiques `Core`", "lot-03.html", [
        [ "Objectif", "lot-03.html#autotoc_md168", null ],
        [ "⚠️ Décision préalable : ECS maison vs bibliothèque", "lot-03.html#autotoc_md169", null ],
        [ "Périmètre", "lot-03.html#autotoc_md170", [
          [ "Inclus", "lot-03.html#autotoc_md171", null ],
          [ "Exclus (lots ultérieurs)", "lot-03.html#autotoc_md172", null ]
        ] ],
        [ "Exigences couvertes", "lot-03.html#autotoc_md173", null ],
        [ "Découpage", "lot-03.html#autotoc_md174", null ],
        [ "Critères d'acceptation du lot", "lot-03.html#autotoc_md175", null ],
        [ "Dépendances", "lot-03.html#autotoc_md176", null ],
        [ "Navigation des tâches", "lot-03.html#autotoc_md177", null ],
        [ "TACHE-01 — Types mathématiques de `Core`", "lot-03-tache-01-math-core.html", [
          [ "Contexte", "lot-03-tache-01-math-core.html#autotoc_md178", null ],
          [ "Travail à réaliser", "lot-03-tache-01-math-core.html#autotoc_md179", null ],
          [ "Fichiers impactés", "lot-03-tache-01-math-core.html#autotoc_md180", null ],
          [ "Tests (obligatoires)", "lot-03-tache-01-math-core.html#autotoc_md181", null ],
          [ "Points d'attention", "lot-03-tache-01-math-core.html#autotoc_md182", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-01-math-core.html#autotoc_md183", null ],
          [ "Exigences", "lot-03-tache-01-math-core.html#autotoc_md184", null ]
        ] ],
        [ "TACHE-02 — Entités : handles générationnels & cycle de vie", "lot-03-tache-02-entites.html", [
          [ "Contexte", "lot-03-tache-02-entites.html#autotoc_md185", null ],
          [ "Travail à réaliser", "lot-03-tache-02-entites.html#autotoc_md186", null ],
          [ "Fichiers impactés", "lot-03-tache-02-entites.html#autotoc_md187", null ],
          [ "Tests (obligatoires)", "lot-03-tache-02-entites.html#autotoc_md188", null ],
          [ "Points d'attention", "lot-03-tache-02-entites.html#autotoc_md189", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-02-entites.html#autotoc_md190", null ],
          [ "Exigences", "lot-03-tache-02-entites.html#autotoc_md191", null ]
        ] ],
        [ "TACHE-03 — Stockage de composants (sparse set typé)", "lot-03-tache-03-stockage-composants.html", [
          [ "Contexte", "lot-03-tache-03-stockage-composants.html#autotoc_md192", null ],
          [ "Travail à réaliser", "lot-03-tache-03-stockage-composants.html#autotoc_md193", null ],
          [ "Fichiers impactés", "lot-03-tache-03-stockage-composants.html#autotoc_md194", null ],
          [ "Tests (obligatoires)", "lot-03-tache-03-stockage-composants.html#autotoc_md195", null ],
          [ "Points d'attention", "lot-03-tache-03-stockage-composants.html#autotoc_md196", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-03-stockage-composants.html#autotoc_md197", null ],
          [ "Exigences", "lot-03-tache-03-stockage-composants.html#autotoc_md198", null ]
        ] ],
        [ "TACHE-04 — Requêtes / vues multi-composants", "lot-03-tache-04-vues-requetes.html", [
          [ "Contexte", "lot-03-tache-04-vues-requetes.html#autotoc_md199", null ],
          [ "Travail à réaliser", "lot-03-tache-04-vues-requetes.html#autotoc_md200", null ],
          [ "Fichiers impactés", "lot-03-tache-04-vues-requetes.html#autotoc_md201", null ],
          [ "Tests (obligatoires)", "lot-03-tache-04-vues-requetes.html#autotoc_md202", null ],
          [ "Points d'attention", "lot-03-tache-04-vues-requetes.html#autotoc_md203", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-04-vues-requetes.html#autotoc_md204", null ],
          [ "Exigences", "lot-03-tache-04-vues-requetes.html#autotoc_md205", null ]
        ] ],
        [ "TACHE-05 — Systèmes & `World` (orchestration au pas fixe)", "lot-03-tache-05-systemes-world.html", [
          [ "Contexte", "lot-03-tache-05-systemes-world.html#autotoc_md206", null ],
          [ "Travail à réaliser", "lot-03-tache-05-systemes-world.html#autotoc_md207", null ],
          [ "Fichiers impactés", "lot-03-tache-05-systemes-world.html#autotoc_md208", null ],
          [ "Tests (obligatoires)", "lot-03-tache-05-systemes-world.html#autotoc_md209", null ],
          [ "Points d'attention", "lot-03-tache-05-systemes-world.html#autotoc_md210", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-05-systemes-world.html#autotoc_md211", null ],
          [ "Exigences", "lot-03-tache-05-systemes-world.html#autotoc_md212", null ]
        ] ],
        [ "TACHE-06 — Composant `Transform` + système de mouvement (démo)", "lot-03-tache-06-transform-demo.html", [
          [ "Contexte", "lot-03-tache-06-transform-demo.html#autotoc_md213", null ],
          [ "Travail à réaliser", "lot-03-tache-06-transform-demo.html#autotoc_md214", null ],
          [ "Fichiers impactés", "lot-03-tache-06-transform-demo.html#autotoc_md215", null ],
          [ "Tests (obligatoires)", "lot-03-tache-06-transform-demo.html#autotoc_md216", null ],
          [ "Points d'attention", "lot-03-tache-06-transform-demo.html#autotoc_md217", null ],
          [ "Définition de fait (DoD)", "lot-03-tache-06-transform-demo.html#autotoc_md218", null ],
          [ "Exigences", "lot-03-tache-06-transform-demo.html#autotoc_md219", null ]
        ] ]
      ] ],
      [ "LOT-04 — Documentation Doxygen & réorganisation de l'arborescence documentaire", "lot-04.html", [
        [ "Objectif", "lot-04.html#autotoc_md220", null ],
        [ "Périmètre", "lot-04.html#autotoc_md221", [
          [ "Inclus", "lot-04.html#autotoc_md222", null ],
          [ "Exclus (lots ultérieurs)", "lot-04.html#autotoc_md223", null ]
        ] ],
        [ "Décisions de cadrage", "lot-04.html#autotoc_md224", null ],
        [ "Exigences couvertes", "lot-04.html#autotoc_md225", null ],
        [ "Découpage", "lot-04.html#autotoc_md226", null ],
        [ "Critères d'acceptation du lot", "lot-04.html#autotoc_md227", null ],
        [ "Dépendances", "lot-04.html#autotoc_md228", null ],
        [ "Navigation des tâches", "lot-04.html#autotoc_md229", null ],
        [ "TACHE-01 — Réorganisation de l'arborescence documentaire", "lot-04-tache-01-reorganisation-arbo.html", [
          [ "Contexte", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md230", null ],
          [ "Travail à réaliser", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md231", null ],
          [ "Fichiers impactés", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md232", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md233", null ],
          [ "Points d'attention", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md234", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md235", null ],
          [ "Exigences", "lot-04-tache-01-reorganisation-arbo.html#autotoc_md236", null ]
        ] ],
        [ "TACHE-02 — Configuration Doxygen pour le Markdown", "lot-04-tache-02-config-doxygen-markdown.html", [
          [ "Contexte", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md237", null ],
          [ "Travail à réaliser", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md238", null ],
          [ "Fichiers impactés", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md239", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md240", null ],
          [ "Points d'attention", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md241", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md242", null ],
          [ "Exigences", "lot-04-tache-02-config-doxygen-markdown.html#autotoc_md243", null ]
        ] ],
        [ "TACHE-03 — Page d'accueil du projet (mainpage)", "lot-04-tache-03-page-accueil.html", [
          [ "Contexte", "lot-04-tache-03-page-accueil.html#autotoc_md244", null ],
          [ "Travail à réaliser", "lot-04-tache-03-page-accueil.html#autotoc_md245", null ],
          [ "Fichiers impactés", "lot-04-tache-03-page-accueil.html#autotoc_md246", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-03-page-accueil.html#autotoc_md247", null ],
          [ "Points d'attention", "lot-04-tache-03-page-accueil.html#autotoc_md248", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-03-page-accueil.html#autotoc_md249", null ],
          [ "Exigences", "lot-04-tache-03-page-accueil.html#autotoc_md250", null ]
        ] ],
        [ "TACHE-04 — Pages de spécification navigables (conventions incluses)", "lot-04-tache-04-pages-specification.html", [
          [ "Contexte", "lot-04-tache-04-pages-specification.html#autotoc_md251", null ],
          [ "Travail à réaliser", "lot-04-tache-04-pages-specification.html#autotoc_md252", null ],
          [ "Convention d'insertion d'une nouvelle spec (à documenter dans l'index)", "lot-04-tache-04-pages-specification.html#autotoc_md253", null ],
          [ "Fichiers impactés", "lot-04-tache-04-pages-specification.html#autotoc_md254", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-04-pages-specification.html#autotoc_md255", null ],
          [ "Points d'attention", "lot-04-tache-04-pages-specification.html#autotoc_md256", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-04-pages-specification.html#autotoc_md257", null ],
          [ "Exigences", "lot-04-tache-04-pages-specification.html#autotoc_md258", null ]
        ] ],
        [ "TACHE-05 — Pages de lots navigables", "lot-04-tache-05-pages-lots.html", [
          [ "Contexte", "lot-04-tache-05-pages-lots.html#autotoc_md259", null ],
          [ "Travail à réaliser", "lot-04-tache-05-pages-lots.html#autotoc_md260", null ],
          [ "Fichiers impactés", "lot-04-tache-05-pages-lots.html#autotoc_md261", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-05-pages-lots.html#autotoc_md262", null ],
          [ "Points d'attention", "lot-04-tache-05-pages-lots.html#autotoc_md263", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-05-pages-lots.html#autotoc_md264", null ],
          [ "Exigences", "lot-04-tache-05-pages-lots.html#autotoc_md265", null ]
        ] ],
        [ "TACHE-06 — Manuel utilisateur (squelette + première page)", "lot-04-tache-06-manuel-utilisateur.html", [
          [ "Contexte", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md266", null ],
          [ "Travail à réaliser", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md267", null ],
          [ "Fichiers impactés", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md268", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md269", null ],
          [ "Points d'attention", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md270", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md271", null ],
          [ "Exigences", "lot-04-tache-06-manuel-utilisateur.html#autotoc_md272", null ]
        ] ],
        [ "TACHE-07 — CI documentation (WARN_AS_ERROR & déploiement)", "lot-04-tache-07-ci-docs.html", [
          [ "Contexte", "lot-04-tache-07-ci-docs.html#autotoc_md273", null ],
          [ "Travail à réaliser", "lot-04-tache-07-ci-docs.html#autotoc_md274", null ],
          [ "Fichiers impactés", "lot-04-tache-07-ci-docs.html#autotoc_md275", null ],
          [ "Avertissements connus à corriger avant <tt>WARN_AS_ERROR</tt> (relevés en TACHE-02)", "lot-04-tache-07-ci-docs.html#autotoc_md276", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-07-ci-docs.html#autotoc_md277", null ],
          [ "Points d'attention", "lot-04-tache-07-ci-docs.html#autotoc_md278", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-07-ci-docs.html#autotoc_md279", null ],
          [ "Exigences", "lot-04-tache-07-ci-docs.html#autotoc_md280", null ]
        ] ],
        [ "TACHE-08 — Traçabilité des exigences (IDs stables, ancres Doxygen, lint CI)", "lot-04-tache-08-tracabilite-exigences.html", [
          [ "Contexte", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md281", null ],
          [ "Règle à formaliser (dans <tt>conventions.md</tt>)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md282", null ],
          [ "Travail à réaliser", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md283", null ],
          [ "Fichiers impactés", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md284", null ],
          [ "Vérifications (obligatoires)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md285", null ],
          [ "Points d'attention", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md286", null ],
          [ "Définition de fait (DoD)", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md287", null ],
          [ "Exigences", "lot-04-tache-08-tracabilite-exigences.html#autotoc_md288", null ]
        ] ]
      ] ],
      [ "LOT-05 — Rendu 2D : atlas, sprites & caméra", "lot-05.html", [
        [ "Objectif", "lot-05.html#autotoc_md289", null ],
        [ "Périmètre", "lot-05.html#autotoc_md290", [
          [ "Inclus", "lot-05.html#autotoc_md291", null ],
          [ "Exclus (lots ultérieurs)", "lot-05.html#autotoc_md292", null ]
        ] ],
        [ "Décisions de cadrage", "lot-05.html#autotoc_md293", null ],
        [ "Exigences couvertes", "lot-05.html#autotoc_md294", null ],
        [ "Découpage", "lot-05.html#autotoc_md295", null ],
        [ "Critères d'acceptation du lot", "lot-05.html#autotoc_md296", null ],
        [ "Dépendances", "lot-05.html#autotoc_md297", null ],
        [ "Navigation des tâches", "lot-05.html#autotoc_md298", null ],
        [ "TACHE-01 — Composant `Sprite` (données pures)", "lot-05-tache-01-composant-sprite.html", [
          [ "Contexte", "lot-05-tache-01-composant-sprite.html#autotoc_md299", null ],
          [ "Travail à réaliser", "lot-05-tache-01-composant-sprite.html#autotoc_md300", null ],
          [ "Fichiers impactés", "lot-05-tache-01-composant-sprite.html#autotoc_md301", null ],
          [ "Tests (obligatoires si logique)", "lot-05-tache-01-composant-sprite.html#autotoc_md302", null ],
          [ "Points d'attention", "lot-05-tache-01-composant-sprite.html#autotoc_md303", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-01-composant-sprite.html#autotoc_md304", null ],
          [ "Exigences", "lot-05-tache-01-composant-sprite.html#autotoc_md305", null ]
        ] ],
        [ "TACHE-02 — Pipeline de quads texturés (HLSL, blend, nearest)", "lot-05-tache-02-pipeline-quads-textures.html", [
          [ "Contexte", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md306", null ],
          [ "Travail à réaliser", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md307", null ],
          [ "Fichiers impactés", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md308", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md309", null ],
          [ "Points d'attention", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md310", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md311", null ],
          [ "Exigences", "lot-05-tache-02-pipeline-quads-textures.html#autotoc_md312", null ]
        ] ],
        [ "TACHE-03 — Atlas de textures procédural", "lot-05-tache-03-atlas-procedural.html", [
          [ "Contexte", "lot-05-tache-03-atlas-procedural.html#autotoc_md313", null ],
          [ "Travail à réaliser", "lot-05-tache-03-atlas-procedural.html#autotoc_md314", null ],
          [ "Fichiers impactés", "lot-05-tache-03-atlas-procedural.html#autotoc_md315", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-03-atlas-procedural.html#autotoc_md316", null ],
          [ "Points d'attention", "lot-05-tache-03-atlas-procedural.html#autotoc_md317", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-03-atlas-procedural.html#autotoc_md318", null ],
          [ "Exigences", "lot-05-tache-03-atlas-procedural.html#autotoc_md319", null ]
        ] ],
        [ "TACHE-04 — Caméra 2D (monde → écran)", "lot-05-tache-04-camera-2d.html", [
          [ "Contexte", "lot-05-tache-04-camera-2d.html#autotoc_md320", null ],
          [ "Travail à réaliser", "lot-05-tache-04-camera-2d.html#autotoc_md321", null ],
          [ "Fichiers impactés", "lot-05-tache-04-camera-2d.html#autotoc_md322", null ],
          [ "Tests (obligatoires)", "lot-05-tache-04-camera-2d.html#autotoc_md323", null ],
          [ "Points d'attention", "lot-05-tache-04-camera-2d.html#autotoc_md324", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-04-camera-2d.html#autotoc_md325", null ],
          [ "Exigences", "lot-05-tache-04-camera-2d.html#autotoc_md326", null ]
        ] ],
        [ "TACHE-05 — Système de rendu des sprites (ECS → écran)", "lot-05-tache-05-systeme-rendu-sprites.html", [
          [ "Contexte", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md327", null ],
          [ "Travail à réaliser", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md328", null ],
          [ "Fichiers impactés", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md329", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md330", null ],
          [ "Points d'attention", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md331", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md332", null ],
          [ "Exigences", "lot-05-tache-05-systeme-rendu-sprites.html#autotoc_md333", null ]
        ] ],
        [ "TACHE-06 — Câblage du `World` dans la boucle + scène de démo", "lot-05-tache-06-cablage-world-demo.html", [
          [ "Contexte", "lot-05-tache-06-cablage-world-demo.html#autotoc_md334", null ],
          [ "Travail à réaliser", "lot-05-tache-06-cablage-world-demo.html#autotoc_md335", null ],
          [ "Fichiers impactés", "lot-05-tache-06-cablage-world-demo.html#autotoc_md336", null ],
          [ "Vérifications (obligatoires)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md337", null ],
          [ "Points d'attention", "lot-05-tache-06-cablage-world-demo.html#autotoc_md338", null ],
          [ "Définition de fait (DoD)", "lot-05-tache-06-cablage-world-demo.html#autotoc_md339", null ],
          [ "Exigences", "lot-05-tache-06-cablage-world-demo.html#autotoc_md340", null ]
        ] ]
      ] ],
      [ "LOT-06 — Menu principal", "lot-06.html", [
        [ "Objectif", "lot-06.html#autotoc_md341", null ],
        [ "Périmètre", "lot-06.html#autotoc_md342", [
          [ "Inclus", "lot-06.html#autotoc_md343", null ],
          [ "Exclus (lots ultérieurs)", "lot-06.html#autotoc_md344", null ]
        ] ],
        [ "Décisions de cadrage", "lot-06.html#autotoc_md345", null ],
        [ "Exigences couvertes", "lot-06.html#autotoc_md346", null ],
        [ "Découpage", "lot-06.html#autotoc_md347", null ],
        [ "Critères d'acceptation du lot", "lot-06.html#autotoc_md348", null ],
        [ "Dépendances", "lot-06.html#autotoc_md349", null ],
        [ "Navigation des tâches", "lot-06.html#autotoc_md350", null ],
        [ "TACHE-01 — Entrées clavier & souris", "lot-06-tache-01-entrees-clavier-souris.html", [
          [ "Contexte", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md351", null ],
          [ "Travail à réaliser", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md352", null ],
          [ "Fichiers impactés", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md353", null ],
          [ "Tests (obligatoires)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md354", null ],
          [ "Points d'attention", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md355", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md356", null ],
          [ "Exigences", "lot-06-tache-01-entrees-clavier-souris.html#autotoc_md357", null ]
        ] ],
        [ "TACHE-02 — Rendu de texte (police bitmap)", "lot-06-tache-02-rendu-texte-bitmap.html", [
          [ "Contexte", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md358", null ],
          [ "Travail à réaliser", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md359", null ],
          [ "Fichiers impactés", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md360", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md361", null ],
          [ "Points d'attention", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md362", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md363", null ],
          [ "Exigences", "lot-06-tache-02-rendu-texte-bitmap.html#autotoc_md364", null ]
        ] ],
        [ "TACHE-03 — Catalogue de traduction (i18n)", "lot-06-tache-03-catalogue-traduction.html", [
          [ "Contexte", "lot-06-tache-03-catalogue-traduction.html#autotoc_md365", null ],
          [ "Travail à réaliser", "lot-06-tache-03-catalogue-traduction.html#autotoc_md366", null ],
          [ "Fichiers impactés", "lot-06-tache-03-catalogue-traduction.html#autotoc_md367", null ],
          [ "Tests (obligatoires)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md368", null ],
          [ "Points d'attention", "lot-06-tache-03-catalogue-traduction.html#autotoc_md369", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-03-catalogue-traduction.html#autotoc_md370", null ],
          [ "Exigences", "lot-06-tache-03-catalogue-traduction.html#autotoc_md371", null ]
        ] ],
        [ "TACHE-04 — États d'application (écrans)", "lot-06-tache-04-etats-application.html", [
          [ "Contexte", "lot-06-tache-04-etats-application.html#autotoc_md372", null ],
          [ "Travail à réaliser", "lot-06-tache-04-etats-application.html#autotoc_md373", null ],
          [ "Fichiers impactés", "lot-06-tache-04-etats-application.html#autotoc_md374", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-04-etats-application.html#autotoc_md375", null ],
          [ "Points d'attention", "lot-06-tache-04-etats-application.html#autotoc_md376", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-04-etats-application.html#autotoc_md377", null ],
          [ "Exigences", "lot-06-tache-04-etats-application.html#autotoc_md378", null ]
        ] ],
        [ "TACHE-05 — Écran de menu principal", "lot-06-tache-05-ecran-menu-principal.html", [
          [ "Contexte", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md379", null ],
          [ "Travail à réaliser", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md380", null ],
          [ "Fichiers impactés", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md381", null ],
          [ "Tests (obligatoires)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md382", null ],
          [ "Points d'attention", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md383", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md384", null ],
          [ "Exigences", "lot-06-tache-05-ecran-menu-principal.html#autotoc_md385", null ]
        ] ],
        [ "TACHE-06 — Écrans cibles (jeu démo + éditeur placeholder)", "lot-06-tache-06-ecrans-cibles.html", [
          [ "Contexte", "lot-06-tache-06-ecrans-cibles.html#autotoc_md386", null ],
          [ "Travail à réaliser", "lot-06-tache-06-ecrans-cibles.html#autotoc_md387", null ],
          [ "Fichiers impactés", "lot-06-tache-06-ecrans-cibles.html#autotoc_md388", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md389", null ],
          [ "Points d'attention", "lot-06-tache-06-ecrans-cibles.html#autotoc_md390", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-06-ecrans-cibles.html#autotoc_md391", null ],
          [ "Exigences", "lot-06-tache-06-ecrans-cibles.html#autotoc_md392", null ]
        ] ],
        [ "TACHE-07 — Intégration `main` (boucle pilotée par l'écran)", "lot-06-tache-07-integration-main.html", [
          [ "Contexte", "lot-06-tache-07-integration-main.html#autotoc_md393", null ],
          [ "Travail à réaliser", "lot-06-tache-07-integration-main.html#autotoc_md394", null ],
          [ "Fichiers impactés", "lot-06-tache-07-integration-main.html#autotoc_md395", null ],
          [ "Vérifications (obligatoires)", "lot-06-tache-07-integration-main.html#autotoc_md396", null ],
          [ "Points d'attention", "lot-06-tache-07-integration-main.html#autotoc_md397", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-07-integration-main.html#autotoc_md398", null ],
          [ "Exigences", "lot-06-tache-07-integration-main.html#autotoc_md399", null ]
        ] ],
        [ "TACHE-08 — Sélecteur de langue", "lot-06-tache-08-selecteur-langue.html", [
          [ "Contexte", "lot-06-tache-08-selecteur-langue.html#autotoc_md400", null ],
          [ "Travail à réaliser", "lot-06-tache-08-selecteur-langue.html#autotoc_md401", null ],
          [ "Fichiers impactés", "lot-06-tache-08-selecteur-langue.html#autotoc_md402", null ],
          [ "Tests (obligatoires)", "lot-06-tache-08-selecteur-langue.html#autotoc_md403", null ],
          [ "Points d'attention", "lot-06-tache-08-selecteur-langue.html#autotoc_md404", null ],
          [ "Définition de fait (DoD)", "lot-06-tache-08-selecteur-langue.html#autotoc_md405", null ],
          [ "Exigences", "lot-06-tache-08-selecteur-langue.html#autotoc_md406", null ]
        ] ]
      ] ],
      [ "LOT-07 — Niveaux : modèle et chargement", "lot-07.html", [
        [ "Objectif", "lot-07.html#autotoc_md407", null ],
        [ "Périmètre", "lot-07.html#autotoc_md408", [
          [ "Inclus", "lot-07.html#autotoc_md409", null ],
          [ "Exclus (lots ultérieurs)", "lot-07.html#autotoc_md410", null ]
        ] ],
        [ "Décisions de cadrage", "lot-07.html#autotoc_md411", null ],
        [ "Exigences couvertes", "lot-07.html#autotoc_md412", null ],
        [ "Découpage", "lot-07.html#autotoc_md413", null ],
        [ "Critères d'acceptation du lot", "lot-07.html#autotoc_md414", null ],
        [ "Dépendances", "lot-07.html#autotoc_md415", null ],
        [ "Navigation des tâches", "lot-07.html#autotoc_md416", null ],
        [ "TACHE-01 — Dépendance JSON (nlohmann/json épinglé)", "lot-07-tache-01-dependance-json.html", [
          [ "Contexte", "lot-07-tache-01-dependance-json.html#autotoc_md417", null ],
          [ "Travail à réaliser", "lot-07-tache-01-dependance-json.html#autotoc_md418", null ],
          [ "Fichiers impactés", "lot-07-tache-01-dependance-json.html#autotoc_md419", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-01-dependance-json.html#autotoc_md420", null ],
          [ "Points d'attention", "lot-07-tache-01-dependance-json.html#autotoc_md421", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-01-dependance-json.html#autotoc_md422", null ],
          [ "Exigences", "lot-07-tache-01-dependance-json.html#autotoc_md423", null ]
        ] ],
        [ "TACHE-02 — Modèle de tuiles et de niveau", "lot-07-tache-02-modele-niveau.html", [
          [ "Contexte", "lot-07-tache-02-modele-niveau.html#autotoc_md424", null ],
          [ "Travail à réaliser", "lot-07-tache-02-modele-niveau.html#autotoc_md425", null ],
          [ "Fichiers impactés", "lot-07-tache-02-modele-niveau.html#autotoc_md426", null ],
          [ "Tests (obligatoires)", "lot-07-tache-02-modele-niveau.html#autotoc_md427", null ],
          [ "Points d'attention", "lot-07-tache-02-modele-niveau.html#autotoc_md428", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-02-modele-niveau.html#autotoc_md429", null ],
          [ "Exigences", "lot-07-tache-02-modele-niveau.html#autotoc_md430", null ]
        ] ],
        [ "TACHE-03 — Chargement du niveau (JSON)", "lot-07-tache-03-chargement-json.html", [
          [ "Contexte", "lot-07-tache-03-chargement-json.html#autotoc_md431", null ],
          [ "Travail à réaliser", "lot-07-tache-03-chargement-json.html#autotoc_md432", null ],
          [ "Fichiers impactés", "lot-07-tache-03-chargement-json.html#autotoc_md433", null ],
          [ "Tests (obligatoires)", "lot-07-tache-03-chargement-json.html#autotoc_md434", null ],
          [ "Points d'attention", "lot-07-tache-03-chargement-json.html#autotoc_md435", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-03-chargement-json.html#autotoc_md436", null ],
          [ "Exigences", "lot-07-tache-03-chargement-json.html#autotoc_md437", null ]
        ] ],
        [ "TACHE-04 — Validation du niveau", "lot-07-tache-04-validation.html", [
          [ "Contexte", "lot-07-tache-04-validation.html#autotoc_md438", null ],
          [ "Travail à réaliser", "lot-07-tache-04-validation.html#autotoc_md439", null ],
          [ "Fichiers impactés", "lot-07-tache-04-validation.html#autotoc_md440", null ],
          [ "Tests (obligatoires)", "lot-07-tache-04-validation.html#autotoc_md441", null ],
          [ "Points d'attention", "lot-07-tache-04-validation.html#autotoc_md442", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-04-validation.html#autotoc_md443", null ],
          [ "Exigences", "lot-07-tache-04-validation.html#autotoc_md444", null ]
        ] ],
        [ "TACHE-05 — Niveau de démonstration", "lot-07-tache-05-niveau-demo.html", [
          [ "Contexte", "lot-07-tache-05-niveau-demo.html#autotoc_md445", null ],
          [ "Travail à réaliser", "lot-07-tache-05-niveau-demo.html#autotoc_md446", null ],
          [ "Fichiers impactés", "lot-07-tache-05-niveau-demo.html#autotoc_md447", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-05-niveau-demo.html#autotoc_md448", null ],
          [ "Points d'attention", "lot-07-tache-05-niveau-demo.html#autotoc_md449", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-05-niveau-demo.html#autotoc_md450", null ],
          [ "Exigences", "lot-07-tache-05-niveau-demo.html#autotoc_md451", null ]
        ] ],
        [ "TACHE-06 — Rendu du niveau + intégration « Charger niveau »", "lot-07-tache-06-rendu-integration.html", [
          [ "Contexte", "lot-07-tache-06-rendu-integration.html#autotoc_md452", null ],
          [ "Travail à réaliser", "lot-07-tache-06-rendu-integration.html#autotoc_md453", null ],
          [ "Fichiers impactés", "lot-07-tache-06-rendu-integration.html#autotoc_md454", null ],
          [ "Vérifications (obligatoires)", "lot-07-tache-06-rendu-integration.html#autotoc_md455", null ],
          [ "Points d'attention", "lot-07-tache-06-rendu-integration.html#autotoc_md456", null ],
          [ "Définition de fait (DoD)", "lot-07-tache-06-rendu-integration.html#autotoc_md457", null ],
          [ "Exigences", "lot-07-tache-06-rendu-integration.html#autotoc_md458", null ]
        ] ]
      ] ]
    ] ],
    [ "Manuel utilisateur", "manuel.html", [
      [ "Pages", "manuel.html#autotoc_md460", null ],
      [ "À venir", "manuel.html#autotoc_md461", null ],
      [ "Télécharger et lancer le jeu", "manuel-telecharger.html", [
        [ "Prérequis", "manuel-telecharger.html#autotoc_md462", null ],
        [ "Étapes", "manuel-telecharger.html#autotoc_md463", null ],
        [ "Remarques", "manuel-telecharger.html#autotoc_md464", null ]
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
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"Assert_8cpp.html",
"View_8h_source.html",
"classcore_1_1World.html#a8f9461b813faa6de969b2b6ed4b76251",
"classhmi_1_1Localization.html#a74443598fef691b1c7c278bcf364bcb2",
"classhmi_1_1Window.html#af8129c90a7b8fabebad8db3755551a59",
"lot-04-tache-01-reorganisation-arbo.html#autotoc_md231",
"lot-07-tache-06-rendu-integration.html",
"structcore_1_1MemoryLogSink_1_1Entry.html",
"test__level__loader_8cpp.html#a04f28bd87a14a88655c836895a18daaf"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
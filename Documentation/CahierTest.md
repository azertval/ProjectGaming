# Cahier de test {#cahiertest}

**321 cas de test**, générés depuis les blocs `\castest{...}` du code par `scripts/generate_cahier_test.py` (ne pas éditer directement — modifier le commentaire du test concerné puis relancer le script). Organisés ici selon l'arborescence de `Source/Test/` pour rester lisibles page par page.

## Tests unitaires (271)

### Core

**`test_core.cpp`**

- **Vérifie que la version du moteur n'est pas vide.** *(criticité : Majeur)* — catégorie : Unitaire · Engine
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/test_core.cpp:12` — `EngineTest.VersionNonVide`

#### Diagnostics (14)

**`test_assert.cpp`**

- **Une condition vraie n'invoque pas le gestionnaire d'assertion.** *(criticité : Majeur)* — catégorie : Unitaire · Assert
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_assert.cpp:14` — `AssertTest.ConditionVraieNInvoquePasLeHandler`
- **Une condition fausse invoque le gestionnaire une fois, avec le message (Debug uniquement).** *(criticité : Majeur)* — catégorie : Unitaire · Assert
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_assert.cpp:37` — `AssertTest.ConditionFausseInvoqueLeHandler`

**`test_log_format.cpp`**

- **La ligne formatée contient horodatage, niveau, catégorie, position source et message.** *(criticité : Majeur)* — catégorie : Unitaire · Log Format
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_log_format.cpp:14` — `LogFormatTest.LigneContientTousLesChamps`
- **Le chemin source est réduit à son nom de fichier dans la ligne.** *(criticité : Majeur)* — catégorie : Unitaire · Log Format
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_log_format.cpp:37` — `LogFormatTest.CheminReduitAuNomDeFichier`
- **fileName isole le nom de fichier des chemins Windows et POSIX, ou renvoie l'entrée.** *(criticité : Majeur)* — catégorie : Unitaire · Log Format
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_log_format.cpp:57` — `LogFormatTest.FileNameIsoleLeNom`
- **L'horodatage courant respecte le format HH:MM:SS (longueur 8).** *(criticité : Majeur)* — catégorie : Unitaire · Log Format
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_log_format.cpp:74` — `LogFormatTest.HorodatageFormatHeure`

**`test_log_level_parse.cpp`**

- **Les noms de niveaux reconnus sont convertis (y compris l'alias « warn »).** *(criticité : Majeur)* — catégorie : Unitaire · Log Level Parse
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_log_level_parse.cpp:12` — `LogLevelParseTest.NiveauxReconnus`
- **L'analyse est insensible à la casse.** *(criticité : Majeur)* — catégorie : Unitaire · Log Level Parse
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_log_level_parse.cpp:30` — `LogLevelParseTest.InsensibleALaCasse`
- **Une valeur inconnue ou vide n'est pas convertie.** *(criticité : Majeur)* — catégorie : Unitaire · Log Level Parse
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_log_level_parse.cpp:46` — `LogLevelParseTest.ValeurInconnue`

**`test_logger.cpp`**

- **Un message au-dessus du niveau minimal est diffusé ; en dessous, il est filtré.** *(criticité : Majeur)* — catégorie : Unitaire · Logger
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_logger.cpp:15` — `LoggerTest.FiltreParNiveauMinimal`
- **Le même message atteint tous les sinks enregistrés.** *(criticité : Majeur)* — catégorie : Unitaire · Logger
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_logger.cpp:41` — `LoggerTest.DiffuseAPlusieursSinks`
- **clearSinks retire les destinations : plus rien n'est diffusé ensuite.** *(criticité : Majeur)* — catégorie : Unitaire · Logger
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_logger.cpp:66` — `LoggerTest.ClearSinksArreteLaDiffusion`

**`test_sinks.cpp`**

- **Le sink mémoire conserve fidèlement niveau et texte, dans l'ordre.** *(criticité : Majeur)* — catégorie : Unitaire · Memory Log Sink
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_sinks.cpp:12` — `MemoryLogSinkTest.ConserveNiveauEtTexteDansLOrdre`
- **clear vide les messages mémorisés.** *(criticité : Majeur)* — catégorie : Unitaire · Memory Log Sink
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Diagnostics/test_sinks.cpp:34` — `MemoryLogSinkTest.ClearVideLesMessages`

#### Ecs (34)

**`test_component_pool.cpp`**

- **`add` puis `get` renvoie la valeur stockée ; `has` est cohérent.** *(criticité : Majeur)* — catégorie : Unitaire · Component Pool
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_component_pool.cpp:25` — `ComponentPoolTest.AjoutPuisAcces`
- **`get` renvoie une référence modifiable sur le composant stocké.** *(criticité : Majeur)* — catégorie : Unitaire · Component Pool
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_component_pool.cpp:49` — `ComponentPoolTest.GetRenvoieReferenceModifiable`
- **`remove` d'un élément au milieu (swap-and-pop) laisse les autres composants accessibles et corrects, et le stockage dense reste contigu.** *(criticité : Majeur)* — catégorie : Unitaire · Component Pool
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_component_pool.cpp:71` — `ComponentPoolTest.RemoveAuMilieuSwapAndPop`
- **Retirer le dernier élément ne perturbe pas les précédents.** *(criticité : Majeur)* — catégorie : Unitaire · Component Pool
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_component_pool.cpp:107` — `ComponentPoolTest.RemoveDernierElement`
- **Un handle périmé (index recyclé, génération différente) ne possède pas le composant de l'ancienne entité.** *(criticité : Majeur)* — catégorie : Unitaire · Component Pool
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_component_pool.cpp:134` — `ComponentPoolTest.HandlePerimeNePossedePasLeComposant`
- **`removeIfPresent` retire si le composant existe, sinon ne fait rien.** *(criticité : Majeur)* — catégorie : Unitaire · Component Pool
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_component_pool.cpp:161` — `ComponentPoolTest.RemoveIfPresent`
- **`get` sur une entité absente viole une précondition (assertion en Debug).** *(criticité : Majeur)* — catégorie : Unitaire · Component Pool
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_component_pool.cpp:182` — `ComponentPoolTest.GetSurEntiteAbsenteViolePrecondition`

**`test_entity_manager.cpp`**

- **`create` renvoie des entités vivantes et distinctes.** *(criticité : Majeur)* — catégorie : Unitaire · Entity Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:13` — `EntityManagerTest.CreeEntitesVivantesEtDistinctes`
- **Après destruction, l'ancien handle n'est plus vivant.** *(criticité : Majeur)* — catégorie : Unitaire · Entity Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:34` — `EntityManagerTest.DestructionInvalideLeHandle`
- **Un index recyclé produit une génération différente : l'ancien handle reste invalide, le nouveau est valide.** *(criticité : Majeur)* — catégorie : Unitaire · Entity Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:55` — `EntityManagerTest.RecyclageChangeLaGeneration`
- **Détruire un handle périmé est sans effet (idempotent) et ne touche pas l'entité vivante qui occupe désormais le même index.** *(criticité : Majeur)* — catégorie : Unitaire · Entity Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:83` — `EntityManagerTest.DestructionHandlePerimeSansEffet`
- **L'entité invalide conventionnelle n'est jamais vivante.** *(criticité : Majeur)* — catégorie : Unitaire · Entity Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:107` — `EntityManagerTest.EntiteInvalideJamaisVivante`
- **Le handle invalide se compare comme tel.** *(criticité : Majeur)* — catégorie : Unitaire · Entity Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:125` — `EntityManagerTest.EgaliteHandleInvalide`

**`test_player_components.cpp`**

- **Collider par défaut nul** *(criticité : Mineur)* — catégorie : Unitaire · Composants du personnage
  - Étapes : 1. Construire un `core::Collider` par défaut. 2. Lire `size.x` et `size.y`.
  - Résultat attendu : La boîte est nulle, `size` vaut (0, 0).
  - `Source/Test/Unit/Core/Ecs/test_player_components.cpp:17` — `PlayerComponentsTest.ColliderParDefautEstNul`
- **Player par défaut (en l'air, budgets illimités)** *(criticité : Mineur)* — catégorie : Unitaire · Composants du personnage
  - Étapes : 1. Construire un `core::Player` par défaut. 2. Lire ses champs.
  - Résultat attendu : Pas au sol ; minuteries et compteurs à 0 ; orienté à droite ; dash indisponible ; budgets sauts/dashs à -1 (illimité) ; masse à 1,0 (`EX-GP-019`).
  - `Source/Test/Unit/Core/Ecs/test_player_components.cpp:31` — `PlayerComponentsTest.PlayerParDefautPasAuSol`
- **PlayerInput par défaut neutre** *(criticité : Mineur)* — catégorie : Unitaire · Composants du personnage
  - Étapes : 1. Construire un `core::PlayerInput` par défaut. 2. Lire ses champs.
  - Résultat attendu : `moveX`/`moveY` nuls ; aucun front de saut ni de dash.
  - `Source/Test/Unit/Core/Ecs/test_player_components.cpp:56` — `PlayerComponentsTest.PlayerInputParDefautImmobile`
- **PhysicsConfig par défaut plausible** *(criticité : Majeur)* — catégorie : Unitaire · Composants du personnage
  - Étapes : 1. Construire un `core::PhysicsConfig` par défaut. 2. Vérifier chaque réglage.
  - Résultat attendu : Vitesses/temps > 0 ; `jumpCutFactor` ∈ [0, 1] ; multiplicateurs de chute/fast-fall > 1 ; apex ∈ ]0, 1[ ; au moins 1 saut aérien.
  - `Source/Test/Unit/Core/Ecs/test_player_components.cpp:73` — `PlayerComponentsTest.PhysicsConfigParDefautPlausible`
- **Taille humanoïde et spawn centré** *(criticité : Majeur)* — catégorie : Unitaire · Composants du personnage
  - Étapes : 1. Lire `playerSize()`. 2. Calculer `playerSpawnPosition(3, 5)`.
  - Résultat attendu : Taille (0,4 ; 0,8) ; position centrée dans la tuile : (3,3 ; 5,1).
  - `Source/Test/Unit/Core/Ecs/test_player_components.cpp:108` — `PlayerComponentsTest.TailleEtSpawnHumanoide`
- **Agrégation des composants** *(criticité : Mineur)* — catégorie : Unitaire · Composants du personnage
  - Étapes : 1. Initialiser `Collider` et `PlayerInput` par accolades. 2. Lire les champs.
  - Résultat attendu : Les valeurs fournies sont bien affectées (agrégats).
  - `Source/Test/Unit/Core/Ecs/test_player_components.cpp:126` — `PlayerComponentsTest.AggregationRenseigneLesChamps`

**`test_sprite.cpp`**

- **Un sprite par défaut a une teinte blanche opaque, la couche 0 et une région nulle.** *(criticité : Majeur)* — catégorie : Unitaire · Sprite
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_sprite.cpp:12` — `SpriteTest.ValeursParDefaut`
- **Les champs sont librement assignables (donnée pure).** *(criticité : Majeur)* — catégorie : Unitaire · Sprite
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_sprite.cpp:37` — `SpriteTest.ChampsAssignables`
- **Le composant est utilisable comme un composant d'ECS (stockage/copie de données pures).** *(criticité : Majeur)* — catégorie : Unitaire · Sprite
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_sprite.cpp:62` — `SpriteTest.CopieValeur`

**`test_view.cpp`**

- **Une vue <A, B> itère exactement les entités possédant A et B.** *(criticité : Majeur)* — catégorie : Unitaire · View
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_view.cpp:27` — `ViewTest.SelectionneUniquementLIntersection`
- **Les composants fournis par la vue correspondent bien à l'entité itérée.** *(criticité : Majeur)* — catégorie : Unitaire · View
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_view.cpp:58` — `ViewTest.ComposantsCorrespondentALEntite`
- **La modification d'un composant via la vue est visible ensuite (référence).** *(criticité : Majeur)* — catégorie : Unitaire · View
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_view.cpp:92` — `ViewTest.ModificationViaVueEstVisible`
- **Une vue sans entité correspondante s'itère sans erreur (aucune visite).** *(criticité : Majeur)* — catégorie : Unitaire · View
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_view.cpp:118` — `ViewTest.VueVideNIterePas`
- **L'itération par `for` visite les mêmes entités que `each`.** *(criticité : Majeur)* — catégorie : Unitaire · View
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_view.cpp:145` — `ViewTest.IterationForEtEachCoherentes`

**`test_world.cpp`**

- **Le cycle add/has/get/remove d'un composant est cohérent via le `World`.** *(criticité : Majeur)* — catégorie : Unitaire · World
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_world.cpp:53` — `WorldTest.CycleComposant`
- **`hasComponent` est faux quand aucune pool du type n'existe encore.** *(criticité : Majeur)* — catégorie : Unitaire · World
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_world.cpp:80` — `WorldTest.HasComponentSansPool`
- **`destroyEntity` retire l'entité de toutes les pools et la rend non vivante.** *(criticité : Majeur)* — catégorie : Unitaire · World
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_world.cpp:96` — `WorldTest.DestroyEntityRetireTousLesComposants`
- **Les systèmes enregistrés s'exécutent dans l'ordre d'enregistrement.** *(criticité : Majeur)* — catégorie : Unitaire · World
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_world.cpp:119` — `WorldTest.SystemesExecutesDansLOrdre`
- **`update` appelé N fois exécute chaque système N fois (cadencement déterministe).** *(criticité : Majeur)* — catégorie : Unitaire · World
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_world.cpp:142` — `WorldTest.UpdateNFoisExecuteNFois`
- **Le `fixedDelta` passé à `update` est transmis tel quel aux systèmes.** *(criticité : Majeur)* — catégorie : Unitaire · World
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_world.cpp:166` — `WorldTest.FixedDeltaTransmisAuxSystemes`
- **La vue exposée par le `World` itère l'intersection des composants.** *(criticité : Majeur)* — catégorie : Unitaire · World
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Ecs/test_world.cpp:186` — `WorldTest.ViewViaWorld`

#### Gameplay (5)

**`test_mechanism_controller.cpp`**

- **Au départ, la porte est **fermée** (solide) ; toucher l'interrupteur l'**ouvre**.** *(criticité : Majeur)* — catégorie : Unitaire · Mechanism Controller
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:53` — `MechanismControllerTest.ContactOuvreLaPorte`
- **La bascule est **sur front** : rester sur l'interrupteur ne re-bascule pas ; revenir bascule.** *(criticité : Majeur)* — catégorie : Unitaire · Mechanism Controller
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:75` — `MechanismControllerTest.BasculeSurFront`
- **Loin de l'interrupteur, rien ne change (la porte reste fermée).** *(criticité : Majeur)* — catégorie : Unitaire · Mechanism Controller
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:102` — `MechanismControllerTest.SansContactRienNeChange`
- **Une plaque de pression ouvre la porte tant que le poids y repose, et la referme dès qu'il en part.** *(criticité : Majeur)* — catégorie : Unitaire · Mechanism Controller
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:120` — `MechanismControllerTest.PlaqueDePressionActivationContinue`
- **Un poids insuffisant sur la plaque de pression n'ouvre pas la porte.** *(criticité : Majeur)* — catégorie : Unitaire · Mechanism Controller
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:151` — `MechanismControllerTest.PlaqueDePressionPoidsInsuffisant`

#### Levels (70)

**`test_level.cpp`**

- **Une grille neuve a les bonnes dimensions et n'est composée que de cases Empty.** *(criticité : Majeur)* — catégorie : Unitaire · Tile Map
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level.cpp:14` — `TileMapTest.GrilleNeuveVideAuxBonnesDimensions`
- **Écrire puis lire une tuile restitue le type posé.** *(criticité : Majeur)* — catégorie : Unitaire · Tile Map
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level.cpp:37` — `TileMapTest.EcritureLectureDUneTuile`
- **Les bornes de la grille sont correctement détectées.** *(criticité : Majeur)* — catégorie : Unitaire · Tile Map
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level.cpp:55` — `TileMapTest.Bornes`
- **Seules les tuiles solides bloquent statiquement.** *(criticité : Majeur)* — catégorie : Unitaire · Tile Map
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level.cpp:75` — `TileMapTest.Solidite`
- **`isSolid(TileType)` : vrai seulement pour Solid.** *(criticité : Majeur)* — catégorie : Unitaire · Tile Map
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level.cpp:96` — `TileMapTest.IsSolidParType`
- **Un Level restitue ses composantes (nom, grille, entrée/sortie, mécanismes).** *(criticité : Majeur)* — catégorie : Unitaire · Level
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level.cpp:113` — `LevelTest.RestitueSesComposantes`

**`test_level_draft.cpp`**

- **Un brouillon vierge a une grille entièrement vide, sans entrée ni sortie.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:23` — `LevelDraftTest.BrouillonVierge`
- **paintTile pose le type demandé sur la case visée.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:46` — `LevelDraftTest.PaintTilePoseLeType`
- **Poser une seconde entrée déplace la première (unicité, EX-EDIT-004).** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:62` — `LevelDraftTest.SetEntryDeplaceLEntreeExistante`
- **Poser une seconde sortie déplace la première (unicité, EX-EDIT-004).** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:84` — `LevelDraftTest.SetExitDeplaceLaSortieExistante`
- **Peindre par-dessus l'entrée invalide la position d'entrée mémorisée.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:103` — `LevelDraftTest.PeindrePardessusLEntreeLInvalide`
- **Lier un interrupteur à une porte crée un mécanisme ; le délier le retire.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:121` — `LevelDraftTest.LierPuisDelierUnMecanisme`
- **Lier une plaque de pression à une porte crée un mécanisme.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - Résultat attendu : Lier une plaque de pression à une porte crée un mécanisme, comme un interrupteur.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:146` — `LevelDraftTest.LierUnePlaqueDePression`
- **Relier une porte déjà liée remplace la liaison précédente (une porte, un interrupteur).** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:167` — `LevelDraftTest.RelierUnePorteRemplaceLaLiaisonPrecedente`
- **Un interrupteur peut ouvrir plusieurs portes.** *(criticité : Mineur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:192` — `LevelDraftTest.UnInterrupteurPeutOuvrirPlusieursPortes`
- **Peindre par-dessus un interrupteur retire les liaisons qui le référencent.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:214` — `LevelDraftTest.PeindrePardessusUnInterrupteurRetireSesLiaisons`
- **Agrandir la grille conserve le contenu existant et complète en cases vides.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:234` — `LevelDraftTest.AgrandirConserveLeContenu`
- **Réduire la grille tronque le contenu hors bornes et invalide l'entrée perdue.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:257` — `LevelDraftTest.ReduireTronqueEtInvalideLEntreePerdue`
- **Réduire la grille retire les mécanismes dont une extrémité sort des bornes.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:281` — `LevelDraftTest.ReduireRetireLesMecanismesHorsBornes`
- **toLevel() sur un brouillon complet et valide produit un niveau conforme.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:302` — `LevelDraftTest.ToLevelSurBrouillonValideReussit`
- **toLevel() sur un brouillon sans sortie échoue avec un message récupérable (EX-EDIT-007).** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:326` — `LevelDraftTest.ToLevelSansSortieEchoueProprement`
- **Un brouillon reconstruit depuis un niveau existant restitue son contenu.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:347` — `LevelDraftTest.FromLevelRestitueLeContenu`
- **Un brouillon neuf ne peut ni annuler ni refaire.** *(criticité : Mineur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:369` — `LevelDraftTest.BrouillonNeufSansHistorique`
- **undo() après une peinture restitue l'état exact précédent.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:385` — `LevelDraftTest.UndoApresPeintureRestitueLEtatPrecedent`
- **redo() après un undo() restitue l'état muté.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:408` — `LevelDraftTest.RedoApresUndoRestitueLEtatMute`
- **Une séquence de N mutations suivie de N undo() restitue l'état initial exact.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:431` — `LevelDraftTest.SequenceDeMutationsPuisUndoRestitueLEtatInitial`
- **Une nouvelle mutation après un undo() invalide la branche de refaire.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:463` — `LevelDraftTest.MutationApresUndoInvalideLeRefaire`
- **undo()/redo() sur une pile vide est sans effet (pas de plantage).** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:485` — `LevelDraftTest.UndoRedoSurPileVideSansEffet`
- **L'annulation d'une liaison de mécanisme restitue la liaison précédente.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:502` — `LevelDraftTest.UndoApresLiaisonMecanismeRestitueLAbsenceDeLiaison`
- **paintRegion applique un bloc homogène comme une succession de paintTile équivalente.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:524` — `LevelDraftTest.PaintRegionAppliqueLeBlocEntier`
- **paintRegion ne pousse qu'un seul snapshot undo pour tout le bloc.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:550` — `LevelDraftTest.PaintRegionUnSeulSnapshotUndo`
- **paintRegion découpe silencieusement le bloc aux bords de la grille.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:574` — `LevelDraftTest.PaintRegionDecoupeAuxBords`
- **paintRegion qui inclut une position d'entrée déplace l'entrée existante.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:597` — `LevelDraftTest.PaintRegionDeplaceLEntree`
- **paintRegion qui recouvre un interrupteur retire les liaisons qui le référencent.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:617` — `LevelDraftTest.PaintRegionRetireLesLiaisonsRecouvertes`
- **paintRegion avec un bloc vide est sans effet.** *(criticité : Mineur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:640` — `LevelDraftTest.PaintRegionBlocVideSansEffet`
- **wouldResizeDropContent détecte la perte de l'entrée, de la sortie ou d'une liaison.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:656` — `LevelDraftTest.WouldResizeDropContentDetecteLaPerte`
- **wouldResizeDropContent est faux sur un brouillon vierge, quelle que soit la taille visee.** *(criticité : Mineur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:685` — `LevelDraftTest.WouldResizeDropContentFauxSurBrouillonVierge`
- **L'annulation d'un redimensionnement restitue les dimensions et le contenu précédents.** *(criticité : Majeur)* — catégorie : Unitaire · Level Draft
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_draft.cpp:702` — `LevelDraftTest.UndoApresRedimensionnementRestitueLesDimensions`

**`test_level_loader.cpp`**

- **Un niveau valide est chargé avec ses dimensions, tuiles, entrée/sortie et mécanismes.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:33` — `LevelLoaderTest.ChargeUnNiveauValide`
- **Une plaque de pression se charge comme un interrupteur.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - Résultat attendu : Une plaque de pression se charge comme un interrupteur : même règle d'identifiant, même résolution de liaison vers une porte.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:64` — `LevelLoaderTest.ChargeUnePlaqueDePression`
- **Les budgets de mouvements (EX-GP-024) sont chargés s'ils sont présents, illimités (-1) sinon.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:98` — `LevelLoaderTest.BudgetsOptionnels`
- **Un JSON syntaxiquement invalide est rejeté sans plantage.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:126` — `LevelLoaderTest.JsonMalformeRejete`
- **Un champ obligatoire manquant est rejeté.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:143` — `LevelLoaderTest.ChampManquantRejete`
- **Un type de tuile inconnu est rejeté avec un message.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:160` — `LevelLoaderTest.TypeDeTuileInconnuRejete`
- **Une tuile hors des bornes est rejetée.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:178` — `LevelLoaderTest.TuileHorsBornesRejetee`
- **Une porte liée à un interrupteur inexistant est rejetée.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:195` — `LevelLoaderTest.LiaisonMecanismeNonResolueRejetee`
- **Plusieurs entrées sont rejetées (une seule attendue).** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:219` — `LevelLoaderTest.PlusieursEntreesRejetees`
- **Plusieurs sorties sont rejetées (une seule attendue).** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:242` — `LevelLoaderTest.PlusieursSortiesRejetees`
- **Deux tuiles à la même position sont rejetées.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:265` — `LevelLoaderTest.PositionEnDoubleRejetee`
- **Un champ 'tiles' qui n'est pas une liste est rejeté.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:289` — `LevelLoaderTest.TilesNonListeRejete`
- **Des dimensions non positives sont rejetées.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:306` — `LevelLoaderTest.DimensionsNonPositivesRejetees`
- **Un interrupteur sans 'id' est rejeté.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:323` — `LevelLoaderTest.InterrupteurSansIdRejete`
- **Deux interrupteurs avec le même identifiant sont rejetés.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:346` — `LevelLoaderTest.IdentifiantInterrupteurEnDoubleRejete`
- **Un niveau sans entrée est rejeté.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:370` — `LevelLoaderTest.NiveauSansEntreeRejete`
- **Un niveau sans sortie est rejeté.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:387` — `LevelLoaderTest.NiveauSansSortieRejete`
- **Charger un fichier inexistant échoue proprement (récupérable).** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:404` — `LevelLoaderTest.FichierIntrouvableRejete`
- **Une porte sans 'opensWith' est une simple tuile : chargement valide, aucun mécanisme.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:422` — `LevelLoaderTest.PorteSansLiaisonEstValide`
- **Le niveau de démonstration livré (Source/Elements/Levels) se charge et se valide.** *(criticité : Majeur)* — catégorie : Unitaire · Level Loader
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_loader.cpp:446` — `LevelLoaderTest.NiveauDeDemoLivreValide`

**`test_level_outcome.cpp`**

- **En zone libre (ni sortie, ni danger, dans les limites) : partie en cours.** *(criticité : Majeur)* — catégorie : Unitaire · Level Outcome
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_outcome.cpp:36` — `LevelOutcomeTest.ZoneLibreEstEnCours`
- **La boîte recouvrant la case de sortie : niveau gagné.** *(criticité : Majeur)* — catégorie : Unitaire · Level Outcome
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_outcome.cpp:51` — `LevelOutcomeTest.SurLaSortieEstGagne`
- **La boîte recouvrant une tuile Danger : niveau perdu.** *(criticité : Majeur)* — catégorie : Unitaire · Level Outcome
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_outcome.cpp:66` — `LevelOutcomeTest.SurUnDangerEstPerdu`
- **La boîte sous la limite basse (chute dans le vide) : niveau perdu.** *(criticité : Majeur)* — catégorie : Unitaire · Level Outcome
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_outcome.cpp:81` — `LevelOutcomeTest.SousLaLimiteBasseEstPerdu`
- **Recouvrement simultané sortie + danger : l'échec est prioritaire (déterminisme).** *(criticité : Majeur)* — catégorie : Unitaire · Level Outcome
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_outcome.cpp:96` — `LevelOutcomeTest.EchecPrioritaireSurSucces`

**`test_level_writer.cpp`**

- **Sérialiser puis recharger un niveau produit un niveau équivalent (round-trip).** *(criticité : Majeur)* — catégorie : Unitaire · Level Writer
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_writer.cpp:36` — `LevelWriterTest.RoundTripPreserveLeContenu`
- **Une plaque de pression survit au round-trip.** *(criticité : Majeur)* — catégorie : Unitaire · Level Writer
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - Résultat attendu : Une plaque de pression survit au round-trip : `TileType` et liaison préservés.
  - `Source/Test/Unit/Core/Levels/test_level_writer.cpp:81` — `LevelWriterTest.PlaqueDePressionSurvitAuRoundTrip`
- **Les budgets illimités (-1) ne sont pas écrits dans le JSON produit.** *(criticité : Mineur)* — catégorie : Unitaire · Level Writer
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_writer.cpp:116` — `LevelWriterTest.BudgetsIllimitesOmisDuJson`
- **Un interrupteur non relié à une porte est tout de même sérialisé, avec un identifiant.** *(criticité : Majeur)* — catégorie : Unitaire · Level Writer
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_writer.cpp:142` — `LevelWriterTest.InterrupteurNonRelieRegenereUnIdentifiant`
- **Le niveau puzzle livré (demo4.json) survit à un round-trip.** *(criticité : Majeur)* — catégorie : Unitaire · Level Writer
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_writer.cpp:168` — `LevelWriterTest.NiveauPuzzleLivreSurvieAuRoundTrip`
- **saveToFile écrit un fichier qui se recharge à l'identique (round-trip disque).** *(criticité : Majeur)* — catégorie : Unitaire · Level Writer
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_writer.cpp:192` — `LevelWriterTest.SaveToFileEcritUnFichierRechargeable`
- **saveToFile vers un dossier inexistant échoue proprement (récupérable, EX-NFR-040).** *(criticité : Majeur)* — catégorie : Unitaire · Level Writer
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Levels/test_level_writer.cpp:219` — `LevelWriterTest.SaveToFileVersDossierInexistantEchoueProprement`

#### Math (20)

**`test_math_utils.cpp`**

- **approximatelyEqual : égalité exacte** *(criticité : Mineur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Comparer des valeurs identiques (1, 0, -2,5).
  - Résultat attendu : `approximatelyEqual` renvoie vrai.
  - `Source/Test/Unit/Core/Math/test_math_utils.cpp:12` — `MathUtilsTest.EgaliteExacte`
- **approximatelyEqual : tolérance par défaut** *(criticité : Majeur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Comparer 1 et 1+1e-6. 2. Comparer 1 et 1,1.
  - Résultat attendu : Égal sous la tolérance ; différent au-delà.
  - `Source/Test/Unit/Core/Math/test_math_utils.cpp:26` — `MathUtilsTest.ToleranceParDefaut`
- **approximatelyEqual : mise à l'échelle des grandes magnitudes** *(criticité : Majeur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Comparer 1e6 et 1e6+1. 2. Comparer 1e6 et 1e6+1000.
  - Résultat attendu : Égal pour un petit écart relatif ; différent pour un grand.
  - `Source/Test/Unit/Core/Math/test_math_utils.cpp:39` — `MathUtilsTest.MiseALEchelleGrandesMagnitudes`
- **approximatelyEqual : signes opposés** *(criticité : Mineur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Comparer 2 et -2.
  - Résultat attendu : Non égaux.
  - `Source/Test/Unit/Core/Math/test_math_utils.cpp:52` — `MathUtilsTest.SignesOpposes`
- **approximatelyEqual : tolérance explicite** *(criticité : Mineur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Comparer 1 et 1,5 sans tolérance. 2. Recomparer avec une tolérance de 1.
  - Résultat attendu : Différent par défaut ; égal avec la tolérance élargie.
  - `Source/Test/Unit/Core/Math/test_math_utils.cpp:64` — `MathUtilsTest.ToleranceExplicite`

**`test_rect.cpp`**

- **Les bords exposés découlent de la position et de la taille.** *(criticité : Majeur)* — catégorie : Unitaire · Rect
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Math/test_rect.cpp:17` — `RectTest.Bords`
- **Un point strictement intérieur est contenu.** *(criticité : Majeur)* — catégorie : Unitaire · Rect
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Math/test_rect.cpp:35` — `RectTest.ContientPointInterieur`
- **Contenance inclusive haut/gauche, exclusive bas/droit.** *(criticité : Majeur)* — catégorie : Unitaire · Rect
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Math/test_rect.cpp:50` — `RectTest.ContientBords`
- **Un point extérieur n'est pas contenu.** *(criticité : Majeur)* — catégorie : Unitaire · Rect
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Math/test_rect.cpp:67` — `RectTest.NeContientPasExterieur`
- **Deux rectangles qui se recouvrent s'intersectent (relation symétrique).** *(criticité : Majeur)* — catégorie : Unitaire · Rect
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Math/test_rect.cpp:83` — `RectTest.IntersectionRecouvrement`
- **Un simple contact par un bord ne compte pas comme intersection.** *(criticité : Majeur)* — catégorie : Unitaire · Rect
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Math/test_rect.cpp:100` — `RectTest.ContactBordSansIntersection`
- **Deux rectangles disjoints ne s'intersectent pas.** *(criticité : Majeur)* — catégorie : Unitaire · Rect
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Math/test_rect.cpp:116` — `RectTest.Disjonction`

**`test_vector2.cpp`**

- **Vector2 : addition et soustraction** *(criticité : Majeur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Poser deux vecteurs. 2. Calculer a+b et a-b.
  - Résultat attendu : Opérations composante à composante correctes.
  - `Source/Test/Unit/Core/Math/test_vector2.cpp:16` — `Vector2Test.AdditionSoustraction`
- **Vector2 : échelle par un scalaire** *(criticité : Majeur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Poser un vecteur. 2. Multiplier/diviser par un scalaire, prendre l'opposé.
  - Résultat attendu : Chaque composante est mise à l'échelle ; l'opposé inverse les signes.
  - `Source/Test/Unit/Core/Math/test_vector2.cpp:31` — `Vector2Test.EchelleScalaire`
- **Vector2 : opérateurs composés** *(criticité : Mineur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Appliquer +=, -=, *=, /= à un vecteur.
  - Résultat attendu : Le vecteur est modifié en place, résultats corrects.
  - `Source/Test/Unit/Core/Math/test_vector2.cpp:47` — `Vector2Test.OperateursComposes`
- **Vector2 : produit scalaire** *(criticité : Majeur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Calculer a·b. 2. Calculer le produit de deux vecteurs orthogonaux.
  - Résultat attendu : Somme des produits (11) ; produit nul pour des vecteurs orthogonaux.
  - `Source/Test/Unit/Core/Math/test_vector2.cpp:67` — `Vector2Test.ProduitScalaire`
- **Vector2 : longueur** *(criticité : Majeur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Calculer `lengthSquared` et `length` de (3, 4).
  - Résultat attendu : 25 et 5 (théorème de Pythagore).
  - `Source/Test/Unit/Core/Math/test_vector2.cpp:83` — `Vector2Test.Longueur`
- **Vector2 : normalisation (non nul)** *(criticité : Majeur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Normaliser (3, 4).
  - Résultat attendu : Longueur 1 ; vecteur (0,6 ; 0,8).
  - `Source/Test/Unit/Core/Math/test_vector2.cpp:97` — `Vector2Test.NormalisationNonNulle`
- **Vector2 : normalisation (vecteur nul)** *(criticité : Majeur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Normaliser le vecteur nul.
  - Résultat attendu : Le vecteur nul (pas de division par zéro).
  - `Source/Test/Unit/Core/Math/test_vector2.cpp:111` — `Vector2Test.NormalisationVecteurNul`
- **Vector2 : égalité approchée** *(criticité : Majeur)* — catégorie : Unitaire · Mathématiques
  - Étapes : 1. Comparer (0,1+0,2 ; 1) à (0,3 ; 1). 2. Comparer deux vecteurs distincts.
  - Résultat attendu : Égal malgré l'erreur flottante ; différent pour des vecteurs distincts.
  - `Source/Test/Unit/Core/Math/test_vector2.cpp:123` — `Vector2Test.EgaliteApprochee`

#### Physics (8)

**`test_swept_collision.cpp`**

- **Balayage : trajet libre** *(criticité : Majeur)* — catégorie : Unitaire · Physique · Balayage AABB
  - Étapes : 1. Grille sans tuile solide. 2. Balayer une boîte 1×1 d'un déplacement (3, 2).
  - Résultat attendu : Aucun contact ; la boîte atteint exactement la position visée.
  - `Source/Test/Unit/Core/Physics/test_swept_collision.cpp:30` — `SweptCollisionTest.TrajetLibre`
- **Balayage : butée horizontale (mur à droite)** *(criticité : Bloquant)* — catégorie : Unitaire · Physique · Balayage AABB
  - Étapes : 1. Placer un mur solide à droite. 2. Balayer la boîte vers la droite.
  - Résultat attendu : La boîte s'arrête au ras du mur (bord droit = bord du mur) ; normale horizontale.
  - `Source/Test/Unit/Core/Physics/test_swept_collision.cpp:50` — `SweptCollisionTest.ButeeHorizontale`
- **Balayage : butée verticale (sol)** *(criticité : Bloquant)* — catégorie : Unitaire · Physique · Balayage AABB
  - Étapes : 1. Placer un sol solide en dessous. 2. Balayer la boîte vers le bas.
  - Résultat attendu : La boîte se pose sur le sol (bord bas = haut du sol) ; normale vers le haut.
  - `Source/Test/Unit/Core/Physics/test_swept_collision.cpp:70` — `SweptCollisionTest.ButeeVerticale`
- **Balayage : non-tunneling à vitesse élevée** *(criticité : Bloquant)* — catégorie : Unitaire · Physique · Balayage AABB
  - Étapes : 1. Mur à 5 tuiles. 2. Balayer d'un déplacement de 10 (≫ une tuile) vers le mur.
  - Résultat attendu : La boîte s'arrête au ras du mur ; elle ne le traverse pas (garantie continue).
  - `Source/Test/Unit/Core/Physics/test_swept_collision.cpp:90` — `SweptCollisionTest.NonTunneling`
- **Balayage : glissement le long d'un mur** *(criticité : Bloquant)* — catégorie : Unitaire · Physique · Balayage AABB
  - Étapes : 1. Mur vertical à droite. 2. Balayer d'un déplacement diagonal (droite + bas).
  - Résultat attendu : Bloquée en X (au ras du mur), mais la descente complète en Y a lieu (glissement).
  - `Source/Test/Unit/Core/Physics/test_swept_collision.cpp:110` — `SweptCollisionTest.GlissementLeLongDuMur`
- **Balayage : butée horizontale (mur à gauche)** *(criticité : Majeur)* — catégorie : Unitaire · Physique · Balayage AABB
  - Étapes : 1. Mur solide à gauche. 2. Balayer la boîte vers la gauche.
  - Résultat attendu : La boîte s'arrête au ras du mur (bord gauche = bord droit du mur) ; normale opposée.
  - `Source/Test/Unit/Core/Physics/test_swept_collision.cpp:132` — `SweptCollisionTest.ButeeGauche`
- **Balayage : butée verticale (plafond)** *(criticité : Majeur)* — catégorie : Unitaire · Physique · Balayage AABB
  - Étapes : 1. Plafond solide au-dessus. 2. Balayer la boîte vers le haut.
  - Résultat attendu : La boîte s'arrête sous le plafond (bord haut = bas du plafond) ; normale vers le bas.
  - `Source/Test/Unit/Core/Physics/test_swept_collision.cpp:151` — `SweptCollisionTest.ButeePlafond`
- **Balayage : marcher sur un sol sans blocage horizontal** *(criticité : Majeur)* — catégorie : Unitaire · Physique · Balayage AABB
  - Étapes : 1. Sol continu ; boîte posée dessus. 2. Balayer horizontalement le long du sol.
  - Résultat attendu : Aucun blocage : la « peau » perpendiculaire évite de confondre marcher sur / buter contre.
  - `Source/Test/Unit/Core/Physics/test_swept_collision.cpp:171` — `SweptCollisionTest.MarcheSurLeSolSansBlocageHorizontal`

#### Time (6)

**`test_fixed_timestep.cpp`**

- **Un temps écoulé égal au pas fixe produit exactement un pas.** *(criticité : Majeur)* — catégorie : Unitaire · Fixed Timestep
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:16` — `FixedTimestepTest.UnPasExact`
- **Un temps écoulé inférieur au pas ne produit aucun pas.** *(criticité : Majeur)* — catégorie : Unitaire · Fixed Timestep
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:31` — `FixedTimestepTest.TempsInsuffisant`
- **Un temps écoulé nul ou négatif ne produit aucun pas.** *(criticité : Majeur)* — catégorie : Unitaire · Fixed Timestep
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:46` — `FixedTimestepTest.TempsNulOuNegatif`
- **2,5 pas donnent 2 pas, et le reste (0,5 pas) est conservé puis complété.** *(criticité : Majeur)* — catégorie : Unitaire · Fixed Timestep
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:62` — `FixedTimestepTest.ResteConserve`
- **Un temps écoulé énorme est plafonné (anti-spirale de la mort).** *(criticité : Majeur)* — catégorie : Unitaire · Fixed Timestep
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:82` — `FixedTimestepTest.PlafondAntiSpirale`
- **Le pas fixe exposé correspond à la configuration.** *(criticité : Majeur)* — catégorie : Unitaire · Fixed Timestep
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:100` — `FixedTimestepTest.PasFixeExpose`

### HMI

#### Editor (38)

**`test_level_name_validation.cpp`**

- **Un nom simple, sans caractère interdit, est valide.** *(criticité : Majeur)* — catégorie : Unitaire · Level Name Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:12` — `LevelNameValidationTest.NomSimpleValide`
- **Un nom vide ou composé uniquement d'espaces est invalide.** *(criticité : Majeur)* — catégorie : Unitaire · Level Name Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:27` — `LevelNameValidationTest.NomVideOuEspacesInvalide`
- **Un nom contenant un caractère interdit par le système de fichiers Windows est invalide.** *(criticité : Majeur)* — catégorie : Unitaire · Level Name Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:42` — `LevelNameValidationTest.CaractereInterditInvalide`
- **Un nom accentué (Unicode) reste valide.** *(criticité : Mineur)* — catégorie : Unitaire · Level Name Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:67` — `LevelNameValidationTest.NomAccentueValide`
- **trimLevelName retire les espaces de bord sans toucher au contenu.** *(criticité : Mineur)* — catégorie : Unitaire · Level Name Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:81` — `LevelNameValidationTest.TrimRetireLesEspacesDeBord`

**`test_level_picker.cpp`**

- **La sélection initiale pointe le premier choix (« Nouveau niveau »).** *(criticité : Mineur)* — catégorie : Unitaire · Level Picker
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_picker.cpp:34` — `LevelPickerTest.SelectionInitialeEstLePremierChoix`
- **Bas puis Haut déplacent la sélection, avec bouclage aux extrémités.** *(criticité : Majeur)* — catégorie : Unitaire · Level Picker
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_picker.cpp:50` — `LevelPickerTest.FlechesDeplacentLaSelectionAvecBouclage`
- **Entrée confirme l'indice actuellement sélectionné.** *(criticité : Majeur)* — catégorie : Unitaire · Level Picker
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_picker.cpp:84` — `LevelPickerTest.EntreeConfirmeLaSelection`
- **Sans appui, update() ne confirme rien.** *(criticité : Mineur)* — catégorie : Unitaire · Level Picker
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_picker.cpp:113` — `LevelPickerTest.SansAppuiAucuneConfirmation`
- **Survoler un choix à la souris déplace la sélection dessus (sans confirmer).** *(criticité : Majeur)* — catégorie : Unitaire · Level Picker
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_picker.cpp:131` — `LevelPickerTest.SurvolSourisDeplaceLaSelection`
- **Un clic gauche sur un choix survolé le confirme.** *(criticité : Majeur)* — catégorie : Unitaire · Level Picker
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_picker.cpp:153` — `LevelPickerTest.ClicGaucheConfirmeLeChoixSurvole`
- **La souris hors de tout choix ne change ni la sélection ni la confirmation.** *(criticité : Mineur)* — catégorie : Unitaire · Level Picker
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_picker.cpp:177` — `LevelPickerTest.SourisHorsChoixNeChangeRien`
- **forDirectory sur un dossier inexistant ne propose que « Nouveau niveau ».** *(criticité : Majeur)* — catégorie : Unitaire · Level Picker
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_picker.cpp:200` — `LevelPickerTest.ForDirectoryDossierInexistantNeProposeQueNouveauNiveau`

**`test_level_size_validation.cpp`**

- **Un format « largeur x hauteur » valide est analysé correctement.** *(criticité : Majeur)* — catégorie : Unitaire · Level Size Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:12` — `LevelSizeValidationTest.FormatValideAnalyseCorrectement`
- **Les espaces autour du séparateur et une casse différente sont tolérés.** *(criticité : Majeur)* — catégorie : Unitaire · Level Size Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:29` — `LevelSizeValidationTest.EspacesEtCasseTolerees`
- **Le séparateur `*` est accepté comme alternative à `x`/`X`.** *(criticité : Majeur)* — catégorie : Unitaire · Level Size Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:47` — `LevelSizeValidationTest.SeparateurEtoileAccepte`
- **Un texte sans séparateur est refusé.** *(criticité : Majeur)* — catégorie : Unitaire · Level Size Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:62` — `LevelSizeValidationTest.SansSeparateurRefuse`
- **Une dimension non numérique, nulle, négative ou partiellement numérique est refusée.** *(criticité : Majeur)* — catégorie : Unitaire · Level Size Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:77` — `LevelSizeValidationTest.DimensionInvalideRefusee`
- **Une dimension au-delà du plafond est refusée ; le plafond lui-même est accepté.** *(criticité : Majeur)* — catégorie : Unitaire · Level Size Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:96` — `LevelSizeValidationTest.PlafondRespecte`
- **isValidLevelSize reste cohérent avec parseLevelSize.** *(criticité : Mineur)* — catégorie : Unitaire · Level Size Validation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:116` — `LevelSizeValidationTest.IsValidCoherentAvecParse`

**`test_text_input_field.cpp`**

- **Les caractères tapés s'accumulent dans le texte du champ, dans l'ordre de saisie.** *(criticité : Majeur)* — catégorie : Unitaire · Text Input Field
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:13` — `TextInputFieldTest.CaracteresTapesSAccumulent`
- **Retour arrière retire le dernier caractère du texte pré-rempli.** *(criticité : Majeur)* — catégorie : Unitaire · Text Input Field
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:38` — `TextInputFieldTest.RetourArriereRetireLeDernierCaractere`
- **Retour arrière retire un caractère accentué entier (UTF-8 multi-octets), pas un octet.** *(criticité : Majeur)* — catégorie : Unitaire · Text Input Field
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:59` — `TextInputFieldTest.RetourArriereRetireUnCaractereAccentueEntier`
- **Sans validateur, Entrée confirme toujours la saisie courante.** *(criticité : Majeur)* — catégorie : Unitaire · Text Input Field
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:87` — `TextInputFieldTest.EntreeConfirmeSansValidateur`
- **Un validateur qui refuse le texte empêche la confirmation et marque un refus.** *(criticité : Majeur)* — catégorie : Unitaire · Text Input Field
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:109` — `TextInputFieldTest.EntreeRefuseeParLeValidateur`
- **Modifier le texte après un refus efface l'indicateur de refus.** *(criticité : Majeur)* — catégorie : Unitaire · Text Input Field
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:132` — `TextInputFieldTest.EditionApresRefusEffaceLeRefus`
- **Échap annule la saisie sans modifier le texte du champ.** *(criticité : Majeur)* — catégorie : Unitaire · Text Input Field
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:159` — `TextInputFieldTest.EchapAnnule`
- **Une fois confirmé, update() n'a plus aucun effet sur le champ.** *(criticité : Mineur)* — catégorie : Unitaire · Text Input Field
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:181` — `TextInputFieldTest.SansEffetApresConfirmation`

**`test_tile_palette.cpp`**

- **La palette expose au moins une entrée par type de tuile éditable.** *(criticité : Mineur)* — catégorie : Unitaire · Tile Palette
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:13` — `TilePaletteTest.ExposeUneEntreeParType`
- **La sélection par défaut est Solid.** *(criticité : Mineur)* — catégorie : Unitaire · Tile Palette
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:28` — `TilePaletteTest.SelectionParDefautEstSolid`
- **Cliquer une entrée de la palette sélectionne son type et signale le clic consommé.** *(criticité : Majeur)* — catégorie : Unitaire · Tile Palette
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:43` — `TilePaletteTest.ClicSurUneEntreeSelectionneSonType`
- **Cliquer hors de la palette ne change pas la sélection et n'est pas consommé.** *(criticité : Majeur)* — catégorie : Unitaire · Tile Palette
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:66` — `TilePaletteTest.ClicHorsPaletteNonConsomme`
- **Chaque entrée porte un libellé non vide.** *(criticité : Mineur)* — catégorie : Unitaire · Tile Palette
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:87` — `TilePaletteTest.ChaqueEntreePorteUnLibelle`

**`test_tool_bar.cpp`**

- **La sélection par défaut est l'outil Pinceau.** *(criticité : Mineur)* — catégorie : Unitaire · Tool Bar
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:13` — `ToolBarTest.SelectionParDefautEstPinceau`
- **La barre expose une entrée pour chacun des trois outils.** *(criticité : Mineur)* — catégorie : Unitaire · Tool Bar
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:28` — `ToolBarTest.ExposeTroisEntrees`
- **Cliquer une entrée de la barre sélectionne son outil et signale le clic consommé.** *(criticité : Majeur)* — catégorie : Unitaire · Tool Bar
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:43` — `ToolBarTest.ClicSurUneEntreeSelectionneSonOutil`
- **Cliquer hors de la barre ne change pas la sélection et n'est pas consommé.** *(criticité : Majeur)* — catégorie : Unitaire · Tool Bar
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:66` — `ToolBarTest.ClicHorsBarreNonConsomme`
- **selectNext() fait défiler les outils dans l'ordre attendu, en boucle.** *(criticité : Majeur)* — catégorie : Unitaire · Tool Bar
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:87` — `ToolBarTest.SelectNextDefileEnBoucle`

#### Graphics (9)

**`test_camera2d.cpp`**

- **Le centre de la caméra se projette au centre de l'écran.** *(criticité : Majeur)* — catégorie : Unitaire · Camera2 D
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:21` — `Camera2DTest.CentreAuMilieuDeLEcran`
- **Une unité monde vaut 16 pixels ; l'axe Y va vers le bas.** *(criticité : Majeur)* — catégorie : Unitaire · Camera2 D
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:40` — `Camera2DTest.EchelleEtAxeY`
- **Le zoom multiplie l'échelle en pixels.** *(criticité : Majeur)* — catégorie : Unitaire · Camera2 D
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:62` — `Camera2DTest.Zoom`
- **`screenToWorld` est la réciproque de `worldToScreen`.** *(criticité : Majeur)* — catégorie : Unitaire · Camera2 D
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:80` — `Camera2DTest.ConversionsReciproques`
- **La matrice de projection envoie le centre de la caméra à l'origine du clip space.** *(criticité : Majeur)* — catégorie : Unitaire · Camera2 D
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:101` — `Camera2DTest.ProjectionCentreVersOrigineClip`
- **Un coin de l'écran correspond à un bord du clip space (±1).** *(criticité : Majeur)* — catégorie : Unitaire · Camera2 D
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:128` — `Camera2DTest.BordEcranVersBordClip`
- **fitZoom reste entier tant que le facteur brut est supérieur ou égal à 1 (petit niveau).** *(criticité : Majeur)* — catégorie : Unitaire · Camera2 D
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:152` — `Camera2DTest.FitZoomEntierPourPetitNiveau`
- **fitZoom devient fractionnaire pour un niveau plus grand que la surface disponible.** *(criticité : Majeur)* — catégorie : Unitaire · Camera2 D
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:171` — `Camera2DTest.FitZoomFractionnairePourGrandNiveau`
- **fitZoom applique la marge avant l'arrondi.** *(criticité : Mineur)* — catégorie : Unitaire · Camera2 D
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:192` — `Camera2DTest.FitZoomAppliqueLaMarge`

#### Input (24)

**`test_input_state.cpp`**

- **Une touche passée d'« absente » à « présente » est « pressée » exactement une frame.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:12` — `InputStateTest.FrontMontantClavier`
- **Une touche restée enfoncée n'est « pressée » qu'à la première frame.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:38` — `InputStateTest.MaintienClavier`
- **Le relâchement d'une touche est détecté « relâchée » pendant exactement une frame.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:63` — `InputStateTest.FrontDescendantClavier`
- **Les touches sont indépendantes : un front sur l'une n'affecte pas les autres.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:90` — `InputStateTest.TouchesIndependantes`
- **Un bouton de souris suit la même logique pressé/cliqué/relâché que les touches.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:111` — `InputStateTest.BoutonSouris`
- **La position de la souris reflète le dernier déplacement injecté.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:140` — `InputStateTest.PositionSouris`
- **Les incréments de molette d'une frame s'additionnent et repartent de zéro ensuite.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:162` — `InputStateTest.MoletteAccumuleEtSeReinitialise`
- **Les caractères tapés s'accumulent dans l'ordre puis sont vidés à la frame suivante.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:186` — `InputStateTest.CaracteresTapesAccumulesEtVides`
- **Un bouton manette seul rend `keyDown`/`keyPressed` vrais, comme au clavier.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:215` — `InputStateTest.ManetteSeuleActiveLaTouche`
- **Clavier et manette combinés sur la même touche ne produisent pas de double front.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:243` — `InputStateTest.ClavierEtManetteMemeToucheUnSeulFront`
- **La manette relâchée ne masque jamais une touche clavier réellement maintenue.** *(criticité : Majeur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:280` — `InputStateTest.ManetteRelacheeNeMasquePasLeClavier`
- **`gamepadConnected` reflète le dernier `setGamepadConnected` appelé.** *(criticité : Mineur)* — catégorie : Unitaire · Input State
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_input_state.cpp:308` — `InputStateTest.GamepadConnecteReecrasable`

**`test_player_input_mapper.cpp`**

- **Flèche gauche seule → intention vers la gauche (-1).** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:27` — `PlayerInputMapperTest.FlecheGauche`
- **Flèche droite seule → intention vers la droite (+1).** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:41` — `PlayerInputMapperTest.FlecheDroite`
- **Touches alternatives ZQSD : Q → gauche, D → droite.** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:55` — `PlayerInputMapperTest.TouchesAlternativesQetD`
- **Aucune touche → intention nulle (immobile).** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:70` — `PlayerInputMapperTest.AucuneTouche`
- **Gauche et droite simultanées → neutralisation (0).** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:84` — `PlayerInputMapperTest.GaucheEtDroiteSeNeutralisent`
- **Espace fraîchement enfoncée → saut **pressé** (front) et **maintenu**.** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:98` — `PlayerInputMapperTest.EspacePresseeDeclencheLeSaut`
- **`W` équivaut à Espace pour le saut.** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:114` — `PlayerInputMapperTest.WEquivautEspacePourLeSaut`
- **Saut **maintenu** sans nouveau front → `jumpHeld` vrai mais `jumpPressed` faux.** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:128` — `PlayerInputMapperTest.SautMaintenuN_estPasUnFront`
- **Aucune touche de saut → ni pressé ni maintenu.** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:150` — `PlayerInputMapperTest.PasDeSaut`
- **Déplacement et saut sont indépendants (axes distincts).** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:166` — `PlayerInputMapperTest.DeplacementEtSautIndependants`
- **Maj fraîchement enfoncée → dash **pressé** (front).** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:183` — `PlayerInputMapperTest.MajDeclencheLeDash`
- **Visée verticale (y vers le bas) : Bas → +1, Haut → -1, les deux → 0.** *(criticité : Majeur)* — catégorie : Unitaire · Player Input Mapper
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:198` — `PlayerInputMapperTest.ViseeVerticale`

#### Interface (34)

**`test_language_selector.cpp`**

- **Le bouton est ancré au coin bas-droit, marge comprise.** *(criticité : Majeur)* — catégorie : Unitaire · Language Selector
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_language_selector.cpp:29` — `LanguageSelectorTest.RectangleAncreBasDroite`
- **La bascule renvoie l'autre langue (français ↔ anglais).** *(criticité : Majeur)* — catégorie : Unitaire · Language Selector
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_language_selector.cpp:51` — `LanguageSelectorTest.BasculeAlterneLesLangues`
- **Un clic dans le bouton demande la bascule vers l'autre langue.** *(criticité : Majeur)* — catégorie : Unitaire · Language Selector
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_language_selector.cpp:67` — `LanguageSelectorTest.ClicDansLeBoutonBascule`
- **Un clic hors du bouton ne demande aucune bascule.** *(criticité : Majeur)* — catégorie : Unitaire · Language Selector
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_language_selector.cpp:90` — `LanguageSelectorTest.ClicHorsBoutonNeBasculePas`
- **Sans clic, aucune bascule même si la souris est sur le bouton.** *(criticité : Majeur)* — catégorie : Unitaire · Language Selector
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_language_selector.cpp:108` — `LanguageSelectorTest.SansClicPasDeBascule`

**`test_level_sequence.cpp`**

- **Une séquence vide est signalée comme telle.** *(criticité : Majeur)* — catégorie : Unitaire · Level Sequence
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_level_sequence.cpp:12` — `LevelSequenceTest.SequenceVide`
- **La séquence démarre sur le premier niveau, dans l'ordre fourni.** *(criticité : Majeur)* — catégorie : Unitaire · Level Sequence
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_level_sequence.cpp:28` — `LevelSequenceTest.DemarreSurLePremier`
- **`advance` parcourt les niveaux dans l'ordre jusqu'au dernier.** *(criticité : Majeur)* — catégorie : Unitaire · Level Sequence
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_level_sequence.cpp:47` — `LevelSequenceTest.AvanceDansLOrdre`
- **Au-delà du dernier niveau, `advance` est sans effet (pas de dépassement).** *(criticité : Majeur)* — catégorie : Unitaire · Level Sequence
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_level_sequence.cpp:70` — `LevelSequenceTest.NeDepassePasLeDernier`

**`test_menu_model.cpp`**

- **À l'ouverture, la première option (Charger niveau) est sélectionnée.** *(criticité : Majeur)* — catégorie : Unitaire · Menu Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_menu_model.cpp:63` — `MenuModelTest.SelectionParDefaut`
- **Valider par défaut (Entrée) bascule vers l'écran de jeu.** *(criticité : Majeur)* — catégorie : Unitaire · Menu Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_menu_model.cpp:82` — `MenuModelTest.ValiderChargeNiveau`
- **La flèche bas déplace la sélection ; valider mène alors au Mode Edition.** *(criticité : Majeur)* — catégorie : Unitaire · Menu Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_menu_model.cpp:101` — `MenuModelTest.FlecheBasPuisValider`
- **La flèche haut depuis la première option boucle sur la dernière (Quitter).** *(criticité : Majeur)* — catégorie : Unitaire · Menu Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_menu_model.cpp:124` — `MenuModelTest.FlecheHautBoucleSurQuitter`
- **La troisième option (Options) bascule vers l'écran de réglages.** *(criticité : Majeur)* — catégorie : Unitaire · Menu Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_menu_model.cpp:145` — `MenuModelTest.OptionOptionsBasculeVersEcranOptions`
- **Quatre flèches bas ramènent à la première option (bouclage déterministe).** *(criticité : Majeur)* — catégorie : Unitaire · Menu Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_menu_model.cpp:168` — `MenuModelTest.BouclageBasComplet`
- **Le survol d'une option à la souris la sélectionne.** *(criticité : Majeur)* — catégorie : Unitaire · Menu Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_menu_model.cpp:192` — `MenuModelTest.SurvolSourisSelectionne`
- **Un clic gauche sur une option la valide.** *(criticité : Majeur)* — catégorie : Unitaire · Menu Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_menu_model.cpp:211` — `MenuModelTest.ClicValideOption`
- **Un clic hors de toute option ne valide rien.** *(criticité : Majeur)* — catégorie : Unitaire · Menu Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_menu_model.cpp:231` — `MenuModelTest.ClicHorsOptionNeValidePas`

**`test_options_model.cpp`**

- **À l'ouverture, la première option (V-Sync) est sélectionnée et affiche l'état fourni.** *(criticité : Majeur)* — catégorie : Unitaire · Options Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_options_model.cpp:53` — `OptionsModelTest.SelectionParDefautEtLibelleVSync`
- **Valider la première option (V-Sync) renvoie l'action ToggleVSync.** *(criticité : Majeur)* — catégorie : Unitaire · Options Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_options_model.cpp:73` — `OptionsModelTest.ValiderVSyncRenvoieToggle`
- **Flèche bas puis valider (Retour) renvoie l'action Back.** *(criticité : Majeur)* — catégorie : Unitaire · Options Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_options_model.cpp:92` — `OptionsModelTest.FlecheBasPuisValiderRenvoieBack`
- **La flèche haut depuis la première option boucle sur la dernière (Retour).** *(criticité : Majeur)* — catégorie : Unitaire · Options Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_options_model.cpp:115` — `OptionsModelTest.FlecheHautBoucleSurRetour`
- **Le survol d'une option à la souris la sélectionne ; un clic la valide.** *(criticité : Majeur)* — catégorie : Unitaire · Options Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_options_model.cpp:133` — `OptionsModelTest.ClicSourisValideRetour`
- **`setVSyncEnabled` resynchronise le libellé affiché après une bascule.** *(criticité : Majeur)* — catégorie : Unitaire · Options Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_options_model.cpp:154` — `OptionsModelTest.SetVSyncEnabledChangeLeLibelle`
- **Un clic hors de toute option ne valide rien.** *(criticité : Mineur)* — catégorie : Unitaire · Options Model
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_options_model.cpp:173` — `OptionsModelTest.ClicHorsOptionNeValidePas`

**`test_save_log_button.cpp`**

- **Le bouton est à gauche du bouton de langue, aligné sur le même bord bas.** *(criticité : Majeur)* — catégorie : Unitaire · Save Log Button
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_save_log_button.cpp:30` — `SaveLogButtonTest.AGaucheDuBoutonLangue`
- **Un clic dans le bouton est détecté.** *(criticité : Majeur)* — catégorie : Unitaire · Save Log Button
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_save_log_button.cpp:50` — `SaveLogButtonTest.ClicDansLeBouton`
- **Un clic hors du bouton n'est pas détecté.** *(criticité : Majeur)* — catégorie : Unitaire · Save Log Button
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_save_log_button.cpp:69` — `SaveLogButtonTest.ClicHorsBouton`

**`test_screen_manager.cpp`**

- **La construction fabrique l'écran initial demandé.** *(criticité : Majeur)* — catégorie : Unitaire · Screen Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_screen_manager.cpp:49` — `ScreenManagerTest.ConstructionCreeEcranInitial`
- **Une transition « rester » conserve l'écran actif et n'en fabrique pas d'autre.** *(criticité : Majeur)* — catégorie : Unitaire · Screen Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_screen_manager.cpp:68` — `ScreenManagerTest.TransitionNoneResteSurEcran`
- **Une transition « basculer » remplace l'écran actif par celui demandé.** *(criticité : Majeur)* — catégorie : Unitaire · Screen Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_screen_manager.cpp:91` — `ScreenManagerTest.TransitionSwitchRemplaceEcran`
- **Une transition « quitter » ferme l'application et se propage à la boucle.** *(criticité : Majeur)* — catégorie : Unitaire · Screen Manager
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_screen_manager.cpp:116` — `ScreenManagerTest.TransitionQuitFermeApplication`

**`test_session_log.cpp`**

- **Chaque message donne une ligne, dans l'ordre d'arrivée.** *(criticité : Majeur)* — catégorie : Unitaire · Session Log
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_session_log.cpp:15` — `SessionLogTest.SerialiseUneLigneParMessage`
- **Une session sans message produit un texte vide.** *(criticité : Majeur)* — catégorie : Unitaire · Session Log
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Interface/test_session_log.cpp:34` — `SessionLogTest.VideDonneChaineVide`

#### Localization (8)

**`test_localization.cpp`**

- **L'analyse ignore les lignes vides et les commentaires, et retire les espaces autour de '='.** *(criticité : Majeur)* — catégorie : Unitaire · Localization
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Localization/test_localization.cpp:17` — `LocalizationTest.AnalyseIgnoreCommentairesEtEspaces`
- **Seul le premier '=' sépare ; un '=' dans la valeur est conservé.** *(criticité : Majeur)* — catégorie : Unitaire · Localization
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Localization/test_localization.cpp:42` — `LocalizationTest.AnalyseConserveEgalDansLaValeur`
- **Une clé existante est résolue dans la langue active.** *(criticité : Majeur)* — catégorie : Unitaire · Localization
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Localization/test_localization.cpp:59` — `LocalizationTest.CleExistanteResolue`
- **Une clé inconnue partout est renvoyée telle quelle (repli déterministe, pas de plantage).** *(criticité : Majeur)* — catégorie : Unitaire · Localization
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Localization/test_localization.cpp:77` — `LocalizationTest.CleInconnueRenvoieLaCle`
- **Changer de langue résout les valeurs de la nouvelle langue.** *(criticité : Majeur)* — catégorie : Unitaire · Localization
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Localization/test_localization.cpp:96` — `LocalizationTest.ChangementDeLangue`
- **Une clé manquante dans la langue active retombe sur la langue par défaut.** *(criticité : Majeur)* — catégorie : Unitaire · Localization
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Localization/test_localization.cpp:115` — `LocalizationTest.RepliSurLangueParDefaut`
- **Charger une langue absente échoue proprement et conserve la langue active (récupérable).** *(criticité : Majeur)* — catégorie : Unitaire · Localization
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Localization/test_localization.cpp:134` — `LocalizationTest.LangueAbsenteEstRecuperable`
- **Le catalogue français livré (Source/Elements/Localization) se charge et résout ses clés.** *(criticité : Majeur)* — catégorie : Unitaire · Localization
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Unit/HMI/Localization/test_localization.cpp:155` — `LocalizationTest.CatalogueFrancaisLivreSeCharge`

## Tests d'intégration (48)

### Animation Personnage — `test_animation_personnage.cpp` (5)

- **Au sol et immobile, le clip est Idle et l'image alterne 0/1 après chaque durée d'image.** *(criticité : Majeur)* — catégorie : Integration · Animation Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_animation_personnage.cpp:36` — `AnimationPersonnageIntegration.ImmobileAuSolEstEnRepos`
- **Au sol et en mouvement, le clip est Run et l'image boucle sur les 4 images dans l'ordre.** *(criticité : Majeur)* — catégorie : Integration · Animation Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_animation_personnage.cpp:67` — `AnimationPersonnageIntegration.EnMouvementAuSolCourt`
- **En l'air, le clip est Jump et l'image reste figée sur 0, quelle que soit la durée.** *(criticité : Majeur)* — catégorie : Integration · Animation Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_animation_personnage.cpp:100` — `AnimationPersonnageIntegration.EnLAirEstEnSaut`
- **Un changement de clip réinitialise immédiatement l'image et le chronomètre à zéro.** *(criticité : Majeur)* — catégorie : Integration · Animation Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_animation_personnage.cpp:125` — `AnimationPersonnageIntegration.ChangementDeClipReinitialiseLImage`
- **Une entité sans composant Animation n'est pas affectée par le système.** *(criticité : Mineur)* — catégorie : Integration · Animation Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_animation_personnage.cpp:166` — `AnimationPersonnageIntegration.EntiteSansAnimationIgnoree`

### Boucle Simulation — `test_boucle_simulation.cpp` (2)

- **Le cadenceur découpe un temps réel variable en pas fixes, et la position finale correspond exactement au nombre de pas réellement exécutés (aucune dérive).** *(criticité : Majeur)* — catégorie : Integration · Boucle Simulation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_boucle_simulation.cpp:50` — `BoucleSimulationIntegration.CadenceurPiloteLaSimulation`
- **Déterminisme : même séquence de frames et même état initial -> même résultat.** *(criticité : Majeur)* — catégorie : Integration · Boucle Simulation
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_boucle_simulation.cpp:76` — `BoucleSimulationIntegration.MemeEntreeMemeResultat`

### Ecs Mouvement — `test_ecs_mouvement.cpp` (4)

- **Une entité Transform + Velocity avance de `velocity * fixedDelta` par pas ; après N pas, la position attendue est déterministe.** *(criticité : Majeur)* — catégorie : Integration · Ecs Mouvement
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_ecs_mouvement.cpp:25` — `EcsMouvementIntegration.EntiteMobileAvanceDeVitesseFoisPas`
- **Une entité sans Velocity ne bouge pas.** *(criticité : Majeur)* — catégorie : Integration · Ecs Mouvement
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_ecs_mouvement.cpp:55` — `EcsMouvementIntegration.EntiteSansVelociteNeBougePas`
- **Deux entités de vitesses différentes évoluent indépendamment.** *(criticité : Majeur)* — catégorie : Integration · Ecs Mouvement
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_ecs_mouvement.cpp:81` — `EcsMouvementIntegration.DeuxEntitesEvoluentIndependamment`
- **Enregistré dans le World, le système s'exécute via World::update (chaîne complète).** *(criticité : Majeur)* — catégorie : Integration · Ecs Mouvement
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_ecs_mouvement.cpp:114` — `EcsMouvementIntegration.IntegrationViaWorldUpdate`

### Niveau Ecs — `test_niveau_ecs.cpp` (2)

- **Le niveau chargé peuple le monde d'une entité par tuile non vide, bien placée et typée.** *(criticité : Majeur)* — catégorie : Integration · Niveau Ecs
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_niveau_ecs.cpp:49` — `NiveauEcsIntegration.DuJsonAuxEntites`
- **Le niveau de démonstration livré, chargé depuis le fichier, peuple bien le monde.** *(criticité : Majeur)* — catégorie : Integration · Niveau Ecs
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_niveau_ecs.cpp:85` — `NiveauEcsIntegration.FichierDemoVersMonde`

### Physique Personnage — `test_physique_personnage.cpp` (35)

- **Sans sol, le personnage tombe et sa vitesse verticale croît (gravité continue).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:269` — `PhysiquePersonnageIntegration.TombeSousGraviteVitesseCroissante`
- **En chute prolongée, la vitesse verticale converge vers une vitesse terminale sans la dépasser.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:301` — `PhysiquePersonnageIntegration.ChuteConvergeVersUneVitesseTerminale`
- **L'accélération verticale décroît à mesure que la vitesse approche le régime permanent.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:336` — `PhysiquePersonnageIntegration.AccelerationDeChuteDecroitVersLeRegimePermanent`
- **Une masse plus grande produit une vitesse terminale plus élevée, à traînée égale.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:376` — `PhysiquePersonnageIntegration.MasseSuperieureTombePlusVite`
- **Le personnage se pose sur le sol : vitesse verticale annulée, état « au sol » vrai.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:405` — `PhysiquePersonnageIntegration.AtterritSurLeSolEtEstAuSol`
- **Poussé contre un mur, le personnage s'arrête au ras du mur (blocage horizontal).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:435` — `PhysiquePersonnageIntegration.BloqueParUnMurADroite`
- **Sur terrain libre, l'avancée horizontale vaut vitesse × temps (vitesse constante).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:464` — `PhysiquePersonnageIntegration.AvanceAVitesseConstante`
- **Même en chute très rapide (grand pas), le personnage ne traverse pas le sol (balayage continu).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:495` — `PhysiquePersonnageIntegration.NeTraversePasLeSolEnChuteRapide`
- **Mêmes entrées → même résultat : la simulation est déterministe (pas fixe).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:527` — `PhysiquePersonnageIntegration.Deterministe`
- **Au sol, une pression de saut fait décoller le personnage (vitesse ascendante, il s'élève).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:557` — `PhysiquePersonnageIntegration.SauteDepuisLeSol`
- **En l'air, une pression de saut n'a aucun effet : pas de double saut (`EX-GP-013`).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:597` — `PhysiquePersonnageIntegration.PasDeSautEnLAir`
- **Hauteur de saut variable : maintenir le bouton saute plus haut que le relâcher tôt.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:626` — `PhysiquePersonnageIntegration.HauteurDeSautVariable`
- **Double saut : 1 saut au sol + N sauts aériens (paramétrable), rechargés au contact du sol.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:646` — `PhysiquePersonnageIntegration.DoubleSautNombreParametrable`
- **Gravité asymétrique : la chute accélère plus vite que la montée ne décélère (EX-GP-018).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:664` — `PhysiquePersonnageIntegration.ChutePlusRapideQueLaMontee`
- **Apex hang : près du sommet du saut, la gravité est réduite (contrôle flottant).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:713` — `PhysiquePersonnageIntegration.ApexHangReduitLaGravite`
- **Fast-fall : maintenir « bas » en l'air fait tomber plus loin qu'une chute libre normale.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:760` — `PhysiquePersonnageIntegration.FastFallAccelereLaChute`
- **Dash horizontal : une ruée rapide (≫ vitesse normale) fait parcourir une grande distance.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:788` — `PhysiquePersonnageIntegration.DashHorizontalRapide`
- **Dash diagonal (8 directions) : viser haut-droite envoie en +X et −Y.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:825` — `PhysiquePersonnageIntegration.DashDiagonalHautDroite`
- **Pendant un dash horizontal, la gravité est suspendue (la composante verticale reste nulle).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:856` — `PhysiquePersonnageIntegration.GraviteSuspenduePendantDash`
- **Une seule ruée par phase aérienne : le second dash en l'air est refusé jusqu'au retour au sol.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:887` — `PhysiquePersonnageIntegration.DashUneSeuleFoisEnLAir`
- **Un dash vers un mur ne le traverse pas (résolu par le balayage).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:942` — `PhysiquePersonnageIntegration.DashNeTraversePasLeMur`
- **Budget de sauts (EX-GP-024) : avec 1 saut, le premier fonctionne, le suivant est refusé.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:970` — `PhysiquePersonnageIntegration.BudgetDeSautsRefuseAuDela`
- **Budget de dashs (EX-GP-024) : avec 1 dash, le premier fonctionne, le suivant est refusé.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1007` — `PhysiquePersonnageIntegration.BudgetDeDashsRefuseAuDela`
- **Wall slide : collé à un mur en l'air, la vitesse de chute est plafonnée (descente ralentie).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1044` — `PhysiquePersonnageIntegration.WallSlideRalentitLaChute`
- **Wall jump : contre un mur à droite, un saut éjecte vers la gauche et le haut.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1076` — `PhysiquePersonnageIntegration.WallJumpEjecteAlOpposeDuMur`
- **Sans mur, la logique de wall jump ne s'active pas (et sans saut aérien, aucun saut en l'air).** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1112` — `PhysiquePersonnageIntegration.PasDeWallJumpSansMur`
- **Le niveau 2 est franchissable **avec le saut** : en avançant et sautant, on atteint la sortie.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1143` — `PhysiquePersonnageIntegration.Niveau2FranchissableAvecSaut`
- **Le niveau 2 **exige** le saut : en avançant seulement (sans sauter), la marche bloque.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1166` — `PhysiquePersonnageIntegration.Niveau2RequiertLeSaut`
- **Le niveau 3 (couloir bas + fosse) est franchissable **au dash** : avancer + dasher franchit.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1188` — `PhysiquePersonnageIntegration.Niveau3FranchissableAvecDash`
- **Le niveau 3 **exige** le dash : en avançant seulement, on tombe dans la fosse de danger.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1210` — `PhysiquePersonnageIntegration.Niveau3RequiertLeDash`
- **Niveau 4 (puzzle) : toucher l'interrupteur ouvre la porte → la sortie devient atteignable.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1232` — `PhysiquePersonnageIntegration.Niveau4FranchissableAvecInterrupteur`
- **Une porte **fermée** (interrupteur non touché) **bloque** : la sortie reste inatteignable.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1253` — `PhysiquePersonnageIntegration.PorteFermeeBloque`
- **Coyote time : sauter juste après avoir quitté un bord fonctionne ; trop tard, non.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1286` — `PhysiquePersonnageIntegration.CoyoteTimeAutoriseUnSautJusteApresLeBord`
- **Jump buffering : un saut pré-appuyé peu avant l'atterrissage s'exécute à la pose ; trop tôt, non.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1303` — `PhysiquePersonnageIntegration.JumpBufferingHonoreUnSautPreAppuye`
- **Le niveau de démonstration livré est **franchissable** sans saut : en maintenant « droite », le personnage descend l'escalier de paliers et atteint la sortie sans jamais mourir.** *(criticité : Majeur)* — catégorie : Integration · Physique Personnage
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Integration/test_physique_personnage.cpp:1321` — `PhysiquePersonnageIntegration.NiveauDemoEstFranchissableEnAllantADroite`

## Tests système (2)

### Parcours Complet — `test_parcours_complet.cpp` (1)

- **Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, puis « retour au titre ». Reproduit la boucle titre → niveau 1 → niveau 2 → titre du jeu.** *(criticité : Majeur)* — catégorie : Systeme · Parcours Complet
  - Étapes : 1. Mettre en place le contexte du test (arrangement). 2. Executer le scenario et verifier les assertions.
  - `Source/Test/Systeme/test_parcours_complet.cpp:91` — `ParcoursCompletSysteme.FranchitTouteLaSequence`

### Éditeur de niveaux — `test_parcours_edition.cpp` (1)

- **Parcours complet d'édition : peindre, lier un mécanisme, annuler une erreur, redimensionner, enregistrer, recharger, et confirmer que le niveau produit est directement jouable.** *(criticité : Critique)* — catégorie : Système · Éditeur de niveaux
  - Étapes : 1. Peindre un niveau (entrée, sortie, interrupteur/porte liés) dans un `LevelDraft`. 2. Peindre une tuile par erreur puis l'annuler (`undo`). 3. Redimensionner la grille. 4. Valider, enregistrer sur disque, recharger. 5. Vérifier le contenu rechargé et l'issue (`evaluateOutcome`) à l'entrée et à la sortie.
  - Résultat attendu : Le niveau rechargé restitue exactement le contenu édité (dont l'annulation) et se comporte comme un niveau livré normalement : `Playing` à l'entrée, `Won` à la sortie.
  - `Source/Test/Systeme/test_parcours_edition.cpp:26` — `ParcoursEditionSysteme.CreerEditerEnregistrerRechargerEtJouer`

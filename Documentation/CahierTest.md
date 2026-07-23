# Cahier de test {#cahiertest}

**321 cas de test**, générés depuis les blocs `\castest{...}` du code par `scripts/generate_cahier_test.py` (ne pas éditer directement — modifier le commentaire du test concerné puis relancer le script). Organisés ici selon l'arborescence de `Source/Test/` pour rester lisibles page par page.

## Tests unitaires (271)

### Core

**`test_core.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **EngineTest.VersionNonVide** (Majeur)<br/><sub>`Source/Test/Unit/Core/test_core.cpp:12`</sub> | Vérifie que la version du moteur n'est pas vide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

#### Diagnostics (14)

**`test_assert.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **AssertTest.ConditionVraieNInvoquePasLeHandler** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_assert.cpp:14`</sub> | Une condition vraie n'invoque pas le gestionnaire d'assertion. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **AssertTest.ConditionFausseInvoqueLeHandler** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_assert.cpp:37`</sub> | Une condition fausse invoque le gestionnaire une fois, avec le message (Debug uniquement). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_log_format.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LogFormatTest.LigneContientTousLesChamps** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_log_format.cpp:14`</sub> | La ligne formatée contient horodatage, niveau, catégorie, position source et message. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LogFormatTest.CheminReduitAuNomDeFichier** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_log_format.cpp:37`</sub> | Le chemin source est réduit à son nom de fichier dans la ligne. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LogFormatTest.FileNameIsoleLeNom** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_log_format.cpp:57`</sub> | fileName isole le nom de fichier des chemins Windows et POSIX, ou renvoie l'entrée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LogFormatTest.HorodatageFormatHeure** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_log_format.cpp:74`</sub> | L'horodatage courant respecte le format HH:MM:SS (longueur 8). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_log_level_parse.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LogLevelParseTest.NiveauxReconnus** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_log_level_parse.cpp:12`</sub> | Les noms de niveaux reconnus sont convertis (y compris l'alias « warn »). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LogLevelParseTest.InsensibleALaCasse** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_log_level_parse.cpp:30`</sub> | L'analyse est insensible à la casse. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LogLevelParseTest.ValeurInconnue** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_log_level_parse.cpp:46`</sub> | Une valeur inconnue ou vide n'est pas convertie. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_logger.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LoggerTest.FiltreParNiveauMinimal** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_logger.cpp:15`</sub> | Un message au-dessus du niveau minimal est diffusé ; en dessous, il est filtré. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LoggerTest.DiffuseAPlusieursSinks** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_logger.cpp:41`</sub> | Le même message atteint tous les sinks enregistrés. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LoggerTest.ClearSinksArreteLaDiffusion** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_logger.cpp:66`</sub> | clearSinks retire les destinations : plus rien n'est diffusé ensuite. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_sinks.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **MemoryLogSinkTest.ConserveNiveauEtTexteDansLOrdre** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_sinks.cpp:12`</sub> | Le sink mémoire conserve fidèlement niveau et texte, dans l'ordre. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MemoryLogSinkTest.ClearVideLesMessages** (Majeur)<br/><sub>`Source/Test/Unit/Core/Diagnostics/test_sinks.cpp:34`</sub> | clear vide les messages mémorisés. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

#### Ecs (34)

**`test_component_pool.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **ComponentPoolTest.AjoutPuisAcces** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_component_pool.cpp:25`</sub> | `add` puis `get` renvoie la valeur stockée ; `has` est cohérent. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ComponentPoolTest.GetRenvoieReferenceModifiable** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_component_pool.cpp:49`</sub> | `get` renvoie une référence modifiable sur le composant stocké. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ComponentPoolTest.RemoveAuMilieuSwapAndPop** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_component_pool.cpp:71`</sub> | `remove` d'un élément au milieu (swap-and-pop) laisse les autres composants accessibles et corrects, et le stockage dense reste contigu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ComponentPoolTest.RemoveDernierElement** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_component_pool.cpp:107`</sub> | Retirer le dernier élément ne perturbe pas les précédents. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ComponentPoolTest.HandlePerimeNePossedePasLeComposant** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_component_pool.cpp:134`</sub> | Un handle périmé (index recyclé, génération différente) ne possède pas le composant de l'ancienne entité. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ComponentPoolTest.RemoveIfPresent** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_component_pool.cpp:161`</sub> | `removeIfPresent` retire si le composant existe, sinon ne fait rien. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ComponentPoolTest.GetSurEntiteAbsenteViolePrecondition** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_component_pool.cpp:182`</sub> | `get` sur une entité absente viole une précondition (assertion en Debug). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_entity_manager.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **EntityManagerTest.CreeEntitesVivantesEtDistinctes** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:13`</sub> | `create` renvoie des entités vivantes et distinctes. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **EntityManagerTest.DestructionInvalideLeHandle** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:34`</sub> | Après destruction, l'ancien handle n'est plus vivant. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **EntityManagerTest.RecyclageChangeLaGeneration** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:55`</sub> | Un index recyclé produit une génération différente : l'ancien handle reste invalide, le nouveau est valide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **EntityManagerTest.DestructionHandlePerimeSansEffet** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:83`</sub> | Détruire un handle périmé est sans effet (idempotent) et ne touche pas l'entité vivante qui occupe désormais le même index. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **EntityManagerTest.EntiteInvalideJamaisVivante** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:107`</sub> | L'entité invalide conventionnelle n'est jamais vivante. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **EntityManagerTest.EgaliteHandleInvalide** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_entity_manager.cpp:125`</sub> | Le handle invalide se compare comme tel. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_player_components.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **PlayerComponentsTest.ColliderParDefautEstNul** (Mineur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_player_components.cpp:17`</sub> | Collider par défaut nul | 1. Construire un `core::Collider` par défaut.<br/>2. Lire `size.x` et `size.y`. | La boîte est nulle, `size` vaut (0, 0). |
| **PlayerComponentsTest.PlayerParDefautPasAuSol** (Mineur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_player_components.cpp:31`</sub> | Player par défaut (en l'air, budgets illimités) | 1. Construire un `core::Player` par défaut.<br/>2. Lire ses champs. | Pas au sol ; minuteries et compteurs à 0 ; orienté à droite ; dash indisponible ; budgets sauts/dashs à -1 (illimité) ; masse à 1,0 (`EX-GP-019`). |
| **PlayerComponentsTest.PlayerInputParDefautImmobile** (Mineur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_player_components.cpp:56`</sub> | PlayerInput par défaut neutre | 1. Construire un `core::PlayerInput` par défaut.<br/>2. Lire ses champs. | `moveX`/`moveY` nuls ; aucun front de saut ni de dash. |
| **PlayerComponentsTest.PhysicsConfigParDefautPlausible** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_player_components.cpp:73`</sub> | PhysicsConfig par défaut plausible | 1. Construire un `core::PhysicsConfig` par défaut.<br/>2. Vérifier chaque réglage. | Vitesses/temps > 0 ; `jumpCutFactor` ∈ [0, 1] ; multiplicateurs de chute/fast-fall > 1 ; apex ∈ ]0, 1[ ; au moins 1 saut aérien. |
| **PlayerComponentsTest.TailleEtSpawnHumanoide** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_player_components.cpp:108`</sub> | Taille humanoïde et spawn centré | 1. Lire `playerSize()`.<br/>2. Calculer `playerSpawnPosition(3, 5)`. | Taille (0,4 ; 0,8) ; position centrée dans la tuile : (3,3 ; 5,1). |
| **PlayerComponentsTest.AggregationRenseigneLesChamps** (Mineur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_player_components.cpp:126`</sub> | Agrégation des composants | 1. Initialiser `Collider` et `PlayerInput` par accolades.<br/>2. Lire les champs. | Les valeurs fournies sont bien affectées (agrégats). |

**`test_sprite.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **SpriteTest.ValeursParDefaut** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_sprite.cpp:12`</sub> | Un sprite par défaut a une teinte blanche opaque, la couche 0 et une région nulle. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **SpriteTest.ChampsAssignables** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_sprite.cpp:37`</sub> | Les champs sont librement assignables (donnée pure). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **SpriteTest.CopieValeur** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_sprite.cpp:62`</sub> | Le composant est utilisable comme un composant d'ECS (stockage/copie de données pures). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_view.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **ViewTest.SelectionneUniquementLIntersection** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_view.cpp:27`</sub> | Une vue <A, B> itère exactement les entités possédant A et B. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ViewTest.ComposantsCorrespondentALEntite** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_view.cpp:58`</sub> | Les composants fournis par la vue correspondent bien à l'entité itérée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ViewTest.ModificationViaVueEstVisible** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_view.cpp:92`</sub> | La modification d'un composant via la vue est visible ensuite (référence). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ViewTest.VueVideNIterePas** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_view.cpp:118`</sub> | Une vue sans entité correspondante s'itère sans erreur (aucune visite). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ViewTest.IterationForEtEachCoherentes** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_view.cpp:145`</sub> | L'itération par `for` visite les mêmes entités que `each`. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_world.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **WorldTest.CycleComposant** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_world.cpp:53`</sub> | Le cycle add/has/get/remove d'un composant est cohérent via le `World`. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **WorldTest.HasComponentSansPool** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_world.cpp:80`</sub> | `hasComponent` est faux quand aucune pool du type n'existe encore. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **WorldTest.DestroyEntityRetireTousLesComposants** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_world.cpp:96`</sub> | `destroyEntity` retire l'entité de toutes les pools et la rend non vivante. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **WorldTest.SystemesExecutesDansLOrdre** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_world.cpp:119`</sub> | Les systèmes enregistrés s'exécutent dans l'ordre d'enregistrement. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **WorldTest.UpdateNFoisExecuteNFois** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_world.cpp:142`</sub> | `update` appelé N fois exécute chaque système N fois (cadencement déterministe). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **WorldTest.FixedDeltaTransmisAuxSystemes** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_world.cpp:166`</sub> | Le `fixedDelta` passé à `update` est transmis tel quel aux systèmes. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **WorldTest.ViewViaWorld** (Majeur)<br/><sub>`Source/Test/Unit/Core/Ecs/test_world.cpp:186`</sub> | La vue exposée par le `World` itère l'intersection des composants. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

#### Gameplay (5)

**`test_mechanism_controller.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **MechanismControllerTest.ContactOuvreLaPorte** (Majeur)<br/><sub>`Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:53`</sub> | Au départ, la porte est **fermée** (solide) ; toucher l'interrupteur l'**ouvre**. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MechanismControllerTest.BasculeSurFront** (Majeur)<br/><sub>`Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:75`</sub> | La bascule est **sur front** : rester sur l'interrupteur ne re-bascule pas ; revenir bascule. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MechanismControllerTest.SansContactRienNeChange** (Majeur)<br/><sub>`Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:102`</sub> | Loin de l'interrupteur, rien ne change (la porte reste fermée). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MechanismControllerTest.PlaqueDePressionActivationContinue** (Majeur)<br/><sub>`Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:120`</sub> | Une plaque de pression ouvre la porte tant que le poids y repose, et la referme dès qu'il en part. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MechanismControllerTest.PlaqueDePressionPoidsInsuffisant** (Majeur)<br/><sub>`Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp:151`</sub> | Un poids insuffisant sur la plaque de pression n'ouvre pas la porte. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

#### Levels (70)

**`test_level.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **TileMapTest.GrilleNeuveVideAuxBonnesDimensions** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level.cpp:14`</sub> | Une grille neuve a les bonnes dimensions et n'est composée que de cases Empty. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TileMapTest.EcritureLectureDUneTuile** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level.cpp:37`</sub> | Écrire puis lire une tuile restitue le type posé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TileMapTest.Bornes** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level.cpp:55`</sub> | Les bornes de la grille sont correctement détectées. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TileMapTest.Solidite** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level.cpp:75`</sub> | Seules les tuiles solides bloquent statiquement. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TileMapTest.IsSolidParType** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level.cpp:96`</sub> | `isSolid(TileType)` : vrai seulement pour Solid. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelTest.RestitueSesComposantes** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level.cpp:113`</sub> | Un Level restitue ses composantes (nom, grille, entrée/sortie, mécanismes). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_level_draft.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LevelDraftTest.BrouillonVierge** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:23`</sub> | Un brouillon vierge a une grille entièrement vide, sans entrée ni sortie. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.PaintTilePoseLeType** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:46`</sub> | paintTile pose le type demandé sur la case visée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.SetEntryDeplaceLEntreeExistante** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:62`</sub> | Poser une seconde entrée déplace la première (unicité, EX-EDIT-004). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.SetExitDeplaceLaSortieExistante** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:84`</sub> | Poser une seconde sortie déplace la première (unicité, EX-EDIT-004). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.PeindrePardessusLEntreeLInvalide** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:103`</sub> | Peindre par-dessus l'entrée invalide la position d'entrée mémorisée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.LierPuisDelierUnMecanisme** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:121`</sub> | Lier un interrupteur à une porte crée un mécanisme ; le délier le retire. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.LierUnePlaqueDePression** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:146`</sub> | Lier une plaque de pression à une porte crée un mécanisme. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | Lier une plaque de pression à une porte crée un mécanisme, comme un interrupteur. |
| **LevelDraftTest.RelierUnePorteRemplaceLaLiaisonPrecedente** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:167`</sub> | Relier une porte déjà liée remplace la liaison précédente (une porte, un interrupteur). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.UnInterrupteurPeutOuvrirPlusieursPortes** (Mineur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:192`</sub> | Un interrupteur peut ouvrir plusieurs portes. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.PeindrePardessusUnInterrupteurRetireSesLiaisons** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:214`</sub> | Peindre par-dessus un interrupteur retire les liaisons qui le référencent. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.AgrandirConserveLeContenu** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:234`</sub> | Agrandir la grille conserve le contenu existant et complète en cases vides. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.ReduireTronqueEtInvalideLEntreePerdue** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:257`</sub> | Réduire la grille tronque le contenu hors bornes et invalide l'entrée perdue. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.ReduireRetireLesMecanismesHorsBornes** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:281`</sub> | Réduire la grille retire les mécanismes dont une extrémité sort des bornes. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.ToLevelSurBrouillonValideReussit** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:302`</sub> | toLevel() sur un brouillon complet et valide produit un niveau conforme. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.ToLevelSansSortieEchoueProprement** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:326`</sub> | toLevel() sur un brouillon sans sortie échoue avec un message récupérable (EX-EDIT-007). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.FromLevelRestitueLeContenu** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:347`</sub> | Un brouillon reconstruit depuis un niveau existant restitue son contenu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.BrouillonNeufSansHistorique** (Mineur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:369`</sub> | Un brouillon neuf ne peut ni annuler ni refaire. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.UndoApresPeintureRestitueLEtatPrecedent** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:385`</sub> | undo() après une peinture restitue l'état exact précédent. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.RedoApresUndoRestitueLEtatMute** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:408`</sub> | redo() après un undo() restitue l'état muté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.SequenceDeMutationsPuisUndoRestitueLEtatInitial** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:431`</sub> | Une séquence de N mutations suivie de N undo() restitue l'état initial exact. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.MutationApresUndoInvalideLeRefaire** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:463`</sub> | Une nouvelle mutation après un undo() invalide la branche de refaire. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.UndoRedoSurPileVideSansEffet** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:485`</sub> | undo()/redo() sur une pile vide est sans effet (pas de plantage). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.UndoApresLiaisonMecanismeRestitueLAbsenceDeLiaison** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:502`</sub> | L'annulation d'une liaison de mécanisme restitue la liaison précédente. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.PaintRegionAppliqueLeBlocEntier** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:524`</sub> | paintRegion applique un bloc homogène comme une succession de paintTile équivalente. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.PaintRegionUnSeulSnapshotUndo** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:550`</sub> | paintRegion ne pousse qu'un seul snapshot undo pour tout le bloc. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.PaintRegionDecoupeAuxBords** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:574`</sub> | paintRegion découpe silencieusement le bloc aux bords de la grille. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.PaintRegionDeplaceLEntree** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:597`</sub> | paintRegion qui inclut une position d'entrée déplace l'entrée existante. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.PaintRegionRetireLesLiaisonsRecouvertes** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:617`</sub> | paintRegion qui recouvre un interrupteur retire les liaisons qui le référencent. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.PaintRegionBlocVideSansEffet** (Mineur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:640`</sub> | paintRegion avec un bloc vide est sans effet. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.WouldResizeDropContentDetecteLaPerte** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:656`</sub> | wouldResizeDropContent détecte la perte de l'entrée, de la sortie ou d'une liaison. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.WouldResizeDropContentFauxSurBrouillonVierge** (Mineur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:685`</sub> | wouldResizeDropContent est faux sur un brouillon vierge, quelle que soit la taille visee. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelDraftTest.UndoApresRedimensionnementRestitueLesDimensions** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_draft.cpp:702`</sub> | L'annulation d'un redimensionnement restitue les dimensions et le contenu précédents. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_level_loader.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LevelLoaderTest.ChargeUnNiveauValide** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:33`</sub> | Un niveau valide est chargé avec ses dimensions, tuiles, entrée/sortie et mécanismes. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.ChargeUnePlaqueDePression** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:64`</sub> | Une plaque de pression se charge comme un interrupteur. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | Une plaque de pression se charge comme un interrupteur : même règle d'identifiant, même résolution de liaison vers une porte. |
| **LevelLoaderTest.BudgetsOptionnels** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:98`</sub> | Les budgets de mouvements (EX-GP-024) sont chargés s'ils sont présents, illimités (-1) sinon. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.JsonMalformeRejete** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:126`</sub> | Un JSON syntaxiquement invalide est rejeté sans plantage. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.ChampManquantRejete** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:143`</sub> | Un champ obligatoire manquant est rejeté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.TypeDeTuileInconnuRejete** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:160`</sub> | Un type de tuile inconnu est rejeté avec un message. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.TuileHorsBornesRejetee** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:178`</sub> | Une tuile hors des bornes est rejetée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.LiaisonMecanismeNonResolueRejetee** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:195`</sub> | Une porte liée à un interrupteur inexistant est rejetée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.PlusieursEntreesRejetees** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:219`</sub> | Plusieurs entrées sont rejetées (une seule attendue). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.PlusieursSortiesRejetees** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:242`</sub> | Plusieurs sorties sont rejetées (une seule attendue). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.PositionEnDoubleRejetee** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:265`</sub> | Deux tuiles à la même position sont rejetées. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.TilesNonListeRejete** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:289`</sub> | Un champ 'tiles' qui n'est pas une liste est rejeté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.DimensionsNonPositivesRejetees** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:306`</sub> | Des dimensions non positives sont rejetées. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.InterrupteurSansIdRejete** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:323`</sub> | Un interrupteur sans 'id' est rejeté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.IdentifiantInterrupteurEnDoubleRejete** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:346`</sub> | Deux interrupteurs avec le même identifiant sont rejetés. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.NiveauSansEntreeRejete** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:370`</sub> | Un niveau sans entrée est rejeté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.NiveauSansSortieRejete** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:387`</sub> | Un niveau sans sortie est rejeté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.FichierIntrouvableRejete** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:404`</sub> | Charger un fichier inexistant échoue proprement (récupérable). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.PorteSansLiaisonEstValide** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:422`</sub> | Une porte sans 'opensWith' est une simple tuile : chargement valide, aucun mécanisme. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelLoaderTest.NiveauDeDemoLivreValide** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_loader.cpp:446`</sub> | Le niveau de démonstration livré (Source/Elements/Levels) se charge et se valide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_level_outcome.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LevelOutcomeTest.ZoneLibreEstEnCours** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_outcome.cpp:36`</sub> | En zone libre (ni sortie, ni danger, dans les limites) : partie en cours. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelOutcomeTest.SurLaSortieEstGagne** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_outcome.cpp:51`</sub> | La boîte recouvrant la case de sortie : niveau gagné. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelOutcomeTest.SurUnDangerEstPerdu** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_outcome.cpp:66`</sub> | La boîte recouvrant une tuile Danger : niveau perdu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelOutcomeTest.SousLaLimiteBasseEstPerdu** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_outcome.cpp:81`</sub> | La boîte sous la limite basse (chute dans le vide) : niveau perdu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelOutcomeTest.EchecPrioritaireSurSucces** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_outcome.cpp:96`</sub> | Recouvrement simultané sortie + danger : l'échec est prioritaire (déterminisme). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_level_writer.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LevelWriterTest.RoundTripPreserveLeContenu** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_writer.cpp:36`</sub> | Sérialiser puis recharger un niveau produit un niveau équivalent (round-trip). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelWriterTest.PlaqueDePressionSurvitAuRoundTrip** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_writer.cpp:81`</sub> | Une plaque de pression survit au round-trip. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | Une plaque de pression survit au round-trip : `TileType` et liaison préservés. |
| **LevelWriterTest.BudgetsIllimitesOmisDuJson** (Mineur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_writer.cpp:116`</sub> | Les budgets illimités (-1) ne sont pas écrits dans le JSON produit. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelWriterTest.InterrupteurNonRelieRegenereUnIdentifiant** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_writer.cpp:142`</sub> | Un interrupteur non relié à une porte est tout de même sérialisé, avec un identifiant. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelWriterTest.NiveauPuzzleLivreSurvieAuRoundTrip** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_writer.cpp:168`</sub> | Le niveau puzzle livré (demo4.json) survit à un round-trip. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelWriterTest.SaveToFileEcritUnFichierRechargeable** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_writer.cpp:192`</sub> | saveToFile écrit un fichier qui se recharge à l'identique (round-trip disque). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelWriterTest.SaveToFileVersDossierInexistantEchoueProprement** (Majeur)<br/><sub>`Source/Test/Unit/Core/Levels/test_level_writer.cpp:219`</sub> | saveToFile vers un dossier inexistant échoue proprement (récupérable, EX-NFR-040). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

#### Math (20)

**`test_math_utils.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **MathUtilsTest.EgaliteExacte** (Mineur)<br/><sub>`Source/Test/Unit/Core/Math/test_math_utils.cpp:12`</sub> | approximatelyEqual : égalité exacte | 1. Comparer des valeurs identiques (1, 0, -2,5). | `approximatelyEqual` renvoie vrai. |
| **MathUtilsTest.ToleranceParDefaut** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_math_utils.cpp:26`</sub> | approximatelyEqual : tolérance par défaut | 1. Comparer 1 et 1+1e-6.<br/>2. Comparer 1 et 1,1. | Égal sous la tolérance ; différent au-delà. |
| **MathUtilsTest.MiseALEchelleGrandesMagnitudes** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_math_utils.cpp:39`</sub> | approximatelyEqual : mise à l'échelle des grandes magnitudes | 1. Comparer 1e6 et 1e6+1.<br/>2. Comparer 1e6 et 1e6+1000. | Égal pour un petit écart relatif ; différent pour un grand. |
| **MathUtilsTest.SignesOpposes** (Mineur)<br/><sub>`Source/Test/Unit/Core/Math/test_math_utils.cpp:52`</sub> | approximatelyEqual : signes opposés | 1. Comparer 2 et -2. | Non égaux. |
| **MathUtilsTest.ToleranceExplicite** (Mineur)<br/><sub>`Source/Test/Unit/Core/Math/test_math_utils.cpp:64`</sub> | approximatelyEqual : tolérance explicite | 1. Comparer 1 et 1,5 sans tolérance.<br/>2. Recomparer avec une tolérance de 1. | Différent par défaut ; égal avec la tolérance élargie. |

**`test_rect.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **RectTest.Bords** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_rect.cpp:17`</sub> | Les bords exposés découlent de la position et de la taille. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **RectTest.ContientPointInterieur** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_rect.cpp:35`</sub> | Un point strictement intérieur est contenu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **RectTest.ContientBords** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_rect.cpp:50`</sub> | Contenance inclusive haut/gauche, exclusive bas/droit. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **RectTest.NeContientPasExterieur** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_rect.cpp:67`</sub> | Un point extérieur n'est pas contenu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **RectTest.IntersectionRecouvrement** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_rect.cpp:83`</sub> | Deux rectangles qui se recouvrent s'intersectent (relation symétrique). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **RectTest.ContactBordSansIntersection** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_rect.cpp:100`</sub> | Un simple contact par un bord ne compte pas comme intersection. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **RectTest.Disjonction** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_rect.cpp:116`</sub> | Deux rectangles disjoints ne s'intersectent pas. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_vector2.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **Vector2Test.AdditionSoustraction** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_vector2.cpp:16`</sub> | Vector2 : addition et soustraction | 1. Poser deux vecteurs.<br/>2. Calculer a+b et a-b. | Opérations composante à composante correctes. |
| **Vector2Test.EchelleScalaire** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_vector2.cpp:31`</sub> | Vector2 : échelle par un scalaire | 1. Poser un vecteur.<br/>2. Multiplier/diviser par un scalaire, prendre l'opposé. | Chaque composante est mise à l'échelle ; l'opposé inverse les signes. |
| **Vector2Test.OperateursComposes** (Mineur)<br/><sub>`Source/Test/Unit/Core/Math/test_vector2.cpp:47`</sub> | Vector2 : opérateurs composés | 1. Appliquer +=, -=, *=, /= à un vecteur. | Le vecteur est modifié en place, résultats corrects. |
| **Vector2Test.ProduitScalaire** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_vector2.cpp:67`</sub> | Vector2 : produit scalaire | 1. Calculer a·b.<br/>2. Calculer le produit de deux vecteurs orthogonaux. | Somme des produits (11) ; produit nul pour des vecteurs orthogonaux. |
| **Vector2Test.Longueur** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_vector2.cpp:83`</sub> | Vector2 : longueur | 1. Calculer `lengthSquared` et `length` de (3, 4). | 25 et 5 (théorème de Pythagore). |
| **Vector2Test.NormalisationNonNulle** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_vector2.cpp:97`</sub> | Vector2 : normalisation (non nul) | 1. Normaliser (3, 4). | Longueur 1 ; vecteur (0,6 ; 0,8). |
| **Vector2Test.NormalisationVecteurNul** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_vector2.cpp:111`</sub> | Vector2 : normalisation (vecteur nul) | 1. Normaliser le vecteur nul. | Le vecteur nul (pas de division par zéro). |
| **Vector2Test.EgaliteApprochee** (Majeur)<br/><sub>`Source/Test/Unit/Core/Math/test_vector2.cpp:123`</sub> | Vector2 : égalité approchée | 1. Comparer (0,1+0,2 ; 1) à (0,3 ; 1).<br/>2. Comparer deux vecteurs distincts. | Égal malgré l'erreur flottante ; différent pour des vecteurs distincts. |

#### Physics (8)

**`test_swept_collision.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **SweptCollisionTest.TrajetLibre** (Majeur)<br/><sub>`Source/Test/Unit/Core/Physics/test_swept_collision.cpp:30`</sub> | Balayage : trajet libre | 1. Grille sans tuile solide.<br/>2. Balayer une boîte 1×1 d'un déplacement (3, 2). | Aucun contact ; la boîte atteint exactement la position visée. |
| **SweptCollisionTest.ButeeHorizontale** (Bloquant)<br/><sub>`Source/Test/Unit/Core/Physics/test_swept_collision.cpp:50`</sub> | Balayage : butée horizontale (mur à droite) | 1. Placer un mur solide à droite.<br/>2. Balayer la boîte vers la droite. | La boîte s'arrête au ras du mur (bord droit = bord du mur) ; normale horizontale. |
| **SweptCollisionTest.ButeeVerticale** (Bloquant)<br/><sub>`Source/Test/Unit/Core/Physics/test_swept_collision.cpp:70`</sub> | Balayage : butée verticale (sol) | 1. Placer un sol solide en dessous.<br/>2. Balayer la boîte vers le bas. | La boîte se pose sur le sol (bord bas = haut du sol) ; normale vers le haut. |
| **SweptCollisionTest.NonTunneling** (Bloquant)<br/><sub>`Source/Test/Unit/Core/Physics/test_swept_collision.cpp:90`</sub> | Balayage : non-tunneling à vitesse élevée | 1. Mur à 5 tuiles.<br/>2. Balayer d'un déplacement de 10 (≫ une tuile) vers le mur. | La boîte s'arrête au ras du mur ; elle ne le traverse pas (garantie continue). |
| **SweptCollisionTest.GlissementLeLongDuMur** (Bloquant)<br/><sub>`Source/Test/Unit/Core/Physics/test_swept_collision.cpp:110`</sub> | Balayage : glissement le long d'un mur | 1. Mur vertical à droite.<br/>2. Balayer d'un déplacement diagonal (droite + bas). | Bloquée en X (au ras du mur), mais la descente complète en Y a lieu (glissement). |
| **SweptCollisionTest.ButeeGauche** (Majeur)<br/><sub>`Source/Test/Unit/Core/Physics/test_swept_collision.cpp:132`</sub> | Balayage : butée horizontale (mur à gauche) | 1. Mur solide à gauche.<br/>2. Balayer la boîte vers la gauche. | La boîte s'arrête au ras du mur (bord gauche = bord droit du mur) ; normale opposée. |
| **SweptCollisionTest.ButeePlafond** (Majeur)<br/><sub>`Source/Test/Unit/Core/Physics/test_swept_collision.cpp:151`</sub> | Balayage : butée verticale (plafond) | 1. Plafond solide au-dessus.<br/>2. Balayer la boîte vers le haut. | La boîte s'arrête sous le plafond (bord haut = bas du plafond) ; normale vers le bas. |
| **SweptCollisionTest.MarcheSurLeSolSansBlocageHorizontal** (Majeur)<br/><sub>`Source/Test/Unit/Core/Physics/test_swept_collision.cpp:171`</sub> | Balayage : marcher sur un sol sans blocage horizontal | 1. Sol continu ; boîte posée dessus.<br/>2. Balayer horizontalement le long du sol. | Aucun blocage : la « peau » perpendiculaire évite de confondre marcher sur / buter contre. |

#### Time (6)

**`test_fixed_timestep.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **FixedTimestepTest.UnPasExact** (Majeur)<br/><sub>`Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:16`</sub> | Un temps écoulé égal au pas fixe produit exactement un pas. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **FixedTimestepTest.TempsInsuffisant** (Majeur)<br/><sub>`Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:31`</sub> | Un temps écoulé inférieur au pas ne produit aucun pas. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **FixedTimestepTest.TempsNulOuNegatif** (Majeur)<br/><sub>`Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:46`</sub> | Un temps écoulé nul ou négatif ne produit aucun pas. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **FixedTimestepTest.ResteConserve** (Majeur)<br/><sub>`Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:62`</sub> | 2,5 pas donnent 2 pas, et le reste (0,5 pas) est conservé puis complété. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **FixedTimestepTest.PlafondAntiSpirale** (Majeur)<br/><sub>`Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:82`</sub> | Un temps écoulé énorme est plafonné (anti-spirale de la mort). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **FixedTimestepTest.PasFixeExpose** (Majeur)<br/><sub>`Source/Test/Unit/Core/Time/test_fixed_timestep.cpp:100`</sub> | Le pas fixe exposé correspond à la configuration. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

### HMI

#### Editor (38)

**`test_level_name_validation.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LevelNameValidationTest.NomSimpleValide** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:12`</sub> | Un nom simple, sans caractère interdit, est valide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelNameValidationTest.NomVideOuEspacesInvalide** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:27`</sub> | Un nom vide ou composé uniquement d'espaces est invalide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelNameValidationTest.CaractereInterditInvalide** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:42`</sub> | Un nom contenant un caractère interdit par le système de fichiers Windows est invalide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelNameValidationTest.NomAccentueValide** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:67`</sub> | Un nom accentué (Unicode) reste valide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelNameValidationTest.TrimRetireLesEspacesDeBord** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_name_validation.cpp:81`</sub> | trimLevelName retire les espaces de bord sans toucher au contenu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_level_picker.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LevelPickerTest.SelectionInitialeEstLePremierChoix** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_picker.cpp:34`</sub> | La sélection initiale pointe le premier choix (« Nouveau niveau »). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelPickerTest.FlechesDeplacentLaSelectionAvecBouclage** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_picker.cpp:50`</sub> | Bas puis Haut déplacent la sélection, avec bouclage aux extrémités. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelPickerTest.EntreeConfirmeLaSelection** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_picker.cpp:84`</sub> | Entrée confirme l'indice actuellement sélectionné. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelPickerTest.SansAppuiAucuneConfirmation** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_picker.cpp:113`</sub> | Sans appui, update() ne confirme rien. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelPickerTest.SurvolSourisDeplaceLaSelection** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_picker.cpp:131`</sub> | Survoler un choix à la souris déplace la sélection dessus (sans confirmer). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelPickerTest.ClicGaucheConfirmeLeChoixSurvole** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_picker.cpp:153`</sub> | Un clic gauche sur un choix survolé le confirme. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelPickerTest.SourisHorsChoixNeChangeRien** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_picker.cpp:177`</sub> | La souris hors de tout choix ne change ni la sélection ni la confirmation. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelPickerTest.ForDirectoryDossierInexistantNeProposeQueNouveauNiveau** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_picker.cpp:200`</sub> | forDirectory sur un dossier inexistant ne propose que « Nouveau niveau ». | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_level_size_validation.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LevelSizeValidationTest.FormatValideAnalyseCorrectement** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:12`</sub> | Un format « largeur x hauteur » valide est analysé correctement. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelSizeValidationTest.EspacesEtCasseTolerees** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:29`</sub> | Les espaces autour du séparateur et une casse différente sont tolérés. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelSizeValidationTest.SeparateurEtoileAccepte** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:47`</sub> | Le séparateur `*` est accepté comme alternative à `x`/`X`. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelSizeValidationTest.SansSeparateurRefuse** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:62`</sub> | Un texte sans séparateur est refusé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelSizeValidationTest.DimensionInvalideRefusee** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:77`</sub> | Une dimension non numérique, nulle, négative ou partiellement numérique est refusée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelSizeValidationTest.PlafondRespecte** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:96`</sub> | Une dimension au-delà du plafond est refusée ; le plafond lui-même est accepté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelSizeValidationTest.IsValidCoherentAvecParse** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp:116`</sub> | isValidLevelSize reste cohérent avec parseLevelSize. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_text_input_field.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **TextInputFieldTest.CaracteresTapesSAccumulent** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:13`</sub> | Les caractères tapés s'accumulent dans le texte du champ, dans l'ordre de saisie. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TextInputFieldTest.RetourArriereRetireLeDernierCaractere** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:38`</sub> | Retour arrière retire le dernier caractère du texte pré-rempli. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TextInputFieldTest.RetourArriereRetireUnCaractereAccentueEntier** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:59`</sub> | Retour arrière retire un caractère accentué entier (UTF-8 multi-octets), pas un octet. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TextInputFieldTest.EntreeConfirmeSansValidateur** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:87`</sub> | Sans validateur, Entrée confirme toujours la saisie courante. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TextInputFieldTest.EntreeRefuseeParLeValidateur** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:109`</sub> | Un validateur qui refuse le texte empêche la confirmation et marque un refus. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TextInputFieldTest.EditionApresRefusEffaceLeRefus** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:132`</sub> | Modifier le texte après un refus efface l'indicateur de refus. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TextInputFieldTest.EchapAnnule** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:159`</sub> | Échap annule la saisie sans modifier le texte du champ. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TextInputFieldTest.SansEffetApresConfirmation** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_text_input_field.cpp:181`</sub> | Une fois confirmé, update() n'a plus aucun effet sur le champ. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_tile_palette.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **TilePaletteTest.ExposeUneEntreeParType** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:13`</sub> | La palette expose au moins une entrée par type de tuile éditable. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TilePaletteTest.SelectionParDefautEstSolid** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:28`</sub> | La sélection par défaut est Solid. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TilePaletteTest.ClicSurUneEntreeSelectionneSonType** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:43`</sub> | Cliquer une entrée de la palette sélectionne son type et signale le clic consommé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TilePaletteTest.ClicHorsPaletteNonConsomme** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:66`</sub> | Cliquer hors de la palette ne change pas la sélection et n'est pas consommé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **TilePaletteTest.ChaqueEntreePorteUnLibelle** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tile_palette.cpp:87`</sub> | Chaque entrée porte un libellé non vide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_tool_bar.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **ToolBarTest.SelectionParDefautEstPinceau** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:13`</sub> | La sélection par défaut est l'outil Pinceau. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ToolBarTest.ExposeTroisEntrees** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:28`</sub> | La barre expose une entrée pour chacun des trois outils. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ToolBarTest.ClicSurUneEntreeSelectionneSonOutil** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:43`</sub> | Cliquer une entrée de la barre sélectionne son outil et signale le clic consommé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ToolBarTest.ClicHorsBarreNonConsomme** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:66`</sub> | Cliquer hors de la barre ne change pas la sélection et n'est pas consommé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ToolBarTest.SelectNextDefileEnBoucle** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Editor/test_tool_bar.cpp:87`</sub> | selectNext() fait défiler les outils dans l'ordre attendu, en boucle. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

#### Graphics (9)

**`test_camera2d.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **Camera2DTest.CentreAuMilieuDeLEcran** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:21`</sub> | Le centre de la caméra se projette au centre de l'écran. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **Camera2DTest.EchelleEtAxeY** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:40`</sub> | Une unité monde vaut 16 pixels ; l'axe Y va vers le bas. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **Camera2DTest.Zoom** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:62`</sub> | Le zoom multiplie l'échelle en pixels. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **Camera2DTest.ConversionsReciproques** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:80`</sub> | `screenToWorld` est la réciproque de `worldToScreen`. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **Camera2DTest.ProjectionCentreVersOrigineClip** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:101`</sub> | La matrice de projection envoie le centre de la caméra à l'origine du clip space. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **Camera2DTest.BordEcranVersBordClip** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:128`</sub> | Un coin de l'écran correspond à un bord du clip space (±1). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **Camera2DTest.FitZoomEntierPourPetitNiveau** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:152`</sub> | fitZoom reste entier tant que le facteur brut est supérieur ou égal à 1 (petit niveau). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **Camera2DTest.FitZoomFractionnairePourGrandNiveau** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:171`</sub> | fitZoom devient fractionnaire pour un niveau plus grand que la surface disponible. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **Camera2DTest.FitZoomAppliqueLaMarge** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Graphics/test_camera2d.cpp:192`</sub> | fitZoom applique la marge avant l'arrondi. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

#### Input (24)

**`test_input_state.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **InputStateTest.FrontMontantClavier** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:12`</sub> | Une touche passée d'« absente » à « présente » est « pressée » exactement une frame. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.MaintienClavier** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:38`</sub> | Une touche restée enfoncée n'est « pressée » qu'à la première frame. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.FrontDescendantClavier** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:63`</sub> | Le relâchement d'une touche est détecté « relâchée » pendant exactement une frame. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.TouchesIndependantes** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:90`</sub> | Les touches sont indépendantes : un front sur l'une n'affecte pas les autres. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.BoutonSouris** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:111`</sub> | Un bouton de souris suit la même logique pressé/cliqué/relâché que les touches. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.PositionSouris** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:140`</sub> | La position de la souris reflète le dernier déplacement injecté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.MoletteAccumuleEtSeReinitialise** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:162`</sub> | Les incréments de molette d'une frame s'additionnent et repartent de zéro ensuite. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.CaracteresTapesAccumulesEtVides** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:186`</sub> | Les caractères tapés s'accumulent dans l'ordre puis sont vidés à la frame suivante. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.ManetteSeuleActiveLaTouche** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:215`</sub> | Un bouton manette seul rend `keyDown`/`keyPressed` vrais, comme au clavier. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.ClavierEtManetteMemeToucheUnSeulFront** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:243`</sub> | Clavier et manette combinés sur la même touche ne produisent pas de double front. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.ManetteRelacheeNeMasquePasLeClavier** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:280`</sub> | La manette relâchée ne masque jamais une touche clavier réellement maintenue. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **InputStateTest.GamepadConnecteReecrasable** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Input/test_input_state.cpp:308`</sub> | `gamepadConnected` reflète le dernier `setGamepadConnected` appelé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_player_input_mapper.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **PlayerInputMapperTest.FlecheGauche** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:27`</sub> | Flèche gauche seule → intention vers la gauche (-1). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.FlecheDroite** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:41`</sub> | Flèche droite seule → intention vers la droite (+1). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.TouchesAlternativesQetD** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:55`</sub> | Touches alternatives ZQSD : Q → gauche, D → droite. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.AucuneTouche** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:70`</sub> | Aucune touche → intention nulle (immobile). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.GaucheEtDroiteSeNeutralisent** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:84`</sub> | Gauche et droite simultanées → neutralisation (0). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.EspacePresseeDeclencheLeSaut** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:98`</sub> | Espace fraîchement enfoncée → saut **pressé** (front) et **maintenu**. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.WEquivautEspacePourLeSaut** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:114`</sub> | `W` équivaut à Espace pour le saut. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.SautMaintenuN_estPasUnFront** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:128`</sub> | Saut **maintenu** sans nouveau front → `jumpHeld` vrai mais `jumpPressed` faux. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.PasDeSaut** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:150`</sub> | Aucune touche de saut → ni pressé ni maintenu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.DeplacementEtSautIndependants** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:166`</sub> | Déplacement et saut sont indépendants (axes distincts). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.MajDeclencheLeDash** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:183`</sub> | Maj fraîchement enfoncée → dash **pressé** (front). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PlayerInputMapperTest.ViseeVerticale** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp:198`</sub> | Visée verticale (y vers le bas) : Bas → +1, Haut → -1, les deux → 0. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

#### Interface (34)

**`test_language_selector.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LanguageSelectorTest.RectangleAncreBasDroite** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_language_selector.cpp:29`</sub> | Le bouton est ancré au coin bas-droit, marge comprise. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LanguageSelectorTest.BasculeAlterneLesLangues** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_language_selector.cpp:51`</sub> | La bascule renvoie l'autre langue (français ↔ anglais). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LanguageSelectorTest.ClicDansLeBoutonBascule** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_language_selector.cpp:67`</sub> | Un clic dans le bouton demande la bascule vers l'autre langue. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LanguageSelectorTest.ClicHorsBoutonNeBasculePas** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_language_selector.cpp:90`</sub> | Un clic hors du bouton ne demande aucune bascule. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LanguageSelectorTest.SansClicPasDeBascule** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_language_selector.cpp:108`</sub> | Sans clic, aucune bascule même si la souris est sur le bouton. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_level_sequence.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LevelSequenceTest.SequenceVide** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_level_sequence.cpp:12`</sub> | Une séquence vide est signalée comme telle. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelSequenceTest.DemarreSurLePremier** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_level_sequence.cpp:28`</sub> | La séquence démarre sur le premier niveau, dans l'ordre fourni. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelSequenceTest.AvanceDansLOrdre** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_level_sequence.cpp:47`</sub> | `advance` parcourt les niveaux dans l'ordre jusqu'au dernier. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LevelSequenceTest.NeDepassePasLeDernier** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_level_sequence.cpp:70`</sub> | Au-delà du dernier niveau, `advance` est sans effet (pas de dépassement). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_menu_model.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **MenuModelTest.SelectionParDefaut** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_menu_model.cpp:63`</sub> | À l'ouverture, la première option (Charger niveau) est sélectionnée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MenuModelTest.ValiderChargeNiveau** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_menu_model.cpp:82`</sub> | Valider par défaut (Entrée) bascule vers l'écran de jeu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MenuModelTest.FlecheBasPuisValider** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_menu_model.cpp:101`</sub> | La flèche bas déplace la sélection ; valider mène alors au Mode Edition. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MenuModelTest.FlecheHautBoucleSurQuitter** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_menu_model.cpp:124`</sub> | La flèche haut depuis la première option boucle sur la dernière (Quitter). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MenuModelTest.OptionOptionsBasculeVersEcranOptions** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_menu_model.cpp:145`</sub> | La troisième option (Options) bascule vers l'écran de réglages. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MenuModelTest.BouclageBasComplet** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_menu_model.cpp:168`</sub> | Quatre flèches bas ramènent à la première option (bouclage déterministe). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MenuModelTest.SurvolSourisSelectionne** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_menu_model.cpp:192`</sub> | Le survol d'une option à la souris la sélectionne. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MenuModelTest.ClicValideOption** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_menu_model.cpp:211`</sub> | Un clic gauche sur une option la valide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **MenuModelTest.ClicHorsOptionNeValidePas** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_menu_model.cpp:231`</sub> | Un clic hors de toute option ne valide rien. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_options_model.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **OptionsModelTest.SelectionParDefautEtLibelleVSync** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_options_model.cpp:53`</sub> | À l'ouverture, la première option (V-Sync) est sélectionnée et affiche l'état fourni. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **OptionsModelTest.ValiderVSyncRenvoieToggle** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_options_model.cpp:73`</sub> | Valider la première option (V-Sync) renvoie l'action ToggleVSync. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **OptionsModelTest.FlecheBasPuisValiderRenvoieBack** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_options_model.cpp:92`</sub> | Flèche bas puis valider (Retour) renvoie l'action Back. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **OptionsModelTest.FlecheHautBoucleSurRetour** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_options_model.cpp:115`</sub> | La flèche haut depuis la première option boucle sur la dernière (Retour). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **OptionsModelTest.ClicSourisValideRetour** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_options_model.cpp:133`</sub> | Le survol d'une option à la souris la sélectionne ; un clic la valide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **OptionsModelTest.SetVSyncEnabledChangeLeLibelle** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_options_model.cpp:154`</sub> | `setVSyncEnabled` resynchronise le libellé affiché après une bascule. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **OptionsModelTest.ClicHorsOptionNeValidePas** (Mineur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_options_model.cpp:173`</sub> | Un clic hors de toute option ne valide rien. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_save_log_button.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **SaveLogButtonTest.AGaucheDuBoutonLangue** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_save_log_button.cpp:30`</sub> | Le bouton est à gauche du bouton de langue, aligné sur le même bord bas. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **SaveLogButtonTest.ClicDansLeBouton** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_save_log_button.cpp:50`</sub> | Un clic dans le bouton est détecté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **SaveLogButtonTest.ClicHorsBouton** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_save_log_button.cpp:69`</sub> | Un clic hors du bouton n'est pas détecté. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_screen_manager.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **ScreenManagerTest.ConstructionCreeEcranInitial** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_screen_manager.cpp:49`</sub> | La construction fabrique l'écran initial demandé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ScreenManagerTest.TransitionNoneResteSurEcran** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_screen_manager.cpp:68`</sub> | Une transition « rester » conserve l'écran actif et n'en fabrique pas d'autre. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ScreenManagerTest.TransitionSwitchRemplaceEcran** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_screen_manager.cpp:91`</sub> | Une transition « basculer » remplace l'écran actif par celui demandé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **ScreenManagerTest.TransitionQuitFermeApplication** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_screen_manager.cpp:116`</sub> | Une transition « quitter » ferme l'application et se propage à la boucle. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

**`test_session_log.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **SessionLogTest.SerialiseUneLigneParMessage** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_session_log.cpp:15`</sub> | Chaque message donne une ligne, dans l'ordre d'arrivée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **SessionLogTest.VideDonneChaineVide** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Interface/test_session_log.cpp:34`</sub> | Une session sans message produit un texte vide. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

#### Localization (8)

**`test_localization.cpp`**

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **LocalizationTest.AnalyseIgnoreCommentairesEtEspaces** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Localization/test_localization.cpp:17`</sub> | L'analyse ignore les lignes vides et les commentaires, et retire les espaces autour de '='. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LocalizationTest.AnalyseConserveEgalDansLaValeur** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Localization/test_localization.cpp:42`</sub> | Seul le premier '=' sépare ; un '=' dans la valeur est conservé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LocalizationTest.CleExistanteResolue** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Localization/test_localization.cpp:59`</sub> | Une clé existante est résolue dans la langue active. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LocalizationTest.CleInconnueRenvoieLaCle** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Localization/test_localization.cpp:77`</sub> | Une clé inconnue partout est renvoyée telle quelle (repli déterministe, pas de plantage). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LocalizationTest.ChangementDeLangue** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Localization/test_localization.cpp:96`</sub> | Changer de langue résout les valeurs de la nouvelle langue. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LocalizationTest.RepliSurLangueParDefaut** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Localization/test_localization.cpp:115`</sub> | Une clé manquante dans la langue active retombe sur la langue par défaut. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LocalizationTest.LangueAbsenteEstRecuperable** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Localization/test_localization.cpp:134`</sub> | Charger une langue absente échoue proprement et conserve la langue active (récupérable). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **LocalizationTest.CatalogueFrancaisLivreSeCharge** (Majeur)<br/><sub>`Source/Test/Unit/HMI/Localization/test_localization.cpp:155`</sub> | Le catalogue français livré (Source/Elements/Localization) se charge et résout ses clés. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

## Tests d'intégration (48)

### Animation Personnage — `test_animation_personnage.cpp` (5)

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **AnimationPersonnageIntegration.ImmobileAuSolEstEnRepos** (Majeur)<br/><sub>`Source/Test/Integration/test_animation_personnage.cpp:36`</sub> | Au sol et immobile, le clip est Idle et l'image alterne 0/1 après chaque durée d'image. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **AnimationPersonnageIntegration.EnMouvementAuSolCourt** (Majeur)<br/><sub>`Source/Test/Integration/test_animation_personnage.cpp:67`</sub> | Au sol et en mouvement, le clip est Run et l'image boucle sur les 4 images dans l'ordre. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **AnimationPersonnageIntegration.EnLAirEstEnSaut** (Majeur)<br/><sub>`Source/Test/Integration/test_animation_personnage.cpp:100`</sub> | En l'air, le clip est Jump et l'image reste figée sur 0, quelle que soit la durée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **AnimationPersonnageIntegration.ChangementDeClipReinitialiseLImage** (Majeur)<br/><sub>`Source/Test/Integration/test_animation_personnage.cpp:125`</sub> | Un changement de clip réinitialise immédiatement l'image et le chronomètre à zéro. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **AnimationPersonnageIntegration.EntiteSansAnimationIgnoree** (Mineur)<br/><sub>`Source/Test/Integration/test_animation_personnage.cpp:166`</sub> | Une entité sans composant Animation n'est pas affectée par le système. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

### Boucle Simulation — `test_boucle_simulation.cpp` (2)

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **BoucleSimulationIntegration.CadenceurPiloteLaSimulation** (Majeur)<br/><sub>`Source/Test/Integration/test_boucle_simulation.cpp:50`</sub> | Le cadenceur découpe un temps réel variable en pas fixes, et la position finale correspond exactement au nombre de pas réellement exécutés (aucune dérive). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **BoucleSimulationIntegration.MemeEntreeMemeResultat** (Majeur)<br/><sub>`Source/Test/Integration/test_boucle_simulation.cpp:76`</sub> | Déterminisme : même séquence de frames et même état initial -> même résultat. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

### Ecs Mouvement — `test_ecs_mouvement.cpp` (4)

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **EcsMouvementIntegration.EntiteMobileAvanceDeVitesseFoisPas** (Majeur)<br/><sub>`Source/Test/Integration/test_ecs_mouvement.cpp:25`</sub> | Une entité Transform + Velocity avance de `velocity * fixedDelta` par pas ; après N pas, la position attendue est déterministe. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **EcsMouvementIntegration.EntiteSansVelociteNeBougePas** (Majeur)<br/><sub>`Source/Test/Integration/test_ecs_mouvement.cpp:55`</sub> | Une entité sans Velocity ne bouge pas. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **EcsMouvementIntegration.DeuxEntitesEvoluentIndependamment** (Majeur)<br/><sub>`Source/Test/Integration/test_ecs_mouvement.cpp:81`</sub> | Deux entités de vitesses différentes évoluent indépendamment. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **EcsMouvementIntegration.IntegrationViaWorldUpdate** (Majeur)<br/><sub>`Source/Test/Integration/test_ecs_mouvement.cpp:114`</sub> | Enregistré dans le World, le système s'exécute via World::update (chaîne complète). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

### Niveau Ecs — `test_niveau_ecs.cpp` (2)

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **NiveauEcsIntegration.DuJsonAuxEntites** (Majeur)<br/><sub>`Source/Test/Integration/test_niveau_ecs.cpp:49`</sub> | Le niveau chargé peuple le monde d'une entité par tuile non vide, bien placée et typée. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **NiveauEcsIntegration.FichierDemoVersMonde** (Majeur)<br/><sub>`Source/Test/Integration/test_niveau_ecs.cpp:85`</sub> | Le niveau de démonstration livré, chargé depuis le fichier, peuple bien le monde. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

### Physique Personnage — `test_physique_personnage.cpp` (35)

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **PhysiquePersonnageIntegration.TombeSousGraviteVitesseCroissante** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:269`</sub> | Sans sol, le personnage tombe et sa vitesse verticale croît (gravité continue). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.ChuteConvergeVersUneVitesseTerminale** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:301`</sub> | En chute prolongée, la vitesse verticale converge vers une vitesse terminale sans la dépasser. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.AccelerationDeChuteDecroitVersLeRegimePermanent** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:336`</sub> | L'accélération verticale décroît à mesure que la vitesse approche le régime permanent. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.MasseSuperieureTombePlusVite** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:376`</sub> | Une masse plus grande produit une vitesse terminale plus élevée, à traînée égale. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.AtterritSurLeSolEtEstAuSol** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:405`</sub> | Le personnage se pose sur le sol : vitesse verticale annulée, état « au sol » vrai. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.BloqueParUnMurADroite** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:435`</sub> | Poussé contre un mur, le personnage s'arrête au ras du mur (blocage horizontal). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.AvanceAVitesseConstante** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:464`</sub> | Sur terrain libre, l'avancée horizontale vaut vitesse × temps (vitesse constante). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.NeTraversePasLeSolEnChuteRapide** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:495`</sub> | Même en chute très rapide (grand pas), le personnage ne traverse pas le sol (balayage continu). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.Deterministe** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:527`</sub> | Mêmes entrées → même résultat : la simulation est déterministe (pas fixe). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.SauteDepuisLeSol** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:557`</sub> | Au sol, une pression de saut fait décoller le personnage (vitesse ascendante, il s'élève). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.PasDeSautEnLAir** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:597`</sub> | En l'air, une pression de saut n'a aucun effet : pas de double saut (`EX-GP-013`). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.HauteurDeSautVariable** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:626`</sub> | Hauteur de saut variable : maintenir le bouton saute plus haut que le relâcher tôt. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.DoubleSautNombreParametrable** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:646`</sub> | Double saut : 1 saut au sol + N sauts aériens (paramétrable), rechargés au contact du sol. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.ChutePlusRapideQueLaMontee** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:664`</sub> | Gravité asymétrique : la chute accélère plus vite que la montée ne décélère (EX-GP-018). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.ApexHangReduitLaGravite** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:713`</sub> | Apex hang : près du sommet du saut, la gravité est réduite (contrôle flottant). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.FastFallAccelereLaChute** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:760`</sub> | Fast-fall : maintenir « bas » en l'air fait tomber plus loin qu'une chute libre normale. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.DashHorizontalRapide** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:788`</sub> | Dash horizontal : une ruée rapide (≫ vitesse normale) fait parcourir une grande distance. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.DashDiagonalHautDroite** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:825`</sub> | Dash diagonal (8 directions) : viser haut-droite envoie en +X et −Y. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.GraviteSuspenduePendantDash** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:856`</sub> | Pendant un dash horizontal, la gravité est suspendue (la composante verticale reste nulle). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.DashUneSeuleFoisEnLAir** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:887`</sub> | Une seule ruée par phase aérienne : le second dash en l'air est refusé jusqu'au retour au sol. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.DashNeTraversePasLeMur** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:942`</sub> | Un dash vers un mur ne le traverse pas (résolu par le balayage). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.BudgetDeSautsRefuseAuDela** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:970`</sub> | Budget de sauts (EX-GP-024) : avec 1 saut, le premier fonctionne, le suivant est refusé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.BudgetDeDashsRefuseAuDela** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1007`</sub> | Budget de dashs (EX-GP-024) : avec 1 dash, le premier fonctionne, le suivant est refusé. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.WallSlideRalentitLaChute** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1044`</sub> | Wall slide : collé à un mur en l'air, la vitesse de chute est plafonnée (descente ralentie). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.WallJumpEjecteAlOpposeDuMur** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1076`</sub> | Wall jump : contre un mur à droite, un saut éjecte vers la gauche et le haut. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.PasDeWallJumpSansMur** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1112`</sub> | Sans mur, la logique de wall jump ne s'active pas (et sans saut aérien, aucun saut en l'air). | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.Niveau2FranchissableAvecSaut** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1143`</sub> | Le niveau 2 est franchissable **avec le saut** : en avançant et sautant, on atteint la sortie. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.Niveau2RequiertLeSaut** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1166`</sub> | Le niveau 2 **exige** le saut : en avançant seulement (sans sauter), la marche bloque. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.Niveau3FranchissableAvecDash** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1188`</sub> | Le niveau 3 (couloir bas + fosse) est franchissable **au dash** : avancer + dasher franchit. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.Niveau3RequiertLeDash** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1210`</sub> | Le niveau 3 **exige** le dash : en avançant seulement, on tombe dans la fosse de danger. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.Niveau4FranchissableAvecInterrupteur** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1232`</sub> | Niveau 4 (puzzle) : toucher l'interrupteur ouvre la porte → la sortie devient atteignable. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.PorteFermeeBloque** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1253`</sub> | Une porte **fermée** (interrupteur non touché) **bloque** : la sortie reste inatteignable. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.CoyoteTimeAutoriseUnSautJusteApresLeBord** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1286`</sub> | Coyote time : sauter juste après avoir quitté un bord fonctionne ; trop tard, non. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.JumpBufferingHonoreUnSautPreAppuye** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1303`</sub> | Jump buffering : un saut pré-appuyé peu avant l'atterrissage s'exécute à la pose ; trop tôt, non. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |
| **PhysiquePersonnageIntegration.NiveauDemoEstFranchissableEnAllantADroite** (Majeur)<br/><sub>`Source/Test/Integration/test_physique_personnage.cpp:1321`</sub> | Le niveau de démonstration livré est **franchissable** sans saut : en maintenant « droite », le personnage descend l'escalier de paliers et atteint la sortie sans jamais mourir. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

## Tests système (2)

### Parcours Complet — `test_parcours_complet.cpp` (1)

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **ParcoursCompletSysteme.FranchitTouteLaSequence** (Majeur)<br/><sub>`Source/Test/Systeme/test_parcours_complet.cpp:91`</sub> | Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, puis « retour au titre ». Reproduit la boucle titre → niveau 1 → niveau 2 → titre du jeu. | 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les assertions. | *(= brief)* |

### Éditeur de niveaux — `test_parcours_edition.cpp` (1)

| Titre (criticité) | Brief | Étapes | Résultat attendu |
|---|---|---|---|
| **ParcoursEditionSysteme.CreerEditerEnregistrerRechargerEtJouer** (Critique)<br/><sub>`Source/Test/Systeme/test_parcours_edition.cpp:26`</sub> | Parcours complet d'édition : peindre, lier un mécanisme, annuler une erreur, redimensionner, enregistrer, recharger, et confirmer que le niveau produit est directement jouable. | 1. Peindre un niveau (entrée, sortie, interrupteur/porte liés) dans un `LevelDraft`.<br/>2. Peindre une tuile par erreur puis l'annuler (`undo`).<br/>3. Redimensionner la grille.<br/>4. Valider, enregistrer sur disque, recharger.<br/>5. Vérifier le contenu rechargé et l'issue (`evaluateOutcome`) à l'entrée et à la sortie. | Le niveau rechargé restitue exactement le contenu édité (dont l'annulation) et se comporte comme un niveau livré normalement : `Playing` à l'entrée, `Won` à la sortie. |

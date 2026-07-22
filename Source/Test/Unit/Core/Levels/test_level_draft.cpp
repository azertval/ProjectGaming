/**
 * @file test_level_draft.cpp
 * @brief Tests unitaires du modèle d'édition mutable (LOT-14, EX-EDIT-002 à EX-EDIT-007).
 */

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/TileType.h"

namespace {

using core::GridPosition;
using core::LevelDraft;
using core::TileType;

}  // namespace

/**
 * @brief Un brouillon vierge a une grille entièrement vide, sans entrée ni sortie.
 * \castest{<b>Un brouillon vierge a une grille entièrement vide, sans entrée ni sortie.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un brouillon vierge a une grille entièrement vide, sans entrée ni sortie.
 * }
 */
TEST(LevelDraftTest, BrouillonVierge) {
    const LevelDraft draft = LevelDraft::empty("Nouveau", 5, 4);
    EXPECT_EQ(draft.name(), "Nouveau");
    EXPECT_EQ(draft.tileMap().width(), 5);
    EXPECT_EQ(draft.tileMap().height(), 4);
    EXPECT_FALSE(draft.entry().has_value());
    EXPECT_FALSE(draft.exit().has_value());
    EXPECT_TRUE(draft.mechanisms().empty());
    EXPECT_EQ(draft.jumpBudget(), -1);
    EXPECT_EQ(draft.dashBudget(), -1);
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
}

/**
 * @brief paintTile pose le type demandé sur la case visée.
 * \castest{<b>paintTile pose le type demandé sur la case visée.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintTile pose le type demandé sur la case visée.
 * }
 */
TEST(LevelDraftTest, PaintTilePoseLeType) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintTile(1, 1, TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
}

/**
 * @brief Poser une seconde entrée déplace la première (unicité, EX-EDIT-004).
 * \castest{<b>Poser une seconde entrée déplace la première (unicité, EX-EDIT-004).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Poser une seconde entrée déplace la première (unicité, EX-EDIT-004).
 * }
 */
TEST(LevelDraftTest, SetEntryDeplaceLEntreeExistante) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.setEntry(0, 0);
    ASSERT_TRUE(draft.entry().has_value());
    EXPECT_EQ(*draft.entry(), (GridPosition{0, 0}));

    draft.setEntry(2, 2);
    EXPECT_EQ(*draft.entry(), (GridPosition{2, 2}));
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(2, 2), TileType::Entry);
}

/**
 * @brief Poser une seconde sortie déplace la première (unicité, EX-EDIT-004).
 * \castest{<b>Poser une seconde sortie déplace la première (unicité, EX-EDIT-004).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Poser une seconde sortie déplace la première (unicité, EX-EDIT-004).
 * }
 */
TEST(LevelDraftTest, SetExitDeplaceLaSortieExistante) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.setExit(1, 1);
    draft.setExit(3, 3);
    EXPECT_EQ(*draft.exit(), (GridPosition{3, 3}));
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(3, 3), TileType::Exit);
}

/**
 * @brief Peindre par-dessus l'entrée invalide la position d'entrée mémorisée.
 * \castest{<b>Peindre par-dessus l'entrée invalide la position d'entrée mémorisée.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Peindre par-dessus l'entrée invalide la position d'entrée mémorisée.
 * }
 */
TEST(LevelDraftTest, PeindrePardessusLEntreeLInvalide) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.setEntry(1, 1);
    draft.paintTile(1, 1, TileType::Solid);
    EXPECT_FALSE(draft.entry().has_value());
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
}

/**
 * @brief Lier un interrupteur à une porte crée un mécanisme ; le délier le retire.
 * \castest{<b>Lier un interrupteur à une porte crée un mécanisme ; le délier le retire.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Lier un interrupteur à une porte crée un mécanisme ; le délier le retire.
 * }
 */
TEST(LevelDraftTest, LierPuisDelierUnMecanisme) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);

    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    ASSERT_EQ(draft.mechanisms().size(), 1u);
    EXPECT_EQ(draft.mechanisms().front().switchPosition, (GridPosition{0, 0}));
    EXPECT_EQ(draft.mechanisms().front().doorPosition, (GridPosition{3, 3}));

    draft.unlinkMechanism(GridPosition{3, 3});
    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief Relier une porte déjà liée remplace la liaison précédente (une porte, un interrupteur).
 * \castest{<b>Relier une porte déjà liée remplace la liaison précédente (une porte, un
 * interrupteur).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Relier une porte déjà liée remplace la liaison précédente (une porte, un
 * interrupteur).
 * }
 */
TEST(LevelDraftTest, RelierUnePorteRemplaceLaLiaisonPrecedente) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(1, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);

    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    draft.linkMechanism(GridPosition{1, 0}, GridPosition{3, 3});

    ASSERT_EQ(draft.mechanisms().size(), 1u);
    EXPECT_EQ(draft.mechanisms().front().switchPosition, (GridPosition{1, 0}));
}

/**
 * @brief Un interrupteur peut ouvrir plusieurs portes.
 * \castest{<b>Un interrupteur peut ouvrir plusieurs portes.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un interrupteur peut ouvrir plusieurs portes.
 * }
 */
TEST(LevelDraftTest, UnInterrupteurPeutOuvrirPlusieursPortes) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);
    draft.paintTile(3, 0, TileType::Door);

    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 0});

    EXPECT_EQ(draft.mechanisms().size(), 2u);
}

/**
 * @brief Peindre par-dessus un interrupteur retire les liaisons qui le référencent.
 * \castest{<b>Peindre par-dessus un interrupteur retire les liaisons qui le référencent.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Peindre par-dessus un interrupteur retire les liaisons qui le référencent.
 * }
 */
TEST(LevelDraftTest, PeindrePardessusUnInterrupteurRetireSesLiaisons) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});

    draft.paintTile(0, 0, TileType::Empty);
    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief Agrandir la grille conserve le contenu existant et complète en cases vides.
 * \castest{<b>Agrandir la grille conserve le contenu existant et complète en cases
 * vides.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Agrandir la grille conserve le contenu existant et complète en cases vides.
 * }
 */
TEST(LevelDraftTest, AgrandirConserveLeContenu) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.setEntry(1, 1);
    draft.resize(5, 5);

    EXPECT_EQ(draft.tileMap().width(), 5);
    EXPECT_EQ(draft.tileMap().height(), 5);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Entry);
    EXPECT_EQ(*draft.entry(), (GridPosition{1, 1}));
    EXPECT_EQ(draft.tileMap().tile(4, 4), TileType::Empty);
}

/**
 * @brief Réduire la grille tronque le contenu hors bornes et invalide l'entrée perdue.
 * \castest{<b>Réduire la grille tronque le contenu hors bornes et invalide l'entrée
 * perdue.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Réduire la grille tronque le contenu hors bornes et invalide l'entrée perdue.
 * }
 */
TEST(LevelDraftTest, ReduireTronqueEtInvalideLEntreePerdue) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.setEntry(4, 4);
    draft.paintTile(1, 1, TileType::Solid);

    draft.resize(3, 3);

    EXPECT_EQ(draft.tileMap().width(), 3);
    EXPECT_EQ(draft.tileMap().height(), 3);
    EXPECT_FALSE(draft.entry().has_value());
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
}

/**
 * @brief Réduire la grille retire les mécanismes dont une extrémité sort des bornes.
 * \castest{<b>Réduire la grille retire les mécanismes dont une extrémité sort des
 * bornes.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Réduire la grille retire les mécanismes dont une extrémité sort des bornes.
 * }
 */
TEST(LevelDraftTest, ReduireRetireLesMecanismesHorsBornes) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(4, 4, TileType::Door);
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{4, 4});

    draft.resize(2, 2);
    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief toLevel() sur un brouillon complet et valide produit un niveau conforme.
 * \castest{<b>toLevel() sur un brouillon complet et valide produit un niveau conforme.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu toLevel() sur un brouillon complet et valide produit un niveau conforme.
 * }
 */
TEST(LevelDraftTest, ToLevelSurBrouillonValideReussit) {
    LevelDraft draft = LevelDraft::empty("Niveau test", 4, 4);
    draft.setEntry(0, 0);
    draft.setExit(3, 3);
    draft.setJumpBudget(2);

    const core::LevelLoadResult result = draft.toLevel();
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.level->name(), "Niveau test");
    EXPECT_EQ(result.level->entry(), (GridPosition{0, 0}));
    EXPECT_EQ(result.level->exit(), (GridPosition{3, 3}));
    EXPECT_EQ(result.level->jumpBudget(), 2);
}

/**
 * @brief toLevel() sur un brouillon sans sortie échoue avec un message récupérable (EX-EDIT-007).
 * \castest{<b>toLevel() sur un brouillon sans sortie échoue avec un message récupérable
 * (EX-EDIT-007).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu toLevel() sur un brouillon sans sortie échoue avec un message récupérable
 * (EX-EDIT-007).
 * }
 */
TEST(LevelDraftTest, ToLevelSansSortieEchoueProprement) {
    LevelDraft draft = LevelDraft::empty("Incomplet", 4, 4);
    draft.setEntry(0, 0);

    const core::LevelLoadResult result = draft.toLevel();
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
}

/**
 * @brief Un brouillon reconstruit depuis un niveau existant restitue son contenu.
 * \castest{<b>Un brouillon reconstruit depuis un niveau existant restitue son contenu.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un brouillon reconstruit depuis un niveau existant restitue son contenu.
 * }
 */
TEST(LevelDraftTest, FromLevelRestitueLeContenu) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "name": "Depart", "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const LevelDraft draft = LevelDraft::fromLevel(*loaded.level);
    EXPECT_EQ(draft.name(), "Depart");
    EXPECT_EQ(*draft.entry(), (GridPosition{0, 0}));
    EXPECT_EQ(*draft.exit(), (GridPosition{2, 2}));
}

/**
 * @brief Un brouillon neuf ne peut ni annuler ni refaire.
 * \castest{<b>Un brouillon neuf ne peut ni annuler ni refaire.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un brouillon neuf ne peut ni annuler ni refaire.
 * }
 */
TEST(LevelDraftTest, BrouillonNeufSansHistorique) {
    const LevelDraft draft = LevelDraft::empty("N", 4, 4);
    EXPECT_FALSE(draft.canUndo());
    EXPECT_FALSE(draft.canRedo());
}

/**
 * @brief undo() après une peinture restitue l'état exact précédent.
 * \castest{<b>undo() après une peinture restitue l'état exact précédent.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu undo() après une peinture restitue l'état exact précédent.
 * }
 */
TEST(LevelDraftTest, UndoApresPeintureRestitueLEtatPrecedent) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintTile(1, 1, TileType::Solid);
    ASSERT_TRUE(draft.canUndo());

    const bool undone = draft.undo();

    EXPECT_TRUE(undone);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Empty);
    EXPECT_FALSE(draft.canUndo());
    EXPECT_TRUE(draft.canRedo());
}

/**
 * @brief redo() après un undo() restitue l'état muté.
 * \castest{<b>redo() après un undo() restitue l'état muté.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu redo() après un undo() restitue l'état muté.
 * }
 */
TEST(LevelDraftTest, RedoApresUndoRestitueLEtatMute) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintTile(1, 1, TileType::Solid);
    draft.undo();

    const bool redone = draft.redo();

    EXPECT_TRUE(redone);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
    EXPECT_TRUE(draft.canUndo());
    EXPECT_FALSE(draft.canRedo());
}

/**
 * @brief Une séquence de N mutations suivie de N undo() restitue l'état initial exact.
 * \castest{<b>Une séquence de N mutations suivie de N undo() restitue l'état initial
 * exact.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une séquence de N mutations suivie de N undo() restitue l'état initial exact.
 * }
 */
TEST(LevelDraftTest, SequenceDeMutationsPuisUndoRestitueLEtatInitial) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.paintTile(0, 0, TileType::Solid);
    draft.setEntry(1, 1);
    draft.setExit(4, 4);
    draft.paintTile(2, 2, TileType::Danger);

    ASSERT_TRUE(draft.undo());
    ASSERT_TRUE(draft.undo());
    ASSERT_TRUE(draft.undo());
    ASSERT_TRUE(draft.undo());

    EXPECT_FALSE(draft.canUndo());
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(2, 2), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(4, 4), TileType::Empty);
    EXPECT_FALSE(draft.entry().has_value());
    EXPECT_FALSE(draft.exit().has_value());
}

/**
 * @brief Une nouvelle mutation après un undo() invalide la branche de refaire.
 * \castest{<b>Une nouvelle mutation après un undo() invalide la branche de refaire.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une nouvelle mutation après un undo() invalide la branche de refaire.
 * }
 */
TEST(LevelDraftTest, MutationApresUndoInvalideLeRefaire) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintTile(0, 0, TileType::Solid);
    draft.undo();
    ASSERT_TRUE(draft.canRedo());

    draft.paintTile(1, 1, TileType::Danger);

    EXPECT_FALSE(draft.canRedo());
    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Danger);
}

/**
 * @brief undo()/redo() sur une pile vide est sans effet (pas de plantage).
 * \castest{<b>undo()/redo() sur une pile vide est sans effet (pas de plantage).</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu undo()/redo() sur une pile vide est sans effet (pas de plantage).
 * }
 */
TEST(LevelDraftTest, UndoRedoSurPileVideSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    EXPECT_FALSE(draft.undo());
    EXPECT_FALSE(draft.redo());
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
}

/**
 * @brief L'annulation d'une liaison de mécanisme restitue la liaison précédente.
 * \castest{<b>L'annulation d'une liaison de mécanisme restitue la liaison précédente.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'annulation d'une liaison de mécanisme restitue la liaison précédente.
 * }
 */
TEST(LevelDraftTest, UndoApresLiaisonMecanismeRestitueLAbsenceDeLiaison) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});
    ASSERT_EQ(draft.mechanisms().size(), 1u);

    draft.undo();

    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief paintRegion applique un bloc homogène comme une succession de paintTile équivalente.
 * \castest{<b>paintRegion applique un bloc homogène comme une succession de paintTile
 * équivalente.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion applique un bloc homogène comme une succession de paintTile équivalente.
 * }
 */
TEST(LevelDraftTest, PaintRegionAppliqueLeBlocEntier) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    const std::vector<std::vector<TileType>> block = {
        {TileType::Solid, TileType::Solid},
        {TileType::Solid, TileType::Solid},
    };
    draft.paintRegion(1, 1, block);

    EXPECT_EQ(draft.tileMap().tile(1, 1), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(2, 1), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(1, 2), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(2, 2), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
}

/**
 * @brief paintRegion ne pousse qu'un seul snapshot undo pour tout le bloc.
 * \castest{<b>paintRegion ne pousse qu'un seul snapshot undo pour tout le bloc.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion ne pousse qu'un seul snapshot undo pour tout le bloc.
 * }
 */
TEST(LevelDraftTest, PaintRegionUnSeulSnapshotUndo) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    const std::vector<std::vector<TileType>> block = {
        {TileType::Danger, TileType::Danger, TileType::Danger},
    };
    draft.paintRegion(0, 0, block);

    ASSERT_TRUE(draft.undo());
    EXPECT_FALSE(draft.canUndo());
    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(1, 0), TileType::Empty);
    EXPECT_EQ(draft.tileMap().tile(2, 0), TileType::Empty);
}

/**
 * @brief paintRegion découpe silencieusement le bloc aux bords de la grille.
 * \castest{<b>paintRegion découpe silencieusement le bloc aux bords de la grille.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion découpe silencieusement le bloc aux bords de la grille.
 * }
 */
TEST(LevelDraftTest, PaintRegionDecoupeAuxBords) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    const std::vector<std::vector<TileType>> block = {
        {TileType::Solid, TileType::Solid, TileType::Solid},
        {TileType::Solid, TileType::Solid, TileType::Solid},
    };
    draft.paintRegion(1, 2, block);  // deborde largeur (colonne 3) et hauteur (ligne 3)

    EXPECT_EQ(draft.tileMap().tile(1, 2), TileType::Solid);
    EXPECT_EQ(draft.tileMap().tile(2, 2), TileType::Solid);
}

/**
 * @brief paintRegion qui inclut une position d'entrée déplace l'entrée existante (même sémantique
 *        que paintTile).
 * \castest{<b>paintRegion qui inclut une position d'entrée déplace l'entrée existante.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion qui inclut une position d'entrée déplace l'entrée existante.
 * }
 */
TEST(LevelDraftTest, PaintRegionDeplaceLEntree) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.setEntry(0, 0);
    const std::vector<std::vector<TileType>> block = {{TileType::Entry}};
    draft.paintRegion(2, 2, block);

    EXPECT_EQ(draft.tileMap().tile(0, 0), TileType::Empty);
    EXPECT_EQ(*draft.entry(), (GridPosition{2, 2}));
}

/**
 * @brief paintRegion qui recouvre un interrupteur retire les liaisons qui le référencent.
 * \castest{<b>paintRegion qui recouvre un interrupteur retire les liaisons qui le
 * référencent.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion qui recouvre un interrupteur retire les liaisons qui le référencent.
 * }
 */
TEST(LevelDraftTest, PaintRegionRetireLesLiaisonsRecouvertes) {
    LevelDraft draft = LevelDraft::empty("N", 4, 4);
    draft.paintTile(0, 0, TileType::Switch);
    draft.paintTile(3, 3, TileType::Door);
    draft.linkMechanism(GridPosition{0, 0}, GridPosition{3, 3});

    const std::vector<std::vector<TileType>> block = {{TileType::Empty}};
    draft.paintRegion(0, 0, block);

    EXPECT_TRUE(draft.mechanisms().empty());
}

/**
 * @brief paintRegion avec un bloc vide est sans effet (pas de snapshot undo créé).
 * \castest{<b>paintRegion avec un bloc vide est sans effet.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion avec un bloc vide est sans effet.
 * }
 */
TEST(LevelDraftTest, PaintRegionBlocVideSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 3, 3);
    draft.paintRegion(0, 0, {});
    EXPECT_FALSE(draft.canUndo());
}

/**
 * @brief wouldResizeDropContent détecte la perte de l'entrée, de la sortie ou d'une liaison.
 * \castest{<b>wouldResizeDropContent détecte la perte de l'entrée, de la sortie ou d'une
 * liaison.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu wouldResizeDropContent détecte la perte de l'entrée, de la sortie ou d'une liaison.
 * }
 */
TEST(LevelDraftTest, WouldResizeDropContentDetecteLaPerte) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.setEntry(4, 4);
    draft.setExit(0, 4);
    draft.paintTile(4, 0, TileType::Switch);
    draft.paintTile(0, 0, TileType::Door);
    draft.linkMechanism(GridPosition{4, 0}, GridPosition{0, 0});

    // L'entree (4,4) sortirait des bornes d'une grille 3x3.
    EXPECT_TRUE(draft.wouldResizeDropContent(3, 3));
    // Reduire uniquement la largeur : la sortie (0,4) et la liaison (0,0)/(4,0) sont concernees.
    EXPECT_TRUE(draft.wouldResizeDropContent(2, 5));
    // Agrandir ne perd jamais rien.
    EXPECT_FALSE(draft.wouldResizeDropContent(10, 10));
    // Memes dimensions : rien ne bouge.
    EXPECT_FALSE(draft.wouldResizeDropContent(5, 5));
}

/**
 * @brief wouldResizeDropContent est faux sur un brouillon vierge, quelle que soit la taille visee.
 * \castest{<b>wouldResizeDropContent est faux sur un brouillon vierge, quelle que soit la taille
 * visee.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu wouldResizeDropContent est faux sur un brouillon vierge, quelle que soit la taille
 * visee.
 * }
 */
TEST(LevelDraftTest, WouldResizeDropContentFauxSurBrouillonVierge) {
    const LevelDraft draft = LevelDraft::empty("N", 5, 5);
    EXPECT_FALSE(draft.wouldResizeDropContent(1, 1));
}

/**
 * @brief L'annulation d'un redimensionnement restitue les dimensions et le contenu précédents.
 * \castest{<b>L'annulation d'un redimensionnement restitue les dimensions et le contenu
 * précédents.</b><br/>
 * \tcat Unitaire · Level Draft<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'annulation d'un redimensionnement restitue les dimensions et le contenu
 * précédents.
 * }
 */
TEST(LevelDraftTest, UndoApresRedimensionnementRestitueLesDimensions) {
    LevelDraft draft = LevelDraft::empty("N", 5, 5);
    draft.setEntry(4, 4);

    draft.resize(2, 2);
    ASSERT_FALSE(draft.entry().has_value());

    draft.undo();

    EXPECT_EQ(draft.tileMap().width(), 5);
    EXPECT_EQ(draft.tileMap().height(), 5);
    ASSERT_TRUE(draft.entry().has_value());
    EXPECT_EQ(*draft.entry(), (GridPosition{4, 4}));
}

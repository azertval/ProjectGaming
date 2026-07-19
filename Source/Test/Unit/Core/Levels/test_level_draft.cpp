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

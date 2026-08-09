/**
 * @file test_panel_focus.cpp
 * @brief Tests unitaires de la correspondance outil → panneau (LOT-57 TACHE-02, EX-IHM-061).
 *        Logique pure, sans Qt.
 */

#include <gtest/gtest.h>

#include "HMI/Editor/PanelFocus.h"

/**
 * @brief L'outil Lien met en avant le panneau Liens.
 * \castest{<b>L'outil Lien met en avant le panneau Liens.</b><br/>
 * \tcat Unitaire · Mise en avant des panneaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger la table pour l'outil Lien.<br/>2. Verifier le panneau retourne.<br/>
 * \tattendu Le panneau Liens est retourne.
 * }
 */
TEST(PanelFocusTest, OutilLienMetEnAvantLePanneauLiens) {
    EXPECT_EQ(hmi::panelForTool(hmi::EditorTool::Link), hmi::PanelId::Links);
}

/**
 * @brief L'outil Texture par instance met en avant le panneau Textures.
 * \castest{<b>L'outil Texture met en avant le panneau Textures.</b><br/>
 * \tcat Unitaire · Mise en avant des panneaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger la table pour l'outil Texture par instance.<br/>2. Verifier le panneau
 * retourne.<br/>
 * \tattendu Le panneau Textures est retourne.
 * }
 */
TEST(PanelFocusTest, OutilTextureMetEnAvantLePanneauTextures) {
    EXPECT_EQ(hmi::panelForTool(hmi::EditorTool::TextureAssign), hmi::PanelId::Textures);
}

/**
 * @brief Les outils sans panneau dedie (Pinceau, Rectangle, Selection, Decor) ne mettent rien en
 *        avant : leurs controles vivent ailleurs (palette, panneau Outils).
 * \castest{<b>Les outils sans panneau dedie ne mettent rien en avant.</b><br/>
 * \tcat Unitaire · Mise en avant des panneaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger la table pour chaque outil sans panneau dedie.<br/>2. Verifier
 * l'absence de resultat.<br/>
 * \tattendu Chaque appel retourne std::nullopt.
 * }
 */
TEST(PanelFocusTest, OutilsSansPanneauDedieNeMettentRienEnAvant) {
    for (const hmi::EditorTool tool : {hmi::EditorTool::Paint, hmi::EditorTool::Rectangle,
                                       hmi::EditorTool::Selection, hmi::EditorTool::Decor}) {
        EXPECT_EQ(hmi::panelForTool(tool), std::nullopt) << "outil " << static_cast<int>(tool);
    }
}

/**
 * @brief La table ne contient aucun doublon d'outil : chaque outil n'a au plus une entree.
 * \castest{<b>La table ne contient aucun doublon d'outil.</b><br/>
 * \tcat Unitaire · Mise en avant des panneaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Parcourir toutes les paires d'entrees de la table.<br/>2. Comparer leurs outils.<br/>
 * \tattendu Aucune paire distincte ne partage le meme outil.
 * }
 */
TEST(PanelFocusTest, AucunDoublonDOutilDansLaTable) {
    const auto& catalog = hmi::panelFocusCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        for (std::size_t j = i + 1; j < catalog.size(); ++j) {
            EXPECT_NE(catalog[i].tool, catalog[j].tool) << i << " / " << j;
        }
    }
}

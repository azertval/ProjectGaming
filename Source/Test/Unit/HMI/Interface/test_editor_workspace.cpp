/**
 * @file test_editor_workspace.cpp
 * @brief Tests unitaires des espaces de travail de l'éditeur (`LOT-68`, `EX-IHM-073`).
 */

#include <set>

#include <gtest/gtest.h>

#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PanelFocus.h"
#include "HMI/Editor/PixelTool.h"
#include "HMI/Interface/EditorWorkspace.h"

namespace {

/// Les neuf panneaux, énumérés une seule fois ici pour que les tests ci-dessous les parcourent
/// tous. `PANEL_COUNT` ferme la boucle : un panneau ajouté à `PanelId` sans être ajouté ici fait
/// échouer le premier test, plutôt que de passer inaperçu.
constexpr hmi::PanelId ALL_PANELS[] = {
    hmi::PanelId::Palette,     hmi::PanelId::Decors,       hmi::PanelId::Levels,
    hmi::PanelId::Links,       hmi::PanelId::Properties,   hmi::PanelId::Textures,
    hmi::PanelId::PixelCanvas, hmi::PanelId::PixelHistory, hmi::PanelId::PixelPalette,
};

constexpr hmi::EditorTool ALL_TOOLS[] = {
    hmi::EditorTool::Paint,      hmi::EditorTool::Rectangle,     hmi::EditorTool::Selection,
    hmi::EditorTool::Link,       hmi::EditorTool::TextureAssign, hmi::EditorTool::Decor,
    hmi::EditorTool::CameraZone, hmi::EditorTool::Path,
};

constexpr hmi::PixelTool ALL_PIXEL_TOOLS[] = {
    hmi::PixelTool::Brush,      hmi::PixelTool::Eraser,    hmi::PixelTool::Fill,
    hmi::PixelTool::Eyedropper, hmi::PixelTool::Selection,
};

}  // namespace

/**
 * @brief Chaque panneau appartient à **exactement un** espace, et les neuf sont couverts. Un
 *        panneau sans espace resterait affiché dans les deux, ce qui viderait la séparation de
 *        tout son sens — et ne se verrait qu'à l'écran, jamais en relecture.
 * \castest{<b>Chaque panneau appartient a exactement un espace de travail.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Parcourir les neuf panneaux et resoudre leur espace.<br/>2. Verifier que le compte
 * couvert vaut PANEL_COUNT et que les deux espaces sont representes.<br/>
 * \tattendu Les neuf panneaux sont couverts, repartis entre les deux espaces.
 * }
 */
TEST(EditorWorkspaceTest, ChaquePanneauAppartientAUnSeulEspace) {
    std::set<hmi::PanelId> seen;
    int levelPanels = 0;
    int pixelPanels = 0;
    for (const hmi::PanelId panel : ALL_PANELS) {
        EXPECT_TRUE(seen.insert(panel).second) << "panneau enumere deux fois";
        if (hmi::workspaceForPanel(panel) == hmi::EditorWorkspace::Level) {
            ++levelPanels;
        } else {
            ++pixelPanels;
        }
    }
    EXPECT_EQ(seen.size(), hmi::PANEL_COUNT)
        << "un panneau de PanelId n'a pas d'espace : il resterait affiche dans les deux";
    EXPECT_GT(levelPanels, 0);
    EXPECT_GT(pixelPanels, 0);
}

/**
 * @brief Les deux barres d'outils ne sont **jamais** visibles ensemble. C'est la propriété que ce
 *        lot existe pour établir : c'est leur affichage simultané qui produisait une trentaine de
 *        contrôles permanents dont les deux tiers étaient hors contexte.
 * \castest{<b>Les deux barres d'outils ne sont jamais visibles simultanement.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire l'habillage des deux espaces.<br/>2. Verifier qu'exactement une barre est
 * visible dans chacun.<br/>
 * \tattendu Une seule barre par espace, et elle correspond a l'espace.
 * }
 */
TEST(EditorWorkspaceTest, UneSeuleBarreDOutilsParEspace) {
    const hmi::WorkspaceDressing level = hmi::dressingForWorkspace(hmi::EditorWorkspace::Level);
    EXPECT_TRUE(level.levelToolBarVisible);
    EXPECT_FALSE(level.pixelToolBarVisible);

    const hmi::WorkspaceDressing pixel = hmi::dressingForWorkspace(hmi::EditorWorkspace::PixelArt);
    EXPECT_FALSE(pixel.levelToolBarVisible);
    EXPECT_TRUE(pixel.pixelToolBarVisible);
}

/**
 * @brief Le menu « Atelier » n'existe que dans l'espace de l'atelier : ses commandes n'ont aucune
 *        cible ailleurs, et un menu grisé en permanence n'informe de rien.
 * \castest{<b>Le menu de l'atelier n'existe que dans l'espace de l'atelier.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire l'habillage des deux espaces.<br/>
 * \tattendu Le menu de l'atelier n'est visible que dans l'espace PixelArt.
 * }
 */
TEST(EditorWorkspaceTest, MenuAtelierReserveASonEspace) {
    EXPECT_FALSE(hmi::dressingForWorkspace(hmi::EditorWorkspace::Level).workshopMenuVisible);
    EXPECT_TRUE(hmi::dressingForWorkspace(hmi::EditorWorkspace::PixelArt).workshopMenuVisible);
}

/**
 * @brief Les deux familles d'outils restent **disjointes** : tout `EditorTool` mène à l'espace de
 *        niveau, tout `PixelTool` à celui de l'atelier. C'est ce qui permet à la sélection d'un
 *        outil de basculer d'espace sans que personne n'écrive de condition sur un outil précis.
 * \castest{<b>Chaque famille d'outils mene a son propre espace.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre l'espace de chacun des huit outils de niveau.<br/>2. Resoudre celui des
 * cinq outils de canevas.<br/>
 * \tattendu Les outils de niveau menent tous a Level, ceux de canevas tous a PixelArt.
 * }
 */
TEST(EditorWorkspaceTest, LesDeuxFamillesDOutilsSontDisjointes) {
    std::size_t levelTools = 0;
    for (const hmi::EditorTool tool : ALL_TOOLS) {
        EXPECT_EQ(hmi::workspaceForTool(tool), hmi::EditorWorkspace::Level);
        ++levelTools;
    }
    EXPECT_EQ(levelTools, hmi::EDITOR_TOOL_COUNT)
        << "un outil de EditorTool n'est pas couvert par ce test";

    for (const hmi::PixelTool tool : ALL_PIXEL_TOOLS) {
        EXPECT_EQ(hmi::workspaceForPixelTool(tool), hmi::EditorWorkspace::PixelArt);
    }
}

/**
 * @brief Chaque entrée de la table de mise en avant désigne un panneau du **même** espace que son
 *        outil. Un outil de niveau qui mettrait en avant un panneau de l'atelier demanderait à la
 *        fenêtre d'afficher un dock que l'espace actif ne contient pas.
 * \castest{<b>La mise en avant ne traverse jamais la frontiere entre espaces.</b><br/>
 * \tcat Unitaire · Espaces de travail<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Parcourir la table outil -> panneau, puis celle des outils de canevas.<br/>
 * 2. Comparer l'espace de l'outil a celui du panneau qu'il met en avant.<br/>
 * \tattendu Les deux espaces coincident pour chaque entree des deux tables.
 * }
 */
TEST(EditorWorkspaceTest, LaMiseEnAvantResteDansSonEspace) {
    for (const hmi::PanelFocusEntry& entry : hmi::panelFocusCatalog()) {
        EXPECT_EQ(hmi::workspaceForTool(entry.tool), hmi::workspaceForPanel(entry.panel))
            << "un outil de niveau met en avant un panneau d'un autre espace";
    }
    for (const hmi::PixelPanelFocusEntry& entry : hmi::pixelPanelFocusCatalog()) {
        EXPECT_EQ(hmi::workspaceForPixelTool(entry.tool), hmi::workspaceForPanel(entry.panel))
            << "un outil de canevas met en avant un panneau d'un autre espace";
    }
}

#pragma once

#include "HMI/Graphics/BitmapFont.h"
#include "HMI/Graphics/FlagIcons.h"
#include "HMI/Graphics/SaveIcon.h"
#include "HMI/Graphics/SpriteBatch.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Localization/Localization.h"

/**
 * @file HMI/Interface/RenderContext.h
 * @brief Ressources de rendu partagées passées aux écrans lors du dessin.
 */

namespace hmi {

/**
 * @brief Regroupe les ressources de rendu partagées mises à disposition des écrans.
 *
 * Le `ScreenManager` transmet ce contexte à `IScreen::render` : les écrans y puisent le lot de
 * sprites, l'atlas, la police et le catalogue de traduction, sans les posséder ni gérer leur
 * cycle de vie (ces ressources vivent le temps de l'application). Les dimensions de la
 * surface de rendu servent à construire la projection écran (interface en pixels).
 */
struct RenderContext {
    SpriteBatch& spriteBatch;          ///< Pipeline 2D partagé (déjà initialisé).
    const TextureAtlas& atlas;         ///< Atlas de tuiles (scène de jeu).
    const BitmapFont& font;            ///< Police bitmap pour le texte d'interface.
    const Localization& localization;  ///< Catalogue de traduction (textes par clé).
    const FlagIcons& flags;            ///< Icônes de drapeaux (sélecteur de langue).
    const SaveIcon& saveIcon;          ///< Icône d'enregistrement des logs.
    int viewportWidth = 0;             ///< Largeur de la surface de rendu, en pixels.
    int viewportHeight = 0;            ///< Hauteur de la surface de rendu, en pixels.
    /// Facteur d'interpolation `[0, 1[` entre le pas de simulation précédent et le pas courant
    /// (`EX-ARCH-031`) : fraction de pas fixe accumulée mais pas encore consommée à cette frame de
    /// rendu (`core::FixedTimestep::interpolationAlpha`). Lissage du mouvement à haut framerate.
    float interpolationAlpha = 0.0f;
};

}  // namespace hmi

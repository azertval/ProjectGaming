#pragma once

#include "Core/Ecs/Components/Sprite.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/RenderMode.h"

/**
 * @file HMI/Graphics/TileAppearance.h
 * @brief Point de résolution **unique** de l'apparence d'une entité affichée (`EX-REN-046`).
 */

namespace hmi {

/// Texture d'où provient l'apparence résolue.
enum class AppearanceSource {
    /// Atlas du jeu : couleur plate par type de tuile, images du personnage, aides d'édition.
    Atlas,
    /// Damier magenta de repli : texture attendue mais pas encore disponible (`EX-NFR-040`).
    MissingTexture,
};

/// Apparence résolue d'une primitive : quelle texture lier, et quelle région y échantillonner.
struct TileAppearance {
    /// Texture à lier.
    AppearanceSource source = AppearanceSource::Atlas;
    /// Région à échantillonner **dans cette texture**, en pixels.
    core::AtlasRegion region{};
};

/**
 * @brief Résout l'apparence d'une entité affichée selon le mode de rendu courant.
 *
 * **Point d'appel unique** de cette décision, partagé par le jeu et l'éditeur : c'est ce qui
 * garantit que le canevas de l'éditeur montre exactement ce que le joueur verra. Fonction
 * **pure** (aucune dépendance GPU), appelée à la **composition** et non à la construction de la
 * scène : basculer de mode ne reconstruit donc jamais l'ECS, seule la résolution change.
 *
 * Deux branches à ce stade :
 * - `RenderMode::Physique` → la région d'atlas déjà portée par le `core::Sprite`, c'est-à-dire le
 *   résultat de `hmi::regionForTile` — comportement strictement inchangé ;
 * - `RenderMode::Texture` → le damier magenta en entier, aucun skin n'existant encore.
 *
 * Les branches suivantes (`LOT-42` : surcharge par case > skin de tuile > damier) s'inséreront
 * **ici**, sans changer la forme de la fonction ni ses appelants.
 * @param mode           Mode de rendu courant.
 * @param physicalRegion Région d'atlas de l'entité en mode Physique (`core::Sprite::region`).
 * @return L'apparence à utiliser pour composer la primitive.
 */
[[nodiscard]] TileAppearance resolveTileAppearance(
    RenderMode mode, const core::AtlasRegion& physicalRegion) noexcept;

/**
 * @brief Textures liables par la composition d'une scène, et leurs dimensions.
 *
 * Regroupées en une seule donnée parce qu'elles vont toujours ensemble : une texture sans ses
 * dimensions ne permet pas de normaliser des coordonnées de texture, et la composition doit
 * pouvoir servir les deux branches de `hmi::resolveTileAppearance` sans rien demander au GPU.
 */
struct SceneTextures {
    /// Atlas du jeu (`hmi::TextureAtlas::textureView`).
    TextureHandle atlas = nullptr;
    /// Largeur de l'atlas, en pixels.
    int atlasWidth = 0;
    /// Hauteur de l'atlas, en pixels.
    int atlasHeight = 0;
    /// Damier magenta de repli (`hmi::TextureCache::missingTexture`).
    TextureHandle missing = nullptr;
    /// Largeur du damier, en pixels.
    int missingWidth = 0;
    /// Hauteur du damier, en pixels.
    int missingHeight = 0;

    /// @return La texture correspondant à une source résolue (`nullptr` si non fournie).
    [[nodiscard]] TextureHandle textureFor(AppearanceSource source) const noexcept {
        return source == AppearanceSource::Atlas ? atlas : missing;
    }

    /// @return La largeur, en pixels, de la texture correspondant à une source résolue.
    [[nodiscard]] int widthFor(AppearanceSource source) const noexcept {
        return source == AppearanceSource::Atlas ? atlasWidth : missingWidth;
    }

    /// @return La hauteur, en pixels, de la texture correspondant à une source résolue.
    [[nodiscard]] int heightFor(AppearanceSource source) const noexcept {
        return source == AppearanceSource::Atlas ? atlasHeight : missingHeight;
    }
};

}  // namespace hmi

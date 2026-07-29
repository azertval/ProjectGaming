#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Ecs/Components/Sprite.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Graphics/SlopeMask.h"
#include "HMI/Graphics/TileSkinTag.h"

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
    /// Skin assigné au type de tuile, désigné par `hmi::TileAppearance::skinIndex` (`LOT-42`).
    Skin,
};

/// Apparence résolue d'une primitive : quelle texture lier, et quelle région y échantillonner.
struct TileAppearance {
    /// Texture à lier.
    AppearanceSource source = AppearanceSource::Atlas;
    /// Région à échantillonner **dans cette texture**, en pixels.
    core::AtlasRegion region{};
    /// Index du skin dans `hmi::SceneTextures::skins`. N'a de sens que si `source` vaut `Skin` ;
    /// vaut `-1` sinon. Un **index** plutôt qu'un nom de fichier : la composition s'exécute à
    /// chaque image et ne doit ni allouer ni comparer de chaînes.
    int skinIndex = -1;
};

/// Skin déjà chargé, prêt à être lié par la composition.
struct SkinTexture {
    /// Nom logique du fichier (clé de correspondance avec `hmi::SkinEntry::asset`).
    std::string asset;
    /// Type dont la **silhouette** a été appliquée à l'image, s'il y a lieu (`LOT-42` TACHE-03).
    /// Vide pour l'image d'origine, non détourée. Un même fichier peut donc être présent
    /// plusieurs fois — une entrée par découpe — sans qu'une variante n'abîme les autres.
    std::optional<core::TileType> maskType;
    /// Texture chargée (non possédée : propriété du `hmi::TextureCache`).
    TextureHandle texture = nullptr;
    /// Largeur de la texture, en pixels.
    int width = 0;
    /// Hauteur de la texture, en pixels.
    int height = 0;
};

/**
 * @brief Textures liables par la composition d'une scène, et de quoi choisir laquelle lier.
 *
 * Regroupées en une seule donnée parce qu'elles vont toujours ensemble : une texture sans ses
 * dimensions ne permet pas de normaliser des coordonnées de texture, et la composition doit
 * pouvoir servir toutes les branches de `hmi::resolveTileAppearance` sans rien demander au GPU.
 *
 * Porte aussi le **catalogue** et le **jeu courant** (`LOT-42`) : ce sont eux qui décident quelle
 * texture lier, au même titre que les textures elles-mêmes. Les y placer garde la composition
 * paramétrée par une seule donnée, et laisse la fonction de résolution pure et testable sans GPU
 * (`EX-NFR-004`).
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

    /// Skins déjà chargés, adressés par index (`hmi::TileAppearance::skinIndex`).
    std::vector<SkinTexture> skins;
    /// Catalogue des jeux de skins, ou `nullptr` si aucun n'est chargé (tout retombe au damier).
    const SkinCatalog* skinCatalog = nullptr;
    /// Nom du jeu de skins courant. Vide pour utiliser le jeu par défaut du catalogue.
    std::string skinSet;

    /**
     * @brief Index du skin chargé pour un asset et un type de tuile donnés.
     *
     * Compare sans jamais construire de chaîne : cette recherche a lieu pour **chaque primitive
     * de chaque image**, et une clé textuelle assemblée à la volée y allouerait à ce rythme.
     * @param asset Nom du fichier cherché.
     * @param type  Type de tuile, qui détermine si c'est la variante détourée qu'il faut.
     * @return Son index dans `skins`, ou `-1` si la variante voulue n'est pas chargée.
     */
    [[nodiscard]] int skinIndexOf(std::string_view asset, core::TileType type) const noexcept {
        const bool masked = hasSilhouette(type);
        for (std::size_t index = 0; index < skins.size(); ++index) {
            const SkinTexture& skin = skins[index];
            if (skin.asset != asset) {
                continue;
            }
            if (masked ? (skin.maskType == type) : !skin.maskType.has_value()) {
                return static_cast<int>(index);
            }
        }
        return -1;
    }

    /// @return La texture correspondant à une apparence résolue (`nullptr` si non fournie).
    [[nodiscard]] TextureHandle textureFor(const TileAppearance& appearance) const noexcept {
        switch (appearance.source) {
            case AppearanceSource::Atlas:
                return atlas;
            case AppearanceSource::MissingTexture:
                return missing;
            case AppearanceSource::Skin:
                return skinAt(appearance.skinIndex) ? skinAt(appearance.skinIndex)->texture
                                                    : missing;
        }
        return missing;
    }

    /// @return La largeur, en pixels, de la texture correspondant à une apparence résolue.
    [[nodiscard]] int widthFor(const TileAppearance& appearance) const noexcept {
        switch (appearance.source) {
            case AppearanceSource::Atlas:
                return atlasWidth;
            case AppearanceSource::MissingTexture:
                return missingWidth;
            case AppearanceSource::Skin:
                return skinAt(appearance.skinIndex) ? skinAt(appearance.skinIndex)->width
                                                    : missingWidth;
        }
        return missingWidth;
    }

    /// @return La hauteur, en pixels, de la texture correspondant à une apparence résolue.
    [[nodiscard]] int heightFor(const TileAppearance& appearance) const noexcept {
        switch (appearance.source) {
            case AppearanceSource::Atlas:
                return atlasHeight;
            case AppearanceSource::MissingTexture:
                return missingHeight;
            case AppearanceSource::Skin:
                return skinAt(appearance.skinIndex) ? skinAt(appearance.skinIndex)->height
                                                    : missingHeight;
        }
        return missingHeight;
    }

private:
    // Acces borne au tableau de skins : un index invalide rend nullptr, ce qui fait retomber les
    // accesseurs ci-dessus sur le damier plutot que de lire hors des bornes.
    [[nodiscard]] const SkinTexture* skinAt(int index) const noexcept {
        if (index < 0 || static_cast<std::size_t>(index) >= skins.size()) {
            return nullptr;
        }
        return &skins[static_cast<std::size_t>(index)];
    }
};

/**
 * @brief Résout l'apparence d'une entité affichée selon le mode de rendu courant.
 *
 * **Point d'appel unique** de cette décision, partagé par le jeu et l'éditeur : c'est ce qui
 * garantit que le canevas de l'éditeur montre exactement ce que le joueur verra. Fonction
 * **pure** (aucune dépendance GPU), appelée à la **composition** et non à la construction de la
 * scène : basculer de mode ne reconstruit donc jamais l'ECS, seule la résolution change.
 *
 * Ordre de priorité en mode Texture (`LOT-42`) : **skin de tuile > damier**. La surcharge par case
 * viendra s'insérer en tête au `LOT-45`, sans changer la forme de la fonction.
 * - `RenderMode::Physique` → la région d'atlas déjà portée par le `core::Sprite`, c'est-à-dire le
 *   résultat de `hmi::regionForTile` — comportement strictement inchangé, quel que soit le reste ;
 * - `RenderMode::Texture` + entité habillable dont le type est skinné **et** dont le skin est
 *   chargé → la case du skin (l'image entière en mode `single`, la case choisie par les raccords
 *   en mode `bitmask16`) ;
 * - tout autre cas (entité sans `hmi::TileSkinTag`, type non skinné, asset absent ou refusé) → le
 *   damier magenta en entier. C'est un état **normal** du programme d'habillage tant que tous les
 *   types ne sont pas habillés, pas un défaut (`EX-NFR-040`).
 * @param mode           Mode de rendu courant.
 * @param physicalRegion Région d'atlas de l'entité en mode Physique (`core::Sprite::region`).
 * @param tag            Type et voisinage de la tuile, ou `nullptr` pour une entité non habillable
 *                       (personnage, aides d'édition) — qui reste alors au damier en mode Texture.
 * @param textures       Textures liables, catalogue et jeu de skins courant.
 * @return L'apparence à utiliser pour composer la primitive.
 */
[[nodiscard]] TileAppearance resolveTileAppearance(RenderMode mode,
                                                   const core::AtlasRegion& physicalRegion,
                                                   const TileSkinTag* tag,
                                                   const SceneTextures& textures) noexcept;

}  // namespace hmi

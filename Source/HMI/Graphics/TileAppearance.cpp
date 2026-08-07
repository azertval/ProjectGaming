#include "HMI/Graphics/TileAppearance.h"

#include <optional>

#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAutotile.h"

namespace hmi {

// La geometrie composee doit etre identique dans les deux modes : le rendu bascule pour comparer
// l'habillage au physique, pas pour comparer deux tailles de tuiles. Cela n'est vrai que si le
// damier de repli occupe exactement une case.
static_assert(MISSING_TEXTURE_SIZE == TextureAtlas::TILE_SIZE,
              "Le damier de repli doit faire exactement une case, sinon basculer de mode change "
              "la taille des primitives composees.");

namespace {

// Damier magenta en entier : repli commun a toutes les branches qui n'aboutissent pas a un skin.
[[nodiscard]] TileAppearance missingAppearance() noexcept {
    return TileAppearance{AppearanceSource::MissingTexture,
                          core::AtlasRegion{0, 0, MISSING_TEXTURE_SIZE, MISSING_TEXTURE_SIZE}, -1};
}

// Region a echantillonner dans un skin, selon son mode de decoupage.
[[nodiscard]] core::AtlasRegion skinRegion(const SkinEntry& entry, const TileSkinTag& tag,
                                           const SkinTexture& texture) noexcept {
    constexpr int SIZE = TextureAtlas::TILE_SIZE;
    if (entry.mode == SkinMode::Bitmask16) {
        // La case depend du voisinage solide, calcule une fois a la construction de la scene.
        // Bitmask16 exclut l'animation (LOT-46 TACHE-05, limite assumee) : texture.animatedFrame
        // n'est jamais renseigne pour ce mode (voir hmi::sceneTextures), ignore ici de toute facon.
        const AutotileCell cell = autotileCell(tag.neighborMask);
        return core::AtlasRegion{cell.column * SIZE, cell.row * SIZE, SIZE, SIZE};
    }
    // Image courante PAR INSTANCE d'un mecanisme (LOT-47) : prioritaire sur l'horloge partagee par
    // asset ci-dessous, seule capable de distinguer deux tuiles du meme asset a des etats differents
    // (une porte ouverte a cote d'une porte fermee, par exemple).
    if (tag.animatedFrame) {
        return *tag.animatedFrame;
    }
    // Mode single anime (LOT-46 TACHE-05) : l'image COURANTE plutot que l'image entiere.
    if (texture.animatedFrame) {
        return *texture.animatedFrame;
    }
    // Mode single, non anime : l'image entiere, qui fait exactement une case (contrat d'asset
    // TileSkin).
    return core::AtlasRegion{0, 0, SIZE, SIZE};
}

}  // namespace

// Resout l'apparence d'une entite affichee selon le mode de rendu courant (point d'appel unique).
TileAppearance resolveTileAppearance(RenderMode mode, const core::AtlasRegion& physicalRegion,
                                     const TileSkinTag* tag,
                                     const SceneTextures& textures) noexcept {
    if (mode == RenderMode::Physique) {
        // Mode de reference : la region deja resolue par hmi::regionForTile a la construction de
        // la scene. Rien n'est recalcule ici -- c'est ce qui garantit l'absence de regression.
        return TileAppearance{AppearanceSource::Atlas, physicalRegion, -1};
    }

    // Entite non habillable (personnage, aides d'edition) : damier.
    if (tag == nullptr) {
        return missingAppearance();
    }

    // Surcharge de texture par instance (EX-EDIT-043, LOT-45) : prioritaire sur le skin du type,
    // lui-meme prioritaire sur le damier -- verifiee avant tout acces au catalogue de skins. Un
    // asset introuvable retombe directement sur le damier, jamais sur le skin du type.
    if (tag->overrideAsset) {
        const int index = textures.objectIndexOf(*tag->overrideAsset);
        if (index < 0) {
            return missingAppearance();
        }
        // Image courante par instance (LOT-47), meme priorite que pour un skin ci-dessous ; a
        // defaut, l'image entiere (surcharge non animee, comportement inchange depuis LOT-45).
        constexpr int SIZE = TextureAtlas::TILE_SIZE;
        const core::AtlasRegion region =
            tag->animatedFrame ? *tag->animatedFrame : core::AtlasRegion{0, 0, SIZE, SIZE};
        return TileAppearance{AppearanceSource::Override, region, index};
    }

    if (textures.skinCatalog == nullptr) {
        return missingAppearance();
    }

    const std::optional<SkinEntry> entry = textures.skinCatalog->resolve(textures.skinSet, tag->type);
    if (!entry.has_value()) {
        return missingAppearance();  // type pas encore habille : etat normal, pas un defaut.
    }

    // Skin assigne mais texture pas (encore) chargee -- fichier absent, illisible ou refuse par le
    // contrat d'asset : damier, l'avertissement ayant deja ete journalise par le TextureCache.
    const int index = textures.skinIndexOf(entry->asset, tag->type);
    if (index < 0) {
        return missingAppearance();
    }

    return TileAppearance{AppearanceSource::Skin,
                          skinRegion(*entry, *tag, textures.skins[static_cast<std::size_t>(index)]),
                          index};
}

// Indique si la combinaison (mode de skin, type de tuile) exclut l'animation (voir en-tete).
bool animationExcludedForTile(SkinMode mode, core::TileType type) noexcept {
    return mode == SkinMode::Bitmask16 || hasSilhouette(type);
}

}  // namespace hmi

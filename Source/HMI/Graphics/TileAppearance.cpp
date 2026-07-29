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
[[nodiscard]] core::AtlasRegion skinRegion(const SkinEntry& entry,
                                           const TileSkinTag& tag) noexcept {
    constexpr int SIZE = TextureAtlas::TILE_SIZE;
    if (entry.mode == SkinMode::Bitmask16) {
        // La case depend du voisinage solide, calcule une fois a la construction de la scene.
        const AutotileCell cell = autotileCell(tag.neighborMask);
        return core::AtlasRegion{cell.column * SIZE, cell.row * SIZE, SIZE, SIZE};
    }
    // Mode single : l'image entiere, qui fait exactement une case (contrat d'asset TileSkin).
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

    // Entite non habillable (personnage, aides d'edition) ou catalogue absent : damier.
    if (tag == nullptr || textures.skinCatalog == nullptr) {
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

    return TileAppearance{AppearanceSource::Skin, skinRegion(*entry, *tag), index};
}

}  // namespace hmi

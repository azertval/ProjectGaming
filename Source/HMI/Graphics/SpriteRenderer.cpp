#include "HMI/Graphics/SpriteRenderer.h"

#include "Core/Ecs/World.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TextureCache.h"

namespace hmi {

// Soumet une scene composee au pipeline de dessin, une passe par groupe de texture.
void submitComposedScene(SpriteBatch& batch, const DirectX::XMFLOAT4X4& projection,
                         const ComposedScene& scene) {
    const std::vector<ComposedQuad>& quads = scene.quads();
    TextureHandle current = nullptr;
    bool open = false;

    for (const ComposedQuad& composed : quads) {
        if (composed.texture == nullptr) {
            continue;  // primitive sans texture liee : rien a dessiner (robustesse).
        }
        if (!open || composed.texture != current) {
            if (open) {
                batch.end();
            }
            // Seule reconversion vers le type Direct3D de toute la chaine de rendu : la
            // composition ne manipule qu'une identite opaque (cf. hmi::TextureHandle).
            batch.begin(projection, static_cast<ID3D11ShaderResourceView*>(composed.texture));
            current = composed.texture;
            open = true;
        }
        if (composed.kind == QuadKind::Sprite) {
            batch.draw(composed.sprite);
        } else {
            batch.draw(composed.line);
        }
    }

    if (open) {
        batch.end();
    }
}

// Textures liables par la composition d'une scene : atlas, damier de repli et skins (point unique).
SceneTextures sceneTextures(const TextureAtlas& atlas, TextureCache& cache,
                            const SkinCatalog* skins, const std::string& skinSet) {
    SceneTextures textures;
    textures.atlas = atlas.textureView();
    textures.atlasWidth = atlas.width();
    textures.atlasHeight = atlas.height();
    // Resolution a la demande : en mode Physique, le damier n'est jamais cree.
    if (const LoadedTexture* missing = cache.missingTexture()) {
        textures.missing = missing->view.Get();
        textures.missingWidth = missing->width;
        textures.missingHeight = missing->height;
    }

    textures.skinCatalog = skins;
    textures.skinSet = skinSet;
    if (skins == nullptr) {
        return textures;
    }

    // Charge les skins du jeu courant. Le TextureCache ne relit le disque qu'au premier acces et
    // memorise aussi les echecs : cet appel par image reste donc une recherche en table. Un asset
    // absent ou refuse par le contrat n'est simplement pas ajoute -- la resolution le verra
    // manquant et retombera sur le damier, apres l'unique avertissement deja journalise.
    const std::string& effectiveSet = skinSet.empty() ? skins->defaultSetName() : skinSet;
    for (const auto& [type, entry] : skins->assignments(effectiveSet)) {
        if (textures.skinIndexOf(entry.asset, type) >= 0) {
            continue;  // variante deja chargee : plusieurs types carres peuvent partager un asset.
        }
        const AssetFamily family =
            entry.mode == SkinMode::Bitmask16 ? AssetFamily::AutotileSheet : AssetFamily::TileSkin;
        // Un type a silhouette recoit l'image detouree, distincte de l'originale et calculee une
        // seule fois (LOT-42 TACHE-03).
        const LoadedTexture* loaded =
            cache.getMasked(SKINS_SUBDIRECTORY + entry.asset, family, type);
        if (loaded == nullptr) {
            continue;
        }
        const std::optional<core::TileType> maskType =
            hasSilhouette(type) ? std::optional<core::TileType>{type} : std::nullopt;
        textures.skins.push_back(
            SkinTexture{entry.asset, maskType, loaded->view.Get(), loaded->width, loaded->height});
    }
    return textures;
}

// Construit le rendu de sprites.
SpriteRenderer::SpriteRenderer(SpriteBatch& batch, const TextureAtlas& atlas, TextureCache& cache)
    : _batch(&batch), _atlas(&atlas), _cache(&cache) {}

// Dessine toutes les entites affichables du monde, vues par la camera.
void SpriteRenderer::render(core::World& world, const Camera2D& camera, RenderMode mode,
                            float interpolationAlpha) {
    _scene.clear();
    _scene.setVisibleBounds(camera.visibleBounds());
    composeWorldSprites(_scene, world, mode, sceneTextures(*_atlas, *_cache, _skins, _skinSet),
                        interpolationAlpha);
    _scene.sort();
    logStatisticsIfChanged();
    submitComposedScene(*_batch, camera.projectionMatrix(), _scene);
}

// Journalise les compteurs de l'image seulement s'ils ont change depuis la precedente.
void SpriteRenderer::logStatisticsIfChanged() {
    const SceneStatistics statistics = _scene.statistics();
    const bool changed = statistics.considered != _loggedStatistics.considered ||
                         statistics.culled != _loggedStatistics.culled ||
                         statistics.submitted != _loggedStatistics.submitted ||
                         statistics.batches != _loggedStatistics.batches;
    if (!changed) {
        return;
    }
    _loggedStatistics = statistics;
    GRAPHICS_LOG_TRACE(formatSceneStatistics(statistics));
}

}  // namespace hmi

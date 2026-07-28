#include "HMI/Graphics/TextureCache.h"

#include <utility>

#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/MissingTexture.h"

namespace hmi {

// Construit un cache vide pour un device et un dossier d'assets donnes.
TextureCache::TextureCache(ID3D11Device* device, AssetPaths paths)
    : _device(device), _paths(std::move(paths)) {}

// Charge et valide un asset depuis le disque, sans passer par le cache. Les trois causes d'echec
// (chemin introuvable, decodage impossible, dimensions non conformes) sont journalisees
// distinctement : le message doit dire quoi corriger, pas seulement que ca a rate.
std::optional<LoadedTexture> TextureCache::load(const std::string& fileName,
                                                AssetFamily family) const {
    const std::optional<std::filesystem::path> path = _paths.resolve(fileName);
    if (!path) {
        GRAPHICS_LOG_WARNING(missingTextureWarning(fileName));
        return std::nullopt;
    }

    // Decodage d'abord : la validation des dimensions (pure, EX-REN-007) doit s'intercaler entre
    // le decodage et l'upload GPU, sinon un asset de travers est deja devenu une texture.
    const std::optional<DecodedImage> image = decodeImageFile(*path);
    if (!image) {
        GRAPHICS_LOG_WARNING("Asset " + fileName + " illisible ou format non supporte (" +
                             path->string() + ").");
        return std::nullopt;
    }

    const AssetValidation validation = validateAsset(family, fileName, image->width, image->height);
    if (!validation.valid) {
        GRAPHICS_LOG_WARNING(validation.message);
        return std::nullopt;
    }

    std::optional<LoadedTexture> texture =
        createTexture(_device, image->width, image->height, image->pixels);
    if (!texture) {
        GRAPHICS_LOG_WARNING("Creation Direct3D de la texture " + fileName + " impossible.");
        return std::nullopt;
    }
    GRAPHICS_LOG_INFO("Texture chargee : " + fileName + " (" + std::to_string(image->width) + "x" +
                      std::to_string(image->height) + ").");
    return texture;
}

// Obtient la texture d'un asset, en la chargeant au premier acces.
// La texture chargee (propriete du cache), ou nullptr si l'asset est absent/illisible/non conforme.
const LoadedTexture* TextureCache::get(const std::string& fileName, AssetFamily family) {
    // Un echec deja constate est memorise (entree a std::nullopt) : sans cela, un asset manquant
    // relirait le disque et rejouerait son avertissement a chaque image.
    const auto found = _entries.find(fileName);
    if (found != _entries.end()) {
        return found->second ? &*found->second : nullptr;
    }

    const auto inserted = _entries.emplace(fileName, load(fileName, family)).first;
    return inserted->second ? &*inserted->second : nullptr;
}

// Retire une entree du cache, de sorte que le prochain get relise le fichier.
void TextureCache::invalidate(const std::string& fileName) {
    _entries.erase(fileName);
}

// Retire toutes les entrees du cache (rechargement global).
void TextureCache::invalidateAll() {
    _entries.clear();
}

// Texture de repli en damier magenta, creee une seule fois a la demande.
const LoadedTexture* TextureCache::missingTexture() {
    if (!_missingTexture) {
        const ProceduralAtlasImage image = buildMissingTextureImage();
        _missingTexture = createTexture(_device, image.width, image.height, image.pixels);
        if (!_missingTexture) {
            GRAPHICS_LOG_ERROR("Creation de la texture de repli (damier magenta) impossible.");
            return nullptr;
        }
        GRAPHICS_LOG_INFO("Texture de repli (damier magenta) creee.");
    }
    return &*_missingTexture;
}

// Obtient la texture d'un asset, ou le repli en damier magenta a defaut (point unique).
// La texture de l'asset si disponible, sinon le damier ; nullptr si meme le damier a echoue.
const LoadedTexture* resolveOrPlaceholder(TextureCache& cache, const std::string& fileName,
                                          AssetFamily family) {
    if (const LoadedTexture* texture = cache.get(fileName, family)) {
        return texture;
    }
    // L'avertissement nommant l'asset a deja ete emis par `get` lors du premier echec ; le
    // repeter ici a chaque image noierait le journal (GraphicsLog : jamais dans un chemin par
    // image).
    return cache.missingTexture();
}

}  // namespace hmi

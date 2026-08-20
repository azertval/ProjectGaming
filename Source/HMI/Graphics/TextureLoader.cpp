// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/TextureLoader.h"

#include <QImage>
#include <QString>
#include <cstring>
#include <system_error>

#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

// Décode un fichier image (PNG au minimum) en pixels RGBA.
std::optional<DecodedImage> decodeImageFile(const std::filesystem::path& path) {
    QImage source(QString::fromStdWString(path.wstring()));
    if (source.isNull()) {
        GRAPHICS_LOG_WARNING("TextureLoader : echec de decodage de l'image '" + path.string() +
                             "'");
        return std::nullopt;
    }
    // Format_RGBA8888 : quatre octets R,G,B,A en mémoire, alpha NON prémultiplié — le même ordre
    // mémoire que DXGI_FORMAT_R8G8B8A8_UNORM, et cohérent avec le blend state de SpriteBatch
    // (D3D11_BLEND_SRC_ALPHA, pas D3D11_BLEND_ONE) : aucune conversion de canal nécessaire.
    const QImage image = source.convertToFormat(QImage::Format_RGBA8888);

    DecodedImage decoded;
    decoded.width = image.width();
    decoded.height = image.height();
    decoded.pixels.resize(static_cast<std::size_t>(decoded.width) *
                          static_cast<std::size_t>(decoded.height));
    const std::size_t rowBytes = static_cast<std::size_t>(decoded.width) * sizeof(std::uint32_t);
    for (int row = 0; row < decoded.height; ++row) {
        std::memcpy(decoded.pixels.data() + (static_cast<std::size_t>(row) * decoded.width),
                    image.constScanLine(row), rowBytes);
    }
    return decoded;
}

// Écrit un fichier PNG depuis des pixels RGBA déjà en mémoire — symétrique de decodeImageFile.
bool encodeImageFile(const std::filesystem::path& path, const DecodedImage& image) {
    if (image.width <= 0 || image.height <= 0 ||
        image.pixels.size() !=
            static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height)) {
        GRAPHICS_LOG_WARNING("TextureLoader : image invalide, encodage refuse pour '" +
                             path.string() + "'");
        return false;
    }

    std::error_code error;
    const std::filesystem::path directory = path.parent_path();
    if (!directory.empty() && !std::filesystem::is_directory(directory, error)) {
        GRAPHICS_LOG_WARNING("TextureLoader : dossier de destination introuvable pour '" +
                             path.string() + "'");
        return false;
    }

    // Format_RGBA8888 : le meme format non premultiplie que decodeImageFile lit -- aucune
    // conversion de canal, l'aller-retour restitue exactement les memes pixels.
    const QImage output(reinterpret_cast<const uchar*>(image.pixels.data()), image.width,
                        image.height, static_cast<int>(image.width * sizeof(std::uint32_t)),
                        QImage::Format_RGBA8888);

    // Ecriture atomique : fichier temporaire dans le meme dossier (donc le meme volume, condition
    // pour que le remplacement soit atomique), puis remplacement en une seule operation. Un
    // QFileSystemWatcher de rechargement a chaud (LOT-43) ne voit ainsi jamais de fichier tronque.
    const std::filesystem::path temporary =
        directory / (path.stem().wstring() + L".tmp" + path.extension().wstring());
    if (!output.save(QString::fromStdWString(temporary.wstring()), "PNG")) {
        GRAPHICS_LOG_WARNING("TextureLoader : echec d'ecriture temporaire pour '" + path.string() +
                             "'");
        std::filesystem::remove(temporary, error);
        return false;
    }
    std::filesystem::rename(temporary, path, error);
    if (error) {
        GRAPHICS_LOG_WARNING("TextureLoader : echec du remplacement atomique pour '" +
                             path.string() + "'");
        std::filesystem::remove(temporary, error);
        return false;
    }
    return true;
}

// Crée une texture Direct3D 11 immuable à partir de pixels RGBA déjà décodés.
std::optional<LoadedTexture> createTexture(ID3D11Device* device, int width, int height,
                                           const std::vector<std::uint32_t>& pixels) {
    if (width <= 0 || height <= 0) {
        return std::nullopt;
    }

    D3D11_TEXTURE2D_DESC description{};
    description.Width = static_cast<UINT>(width);
    description.Height = static_cast<UINT>(height);
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = pixels.data();
    data.SysMemPitch = static_cast<UINT>(width) * sizeof(std::uint32_t);

    LoadedTexture result;
    result.width = width;
    result.height = height;
    if (FAILED(device->CreateTexture2D(&description, &data, &result.texture))) {
        GRAPHICS_LOG_WARNING("TextureLoader : echec de creation de la texture Direct3D 11");
        return std::nullopt;
    }
    if (FAILED(device->CreateShaderResourceView(result.texture.Get(), nullptr, &result.view))) {
        GRAPHICS_LOG_WARNING("TextureLoader : echec de creation de la vue de la texture");
        return std::nullopt;
    }
    return result;
}

// Décode un fichier image puis crée la texture Direct3D 11 correspondante.
std::optional<LoadedTexture> loadTextureFromFile(ID3D11Device* device,
                                                 const std::filesystem::path& path) {
    const std::optional<DecodedImage> decoded = decodeImageFile(path);
    if (!decoded) {
        return std::nullopt;
    }
    return createTexture(device, decoded->width, decoded->height, decoded->pixels);
}

}  // namespace hmi

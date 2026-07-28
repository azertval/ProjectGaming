#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>

/**
 * @file HMI/Graphics/TextureLoader.h
 * @brief Chargement de textures Direct3D 11 depuis des fichiers image (`EX-REN-041`).
 */

namespace hmi {

/// Pixels RGBA décodés d'un fichier image, alpha **non prémultiplié** (cohérent avec le blend
/// state de `SpriteBatch`, `D3D11_BLEND_SRC_ALPHA`), au format `R8G8B8A8_UNORM`.
struct DecodedImage {
    int width = 0;
    int height = 0;
    std::vector<std::uint32_t> pixels;
};

/// Texture Direct3D 11 chargée et sa vue de ressource (RAII, `ComPtr`).
struct LoadedTexture {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    int width = 0;
    int height = 0;
};

/**
 * @brief Décode un fichier image (PNG au minimum) en pixels RGBA.
 * @param path Chemin du fichier image.
 * @return L'image décodée, ou `std::nullopt` si le fichier est absent/illisible/format non
 *         supporté (erreur récupérable, `EX-NFR-040` — jamais d'exception ici).
 */
[[nodiscard]] std::optional<DecodedImage> decodeImageFile(const std::filesystem::path& path);

/**
 * @brief Crée une texture Direct3D 11 immuable à partir de pixels RGBA déjà décodés.
 * @param device Device Direct3D 11 (crée la texture et sa vue de ressource).
 * @param width  Largeur en pixels (doit être strictement positive).
 * @param height Hauteur en pixels (doit être strictement positive).
 * @param pixels Pixels `R8G8B8A8_UNORM`, taille attendue `width * height`.
 * @return La texture chargée, ou `std::nullopt` en cas d'échec de création côté GPU.
 */
[[nodiscard]] std::optional<LoadedTexture> createTexture(ID3D11Device* device, int width,
                                                         int height,
                                                         const std::vector<std::uint32_t>& pixels);

/**
 * @brief Décode un fichier image puis crée la texture Direct3D 11 correspondante.
 * @param device Device Direct3D 11.
 * @param path   Chemin du fichier image.
 * @return La texture chargée, ou `std::nullopt` si le décodage ou la création GPU échoue.
 */
[[nodiscard]] std::optional<LoadedTexture> loadTextureFromFile(ID3D11Device* device,
                                                               const std::filesystem::path& path);

}  // namespace hmi

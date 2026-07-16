#include "HMI/Graphics/SpriteBatch.h"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#include <d3dcompiler.h>

#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

namespace {
using Microsoft::WRL::ComPtr;

/// Lève une exception si un appel Direct3D a échoué (frontière de construction).
void throwIfFailed(HRESULT result, const char* message) {
    if (FAILED(result)) {
        throw std::runtime_error(message);
    }
}

/// Source HLSL du vertex shader : projette la position monde et transmet UV et couleur.
constexpr char kVertexShaderSource[] = R"(
cbuffer Projection : register(b0) { float4x4 uProjection; };
struct VSInput  { float2 position : POSITION; float2 uv : TEXCOORD0; float4 color : COLOR0; };
struct VSOutput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; float4 color : COLOR0; };
VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(float4(input.position, 0.0f, 1.0f), uProjection);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}
)";

/// Source HLSL du pixel shader : texture d'atlas échantillonnée, multipliée par la teinte.
constexpr char kPixelShaderSource[] = R"(
Texture2D uTexture : register(t0);
SamplerState uSampler : register(s0);
struct PSInput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; float4 color : COLOR0; };
float4 main(PSInput input) : SV_TARGET {
    return uTexture.Sample(uSampler, input.uv) * input.color;
}
)";

/**
 * @brief Compile un shader HLSL à l'exécution.
 * @param source     Code source HLSL.
 * @param entryPoint Nom de la fonction d'entrée.
 * @param target     Profil cible (ex. "vs_5_0").
 * @return Le bytecode compilé.
 */
ComPtr<ID3DBlob> compileShader(const char* source, const char* entryPoint, const char* target) {
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ComPtr<ID3DBlob> bytecode;
    ComPtr<ID3DBlob> errors;
    const HRESULT result = D3DCompile(source, std::strlen(source), nullptr, nullptr, nullptr,
                                      entryPoint, target, flags, 0, &bytecode, &errors);
    if (FAILED(result)) {
        std::string message = "Echec de compilation d'un shader";
        if (errors) {
            message += " : ";
            message.append(static_cast<const char*>(errors->GetBufferPointer()),
                           errors->GetBufferSize());
        }
        throw std::runtime_error(message);
    }
    return bytecode;
}
}  // namespace

/**
 * @brief Construit le pipeline (shaders, buffers, états de rendu).
 * @param device  Device Direct3D 11 (crée les ressources).
 * @param context Contexte immédiat (émet les commandes de dessin), non possédé.
 */
SpriteBatch::SpriteBatch(ID3D11Device* device, ID3D11DeviceContext* context) : _context(context) {
    _vertices.reserve(MAXIMUM_QUADS * 4);

    // Shaders et layout d'entrée.
    const ComPtr<ID3DBlob> vertexBytecode = compileShader(kVertexShaderSource, "main", "vs_5_0");
    const ComPtr<ID3DBlob> pixelBytecode = compileShader(kPixelShaderSource, "main", "ps_5_0");
    throwIfFailed(
        device->CreateVertexShader(vertexBytecode->GetBufferPointer(),
                                   vertexBytecode->GetBufferSize(), nullptr, &_vertexShader),
        "Echec de creation du vertex shader");
    throwIfFailed(device->CreatePixelShader(pixelBytecode->GetBufferPointer(),
                                            pixelBytecode->GetBufferSize(), nullptr, &_pixelShader),
                  "Echec de creation du pixel shader");

    const D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    throwIfFailed(device->CreateInputLayout(layout, static_cast<UINT>(std::size(layout)),
                                            vertexBytecode->GetBufferPointer(),
                                            vertexBytecode->GetBufferSize(), &_inputLayout),
                  "Echec de creation du layout d'entree");

    // Tampon de sommets dynamique (réécrit à chaque lot).
    D3D11_BUFFER_DESC vertexDescription{};
    vertexDescription.ByteWidth = static_cast<UINT>(MAXIMUM_QUADS * 4 * sizeof(Vertex));
    vertexDescription.Usage = D3D11_USAGE_DYNAMIC;
    vertexDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    throwIfFailed(device->CreateBuffer(&vertexDescription, nullptr, &_vertexBuffer),
                  "Echec de creation du tampon de sommets");

    // Tampon d'indices immuable : deux triangles par quad (0,1,2, 0,2,3).
    std::vector<std::uint16_t> indices(MAXIMUM_QUADS * 6);
    for (std::size_t quad = 0; quad < MAXIMUM_QUADS; ++quad) {
        const auto base = static_cast<std::uint16_t>(quad * 4);
        const std::size_t offset = quad * 6;
        indices[offset + 0] = static_cast<std::uint16_t>(base + 0);
        indices[offset + 1] = static_cast<std::uint16_t>(base + 1);
        indices[offset + 2] = static_cast<std::uint16_t>(base + 2);
        indices[offset + 3] = static_cast<std::uint16_t>(base + 0);
        indices[offset + 4] = static_cast<std::uint16_t>(base + 2);
        indices[offset + 5] = static_cast<std::uint16_t>(base + 3);
    }
    D3D11_BUFFER_DESC indexDescription{};
    indexDescription.ByteWidth = static_cast<UINT>(indices.size() * sizeof(std::uint16_t));
    indexDescription.Usage = D3D11_USAGE_IMMUTABLE;
    indexDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA indexData{};
    indexData.pSysMem = indices.data();
    throwIfFailed(device->CreateBuffer(&indexDescription, &indexData, &_indexBuffer),
                  "Echec de creation du tampon d'indices");

    // Tampon constant : matrice de projection.
    D3D11_BUFFER_DESC constantDescription{};
    constantDescription.ByteWidth = sizeof(DirectX::XMFLOAT4X4);
    constantDescription.Usage = D3D11_USAGE_DEFAULT;
    constantDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    throwIfFailed(device->CreateBuffer(&constantDescription, nullptr, &_constantBuffer),
                  "Echec de creation du tampon constant");

    // État de fusion alpha (transparence).
    D3D11_BLEND_DESC blendDescription{};
    blendDescription.RenderTarget[0].BlendEnable = TRUE;
    blendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    throwIfFailed(device->CreateBlendState(&blendDescription, &_blendState),
                  "Echec de creation de l'etat de fusion");

    // Échantillonnage *nearest* (pixel art net), bords fixés (clamp).
    D3D11_SAMPLER_DESC samplerDescription{};
    samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
    throwIfFailed(device->CreateSamplerState(&samplerDescription, &_samplerState),
                  "Echec de creation de l'echantillonneur");

    // Rasterizer 2D : pas de culling (les quads peuvent être vus des deux côtés).
    D3D11_RASTERIZER_DESC rasterizerDescription{};
    rasterizerDescription.FillMode = D3D11_FILL_SOLID;
    rasterizerDescription.CullMode = D3D11_CULL_NONE;
    rasterizerDescription.DepthClipEnable = TRUE;
    throwIfFailed(device->CreateRasterizerState(&rasterizerDescription, &_rasterizerState),
                  "Echec de creation de l'etat de rasterisation");
    GRAPHICS_LOG_TRACE("SpriteBatch : pipeline 2D cree (shaders, buffers, etats)");
}

/**
 * @brief Démarre un lot : configure les états et la texture, et la projection caméra.
 * @param projection Matrice de projection monde → clip (fournie par la caméra).
 * @param texture    Vue de la texture d'atlas à échantillonner (non possédée).
 */
void SpriteBatch::begin(const DirectX::XMFLOAT4X4& projection, ID3D11ShaderResourceView* texture) {
    _texture = texture;

    // HLSL lit les matrices en colonne-major : on téléverse la transposée de la matrice
    // ligne-major de DirectXMath pour que `mul(vecteur, matrice)` soit correct.
    DirectX::XMFLOAT4X4 transposed;
    DirectX::XMStoreFloat4x4(&transposed,
                             DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&projection)));
    _context->UpdateSubresource(_constantBuffer.Get(), 0, nullptr, &transposed, 0, 0);

    _context->IASetInputLayout(_inputLayout.Get());
    _context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _context->VSSetShader(_vertexShader.Get(), nullptr, 0);
    _context->VSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
    _context->PSSetShader(_pixelShader.Get(), nullptr, 0);
    _context->PSSetSamplers(0, 1, _samplerState.GetAddressOf());
    _context->PSSetShaderResources(0, 1, &_texture);

    const float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    _context->OMSetBlendState(_blendState.Get(), blendFactor, 0xffffffff);
    _context->RSSetState(_rasterizerState.Get());

    _vertices.clear();
}

/**
 * @brief Ajoute un quad au lot courant.
 * @param quad Quad à dessiner (unités monde, UV normalisées, teinte).
 */
void SpriteBatch::draw(const SpriteQuad& quad) {
    // Tampon plein : on vide avant d'ajouter (le lot reste dans le même état).
    if (_vertices.size() >= MAXIMUM_QUADS * 4) {
        flush();
    }

    const float left = quad.x;
    const float top = quad.y;
    const float right = quad.x + quad.width;
    const float bottom = quad.y + quad.height;

    // Quatre coins, dans l'ordre attendu par le tampon d'indices (haut-gauche, haut-droit,
    // bas-droit, bas-gauche). Y vers le bas : `top` < `bottom`.
    _vertices.push_back(Vertex{left, top, quad.u0, quad.v0, quad.r, quad.g, quad.b, quad.a});
    _vertices.push_back(Vertex{right, top, quad.u1, quad.v0, quad.r, quad.g, quad.b, quad.a});
    _vertices.push_back(Vertex{right, bottom, quad.u1, quad.v1, quad.r, quad.g, quad.b, quad.a});
    _vertices.push_back(Vertex{left, bottom, quad.u0, quad.v1, quad.r, quad.g, quad.b, quad.a});
}

/// Termine le lot : émet le dessin des quads accumulés.
void SpriteBatch::end() {
    flush();
}

/// Émet le dessin des quads accumulés et vide le tampon.
void SpriteBatch::flush() {
    if (_vertices.empty()) {
        return;
    }

    // Réécriture complète du tampon dynamique (WRITE_DISCARD). En cas d'échec (perte de
    // device), on saute le dessin plutôt que de lever une exception dans la boucle de jeu.
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(_context->Map(_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        return;
    }
    std::memcpy(mapped.pData, _vertices.data(), _vertices.size() * sizeof(Vertex));
    _context->Unmap(_vertexBuffer.Get(), 0);

    const UINT stride = sizeof(Vertex);
    const UINT offset = 0;
    _context->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
    _context->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    const auto indexCount = static_cast<UINT>((_vertices.size() / 4) * 6);
    _context->DrawIndexed(indexCount, 0, 0);

    _vertices.clear();
}

}  // namespace hmi

#pragma once

#include <string>
#include <vector>
#include <map>
#include <d3d11.h>
#include <Effects.h>

#include "Material.h"

#include "Common.h"

struct VertexData
{
    float vertex[3];
    float normal[3];
    float uv[2];
};

struct BatchedVertices
{
    std::string materialName;
    ID3D11Buffer* buffer;
    int count; // vertex count
    struct Material material;
};

class Model
{
public:
    static bool Load(const char* filepath, Vector3 scale, std::vector<struct BatchedVertices>& outBatchedVertices);

    static void Setup(ID3D11Device* device, ID3D11DeviceContext* deviceContext, DirectX::EffectFactory* effectFactory)
    {
        m_d3dDevice = device;
        m_d3dDeviceContext = deviceContext;
        m_d3dEffectFactory = effectFactory;
    }

    static bool HasExtension(const wchar_t* path, const std::wstring& ext);

    static int LoadTexture(const wchar_t* filepath);

    static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTexture(int textureId)
    {
        if (textureId >= 0 && textureId < m_Textures.size())
        {
            return m_Textures[textureId];
        }
        return nullptr;
    }
    static HRESULT CreateBuffer(D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* data, ID3D11Buffer** buffer);
    
    static void UnloadStaticResources()
    {
        for (auto& texture : m_Textures) {
            SafeRelease(texture);
        }
    }

private:
    static std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_Textures;

    static ID3D11Device* m_d3dDevice;
    static ID3D11DeviceContext* m_d3dDeviceContext;
    static DirectX::EffectFactory* m_d3dEffectFactory;
};
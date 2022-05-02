#include "SimpleObj.h"

#include "Window.h"
#include "Shader.h"
#include "Common.h"

using namespace Microsoft::WRL;
using namespace DirectX;

/// <summary>
/// Wrapper to load shader resources
/// </summary>
void SimpleObj::LoadShaderResources()
{
    HRESULT hr;

    // Note:
    // Since we will need to update the contents of the constant buffer in the application,
    // Instead of set the buffer's Usage property to D3D11_USAGE_DYNAMIC and the CPU AccessFlags to D3D11_CPU_ACCESS_WRITE,
    // we'll be using ID3D11DeviceContext::UpdateSubresource method,
    // which it expects constant buffers to be initialized with D3D11_USAGE_DEFAULT usage flag
    // and buffers that are created with the D3D11_USAGE_DEFAULT flag must have their CPU AccessFlags set to 0.

    // Forward Regular
    {
        ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
        std::wstring filename = L"assets/Shaders/Forward/RegularVS.hlsl";
        _int64 size = GetFileSize(filename);
        if (size != m_d3dRegularVertexShaderSize)
        {
            vertexShaderBlob = LoadShader<ID3D11VertexShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, vertexShaderBlob, nullptr, m_d3dRegularVertexShader);
            m_d3dRegularVertexShaderSize = size;

            // Create the input layout for the vertex shader.
            D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
            {
                {
                    "POSITION",                             // semantic name
                    0,                                      // semantic index
                    DXGI_FORMAT_R32G32B32_FLOAT,            // format
                    0,                                      // input slot (used for packed vertex buffers)
                    offsetof(VertexData, vertex),           // aligned byte offset
                    D3D11_INPUT_PER_VERTEX_DATA,            // input slot class
                    0                                       // additional param for slot class: D3D11_INPUT_PER_INSTANCE_DATA
                },
                {
                    "NORMAL",
                    0,
                    DXGI_FORMAT_R32G32B32_FLOAT,
                    0,
                    offsetof(VertexData, normal),
                    D3D11_INPUT_PER_VERTEX_DATA,
                    0
                },
                {
                    "TEXCOORD",
                    0,
                    DXGI_FORMAT_R32G32_FLOAT,
                    0,
                    offsetof(VertexData, uv),
                    D3D11_INPUT_PER_VERTEX_DATA,
                    0
                }
            };

            hr = m_d3dDevice->CreateInputLayout(
                vertexLayoutDesc,                           // input layout description
                _countof(vertexLayoutDesc),                 // amount of the elements
                vertexShaderBlob->GetBufferPointer(),       // pointer to the compiled shader
                vertexShaderBlob->GetBufferSize(),          // size in bytes of the compiled shader
                &m_d3dRegularInputLayout                           // pointer to the input-layout object
            );
            AssertIfFailed(hr, "Load Content", "Unable to create input layout");
        }

        // Load and compile the pixel shader
        ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
        filename = L"assets/Shaders/Forward/ForwardLighting_LoopLightPS.hlsl";
        size = GetFileSize(filename);
        if (size != m_d3dForward_LoopLight_PixelShaderSize)
        {
            pixelShaderBlob = LoadShader<ID3D11PixelShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, pixelShaderBlob, nullptr, m_d3dForward_LoopLight_PixelShader);
            m_d3dForward_LoopLight_PixelShaderSize = size;
        }
    }
    
    // Forward Instanced
    {
        ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
        std::wstring filename = L"assets/Shaders/Forward/InstancedVS.hlsl";
        _int64 size = GetFileSize(filename);
        if (size != m_d3dInstancedVertexShaderSize)
        {
            vertexShaderBlob = LoadShader<ID3D11VertexShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, vertexShaderBlob, nullptr, m_d3dInstancedVertexShader);
            m_d3dInstancedVertexShaderSize = size;

            // Create the input layout for the vertex shader.
            D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
            {
                // Per-vertex data.
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                
                // Per-instance data.
                { "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

                { "NORMALWORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "NORMALWORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "NORMALWORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "NORMALWORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

                { "NORMALVIEWMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "NORMALVIEWMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "NORMALVIEWMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "NORMALVIEWMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

                { "MATERIAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },   // Emissive
                { "MATERIAL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },   // Ambient
                { "MATERIAL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },   // Diffuse
                { "MATERIAL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },   // Specular
                { "MATERIAL", 4, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },             // UseTexture
                { "MATERIAL", 5, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },            // SpecularPower
                { "MATERIAL", 6, DXGI_FORMAT_R32G32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },         // Padding
            };

            hr = m_d3dDevice->CreateInputLayout(
                vertexLayoutDesc,                           // input layout description
                _countof(vertexLayoutDesc),                 // amount of the elements
                vertexShaderBlob->GetBufferPointer(),       // pointer to the compiled shader
                vertexShaderBlob->GetBufferSize(),          // size in bytes of the compiled shader
                &m_d3dInstancedInputLayout                           // pointer to the input-layout object
            );
            AssertIfFailed(hr, "Load Content", "Unable to create instanced input layout");
        }

        ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
        filename = L"assets/Shaders/Forward/ForwardLighting_LoopLightPS_Instanced.hlsl";
        size = GetFileSize(filename);
        if (size != m_d3dForward_LoopLight_InstancedPixelShaderSize)
        {
            pixelShaderBlob = LoadShader<ID3D11PixelShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, pixelShaderBlob, nullptr, m_d3dForward_LoopLight_InstancedPixelShader);
            m_d3dForward_LoopLight_InstancedPixelShaderSize = size;
        }
    }    

    // Deferred Geometry Regular
    {
        ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
        std::wstring filename = L"assets/Shaders/Deferred/DeferredGeometryRegularVS.hlsl";
        _int64 size = GetFileSize(filename);
        if (size != m_d3dDeferredGeometry_RegularVertexShaderSize)
        {
            vertexShaderBlob = LoadShader<ID3D11VertexShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, vertexShaderBlob, nullptr, m_d3dDeferredGeometry_RegularVertexShader);
            m_d3dDeferredGeometry_RegularVertexShaderSize = size;

            // Create the input layout for the vertex shader.
            D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
            {
                {
                    "POSITION",                             // semantic name
                    0,                                      // semantic index
                    DXGI_FORMAT_R32G32B32_FLOAT,            // format
                    0,                                      // input slot (used for packed vertex buffers)
                    offsetof(VertexData, vertex),           // aligned byte offset
                    D3D11_INPUT_PER_VERTEX_DATA,            // input slot class
                    0                                       // additional param for slot class: D3D11_INPUT_PER_INSTANCE_DATA
                },
                {
                    "NORMAL",
                    0,
                    DXGI_FORMAT_R32G32B32_FLOAT,
                    0,
                    offsetof(VertexData, normal),
                    D3D11_INPUT_PER_VERTEX_DATA,
                    0
                },
                {
                    "TEXCOORD",
                    0,
                    DXGI_FORMAT_R32G32_FLOAT,
                    0,
                    offsetof(VertexData, uv),
                    D3D11_INPUT_PER_VERTEX_DATA,
                    0
                }
            };

            hr = m_d3dDevice->CreateInputLayout(
                vertexLayoutDesc,                           // input layout description
                _countof(vertexLayoutDesc),                 // amount of the elements
                vertexShaderBlob->GetBufferPointer(),       // pointer to the compiled shader
                vertexShaderBlob->GetBufferSize(),          // size in bytes of the compiled shader
                &m_d3dDeferredGeometry_RegularInputLayout           // pointer to the input-layout object
            );
            AssertIfFailed(hr, "Load Content", "Unable to create input layout");
        }

        // Load and compile the pixel shader
        ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
        filename = L"assets/Shaders/Deferred/DeferredGeometryRegularPS.hlsl";
        size = GetFileSize(filename);
        if (size != m_d3dDeferredGeometry_RegularPixelShaderSize)
        {
            pixelShaderBlob = LoadShader<ID3D11PixelShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, pixelShaderBlob, nullptr, m_d3dDeferredGeometry_RegularPixelShader);
            m_d3dDeferredGeometry_RegularPixelShaderSize = size;
        }
    }

    // Deferred Geometry Instanced
    {
        ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
        std::wstring filename = L"assets/Shaders/Deferred/DeferredGeometryInstancedVS.hlsl";
        _int64 size = GetFileSize(filename);
        if (size != m_d3dDeferredGeometry_InstancedVertexShaderSize)
        {
            vertexShaderBlob = LoadShader<ID3D11VertexShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, vertexShaderBlob, nullptr, m_d3dDeferredGeometry_InstancedVertexShader);
            m_d3dDeferredGeometry_InstancedVertexShaderSize = size;

            // Create the input layout for the vertex shader.
            D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
            {
                // Per-vertex data.
               { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
               { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
               { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

               // Per-instance data.
               { "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
               { "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
               { "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
               { "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

               { "NORMALWORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
               { "NORMALWORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
               { "NORMALWORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
               { "NORMALWORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

               { "NORMALVIEWMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
               { "NORMALVIEWMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
               { "NORMALVIEWMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
               { "NORMALVIEWMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

               { "MATERIAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },   // Emissive
               { "MATERIAL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },   // Ambient
               { "MATERIAL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },   // Diffuse
               { "MATERIAL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },   // Specular
               { "MATERIAL", 4, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },             // UseTexture
               { "MATERIAL", 5, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },            // SpecularPower
               { "MATERIAL", 6, DXGI_FORMAT_R32G32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            };

            hr = m_d3dDevice->CreateInputLayout(
                vertexLayoutDesc,                           // input layout description
                _countof(vertexLayoutDesc),                 // amount of the elements
                vertexShaderBlob->GetBufferPointer(),       // pointer to the compiled shader
                vertexShaderBlob->GetBufferSize(),          // size in bytes of the compiled shader
                &m_d3dDeferredGeometry_InstancedInputLayout // pointer to the input-layout object
            );
            AssertIfFailed(hr, "Load Content", "Unable to create input layout");
        }

        // Load and compile the pixel shader
        ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
        filename = L"assets/Shaders/Deferred/DeferredGeometryInstancedPS.hlsl";
        size = GetFileSize(filename);
        if (size != m_d3dDeferredGeometry_InstancedPixelShaderSize)
        {
            pixelShaderBlob = LoadShader<ID3D11PixelShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, pixelShaderBlob, nullptr, m_d3dDeferredGeometry_InstancedPixelShader);
            m_d3dDeferredGeometry_InstancedPixelShaderSize = size;
        }
    }

    {
        ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
        std::wstring filename = L"assets/Shaders/DebugVS.hlsl";
        _int64 size = GetFileSize(filename);
        if (size != m_d3dDebugVertexShaderSize)
        {
            vertexShaderBlob = LoadShader<ID3D11VertexShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, vertexShaderBlob, nullptr, m_d3dDebugVertexShader);
            m_d3dDebugVertexShaderSize = size;
        }

        // Load and compile the pixel shader
        ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
        filename = L"assets/Shaders/Deferred/DebugDeferredPS.hlsl";
        size = GetFileSize(filename);
        if (size != m_d3dDebugPixelShaderSize)
        {
            pixelShaderBlob = LoadShader<ID3D11PixelShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, pixelShaderBlob, nullptr, m_d3dDebugPixelShader);
            m_d3dDebugPixelShaderSize = size;
        }
    }

    {
        ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
        std::wstring filename = L"assets/Shaders/Deferred/DeferredLightingVS.hlsl";
        _int64 size = GetFileSize(filename);
        if (size != m_d3dDeferredLightingVertexShaderSize)
        {
            vertexShaderBlob = LoadShader<ID3D11VertexShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, vertexShaderBlob, nullptr, m_d3dDeferredLightingVertexShader);
            m_d3dDeferredLightingVertexShaderSize = size;
        }

        // Load and compile the pixel shader
        ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
        filename = L"assets/Shaders/Deferred/DeferredLighting_LoopLightPS.hlsl";
        size = GetFileSize(filename);
        if (size != m_d3dDeferredLighting_LoopLight_PixelShaderSize)
        {
            pixelShaderBlob = LoadShader<ID3D11PixelShader>(m_d3dDevice, filename, "main", "latest");
            CreateShader(m_d3dDevice, pixelShaderBlob, nullptr, m_d3dDeferredLighting_LoopLight_PixelShader);
            m_d3dDeferredLighting_LoopLight_PixelShaderSize = size;
        }
    }
}

/// <summary>
/// Setup DirextXTK Effect
/// </summary>
void SimpleObj::LoadDebugDraw()
{
    HRESULT hr;

    // Prepare to setup Primitive Batcher
    m_d3dStates = std::make_unique<CommonStates>(m_d3dDevice.Get());
    m_d3dEffect = std::make_unique<BasicEffect>(m_d3dDevice.Get());
    m_d3dEffect->SetVertexColorEnabled(true);

    hr = CreateInputLayoutFromEffect<VertexPositionColor>(m_d3dDevice.Get(), m_d3dEffect.get(), &m_d3dPrimitiveBatchInputLayout);
    AssertIfFailed(hr, "Create Primitive Batch Failed", "Unable to call CreateInputLayoutFromEffect()");

    m_d3dEffectFactory = std::make_unique<EffectFactory>(m_d3dDevice.Get());

    // Create Primitive Batcher
    m_d3dPrimitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_d3dDeviceContext.Get());
}

/// <summary>
/// Setup Texture data
/// </summary>
void SimpleObj::LoadTexture()
{
    try
    {
        m_d3dEffectFactory->CreateTexture(L"assets\\Textures\\grid.png", m_d3dDeviceContext.Get(), &m_GridTexture);

        // Create a sampler state for texture sampling in the pixel shader
        D3D11_SAMPLER_DESC samplerDesc;
        ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.MinLOD = -FLT_MAX;
        samplerDesc.MaxLOD = FLT_MAX;

        HRESULT hr = m_d3dDevice->CreateSamplerState(&samplerDesc, &m_d3dSamplerState);
        AssertIfFailed(hr, "Load Texture", "Failed to create texture sampler.");
    }
    catch (std::exception&)
    {
        DisplayError("Failed to load texture: Textures\\grid.png");
    }
}

/// <summary>
/// Setup light data
/// </summary>
void SimpleObj::LoadLight()
{
    struct Light point;
    point.LightType = (int)LightType::Point;
    point.PositionWS = Vector4(-0.5, 3.0, 0.0, 1.0f);
    point.Strength = 8.0f;
    point.Enabled = true;

    struct Light directional;
    directional.LightType = (int)LightType::Directional;
    directional.PositionWS = Vector4(0.0, 6.0, 0.0, 1.0f);    
    
    auto directionV3 = Vector3(1.0, 0.5, 0.25);
    directionV3.Normalize();
    directional.DirectionWS = Vector4(directionV3.x, directionV3.y, directionV3.z, 1.0f);

    directional.Strength = 0.5f;
    directional.Enabled = true;

    struct Light spotlight;
    spotlight.LightType = (int)LightType::Spotlight;
    spotlight.PositionWS = Vector4(0.178, 4.0, 0.6, 1.0f);
    
    directionV3 = Vector3(0.079, -0.285, 0.976f);
    directionV3.Normalize();
    spotlight.DirectionWS = Vector4(directionV3.x, directionV3.y, directionV3.z, 1.0f);

    //spotlight.DirectionWS.Normalize();
    spotlight.SpotAngle = XMConvertToRadians(16.0f);
    spotlight.Strength = 75.0f;
    spotlight.Enabled = true;

    m_Scene.Lights[0] = point;
    m_Scene.Lights[1] = directional;
    m_Scene.Lights[2] = spotlight;
}

/// <summary>
/// Render render loop
/// </summary>
void SimpleObj::RenderScene(RenderEventArgs& e)
{
    switch (m_RenderMode)
    {
    case RenderMode::Forward:
        RenderScene_Forward(e);
        break;
    case RenderMode::Deferred:
        RenderScene_Deferred(e);
        break;
    }
}

/// <summary>
/// Draw wireframe for debugging
/// </summary>
void SimpleObj::RenderDebug(RenderEventArgs& e)
{
    // set target view to main RTV
    m_d3dDeviceContext->OMSetRenderTargets(
        1,                                      // number of render target to bind
        m_d3dRenderTargetView.GetAddressOf(),   // pointer to an array of render-target view
        m_d3dDepthStencilView.Get()             // pointer to depth-stencil view
    );
    m_d3dDeviceContext->OMSetDepthStencilState(
        m_d3dDepthStencilState.Get(),           // depth stencil state
        1                                       // stencil reference
    );

    m_d3dDeviceContext->OMSetBlendState(m_d3dStates->Opaque(), nullptr, 0xFFFFFFFF);
    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dStates->DepthNone(), 0);
    m_d3dDeviceContext->RSSetState(m_d3dStates->CullNone());
    m_d3dEffect->Apply(m_d3dDeviceContext.Get());

    m_d3dEffect->SetWorld(Matrix::Identity);
    m_d3dEffect->SetView(m_Camera.get_ViewMatrix());

    m_d3dDeviceContext->IASetInputLayout(m_d3dPrimitiveBatchInputLayout.Get());
    m_d3dPrimitiveBatch->Begin();
    {
        float directionalLightDebugLength = 2.0f;
        for (auto i = 0; i < MAX_LIGHTS; ++i)
        {
            auto light = &m_Scene.Lights[i];
            auto position = Vector3(light->PositionWS.x, light->PositionWS.y, light->PositionWS.z);
            auto type = (LightType)light->LightType;
            auto strength = light->Strength;

            if (type == LightType::Point)
            {
                auto constant = light->ConstantAttenuation;
                auto linear = light->LinearAttenuation;
                auto quadratic = light->QuadraticAttenuation;
                auto lightMax = std::fmaxf(std::fmaxf(light->Color.x, light->Color.y), light->Color.z) * strength;
                
                // Reference: https://learnopengl.com/Advanced-Lighting/Deferred-Shading, we use 15/256 as dark threshold
                auto radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 15.0) * lightMax))) / (2 * quadratic);
                
                auto sphere = BoundingSphere(position, radius);
                DX::Draw(m_d3dPrimitiveBatch.get(), sphere, DirectX::Colors::White);
            }

            else if (type == LightType::Directional || type == LightType::Spotlight)
            {
                auto direction = Vector3(light->DirectionWS.x, light->DirectionWS.y, light->DirectionWS.z);
                direction.Normalize();
                // use negative direction to visual actual light dir calculation in shader
                auto v1 = VertexPositionColor(position, Colors::Red);
                auto v2 = VertexPositionColor(position - direction * directionalLightDebugLength, Colors::RoyalBlue);
                m_d3dPrimitiveBatch->DrawLine(v1, v2);
            }
        }
    }
    m_d3dPrimitiveBatch->End();
}

/// <summary>
/// Initialize imgui
/// </summary>
void SimpleObj::SetupImgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(m_Window.get_WindowHandle());
    ImGui_ImplDX11_Init(m_d3dDevice.Get(), m_d3dDeviceContext.Get());
}

/// <summary>
/// Render UI
/// </summary>
void SimpleObj::RenderImgui(RenderEventArgs& e)
{
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // always set render target to main render target
    m_d3dDeviceContext->OMSetRenderTargets(
        1,                                      // number of render target to bind
        m_d3dRenderTargetView.GetAddressOf(),   // pointer to an array of render-target view
        m_d3dDepthStencilView.Get()             // pointer to depth-stencil view
    );
    m_d3dDeviceContext->OMSetDepthStencilState(
        m_d3dDepthStencilState.Get(),           // depth stencil state
        1                                       // stencil reference
    );

    // Setup UI state
    bool show_demo_window = true;
    ImGui::ShowDemoWindow(&show_demo_window);

    const float dragSpeed = 0.1f;
    const float slowDragSpeed = 0.01f;
    const float fastDragSpeed = 5.0f;

    ImGui::Text(format("Fps: %f (%f ms)", 1.0f / e.ElapsedTime, e.ElapsedTime).c_str());

    ImGui::PushID("Render Techniques");
    {
        int renderMode = (int)m_RenderMode;
        if (ImGui::Combo("Render Techniques", &renderMode, "Forward\0Deferred\0"))
        {
            m_RenderMode = (RenderMode)renderMode;
        }

        int lightingcalculation = (int)m_LightingCalculation;
        if (ImGui::Combo("Lighting Calculation", &lightingcalculation, "Loop\0Single\0"))
        {
            m_LightingCalculation = (LightingCalculation)lightingcalculation;
        }
        
        if (m_RenderMode == RenderMode::Forward)
        {
            int lightingSpace = (int)m_LightingSpace;
            if (ImGui::Combo("Lighting Space", &lightingSpace, "World Space\0View Space\0"))
            {
                m_LightingSpace = (LightingSpace)lightingSpace;
            }
        }

        int debugMode = (int)m_DeferredDebugMode;
        if (m_RenderMode == RenderMode::Deferred && 
            ImGui::Combo("Debug Mode", &debugMode, "None\0LightAccumulation\0Diffuse\0Specular\0Normal\0Depth\0"))
        {
            m_DeferredDebugMode = (Deferred_DebugMode)debugMode;
        }

        if (m_DeferredDebugMode == Deferred_DebugMode::Depth)
        {
            float scale = m_DeferredDepthPower;
            ImGui::DragFloat("Depth Scale", &scale, dragSpeed, 1.0f, 1000.0f);
            m_DeferredDepthPower = scale;
        }
    }
    ImGui::PopID();

    if (ImGui::CollapsingHeader("Global Properties"))
    {
        ImGui::PushID("##Global-Properties");

        float ambient[3] = { m_Scene.GlobalAmbient.x, m_Scene.GlobalAmbient.y, m_Scene.GlobalAmbient.z };
        ImGui::ColorEdit3("Ambient", ambient);
        m_Scene.GlobalAmbient.x = ambient[0];
        m_Scene.GlobalAmbient.y = ambient[1];
        m_Scene.GlobalAmbient.z = ambient[2];

        ImGui::PopID();
    }

    if (ImGui::CollapsingHeader("Scene List"))
    {
        auto sceneCount = m_Scene.Count();
        for (int i = 0; i < sceneCount; ++i)
        {
            auto entity = m_Scene.Entities.at(i);
            auto name = entity->Name.c_str();

            ImGui::PushID(format("##Entity:%d-%s", i, name).c_str());

            if (ImGui::TreeNode(name))
            {
                if (ImGui::Button("Reset"))
                {
                    entity->Rotation.x = 0;
                    entity->Rotation.y = 0;
                    entity->Rotation.z = 0;
                    entity->Rotation.w = 0;

                    entity->RotateAxisSpeed.x = 0;
                    entity->RotateAxisSpeed.y = 0;
                    entity->RotateAxisSpeed.z = 0;
                }

                ImGui::SameLine();
                if (ImGui::Button("Gizmo"))
                {
                    m_ShowGizmoWindow = true;
                    m_GizmoWindowNameGetter = [&entity]()
                    {
                        return entity->Name.c_str();
                    };
                    m_GizmoWindowQuaternionGetter = [&entity]()
                    {
                        return quat(entity->Rotation.w, entity->Rotation.x, entity->Rotation.y, entity->Rotation.z);
                    };
                    m_GizmoWindowQuaternionSetter = [&entity](quat value)
                    {
                        entity->Rotation.x = value.x;
                        entity->Rotation.y = value.y;
                        entity->Rotation.z = value.z;
                        entity->Rotation.w = value.w;
                    };
                }

                float position[3] = { entity->PositionWS.x , entity->PositionWS.y , entity->PositionWS.z };
                ImGui::DragFloat3("Position", position, dragSpeed);
                entity->PositionWS.x = position[0];
                entity->PositionWS.y = position[1];
                entity->PositionWS.z = position[2];

                Vector3 v_rotation = entity->Rotation.ToEuler();
                float rotation[3] = { v_rotation.x, v_rotation.y, v_rotation.z };
                ImGui::DragFloat3("Rotation", rotation, dragSpeed);
                entity->Rotation = Quaternion::CreateFromYawPitchRoll(Vector3(rotation));

                Vector3 v_rotationAxisSpeed = entity->RotateAxisSpeed;
                float rotationAxisSpeed[3] = { v_rotationAxisSpeed.x, v_rotationAxisSpeed.y, v_rotationAxisSpeed.z };
                ImGui::DragFloat3("Rotate Axis Speed", rotationAxisSpeed, slowDragSpeed);
                entity->RotateAxisSpeed.x = rotationAxisSpeed[0];
                entity->RotateAxisSpeed.y = rotationAxisSpeed[1];
                entity->RotateAxisSpeed.z = rotationAxisSpeed[2];

                float emissive[3] = { entity->Material.Emissive.x, entity->Material.Emissive.y, entity->Material.Emissive.z };
                ImGui::ColorEdit3("Emissive", emissive);
                entity->Material.Emissive.x = emissive[0];
                entity->Material.Emissive.y = emissive[1];
                entity->Material.Emissive.z = emissive[2];

                float ambient[3] = { entity->Material.Ambient.x, entity->Material.Ambient.y, entity->Material.Ambient.z };
                ImGui::ColorEdit3("Ambient", ambient);
                entity->Material.Ambient.x = ambient[0];
                entity->Material.Ambient.y = ambient[1];
                entity->Material.Ambient.z = ambient[2];

                float diffuse[3] = { entity->Material.Diffuse.x, entity->Material.Diffuse.y, entity->Material.Diffuse.z };
                ImGui::ColorEdit3("Diffuse", diffuse);
                entity->Material.Diffuse.x = diffuse[0];
                entity->Material.Diffuse.y = diffuse[1];
                entity->Material.Diffuse.z = diffuse[2];

                float specular[3] = { entity->Material.Specular.x, entity->Material.Specular.y, entity->Material.Specular.z };
                ImGui::ColorEdit3("Specular", specular);
                entity->Material.Specular.x = specular[0];
                entity->Material.Specular.y = specular[1];
                entity->Material.Specular.z = specular[2];

                bool UseTexture = entity->Material.UseTexture == 1;
                ImGui::Checkbox("UseTexture", &UseTexture);
                entity->Material.UseTexture = UseTexture ? 1 : 0;

                float specularPower = entity->Material.SpecularPower;
                ImGui::DragFloat("Specular Power", &specularPower, fastDragSpeed, 5.0f, 128.0f);
                entity->Material.SpecularPower = specularPower;

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Light List"))
    {
        for (int i = 0; i < MAX_LIGHTS; ++i)
        {
            auto light = &m_Scene.Lights[i];
            auto lightName = format("Light (%d)", i);
            auto name = lightName.c_str();

            auto id = format("##Light:%d", i);
            ImGui::PushID(id.c_str());

            if (ImGui::TreeNode(name))
            {
                bool enabled = light->Enabled == 1;
                ImGui::Checkbox("Enabled", &enabled);
                light->Enabled = enabled ? 1 : 0;

                if ((LightType)light->LightType == LightType::Directional || (LightType)light->LightType == LightType::Spotlight)
                {
                    ImGui::SameLine();
                    if (ImGui::Button("Direction"))
                    {
                        m_ShowDirectionWindow = true;
                        m_DirectionWindowNameGetter = [lightName]() { return lightName.c_str(); };
                        m_DirectionWindowVec3Getter = [i, light]()
                        {
                            return vec3(light->DirectionWS.x, light->DirectionWS.y, light->DirectionWS.z);
                        };
                        m_DirectionWindowVec3Setter = [i, light](vec3 value)
                        {
                            light->DirectionWS.x = value.x;
                            light->DirectionWS.y = value.y;
                            light->DirectionWS.z = value.z;
                        };
                    }
                }

                if ((LightType)light->LightType == LightType::Spotlight)
                {
                    float spotAngle = XMConvertToDegrees(light->SpotAngle);
                    ImGui::DragFloat("SpotAngle", &spotAngle, dragSpeed, 0.0f, 60.0f);
                    light->SpotAngle = XMConvertToRadians(spotAngle);
                }

                float position[3] = { light->PositionWS.x, light->PositionWS.y, light->PositionWS.z };
                ImGui::DragFloat3("Position", position, dragSpeed);
                light->PositionWS.x = position[0];
                light->PositionWS.y = position[1];
                light->PositionWS.z = position[2];

                float rotation[3] = { light->DirectionWS.x, light->DirectionWS.y, light->DirectionWS.z };
                ImGui::DragFloat3("Rotation", rotation, dragSpeed);
                light->DirectionWS.x = rotation[0];
                light->DirectionWS.y = rotation[1];
                light->DirectionWS.z = rotation[2];
                float strength = light->Strength;
                ImGui::DragFloat("Strength", &strength, dragSpeed, 0.0f, 100.0f);
                light->Strength = strength;

                int style_idx = light->LightType;
                if (ImGui::Combo("Type", &style_idx, "Directional\0Point\0SpotLight\0"))
                {
                    switch (style_idx)
                    {
                    case 0: light->LightType = 0; break;
                    case 1: light->LightType = 1; break;
                    case 2: light->LightType = 2; break;
                    default: break;
                    }
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

    if (m_ShowGizmoWindow)
    {
        if (ImGui::Begin(format("Gizmo: %s", m_GizmoWindowNameGetter()).c_str(), &m_ShowGizmoWindow))
        {
            quat qRot = m_GizmoWindowQuaternionGetter();
            ImGui::gizmo3D("##gizmo1", qRot, 300);
            m_GizmoWindowQuaternionSetter(qRot);
        }
        ImGui::End();
    }

    if (m_ShowDirectionWindow)
    {
        if (ImGui::Begin(format("Direction: %s", m_DirectionWindowNameGetter()).c_str(), &m_ShowDirectionWindow))
        {
            vec3 light = m_DirectionWindowVec3Getter();
            light.y *= -1.0f; // coordinate conversion
            light.z *= -1.0f; // coordinate conversion
            ImGui::gizmo3D("##Dir1", light, 300);
            light.y *= -1.0f;
            //light.x *= -1.0f; // coordinate conversion
            light.z *= -1.0f; // coordinate conversion
            m_DirectionWindowVec3Setter(light);
        }
        ImGui::End();
    }

    // Actual Rendering
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

SimpleObj::SimpleObj(Window& window)
    : base(window)
    , m_W(0)
    , m_A(0)
    , m_S(0)
    , m_D(0)
    , m_Q(0)
    , m_E(0)
    , m_bShift(false)
    , m_Pitch(0.0f)
    , m_Yaw(0.0f)
{
    m_Scene.Add(new Entity("cornelBox", "assets/Models/cornelBox.obj", Vector3(0, 0, 0), Quaternion::CreateFromYawPitchRoll(0, 0, 0), boxMaterial));
    m_Scene.Add(new Entity("bunny", "assets/Models/bunny.obj", Vector3(4.5, 0, -4.5), Quaternion::Identity, bunny1Material, true));
    m_Scene.Add(new Entity("bunny", "assets/Models/bunny.obj", Vector3(-4.5, 0, 1.0), Quaternion::CreateFromYawPitchRoll(2.7, 0, 0), bunny2Material, true));

    XMVECTOR cameraPos = XMVectorSet(0, 7.5, 25, 1);
    XMVECTOR cameraTarget = XMVectorSet(0, 7, 25, 1);
    XMVECTOR cameraUp = XMVectorSet(0, 1, 0, 0);
    
    m_Pitch = 0.0f;
    m_Yaw = 180.0f;
    m_Camera.set_LookAt(cameraPos, cameraTarget, cameraUp);

    // Setup camera initlal position
    pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);
    pData->m_InitialCameraPos = m_Camera.get_Translation();
    pData->m_InitialCameraRot = m_Camera.get_Rotation();

    // debug 
    // m_RenderMode = RenderMode::Deferred;
    // m_DeferredDebugMode = Deferred_DebugMode::Depth;
}

SimpleObj::~SimpleObj()
{
    // ComPtr and smart pointer will automatically release itselves

    _aligned_free(pData);
}

void SimpleObj::OnUpdate(UpdateEventArgs& e)
{
    // Update camera position
    float speedMultipler = (m_bShift ? 8.0f : 4.0f);

    XMVECTOR cameraTranslate = XMVectorSet(static_cast<float>(m_D - m_A), 0.0f, static_cast<float>(m_W - m_S), 1.0f) * speedMultipler * e.ElapsedTime;
    XMVECTOR cameraPan = XMVectorSet(0.0f, static_cast<float>(m_E - m_Q), 0.0f, 1.0f) * speedMultipler * e.ElapsedTime;
    m_Camera.Translate(cameraTranslate, Camera::LocalSpace);
    m_Camera.Translate(cameraPan, Camera::WorldSpace);

    XMVECTOR cameraRotation = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_Pitch), XMConvertToRadians(m_Yaw), 0.0f);
    m_Camera.set_Rotation(cameraRotation);

    Matrix viewMatrix = m_Camera.get_ViewMatrix();
    Matrix viewProjectionMatrix = viewMatrix * m_Camera.get_ProjectionMatrix();
    for (auto entity : m_Scene.Entities)
    {
        // update angle
        // entity->RotationAngle += deltaTime * entity->RotateSpeed;
        entity->Rotation *= Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), entity->RotateAxisSpeed.x);
        entity->Rotation *= Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), entity->RotateAxisSpeed.y);
        entity->Rotation *= Quaternion::CreateFromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), entity->RotateAxisSpeed.z);

        auto model = Matrix::Identity;
        model = Matrix::CreateFromYawPitchRoll(entity->Rotation.ToEuler()) * Matrix::CreateTranslation(entity->PositionWS);
        auto modelView = model * viewMatrix;
        entity->WorldMatrix = model;
        entity->InverseTransposeWorldMatrix = model.Transpose().Invert();
        entity->InverseTransposeWorldViewMatrix = modelView.Transpose().Invert();
        entity->WorldViewProjectionMatrix = model * viewProjectionMatrix;
    }

    for (auto &light : m_Scene.Lights)
    {
        auto PositionVS = Vector3(Vector4::Transform(light.PositionWS, viewMatrix));
        light.PositionVS = Vector4(PositionVS.x, PositionVS.y, PositionVS.z, 1.0f);
        
        auto directionVS = Vector3(Vector4::Transform(light.DirectionWS, viewMatrix));
        directionVS.Normalize();
        light.DirectionVS = Vector4(directionVS.x, directionVS.y, directionVS.z, 1.0f);

        /*
        if (light.LightType != (int)LightType::Directional) continue;
        if (!light.Enabled) continue;
        std::cout << "light.DirectionWS = ("
            << light.DirectionWS.x << ", "
            << light.DirectionWS.y << ", "
            << light.DirectionWS.z << ");" << std::endl;
        std::cout << "light.DirectionVS = ("
            << light.DirectionVS.x << ", "
            << light.DirectionVS.y << ", "
            << light.DirectionVS.z << ");" << std::endl;
        std::cout << std::endl;
        */
    }
}

void SimpleObj::Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil)
{
    base::Clear(clearColor, clearDepth, clearStencil);

    // GBuffer lightAccumulation is clear using clearColor, while others clear with pure black
    m_d3dDeviceContext->ClearRenderTargetView(m_d3dRenderTargetView_lightAccumulation.Get(), clearColor);

    m_d3dDeviceContext->ClearDepthStencilView(m_d3dDepthStencilView_depth.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);

    ID3D11RenderTargetView* renderTargetViews[] =
    {
        m_d3dRenderTargetView_diffuse.Get(),
        m_d3dRenderTargetView_specular.Get(),
        m_d3dRenderTargetView_normal.Get(),
    };

    const FLOAT black[4] = { 0, 0, 0, 1 };
    for (auto rtv : renderTargetViews) {
        m_d3dDeviceContext->ClearRenderTargetView(rtv, black);
    }
}

void SimpleObj::OnRender(RenderEventArgs& e)
{
    Clear(DirectX::Colors::CornflowerBlue, 1.0f, 0);

    // Setup Frame CB
    m_FrameConstantBuffer.ViewMatrix = m_Camera.get_ViewMatrix();
    m_FrameConstantBuffer.ProjectionMatrix = m_Camera.get_ProjectionMatrix();
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Frame].Get(), 0, nullptr, &m_FrameConstantBuffer, 0, 0);

    // Setup Light CB
    m_LightPropertiesConstantBuffer.EyePosition = Vector4(m_Camera.get_Translation());
    m_LightPropertiesConstantBuffer.GlobalAmbient = m_Scene.GlobalAmbient;
    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        m_LightPropertiesConstantBuffer.Lights[i] = m_Scene.Lights[i];
    }
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Light].Get(), 0, nullptr, &m_LightPropertiesConstantBuffer, 0, 0);

    // update Debug CB
    m_DebugPropertiesConstantBuffer.DeferredDebugMode = (int)m_DeferredDebugMode;
    m_DebugPropertiesConstantBuffer.DeferredDepthPower = m_DeferredDepthPower;
    m_DebugPropertiesConstantBuffer.LightingSpace = (int)m_LightingSpace;
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Debug].Get(), 0, nullptr, &m_DebugPropertiesConstantBuffer, 0, 0);

    // update ScreenToViewParams CB
    m_ScreenToViewParamsConstantBuffer.InverseView = m_Camera.get_InverseViewMatrix();
    m_ScreenToViewParamsConstantBuffer.InverseProjection = m_Camera.get_InverseProjectionMatrix();
    m_ScreenToViewParamsConstantBuffer.ScreenDimensions = m_ScreenDimensions;
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_ScreenToViewParams].Get(), 0, nullptr, &m_ScreenToViewParamsConstantBuffer, 0, 0);

    RenderScene(e);
    RenderDebug(e);
    RenderImgui(e);
    Present();
}

void SimpleObj::OnKeyPressed(KeyEventArgs& e)
{
    base::OnKeyPressed(e);

    switch (e.Key)
    {
    case KeyCode::Escape:
    {
        // Close the window associated with this demo.
        m_Window.Destroy();
    }
    break;
    case KeyCode::W:
    {
        m_W = 1;
    }
    break;
    case KeyCode::Left:
    case KeyCode::A:
    {
        m_A = 1;
    }
    break;
    case KeyCode::S:
    {
        m_S = 1;
    }
    break;
    case KeyCode::Right:
    case KeyCode::D:
    {
        m_D = 1;
    }
    break;
    case KeyCode::Down:
    case KeyCode::Q:
    {
        m_Q = 1;
    }
    break;
    case KeyCode::Up:
    case KeyCode::E:
    {
        m_E = 1;
    }
    break;
    case KeyCode::R:
    {
        // Reset camera position and orientation
        m_Camera.set_Translation(pData->m_InitialCameraPos);
        m_Camera.set_Rotation(pData->m_InitialCameraRot);
        m_Pitch = 0.0f;
        m_Yaw = 0.0f;
    }
    break;
    case KeyCode::ShiftKey:
    {
        m_bShift = true;
    }
    break;
    case KeyCode::Space:
    {
    }
    break;
    }

    // print camera position
    auto position = m_Camera.get_Translation();
    XMFLOAT4 p; XMStoreFloat4(&p, position);
    printf("Camera position: (%f, %f, %f)\n", p.x, p.y, p.z);
}

void SimpleObj::OnKeyReleased(KeyEventArgs& e)
{
    base::OnKeyReleased(e);

    switch (e.Key)
    {
    case KeyCode::W:
    {
        m_W = 0;
    }
    break;
    case KeyCode::Left:
    case KeyCode::A:
    {
        m_A = 0;
    }
    break;
    case KeyCode::S:
    {
        m_S = 0;
    }
    break;
    case KeyCode::Right:
    case KeyCode::D:
    {
        m_D = 0;
    }
    break;
    case KeyCode::Q:
    case KeyCode::Down:
    {
        m_Q = 0;
    }
    break;
    case KeyCode::E:
    case KeyCode::Up:
    {
        m_E = 0;
    }
    break;
    case KeyCode::ShiftKey:
    {
        m_bShift = false;
    }
    break;
    }
}

void SimpleObj::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    base::OnMouseButtonPressed(e);

    m_PreviousMousePosition = XMINT2(e.X, e.Y);
}

// No minus operator for vector types in the DirectX Math library? I guess we'll create our own!
XMINT2 operator-(const XMINT2& x0, const XMINT2& x1)
{
    return XMINT2(x0.x - x1.x, x0.y - x1.y);
}

void SimpleObj::OnMouseMoved(MouseMotionEventArgs& e)
{
    base::OnMouseMoved(e);

    const float moveSpeed = 0.1f;

    XMINT2 currentMousePosition = XMINT2(e.X, e.Y);
    XMINT2 mouseDelta = m_PreviousMousePosition - currentMousePosition;
    m_PreviousMousePosition = currentMousePosition;

    if (e.LeftButton)
    {
        m_Pitch -= mouseDelta.y * moveSpeed;
        m_Yaw -= mouseDelta.x * moveSpeed;

        // print camera position
        printf("Camera Rotation: (P, Y) = (%f, %f)\n\n", m_Pitch, m_Yaw);
    }
}

void SimpleObj::OnMouseWheel(MouseWheelEventArgs& e)
{
    base::OnMouseWheel(e);
}

void SimpleObj::OnResize(ResizeEventArgs& e)
{
    // Don't forget to call the base class's resize method.
    // The base class handles resizing of the swap chain.
    base::OnResize(e);
    ResizeSwapChain(e.Width, e.Height);

    if (e.Height < 1)
    {
        e.Height = 1;
    }

    float aspectRatio = e.Width / (float)e.Height;

    m_Camera.set_Projection(fovInDegree, aspectRatio, nearPlane, farPlane);
    Matrix projectionMatrix = m_Camera.get_ProjectionMatrix();
    m_d3dEffect->SetProjection(projectionMatrix);

    m_ScreenDimensions = Vector2(e.Width, e.Height);
    
    // Setup the viewports for the camera.
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<FLOAT>(m_Window.get_ClientWidth());
    viewport.Height = static_cast<FLOAT>(m_Window.get_ClientHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_Camera.set_Viewport(viewport);

    m_d3dDeviceContext->RSSetViewports(1, &viewport);
}

bool SimpleObj::ResizeSwapChain(int width, int height)
{
    // Don't allow for 0 size swap chain buffers.
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;

    HRESULT hr;

    m_d3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    // First release the render target and depth/stencil views.
    m_d3dRenderTargetView_lightAccumulation.Reset();
    m_d3dRenderTargetView_diffuse.Reset();
    m_d3dRenderTargetView_specular.Reset();
    m_d3dRenderTargetView_normal.Reset();
    m_d3dDepthStencilView_depth.Reset();

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    {
        hr = m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_d3dRenderTargetView_lightAccumulation_tex);
        AssertIfFailed(hr, "Failed to create texture", "m_d3dRenderTargetView_lightAccumulation_tex");

        hr = m_d3dDevice->CreateShaderResourceView(
            m_d3dRenderTargetView_lightAccumulation_tex.Get(),
            &shaderResourceViewDesc,
            &m_d3dRenderTargetView_lightAccumulation_SRV
        );
        AssertIfFailed(hr, "Failed to create shader resource view", "m_d3dRenderTargetView_lightAccumulation_view");

        hr = m_d3dDevice->CreateRenderTargetView(
            m_d3dRenderTargetView_lightAccumulation_tex.Get(),
            &renderTargetViewDesc,
            &m_d3dRenderTargetView_lightAccumulation
        );
        AssertIfFailed(hr, "Failed to create render target view", "m_d3dRenderTargetView_lightAccumulation");
    }

    {
        hr = m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_d3dRenderTargetView_diffuse_tex);
        AssertIfFailed(hr, "Failed to create texture", "m_d3dRenderTargetView_diffuse_tex");

        hr = m_d3dDevice->CreateShaderResourceView(
            m_d3dRenderTargetView_diffuse_tex.Get(),
            &shaderResourceViewDesc,
            &m_d3dRenderTargetView_diffuse_SRV
        );
        AssertIfFailed(hr, "Failed to create shader resource view", "m_d3dRenderTargetView_diffuse_view");

        hr = m_d3dDevice->CreateRenderTargetView(
            m_d3dRenderTargetView_diffuse_tex.Get(),
            &renderTargetViewDesc,
            &m_d3dRenderTargetView_diffuse
        );
        AssertIfFailed(hr, "Failed to create render target view", "m_d3dRenderTargetView_diffuse");
    }

    {
        hr = m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_d3dRenderTargetView_specular_tex);
        AssertIfFailed(hr, "Failed to create texture", "m_d3dRenderTargetView_specular_tex");

        hr = m_d3dDevice->CreateShaderResourceView(
            m_d3dRenderTargetView_specular_tex.Get(),
            &shaderResourceViewDesc,
            &m_d3dRenderTargetView_specular_SRV
        );
        AssertIfFailed(hr, "Failed to create shader resource view", "m_d3dRenderTargetView_specular_view");

        hr = m_d3dDevice->CreateRenderTargetView(
            m_d3dRenderTargetView_specular_tex.Get(),
            &renderTargetViewDesc,
            &m_d3dRenderTargetView_specular
        );
        AssertIfFailed(hr, "Failed to create render target view", "m_d3dRenderTargetView_specular");
    }

    {
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        renderTargetViewDesc.Format = textureDesc.Format;
        shaderResourceViewDesc.Format = textureDesc.Format;

        hr = m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_d3dRenderTargetView_normal_tex);
        AssertIfFailed(hr, "Failed to create texture", "m_d3dRenderTargetView_normal_tex");

        hr = m_d3dDevice->CreateShaderResourceView(
            m_d3dRenderTargetView_normal_tex.Get(),
            &shaderResourceViewDesc,
            &m_d3dRenderTargetView_normal_SRV
        );
        AssertIfFailed(hr, "Failed to create shader resource view", "m_d3dRenderTargetView_normal_view");

        hr = m_d3dDevice->CreateRenderTargetView(
            m_d3dRenderTargetView_normal_tex.Get(),
            &renderTargetViewDesc,
            &m_d3dRenderTargetView_normal
        );
        AssertIfFailed(hr, "Failed to create render target view", "m_d3dRenderTargetView_normal");
    }

    {
        textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // compatable to DXGI_FORMAT_D24_UNORM_S8_UINT
        textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        
        shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        
        hr = m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_d3dDepthStencilView_depth_tex);
        AssertIfFailed(hr, "Failed to create the Depth/Stencil texture.", "m_d3dDepthStencilView_depth_tex");

        hr = m_d3dDevice->CreateShaderResourceView(
            m_d3dDepthStencilView_depth_tex.Get(),
            &shaderResourceViewDesc,
            &m_d3dDepthStencilView_depth_SRV
        );
        AssertIfFailed(hr, "Failed to create depth stencil view", "m_d3dDepthStencilView_depth_view");

        hr = m_d3dDevice->CreateDepthStencilView(m_d3dDepthStencilView_depth_tex.Get(), &depthStencilViewDesc, &m_d3dDepthStencilView_depth);
        AssertIfFailed(hr, "Failed to create DepthStencilView.", "m_d3dDepthStencilView_depth");
    }
    
    return true;
}

bool SimpleObj::LoadContent()
{
    AssertIfFailed(m_d3dDevice == nullptr, "Load Content", "Device is null");

    HRESULT hr;

    for (auto entity : m_Scene.Entities)
    {
        entity->Model = new Model();
        entity->Model->Load(entity->ModelPath.c_str());

        auto key = entity->Model->Key();
        if (!Model::ContainsVertexBuffer(key))
        {
            // Create an initialize the vertex buffer.
            D3D11_BUFFER_DESC vertexBufferDesc;
            ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
            vertexBufferDesc.ByteWidth = sizeof(VertexData) * entity->Model->VertexCount();    // size of the buffer in bytes
            vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;                                           // how the buffer is expected to be read from and written to
            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;                                  // how the buffer will be bound to the pipeline
            vertexBufferDesc.CPUAccessFlags = 0;                                                    // no CPI access is necessary

            D3D11_SUBRESOURCE_DATA resourceData;
            ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
            resourceData.pSysMem = entity->Model->Head();                                   // pointer to the data to initialize the buffer with
            resourceData.SysMemPitch = 0;                                                   // distance from the beginning of one line of a texture to the nextline.
                                                                                            // No used for now.
            resourceData.SysMemSlicePitch = 0;                                              // distance from the beginning of one depth level to the next. 
                                                                                            // no used for now.
            ID3D11Buffer* buffer = nullptr;
            hr = m_d3dDevice->CreateBuffer(
                &vertexBufferDesc,                                                          // buffer description
                &resourceData,                                                              // pointer to the initialization data
                &buffer                                                                     // pointer to the created buffer object
            );
            std::string message = "Unable to create vertex buffer of " + entity->ModelPath;
            AssertIfFailed(hr, "Load Content", message.c_str());

            Model::AddVertexBuffer(key, buffer);
        }
    }

    for (auto pair : m_Scene.InstancedEntity)
    {
        auto key = pair.first;
        auto instanceCount = pair.second.size();

        std::vector<InstancedObjectConstantBuffer> instanceData;
        for (auto const& instancedEntity : pair.second)
        {
            instanceData.push_back({
                instancedEntity->WorldMatrix,
                instancedEntity->InverseTransposeWorldMatrix,
                instancedEntity->InverseTransposeWorldViewMatrix,
                instancedEntity->Material
                });
        }

        // Create the per-instance vertex buffer.
        D3D11_BUFFER_DESC instanceBufferDesc;
        ZeroMemory(&instanceBufferDesc, sizeof(D3D11_BUFFER_DESC));

        instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        instanceBufferDesc.ByteWidth = sizeof(InstancedObjectConstantBuffer) * instanceCount;
        instanceBufferDesc.CPUAccessFlags = 0;
        instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = instanceData.data();
        resourceData.SysMemPitch = 0;
        resourceData.SysMemSlicePitch = 0;

        ID3D11Buffer* buffer;
        hr = m_d3dDevice->CreateBuffer(&instanceBufferDesc, &resourceData, &buffer);

        Model::AddInstancedVertexBuffer(key, buffer);
    }

    {
        D3D11_BUFFER_DESC frameConstantBufferDesc;
        ZeroMemory(&frameConstantBufferDesc, sizeof(D3D11_BUFFER_DESC));
        frameConstantBufferDesc.ByteWidth = sizeof(struct FrameConstantBuffer);
        frameConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        frameConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        frameConstantBufferDesc.CPUAccessFlags = 0;

        hr = m_d3dDevice->CreateBuffer(&frameConstantBufferDesc, nullptr, &m_d3dConstantBuffers[CB_Frame]);
        AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_Frame");

        D3D11_BUFFER_DESC objectConstantBufferDesc;
        ZeroMemory(&objectConstantBufferDesc, sizeof(D3D11_BUFFER_DESC));
        objectConstantBufferDesc.ByteWidth = sizeof(struct ObjectConstantBuffer);
        objectConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        objectConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        objectConstantBufferDesc.CPUAccessFlags = 0;

        hr = m_d3dDevice->CreateBuffer(&objectConstantBufferDesc, nullptr, &m_d3dConstantBuffers[CB_Object]);
        AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_Object");

        D3D11_BUFFER_DESC materialConstantBufferDesc;
        ZeroMemory(&materialConstantBufferDesc, sizeof(D3D11_BUFFER_DESC));
        materialConstantBufferDesc.ByteWidth = sizeof(struct MaterialProperties);
        materialConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        materialConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        materialConstantBufferDesc.CPUAccessFlags = 0;

        hr = m_d3dDevice->CreateBuffer(&materialConstantBufferDesc, nullptr, &m_d3dConstantBuffers[CB_Material]);
        AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_Material");

        D3D11_BUFFER_DESC lightConstantBufferDesc;
        ZeroMemory(&lightConstantBufferDesc, sizeof(D3D11_BUFFER_DESC));
        lightConstantBufferDesc.ByteWidth = sizeof(struct LightProperties);
        lightConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        lightConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        lightConstantBufferDesc.CPUAccessFlags = 0;

        hr = m_d3dDevice->CreateBuffer(&lightConstantBufferDesc, nullptr, &m_d3dConstantBuffers[CB_Light]);
        AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_Light");

        D3D11_BUFFER_DESC debugConstantBufferDesc;
        ZeroMemory(&debugConstantBufferDesc, sizeof(D3D11_BUFFER_DESC));
        debugConstantBufferDesc.ByteWidth = sizeof(struct DebugProperties);
        debugConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        debugConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        debugConstantBufferDesc.CPUAccessFlags = 0;

        hr = m_d3dDevice->CreateBuffer(&debugConstantBufferDesc, nullptr, &m_d3dConstantBuffers[CB_Debug]);
        AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_Light");

        D3D11_BUFFER_DESC screenToViewParamsConstantBufferDesc;
        ZeroMemory(&screenToViewParamsConstantBufferDesc, sizeof(D3D11_BUFFER_DESC));
        screenToViewParamsConstantBufferDesc.ByteWidth = sizeof(struct ScreenToViewParams);
        screenToViewParamsConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        screenToViewParamsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        screenToViewParamsConstantBufferDesc.CPUAccessFlags = 0;

        hr = m_d3dDevice->CreateBuffer(&screenToViewParamsConstantBufferDesc, nullptr, &m_d3dConstantBuffers[CB_ScreenToViewParams]);
        AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_ScreenToViewParams");
    }

    LoadShaderResources();
    LoadLight();
    LoadDebugDraw();
    LoadTexture();

    // Force a resize event so the camera's projection matrix gets initialized.
    ResizeEventArgs resizeEventArgs(m_Window.get_ClientWidth(), m_Window.get_ClientHeight());
    OnResize(resizeEventArgs);

    SetupImgui();

    return true;
}

void SimpleObj::UnloadContent()
{
    Model::UnloadStaticResources();
}
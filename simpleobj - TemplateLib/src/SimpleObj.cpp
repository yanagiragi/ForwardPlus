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

    // Load and compile the vertex shader
    ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
    std::wstring filename = L"assets/Shaders/SimpleVS.hlsl";
    _int64 size = GetFileSize(filename);
    if (size != m_d3dVertexShaderSize)
    {
        vertexShaderBlob = LoadShader<ComPtr<ID3D11VertexShader>>(m_d3dDevice, filename, "main", "latest");
        m_d3dVertexShader = CreateShader<ComPtr<ID3D11VertexShader>>(m_d3dDevice, vertexShaderBlob, nullptr);
        m_d3dVertexShaderSize = size;

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
            &m_d3dInputLayout                           // pointer to the input-layout object
        );
        AssertIfFailed(hr, "Load Content", "Unable to create input layout");

        // After creating input layouy, the shader blob is no longer needed
        // SafeRelease(vertexShaderBlob);
    }

    // Load and compile the pixel shader
    ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
    filename = L"assets/Shaders/SimplePS.hlsl";
    size = GetFileSize(filename);
    if (size != m_d3dPixelShaderSize)
    {
        pixelShaderBlob = LoadShader<ComPtr<ID3D11PixelShader>>(m_d3dDevice, filename, "main", "latest");
        m_d3dPixelShader = CreateShader<ComPtr<ID3D11PixelShader>>(m_d3dDevice, pixelShaderBlob, nullptr);
        // SafeRelease(pixelShaderBlob);
        m_d3dPixelShaderSize = size;
    }

    // Load and compile the vertex shader
    filename = L"assets/Shaders/InstancedVS.hlsl";
    size = GetFileSize(filename);
    if (size != m_d3dInstancedVertexShaderSize)
    {
        vertexShaderBlob = LoadShader<ComPtr<ID3D11VertexShader>>(m_d3dDevice, filename, "main", "latest");
        m_d3dInstancedVertexShader = CreateShader<ComPtr<ID3D11VertexShader>>(m_d3dDevice, vertexShaderBlob, nullptr);
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
            
            { "NORMALMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "NORMALMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "NORMALMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "NORMALMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

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

        // After creating input layout, the shader blob is no longer needed
        // SafeRelease(vertexShaderBlob);
    }

    pixelShaderBlob = nullptr;
    filename = L"assets/Shaders/InstancedPS.hlsl";
    size = GetFileSize(filename);
    if (size != m_d3dInstancedPixelShaderSize)
    {
        pixelShaderBlob = LoadShader<ComPtr<ID3D11PixelShader>>(m_d3dDevice, filename, "main", "latest");
        m_d3dInstancedPixelShader = CreateShader<ComPtr<ID3D11PixelShader>>(m_d3dDevice, pixelShaderBlob, nullptr);
        // SafeRelease(pixelShaderBlob);
        m_d3dInstancedPixelShaderSize = size;
    }
}

/// <summary>
/// Setup DirextXTK Effect
/// </summary>
void SimpleObj::LoadEffect()
{
    HRESULT hr;

    // Prepare to setup Primitive Batcher
    m_d3dStates = std::make_unique<CommonStates>(m_d3dDevice.Get());
    m_d3dEffect = std::make_unique<BasicEffect>(m_d3dDevice.Get());
    m_d3dEffect->SetVertexColorEnabled(true);

    hr = CreateInputLayoutFromEffect<VertexPositionColor>(m_d3dDevice.Get(), m_d3dEffect.get(), &m_d3dPrimitiveBatchInputLayout);
    AssertIfFailed(hr, "Create Primitive Batch Failed", "Unable to call CreateInputLayoutFromEffect()");

    // setup projection matrix for effect
    m_d3dEffect->SetProjection(m_ApplicationConstantBuffer.projectionMatrix);

    m_d3dEffectFactory = std::make_unique<EffectFactory>(m_d3dDevice.Get());
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
    point.Position = Vector4(-0.5, 3.0, 0.0, 1.0f);
    point.Strength = 1.0f;
    point.Enabled = true;

    struct Light directional;
    directional.LightType = (int)LightType::Directional;
    directional.Position = Vector4(0.0, 6.0, 0.0, 1.0f);
    directional.Direction = Vector4(1.0, 0.5, 0.25, 1.0f);
    directional.Strength = 0.5f;
    directional.Enabled = true;

    struct Light spotlight;
    spotlight.LightType = (int)LightType::Spotlight;
    spotlight.Position = Vector4(0.178, 4.0, 0.6, 1.0f);
    spotlight.Direction = Vector4(0.079, -0.285, 0.976f, 1.0f);
    spotlight.SpotAngle = XMConvertToRadians(16.0f);
    spotlight.Strength = 1.0f;
    spotlight.Enabled = true;

    m_FrameConstantBuffer.lights[0] = point;
    m_FrameConstantBuffer.lights[1] = directional;
    m_FrameConstantBuffer.lights[2] = spotlight;
}

/// <summary>
/// Render render loop
/// </summary>
void SimpleObj::RenderScene()
{
    AssertIfNull(m_d3dDevice, "Render Scene", "Device is null");
    AssertIfNull(m_d3dDeviceContext, "Render Scene", "Device Context is null");

    // Setup the input assembler stage
    m_d3dDeviceContext->IASetInputLayout(m_d3dInputLayout.Get());
    m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Setup the vertex shader stage
    m_d3dDeviceContext->VSSetShader(
        m_d3dVertexShader.Get(),                // pointer to vertex shader
        nullptr,                                // pointer to an array of class-instance interfaces, NULL means shader does not use any interface
        0                                       // number of class-instance interfaces of previous param
    );
    m_d3dDeviceContext->VSSetConstantBuffers(
        0,                                      // start slot
        3,                                      // number of buffers
        m_d3dConstantBuffers->GetAddressOf()    // array of constant buffers
    );

    // Setup the rasterizer stage
    m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());
    D3D11_VIEWPORT viewport = m_Camera.get_Viewport();
    m_d3dDeviceContext->RSSetViewports(
        1,                                      // numbers of the viewport to bind
        &viewport                               // array of viewport
    );

    // Setup the pixel stage stage
    m_d3dDeviceContext->PSSetShader(m_d3dPixelShader.Get(), nullptr, 0);
    m_d3dDeviceContext->PSSetConstantBuffers(
        0,                                      // start slot
        3,                                      // number of buffers
        m_d3dConstantBuffers->GetAddressOf()    // array of constant buffers
    );

    ComPtr<ID3D11SamplerState> samplerStates[] = { m_d3dSamplerState };
    m_d3dDeviceContext->PSSetSamplers(
        0,                                      // start slot
        1,                                      // number of sampler states
        samplerStates->GetAddressOf()           // array of sampler states
    );

    ComPtr<ID3D11ShaderResourceView> textures[] = { m_GridTexture };
    m_d3dDeviceContext->PSSetShaderResources(
        0,                                      // start slot
        1,                                      // number of resources
        textures->GetAddressOf()                // array of resources
    );

    // Setup the output merger stage
    m_d3dDeviceContext->OMSetRenderTargets(
        1,                                      // number of render target to bind
        m_d3dRenderTargetView.GetAddressOf(),   // pointer to an array of render-target view
        m_d3dDepthStencilView.Get()             // pointer to depth-stencil view
    );
    m_d3dDeviceContext->OMSetDepthStencilState(
        m_d3dDepthStencilState.Get(),           // depth stencil state
        1                                       // stencil reference
    );

    // Draw Regular Entities
    {
        UINT vertexStride = sizeof(VertexData);
        UINT offset = 0;
        for (auto entity : m_Scene.Entities)
        {
            if (entity->Instanced)
                continue;

            // bind ConstantBuffers at object level
            m_ObjectConstantBuffer = entity->ConstantBuffer;
            m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Object].Get(), 0, nullptr, &m_ObjectConstantBuffer, 0, 0);

            auto vertexBuffer = entity->Model->VertexBuffer();
            m_d3dDeviceContext->IASetVertexBuffers(
                0,                                      // start slot, should equal to slot we use when CreateInputLayout in LoadContent()
                1,                                      // number of vertex buffers in the array
                &vertexBuffer,                          // pointer to an array of vertex buffers
                &vertexStride,                          // pointer to stride values
                &offset                                 // pointer to offset values
            );
            m_d3dDeviceContext->Draw(
                entity->Model->VertexCount(),
                0
            );
        }
    }
    
    // Draw Instanced Entities
    {
        m_d3dDeviceContext->VSSetShader(m_d3dInstancedVertexShader.Get(), nullptr, 0);
        m_d3dDeviceContext->IASetInputLayout(m_d3dInstancedInputLayout.Get());

        m_d3dDeviceContext->PSSetShader(m_d3dInstancedPixelShader.Get(), nullptr, 0);

        const UINT vertexStride[2] = { sizeof(VertexData), sizeof(ObjectConstantBuffer) };
        const UINT offset[2] = { 0, 0 };
        std::vector<ObjectConstantBuffer> instanceData;
        for (auto const& pair : m_Scene.InstancedEntity)
        {
            auto key = pair.first;
            auto verticesCount = Model::GetVertexCount(key);
            auto size = pair.second.size();

            instanceData.clear();
            for (auto const& instancedEntity : pair.second)
            {
                instanceData.emplace_back(instancedEntity->ConstantBuffer);
            }

            // update perInstanceBuffer
            m_d3dDeviceContext->UpdateSubresource(Model::GetInstancedVertexBuffer(key), 0, nullptr, instanceData.data(), 0, 0);

            ID3D11Buffer* buffers[2] = { Model::GetVertexBuffer(key), Model::GetInstancedVertexBuffer(key) };
            m_d3dDeviceContext->IASetVertexBuffers(0, 2, buffers, vertexStride, offset);

            m_d3dDeviceContext->DrawInstanced(
                verticesCount,
                size,
                0,
                0
            );
        }
    }
}

/// <summary>
/// Draw wireframe for debugging
/// </summary>
void SimpleObj::RenderDebug()
{
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
            auto light = &m_FrameConstantBuffer.lights[i];
            auto position = Vector3(light->Position.x, light->Position.y, light->Position.z);
            auto type = (LightType)light->LightType;
            auto strength = light->Strength;

            if (type == LightType::Point)
            {
                auto radius = sqrtf(strength);
                auto sphere = BoundingSphere(position, radius);
                DX::Draw(m_d3dPrimitiveBatch.get(), sphere, DirectX::Colors::White);
            }

            else if (type == LightType::Directional || type == LightType::Spotlight)
            {
                auto direction = Vector3(light->Direction.x, light->Direction.y, light->Direction.z);
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

    // Setup UI state
    bool show_demo_window = true;
    ImGui::ShowDemoWindow(&show_demo_window);

    const float dragSpeed = 0.1f;
    const float slowDragSpeed = 0.01f;
    const float fastDragSpeed = 5.0f;

    ImGui::Text(format("Fps: %f (%f ms)", 1.0f / e.ElapsedTime, e.ElapsedTime).c_str());

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

                float position[3] = { entity->Position.x , entity->Position.y , entity->Position.z };
                ImGui::DragFloat3("Position", position, dragSpeed);
                entity->Position.x = position[0];
                entity->Position.y = position[1];
                entity->Position.z = position[2];

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

                float specularPower = entity->ConstantBuffer.Material.SpecularPower;
                ImGui::DragFloat("Specular Power", &specularPower, fastDragSpeed, 5.0f, 512.0f);
                entity->ConstantBuffer.Material.SpecularPower = specularPower;

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Light List"))
    {
        for (int i = 0; i < MAX_LIGHTS; ++i)
        {
            auto light = &m_FrameConstantBuffer.lights[i];
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
                            return vec3(light->Direction.x, light->Direction.y, light->Direction.z);
                        };
                        m_DirectionWindowVec3Setter = [i, light](vec3 value)
                        {
                            light->Direction.x = value.x;
                            light->Direction.y = value.y;
                            light->Direction.z = value.z;
                        };
                    }
                }

                if ((LightType)light->LightType == LightType::Spotlight)
                {
                    float spotAngle = XMConvertToDegrees(light->SpotAngle);
                    ImGui::DragFloat("SpotAngle", &spotAngle, dragSpeed, 0.0f, 60.0f);
                    light->SpotAngle = XMConvertToRadians(spotAngle);
                }

                float position[3] = { light->Position.x, light->Position.y, light->Position.z };
                ImGui::DragFloat3("Position", position, dragSpeed);
                light->Position.x = position[0];
                light->Position.y = position[1];
                light->Position.z = position[2];

                float rotation[3] = { light->Direction.x, light->Direction.y, light->Direction.z };
                ImGui::DragFloat3("Rotation", rotation, dragSpeed);
                light->Direction.x = rotation[0];
                light->Direction.y = rotation[1];
                light->Direction.z = rotation[2];
                float strength = light->Strength;
                ImGui::DragFloat("Strength", &strength, dragSpeed, 0.0f, 5.0f);
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
    m_Scene.Add(new Entity("bunny", "assets/Models/bunny.obj", Vector3(4.5, 0, -4.5), Quaternion::Identity, defaultMaterial, true));
    m_Scene.Add(new Entity("bunny", "assets/Models/bunny.obj", Vector3(-4.5, 0, 1.0), Quaternion::CreateFromYawPitchRoll(2.7, 0, 0), defaultMaterial, true));

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
}

SimpleObj::~SimpleObj()
{
    // ComPtr and smart pointer will automatically release itselves
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

    // view matrix
    m_FrameConstantBuffer.viewMatrix = m_Camera.get_ViewMatrix();
    XMStoreFloat4(&m_FrameConstantBuffer.eyePosition, m_Camera.get_Translation());
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Frame].Get(), 0, nullptr, &m_FrameConstantBuffer, 0, 0);

    for (auto entity : m_Scene.Entities)
    {
        // update angle
        // entity->RotationAngle += deltaTime * entity->RotateSpeed;
        entity->Rotation *= Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), entity->RotateAxisSpeed.x);
        entity->Rotation *= Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), entity->RotateAxisSpeed.y);
        entity->Rotation *= Quaternion::CreateFromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), entity->RotateAxisSpeed.z);

        auto model = Matrix::Identity;
        model = Matrix::CreateFromYawPitchRoll(entity->Rotation.ToEuler()) * Matrix::CreateTranslation(entity->Position);
        entity->ConstantBuffer.WorldMatrix = model;
        entity->ConstantBuffer.NormalMatrix = model.Transpose().Invert();
    }
}

void SimpleObj::OnRender(RenderEventArgs& e)
{
    Clear(DirectX::Colors::CornflowerBlue, 1.0f, 0);

    RenderScene();
    RenderDebug();

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

    if (e.Height < 1)
    {
        e.Height = 1;
    }

    float aspectRatio = e.Width / (float)e.Height;

    m_Camera.set_Projection(fov, aspectRatio, nearPlane, farPlane);

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

    // update initial object cb for debugging, should bind buffer each frame
    for (auto entity : m_Scene.Entities)
    {
        if (!entity->Instanced) continue;

        // update angle
        // entity->RotationAngle += deltaTime * entity->RotateSpeed;
        entity->Rotation *= Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), entity->RotateAxisSpeed.x);
        entity->Rotation *= Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), entity->RotateAxisSpeed.y);
        entity->Rotation *= Quaternion::CreateFromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), entity->RotateAxisSpeed.z);

        auto model = Matrix::Identity;
        model = Matrix::CreateFromYawPitchRoll(entity->Rotation.ToEuler()) * Matrix::CreateTranslation(entity->Position);
        entity->ConstantBuffer.WorldMatrix = model;
        entity->ConstantBuffer.NormalMatrix = model.Transpose().Invert();

        entity->ConstantBuffer.Material.SpecularPower = 512.0f;
    }

    for (auto pair : m_Scene.InstancedEntity)
    {
        auto key = pair.first;

        std::vector<ObjectConstantBuffer> instanceData;
        for (auto const& instancedEntity : pair.second)
        {
            instanceData.emplace_back(instancedEntity->ConstantBuffer);
        }

        // Create the per-instance vertex buffer.
        D3D11_BUFFER_DESC instanceBufferDesc;
        ZeroMemory(&instanceBufferDesc, sizeof(D3D11_BUFFER_DESC));

        instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        instanceBufferDesc.ByteWidth = sizeof(ObjectConstantBuffer) * pair.second.size();
        instanceBufferDesc.CPUAccessFlags = 0;
        instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = instanceData.data();
        resourceData.SysMemPitch = 0;
        resourceData.SysMemSlicePitch = 0;

        ID3D11Buffer* d3dPlaneInstanceBuffer;
        hr = m_d3dDevice->CreateBuffer(&instanceBufferDesc, &resourceData, &d3dPlaneInstanceBuffer);

        Model::AddInstancedVertexBuffer(key, d3dPlaneInstanceBuffer);
    }

    // Create the constant buffers for the variables defined in the vertex shader.
    D3D11_BUFFER_DESC applicationConstantBufferDesc;
    ZeroMemory(&applicationConstantBufferDesc, sizeof(D3D11_BUFFER_DESC));
    applicationConstantBufferDesc.ByteWidth = sizeof(struct ApplicationConstantBuffer);
    applicationConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    applicationConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    applicationConstantBufferDesc.CPUAccessFlags = 0;

    hr = m_d3dDevice->CreateBuffer(&applicationConstantBufferDesc, nullptr, &m_d3dConstantBuffers[CB_Application]);
    AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_Application");

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

    LoadShaderResources();

    // Setup the projection matrix.
    RECT clientRect;
    GetClientRect(m_Window.get_WindowHandle(), &clientRect);

    // Compute the exact client dimensions, which is required for a correct projection matrix.
    float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
    float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

    // Setup projection matrix in LH coordinates
    m_ApplicationConstantBuffer.projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.f, float(clientWidth) / float(clientHeight), nearPlane, farPlane);

    // DirectXTK default uses RH coordintate, don't use it
    // m_ApplicationConstantBuffer.projectionMatrix = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI / 4.f, float(clientWidth) / float(clientHeight), 0.1f, 100.f);

    m_d3dDeviceContext->UpdateSubresource(
        m_d3dConstantBuffers[CB_Application].Get(),     // pointer to the destination resource
        0,                                              // zero-based index that identifies the destination subresource
        nullptr,                                        // pointer to the box that defines the portion of the destination subresource
                                                        // to copy the resource data into
        &m_ApplicationConstantBuffer,                   // pointer to the source data in memory
        0,                                              // size of one row of the source data
        0                                               // size of one depth slice of source data
    );

    LoadLight();

    LoadEffect();

    LoadTexture();

    // Create Primitive Batcher
    m_d3dPrimitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_d3dDeviceContext.Get());
    
    // Force a resize event so the camera's projection matrix gets initialized.
    ResizeEventArgs resizeEventArgs(m_Window.get_ClientWidth(), m_Window.get_ClientHeight());
    OnResize(resizeEventArgs);

    SetupImgui();

    return true;
}

void SimpleObj::UnloadContent()
{

}
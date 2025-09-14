#include "SimpleObj.h"
#include <set>

using namespace Microsoft::WRL;
using namespace Yr;

void SimpleObj::RenderScene_Deferred_GeometryPass()
{
    AssertIfNull(m_d3dDevice, "Render Scene", "Device is null");
    AssertIfNull(m_d3dDeviceContext, "Render Scene", "Device Context is null");

    // Setup the output merger 
    ID3D11RenderTargetView* renderTargetViews[] = {
        m_d3dRenderTargetView_lightAccumulation.Get(),
        m_d3dRenderTargetView_diffuse.Get(),
        m_d3dRenderTargetView_specular.Get(),
        m_d3dRenderTargetView_normal.Get()
    };

    m_d3dDeviceContext->OMSetRenderTargets(_countof(renderTargetViews), renderTargetViews, m_d3dDepthStencilView_depth.Get());
    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);

    // set blend state to no blend
    m_d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);

    std::function<void(ComPtr<ID3D11ShaderResourceView>[])> bindTextureDelegate = [&](ComPtr<ID3D11ShaderResourceView> textures[]) -> void {
        m_d3dDeviceContext->PSSetShaderResources(
            0,                                      // start slot
            MAX_TEXTURES,                           // number of resources
            textures->GetAddressOf()                // array of resources
        );
    };

    // Draw Regular Entities
    {
        // Setup the input assembler stage
        m_d3dDeviceContext->IASetInputLayout(m_d3dDeferredGeometry_RegularInputLayout.Get());
        m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Setup the vertex shader stage
        m_d3dDeviceContext->VSSetShader(m_d3dDeferredGeometry_RegularVertexShader.Get(), nullptr, 0);
        m_d3dDeviceContext->VSSetConstantBuffers(0, 1, m_d3dConstantBuffers[CB_Object].GetAddressOf());

        // Setup the pixel stage stage
        m_d3dDeviceContext->PSSetShader(m_d3dDeferredGeometry_RegularPixelShader.Get(), nullptr, 0);
        ID3D11Buffer* pixelShaderConstantBuffers[] = { m_d3dConstantBuffers[CB_Material].Get(), m_d3dConstantBuffers[CB_Light].Get() };
        m_d3dDeviceContext->PSSetConstantBuffers(
            0,                                      // start slot
            2,                                      // number of buffers
            pixelShaderConstantBuffers              // array of constant buffers
        );

        ComPtr<ID3D11SamplerState> samplerStates[] = { m_d3dSamplerState };
        m_d3dDeviceContext->PSSetSamplers(
            0,                                      // start slot
            1,                                      // number of sampler states
            samplerStates->GetAddressOf()           // array of sampler states
        );

        DrawRegularEntities(bindTextureDelegate);
    }

    // Draw Instanced Entities
    {
        m_d3dDeviceContext->IASetInputLayout(m_d3dDeferredGeometry_InstancedInputLayout.Get());

        m_d3dDeviceContext->VSSetShader(m_d3dDeferredGeometry_InstancedVertexShader.Get(), nullptr, 0);
        ID3D11Buffer* vertexShaderConstantBuffers[] = { m_d3dConstantBuffers[CB_Frame].Get() };
        m_d3dDeviceContext->VSSetConstantBuffers(
            0,                                      // start slot
            _countof(vertexShaderConstantBuffers),  // number of buffers
            vertexShaderConstantBuffers             // array of constant buffers
        );

        m_d3dDeviceContext->PSSetShader(m_d3dDeferredGeometry_InstancedPixelShader.Get(), nullptr, 0);
        ID3D11Buffer* pixelShaderConstantBuffers[] = { m_d3dConstantBuffers[CB_Light].Get() };
        m_d3dDeviceContext->PSSetConstantBuffers(
            0,                                      // start slot
            _countof(pixelShaderConstantBuffers),   // number of buffers
            pixelShaderConstantBuffers              // array of constant buffers
        );

        DrawInstancedEntities(bindTextureDelegate);
    }

    ID3D11RenderTargetView* pRTVs[] = { nullptr, nullptr, nullptr, nullptr };
    m_d3dDeviceContext->OMSetRenderTargets(_countof(pRTVs), pRTVs, nullptr);
}

void SimpleObj::RenderScene_Deferred_DebugPass()
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

    // Setup the rasterizer stage
    m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());
    D3D11_VIEWPORT viewport = m_Camera.get_Viewport();
    m_d3dDeviceContext->RSSetViewports(1, &viewport);

    // Setup the vertex shader stage
    m_d3dDeviceContext->VSSetShader(m_d3dDebugVertexShader.Get(), nullptr, 0);

    // Setup the pixel stage stage
    m_d3dDeviceContext->PSSetShader(m_d3dDebugPixelShader.Get(), nullptr, 0);

    // Setup pixel shader cb
    m_d3dDeviceContext->PSSetConstantBuffers(
        0,
        1,
        m_d3dConstantBuffers[CB_Debug].GetAddressOf()
    );

    // Setup the input assembler stage
    m_d3dDeviceContext->IASetInputLayout(nullptr);
    m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // setup textures
    ComPtr<ID3D11SamplerState> samplerStates[] = { m_d3dSamplerState };
    m_d3dDeviceContext->PSSetSamplers(
        0,                                      // start slot
        1,                                      // number of sampler states
        samplerStates->GetAddressOf()           // array of sampler states
    );

    ComPtr<ID3D11ShaderResourceView> textures[] =
    {
        m_d3dRenderTargetView_lightAccumulation_SRV,
        m_d3dRenderTargetView_diffuse_SRV,
        m_d3dRenderTargetView_specular_SRV,
        m_d3dRenderTargetView_normal_SRV,
        m_d3dDepthStencilView_depth_SRV,
    };
    m_d3dDeviceContext->PSSetShaderResources(
        0,                                      // start slot
        _countof(textures),                     // number of resources
        textures->GetAddressOf()                // array of resources
    );

    Draw(4, 0);

    // Unbind SRVs
    ID3D11ShaderResourceView* const pSRV[5] = { NULL, NULL, NULL, NULL, NULL };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(pSRV), pSRV);
}

void SimpleObj::RenderScene_Deferred_LightingPass_Loop()
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

    // Setup the rasterizer stage
    m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());
    D3D11_VIEWPORT viewport = m_Camera.get_Viewport();
    m_d3dDeviceContext->RSSetViewports(1, &viewport);

    // Setup the vertex shader stage
    m_d3dDeviceContext->VSSetShader(m_d3dDeferredLightingVertexShader.Get(), nullptr, 0);

    // Setup the pixel stage stage
    m_d3dDeviceContext->PSSetShader(m_d3dDeferredLighting_LoopLight_PixelShader.Get(), nullptr, 0);
    ID3D11Buffer* buffers[] = {
        m_d3dConstantBuffers[CB_Light].Get(),
        m_d3dConstantBuffers[CB_ScreenToViewParams].Get()
    };
    m_d3dDeviceContext->PSSetConstantBuffers(
        0,
        _countof(buffers),
        buffers
    );

    // Setup the input assembler stage
    m_d3dDeviceContext->IASetInputLayout(nullptr);
    m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // don't use any blend state
    m_d3dDeviceContext->OMSetBlendState(NULL, nullptr, 0xffffffff);

    // setup textures
    ComPtr<ID3D11SamplerState> samplerStates[] = { m_d3dSamplerState };
    m_d3dDeviceContext->PSSetSamplers(
        0,                                      // start slot
        1,                                      // number of sampler states
        samplerStates->GetAddressOf()           // array of sampler states
    );

    ComPtr<ID3D11ShaderResourceView> textures[] =
    {
        m_d3dRenderTargetView_lightAccumulation_SRV,
        m_d3dRenderTargetView_diffuse_SRV,
        m_d3dRenderTargetView_specular_SRV,
        m_d3dRenderTargetView_normal_SRV,
        m_d3dDepthStencilView_depth_SRV
    };
    m_d3dDeviceContext->PSSetShaderResources(
        0,                                      // start slot
        _countof(textures),                     // number of resources
        textures->GetAddressOf()                // array of resources
    );

    Draw(4, 0);

    // Unbind SRVs
    ID3D11ShaderResourceView* const pSRV[5] = { NULL, NULL, NULL, NULL, NULL };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(pSRV), pSRV);
}

void SimpleObj::DrawLightVolume(Light* light)
{
    UINT vertexStride = sizeof(VertexData);
    UINT offset = 0;
    Matrix viewProjectionMatrix = m_Camera.get_ViewMatrix() * m_Camera.get_ProjectionMatrix();

    if (light->LightType == (int)LightType::Point)
    {
        // Setup the vertex shader stage
        m_d3dDeviceContext->VSSetShader(m_d3dDeferredLighting_LightVolume_VertexShader.Get(), nullptr, 0);
        m_d3dDeviceContext->VSSetConstantBuffers(0, 1, m_d3dConstantBuffers[CB_Object].GetAddressOf());

        // Setup the input assembler stage
        m_d3dDeviceContext->IASetInputLayout(m_d3dRegularInputLayout.Get());
        m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Setup CB, leave InverseTransposeWorldMatrix, InverseTransposeWorldViewMatrix non - updated
        auto model = Matrix::CreateScale(light->Range) * Matrix::CreateTranslation(Vector3(light->PositionWS)); // point light has no rotation
        auto WorldViewProjectionMatrix = model * viewProjectionMatrix;
        m_ObjectConstantBuffer.WorldMatrix = model;
        m_ObjectConstantBuffer.WorldViewProjectionMatrix = WorldViewProjectionMatrix;
        m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Object].Get(), 0, nullptr, &m_ObjectConstantBuffer, 0, 0);

        m_d3dDeviceContext->IASetVertexBuffers(
            0,                                      // start slot, should equal to slot we use when CreateInputLayout in LoadContent()
            1,                                      // number of vertex buffers in the array
            &m_lightVolume_sphere.buffer,                          // pointer to an array of vertex buffers
            &vertexStride,                          // pointer to stride values
            &offset                                 // pointer to offset values
        );

        Draw(m_lightVolume_sphere.count, 0);
    }

    else if (light->LightType == (int)LightType::Spotlight)
    {
        // Setup the vertex shader stage
        m_d3dDeviceContext->VSSetShader(m_d3dDeferredLighting_LightVolume_VertexShader.Get(), nullptr, 0);
        m_d3dDeviceContext->VSSetConstantBuffers(0, 1, m_d3dConstantBuffers[CB_Object].GetAddressOf());

        // Setup the input assembler stage
        m_d3dDeviceContext->IASetInputLayout(m_d3dRegularInputLayout.Get());
        m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // hard code number of the model
        const float baseRange = 2.0f;
        const float baseRadius = 1.0; // spot angle = 0.463647609 rad ~= 26.35 degrees
        
        const float coneRadius = tan(light->SpotAngle) * light->Range;
        Matrix model = Matrix::CreateScale(coneRadius / baseRadius, coneRadius / baseRadius, light->Range / baseRange) * Matrix::CreateWorld(Vector3(light->PositionWS), Vector3(light->DirectionWS), Vector3::Up);

        Matrix WorldViewProjectionMatrix = model * viewProjectionMatrix;
        m_ObjectConstantBuffer.WorldViewProjectionMatrix = WorldViewProjectionMatrix;
        m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Object].Get(), 0, nullptr, &m_ObjectConstantBuffer, 0, 0);

        m_d3dDeviceContext->IASetVertexBuffers(
            0,                                      // start slot, should equal to slot we use when CreateInputLayout in LoadContent()
            1,                                      // number of vertex buffers in the array
            &m_lightVolume_cone.buffer,                          // pointer to an array of vertex buffers
            &vertexStride,                          // pointer to stride values
            &offset                                 // pointer to offset values
        );

        Draw(m_lightVolume_cone.count, 0);
    }

    else if (light->LightType == (int)LightType::Directional)
    {
        // backup current state since we'll draw full screen without using any stencil states
        ComPtr<ID3D11DepthStencilState> depthStencilState;
        UINT stencilRef;
        m_d3dDeviceContext->OMGetDepthStencilState(depthStencilState.GetAddressOf(), &stencilRef);

        ComPtr<ID3D11RasterizerState> rasterizerState;
        m_d3dDeviceContext->RSGetState(rasterizerState.GetAddressOf());

        // override state
        m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);
        m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());

        // Setup the vertex shader stage
        m_d3dDeviceContext->VSSetShader(m_d3dDeferredLightingVertexShader.Get(), nullptr, 0);

        // Setup the input assembler stage
        m_d3dDeviceContext->IASetInputLayout(nullptr);
        m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        Draw(4, 0);

        // restore state
        m_d3dDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), stencilRef);
        m_d3dDeviceContext->RSSetState(rasterizerState.Get());
    }
}

void SimpleObj::RenderScene_Deferred_LightingPass_Stencil()
{
    if (m_DeferredDebugMode == Deferred_DebugMode::LightVolume)
    {
        Clear(DirectX::Colors::CornflowerBlue, 1.0, 0);

        m_d3dDeviceContext->OMSetBlendState(NULL, nullptr, 0xffffffff);

        // Setup target view to main RTV
        m_d3dDeviceContext->OMSetRenderTargets(1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get());

        // Setup depth state
        m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);
        
        D3D11_VIEWPORT viewport = m_Camera.get_Viewport();
        m_d3dDeviceContext->RSSetViewports(1, &viewport);

        // Setup the pixel stage stage
        m_d3dDeviceContext->PSSetShader(m_d3dUnlitPixelShader.Get(), nullptr, 0);

        for (int i = 0; i < m_LightCalculationCount; ++i)
        {
            auto light = &m_Scene.Lights[i];
            if (!light->Enabled)
            {
                continue;
            }

            // skip draw directional light light volume when debugging since it is a full screen quad and covers every thing
            if (light->LightType == (int)LightType::Directional)
            {
                continue;
            }

            m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());
            
            DrawLightVolume(light);
        }

        return;
    }

    Matrix viewProjectionMatrix = m_Camera.get_ViewMatrix() * m_Camera.get_ProjectionMatrix();
    UINT vertexStride = sizeof(VertexData);
    UINT offset = 0;

    // Copy lightAccumulation to main RTV
    Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;
    m_d3dRenderTargetView.Get()->GetResource(backBuffer.GetAddressOf());
    m_d3dDeviceContext->CopyResource(backBuffer.Get(), m_d3dRenderTargetView_lightAccumulation_tex.Get());
    backBuffer.Reset();

    m_d3dDepthStencilView.Get()->GetResource(backBuffer.GetAddressOf());
    m_d3dDeviceContext->CopyResource(backBuffer.Get(), m_d3dDepthStencilView_depth_tex.Get());
    backBuffer.Reset();
    
    for (int i = 0; i < m_LightCalculationCount; ++i)
    {
        auto light = &m_Scene.Lights[i];
        if (!light->Enabled)
        {
            continue;
        }

        m_LightingCalculationOptionsConstrantBuffer.LightIndex = i;
        m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_LightCalculationOptions].Get(), 0, nullptr, &m_LightingCalculationOptionsConstrantBuffer, 0, 0);

        // 1. Clear Stencil buffer to 1
        {
            m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);

            m_d3dDeviceContext->ClearDepthStencilView(m_d3dDepthStencilView.Get(), D3D11_CLEAR_STENCIL, 1.0, 1);
        }

        // 2. Unmark pixels in front of the near light boundary
        {
            // Since we don't need to output pixels, set render target color attachment to null
            m_d3dDeviceContext->OMSetRenderTargets(0, nullptr, m_d3dDepthStencilView_depth.Get());

            // Unmark pixels
            m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState_UnmarkPixels.Get(), 1);

            // Cull back
            m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());

            // reset blend state
            m_d3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
            
            // Setup viewport
            D3D11_VIEWPORT viewport = m_Camera.get_Viewport();
            m_d3dDeviceContext->RSSetViewports(1, &viewport);

            // No pixel shader is required
            m_d3dDeviceContext->PSSetShader(nullptr, nullptr, 0);

            // skip directional mask since we always draw full screen quad
            if (light->LightType != (int)LightType::Directional)
            {
                DrawLightVolume(light);
            }
        }

        // 3. Shade pixels that are in front of the far light boundary
        {
            // Set main RTV
            m_d3dDeviceContext->OMSetRenderTargets(1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get());

            // Shader pixels with stencil ref = 1
            m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState_ShadePixels.Get(), 1);

            // Cull front face and disable Depth Clipping
            m_d3dDeviceContext->RSSetState(m_d3dCullFrontRasterizerState.Get());

            // disable depth test and use additive blend
            m_d3dDeviceContext->OMSetBlendState(m_d3dBlendState_Add.Get(), nullptr, 0xffffffff);
            
            // Setup viewport
            D3D11_VIEWPORT viewport = m_Camera.get_Viewport();
            m_d3dDeviceContext->RSSetViewports(1, &viewport);

            // Setup the pixel stage stage
            m_d3dDeviceContext->PSSetShader(m_d3dDeferredLighting_SingleLight_PixelShader.Get(), nullptr, 0);
            
            ID3D11Buffer* buffers[] =
            {
                m_d3dConstantBuffers[CB_Light].Get(),
                m_d3dConstantBuffers[CB_ScreenToViewParams].Get(),
                m_d3dConstantBuffers[CB_LightCalculationOptions].Get()
            };
            m_d3dDeviceContext->PSSetConstantBuffers(0, _countof(buffers), buffers);

            // setup sampler & textures
            ComPtr<ID3D11ShaderResourceView> textures[] =
            {
                m_d3dRenderTargetView_lightAccumulation_SRV,
                m_d3dRenderTargetView_diffuse_SRV,
                m_d3dRenderTargetView_specular_SRV,
                m_d3dRenderTargetView_normal_SRV,
                m_d3dDepthStencilView_depth_SRV
            };
            m_d3dDeviceContext->PSSetShaderResources(0, _countof(textures), textures->GetAddressOf());
            m_d3dDeviceContext->PSSetSamplers(0, 1, m_d3dSamplerState.GetAddressOf());

            // override pixle shader for debugging
            if (m_DeferredDebugMode == Deferred_DebugMode::LightVolumeExact)
            {
                if (light->LightType == (int)LightType::Directional)
                {
                    continue;
                }

                m_d3dDeviceContext->PSSetShader(m_d3dUnlitPixelShader.Get(), nullptr, 0);
            }

            DrawLightVolume(light);

            // Unbind SRVs
            ID3D11ShaderResourceView* const pSRV[5] = { NULL, NULL, NULL, NULL, NULL };
            m_d3dDeviceContext->PSSetShaderResources(0, _countof(pSRV), pSRV);
        }
    }

    // reset to default states
    m_d3dDeviceContext->OMSetDepthStencilState(nullptr, 1);
    m_d3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

    // Unbind RTVs
    ID3D11RenderTargetView* nullRTV = nullptr;
    m_d3dDeviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void SimpleObj::RenderScene_Deferred_LightingPass_Single()
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

    // Setup the rasterizer stage
    m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());
    
    D3D11_VIEWPORT viewport = m_Camera.get_Viewport();
    m_d3dDeviceContext->RSSetViewports(1, &viewport);

    // Setup the vertex shader stage
    m_d3dDeviceContext->VSSetShader(m_d3dDeferredLightingVertexShader.Get(), nullptr, 0);

    // Setup the input assembler stage
    m_d3dDeviceContext->IASetInputLayout(nullptr);
    m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // Setup the pixel stage stage
    m_d3dDeviceContext->PSSetShader(m_d3dDeferredLighting_SingleLight_PixelShader.Get(), nullptr, 0);
    ID3D11Buffer* buffers[] =
    {
        m_d3dConstantBuffers[CB_Light].Get(),
        m_d3dConstantBuffers[CB_ScreenToViewParams].Get(),
        m_d3dConstantBuffers[CB_LightCalculationOptions].Get()
    };
    m_d3dDeviceContext->PSSetConstantBuffers(0, _countof(buffers), buffers);

    // setup textures
    ComPtr<ID3D11SamplerState> samplerStates[] = { m_d3dSamplerState };
    m_d3dDeviceContext->PSSetSamplers(0, 1, samplerStates->GetAddressOf());

    ComPtr<ID3D11ShaderResourceView> textures[] =
    {
        m_d3dRenderTargetView_lightAccumulation_SRV,
        m_d3dRenderTargetView_diffuse_SRV,
        m_d3dRenderTargetView_specular_SRV,
        m_d3dRenderTargetView_normal_SRV,
        m_d3dDepthStencilView_depth_SRV
    };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(textures), textures->GetAddressOf());

    // Copy lightAccumulation to main RTV
    Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;
    m_d3dRenderTargetView.Get()->GetResource(backBuffer.GetAddressOf());
    m_d3dDeviceContext->CopyResource(backBuffer.Get(), m_d3dRenderTargetView_lightAccumulation_tex.Get());
    backBuffer.Reset();

    // disable depth test and use override blend in single light mode
    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState_DisableDepthTest.Get(), 1);
    m_d3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

    for (int i = -1; i < MAX_LIGHTS; ++i)
    {
        if (i != m_LightCalculationCount || (i != -1 && !m_Scene.Lights[i].Enabled))
        {
            continue;
        }

        m_LightingCalculationOptionsConstrantBuffer.LightIndex = i;
        m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_LightCalculationOptions].Get(), 0, nullptr, &m_LightingCalculationOptionsConstrantBuffer, 0, 0);

        Draw(4, 0);
    }

    // Unbind SRVs
    ID3D11ShaderResourceView* const pSRV[5] = { NULL, NULL, NULL, NULL, NULL };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(pSRV), pSRV);

    // reset to default states
    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);
    m_d3dDeviceContext->OMSetBlendState(NULL, nullptr, 0xffffffff);
}

void SimpleObj::RenderScene_Deferred(RenderEventArgs& e)
{
    RenderScene_Deferred_GeometryPass();

    if (m_DeferredDebugMode == Deferred_DebugMode::None || m_DeferredDebugMode == Deferred_DebugMode::LightVolume || m_DeferredDebugMode == Deferred_DebugMode::LightVolumeExact)
    {
        if (m_LightCalculationMode == LightCalculationMode::Loop)
        {
            RenderScene_Deferred_LightingPass_Loop();
        }
        else if (m_LightCalculationMode == LightCalculationMode::Single)
        {
            RenderScene_Deferred_LightingPass_Single();
        }
        else
        {
            RenderScene_Deferred_LightingPass_Stencil();
        }
    }
    else
    {
        RenderScene_Deferred_DebugPass();
    }
}
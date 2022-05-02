#include "SimpleObj.h"
using namespace Microsoft::WRL;

void SimpleObj::RenderScene_Deferred_GeometryPass()
{
    AssertIfNull(m_d3dDevice, "Render Scene", "Device is null");
    AssertIfNull(m_d3dDeviceContext, "Render Scene", "Device Context is null");

    // Setup the input assembler stage
    m_d3dDeviceContext->IASetInputLayout(m_d3dDeferredGeometry_RegularInputLayout.Get());
    m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Setup the vertex shader stage
    m_d3dDeviceContext->VSSetShader(m_d3dDeferredGeometry_RegularVertexShader.Get(), nullptr, 0);
    m_d3dDeviceContext->VSSetConstantBuffers(0, 1, m_d3dConstantBuffers[CB_Object].GetAddressOf());

    // Setup the rasterizer stage
    m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());
    D3D11_VIEWPORT viewport = m_Camera.get_Viewport();
    m_d3dDeviceContext->RSSetViewports(1, &viewport);

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

    ComPtr<ID3D11ShaderResourceView> textures[] = { m_GridTexture };
    m_d3dDeviceContext->PSSetShaderResources(
        0,                                      // start slot
        1,                                      // number of resources
        textures->GetAddressOf()                // array of resources
    );

    // Setup the output merger 
    ID3D11RenderTargetView* renderTargetViews[] = {
        m_d3dRenderTargetView_lightAccumulation.Get(),
        m_d3dRenderTargetView_diffuse.Get(),
        m_d3dRenderTargetView_specular.Get(),
        m_d3dRenderTargetView_normal.Get()
    };

    m_d3dDeviceContext->OMSetRenderTargets(_countof(renderTargetViews), renderTargetViews, m_d3dDepthStencilView_depth.Get());
    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);

    // Draw Regular Entities
    {
        UINT vertexStride = sizeof(VertexData);
        UINT offset = 0;
        for (auto entity : m_Scene.Entities)
        {
            if (entity->Instanced)
                continue;

            // Setup Material CB
            m_MaterialPropertiesConstantBuffer.Material = entity->Material;
            m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Material].Get(), 0, nullptr, &m_MaterialPropertiesConstantBuffer, 0, 0);

            // Setup Object CB
            m_ObjectConstantBuffer.WorldMatrix = entity->WorldMatrix;
            m_ObjectConstantBuffer.InverseTransposeWorldMatrix = entity->InverseTransposeWorldMatrix;
            m_ObjectConstantBuffer.InverseTransposeWorldViewMatrix = entity->InverseTransposeWorldViewMatrix;
            m_ObjectConstantBuffer.WorldViewProjectionMatrix = entity->WorldViewProjectionMatrix;
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

        const UINT vertexStride[2] = { sizeof(VertexData), sizeof(InstancedObjectConstantBuffer) };
        const UINT offset[2] = { 0, 0 };
        std::vector<InstancedObjectConstantBuffer> instanceData;
        for (auto const& pair : m_Scene.InstancedEntity)
        {
            auto key = pair.first;
            auto verticesCount = Model::GetVertexCount(key);
            auto size = pair.second.size();

            instanceData.clear();
            for (auto const& instancedEntity : pair.second)
            {
                instanceData.push_back({
                    instancedEntity->WorldMatrix,
                    instancedEntity->InverseTransposeWorldMatrix,
                    instancedEntity->InverseTransposeWorldViewMatrix,
                    instancedEntity->Material
                    });
            }

            m_d3dDeviceContext->UpdateSubresource(Model::GetInstancedVertexBuffer(key), 0, nullptr, instanceData.data(), 0, 0);

            ID3D11Buffer* buffers[] = { Model::GetVertexBuffer(key), Model::GetInstancedVertexBuffer(key) };
            m_d3dDeviceContext->IASetVertexBuffers(0, _countof(buffers), buffers, vertexStride, offset);

            m_d3dDeviceContext->DrawInstanced(
                verticesCount,
                size,
                0,
                0
            );
        }
    }
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

    m_d3dDeviceContext->Draw(4, 0);

    // Unbind SRVs
    ID3D11ShaderResourceView* const pSRV[5] = { NULL, NULL, NULL, NULL, NULL };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(pSRV), pSRV);
}

// simpled version for now
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

    m_d3dDeviceContext->Draw(4, 0);

    // Unbind SRVs
    ID3D11ShaderResourceView* const pSRV[5] = { NULL, NULL, NULL, NULL, NULL };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(pSRV), pSRV);
}

void SimpleObj::RenderScene_Deferred(RenderEventArgs& e)
{
    RenderScene_Deferred_GeometryPass();
    
    if (m_DeferredDebugMode == Deferred_DebugMode::None)
    {
        RenderScene_Deferred_LightingPass_Loop();
    }
    else 
    {
        RenderScene_Deferred_DebugPass();
    }
}
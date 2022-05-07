#include "SimpleObj.h"
using namespace Microsoft::WRL;

void SimpleObj::RenderScene_Forward(RenderEventArgs& e)
{
    AssertIfNull(m_d3dDevice, "Render Scene", "Device is null");
    AssertIfNull(m_d3dDeviceContext, "Render Scene", "Device Context is null");

   // Setup the input assembler stage
    m_d3dDeviceContext->IASetInputLayout(m_d3dRegularInputLayout.Get());
    m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Setup the vertex shader stage
    m_d3dDeviceContext->VSSetShader(
        m_d3dRegularVertexShader.Get(),         // pointer to vertex shader
        nullptr,                                // pointer to an array of class-instance interfaces, NULL means shader does not use any interface
        0                                       // number of class-instance interfaces of previous param
    );

    ID3D11Buffer* vertexShaderConstantBuffers[] = 
    { 
        m_d3dConstantBuffers[CB_Frame].Get(), 
        m_d3dConstantBuffers[CB_Object].Get()
    };
    m_d3dDeviceContext->VSSetConstantBuffers(
        0,                                      // start slot
        _countof(vertexShaderConstantBuffers),  // number of buffers
        vertexShaderConstantBuffers             // array of constant buffers
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

    // set blend state to no blend
    UINT sampleMask = 0xffffffff;
    m_d3dDeviceContext->OMSetBlendState(NULL, NULL, sampleMask);

    if (m_LightingCalculation == LightingCalculation::Loop)
    {
        // Draw Regular Entities
        {
            // Setup the pixel stage stage
            m_d3dDeviceContext->PSSetShader(m_d3dForward_LoopLight_PixelShader.Get(), nullptr, 0);
            ID3D11Buffer* pixelShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Material].Get(),
                m_d3dConstantBuffers[CB_Light].Get(),
                m_d3dConstantBuffers[CB_LightCalculationOptions].Get(),
            };
            m_d3dDeviceContext->PSSetConstantBuffers(
                0,                                      // start slot
                _countof(pixelShaderConstantBuffers),   // number of buffers
                pixelShaderConstantBuffers              // array of constant buffers
            );

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
            m_d3dDeviceContext->IASetInputLayout(m_d3dInstancedInputLayout.Get());
            m_d3dDeviceContext->VSSetShader(m_d3dInstancedVertexShader.Get(), nullptr, 0);

            m_d3dDeviceContext->OMSetRenderTargets(
                1,                                      // number of render target to bind
                m_d3dRenderTargetView.GetAddressOf(),   // pointer to an array of render-target view
                nullptr             // pointer to depth-stencil view
            );

            ID3D11Buffer* vertexShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Frame].Get()
            };
            m_d3dDeviceContext->VSSetConstantBuffers(
                0,                                      // start slot
                _countof(vertexShaderConstantBuffers),  // number of buffers
                vertexShaderConstantBuffers             // array of constant buffers
            );

            m_d3dDeviceContext->PSSetShader(m_d3dForward_LoopLight_InstancedPixelShader.Get(), nullptr, 0);

            ID3D11Buffer* pixelShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Light].Get(),
                m_d3dConstantBuffers[CB_LightCalculationOptions].Get(),
            };
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

                // update perInstanceBuffer
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
    
    else if (m_LightingCalculation == LightingCalculation::Single)
    {   
        // Draw Regular Entities
        {
            // Setup the pixel stage stage
            m_d3dDeviceContext->PSSetShader(m_d3dForward_SingleLight_PixelShader.Get(), nullptr, 0);
            ID3D11Buffer* pixelShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Material].Get(),
                m_d3dConstantBuffers[CB_Light].Get(),
                m_d3dConstantBuffers[CB_LightCalculationOptions].Get(),
            };
            m_d3dDeviceContext->PSSetConstantBuffers(
                0,                                      // start slot
                _countof(pixelShaderConstantBuffers),   // number of buffers
                pixelShaderConstantBuffers              // array of constant buffers
            );

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

                for (int i = 0, drawCountIndex = 0; i < MAX_LIGHTS; ++i)
                {
                    //if (i > 1) continue;

                    if (drawCountIndex == 0)
                    {
                        // set blend state to no blend
                        m_d3dDeviceContext->OMSetBlendState(NULL, nullptr, sampleMask);
                    }
                    else
                    {
                        // set blend state to add
                        m_d3dDeviceContext->OMSetBlendState(NULL, nullptr, sampleMask);
                        //m_d3dDeviceContext->OMSetBlendState(m_d3dBlendState_Add.Get(), NULL, sampleMask);
                    }

                    m_LightingCalculationOptionsConstrantBuffer.LightIndex = i;
                    m_LightingCalculationOptionsConstrantBuffer.LightingSpace = (int)m_LightingSpace;
                    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_LightCalculationOptions].Get(), 0, nullptr, &m_LightingCalculationOptionsConstrantBuffer, 0, 0);

                    m_d3dDeviceContext->Draw(
                        entity->Model->VertexCount(),
                        0
                    );

                    ++drawCountIndex;
                }
            }
        }

        /*// Draw Instanced Entities
        {
            m_d3dDeviceContext->IASetInputLayout(m_d3dInstancedInputLayout.Get());
            m_d3dDeviceContext->VSSetShader(m_d3dInstancedVertexShader.Get(), nullptr, 0);

            ID3D11Buffer* vertexShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Frame].Get()
            };
            m_d3dDeviceContext->VSSetConstantBuffers(
                0,                                      // start slot
                _countof(vertexShaderConstantBuffers),  // number of buffers
                vertexShaderConstantBuffers             // array of constant buffers
            );

            m_d3dDeviceContext->PSSetShader(m_d3dForward_LoopLight_InstancedPixelShader.Get(), nullptr, 0);

            ID3D11Buffer* pixelShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Light].Get(),
                m_d3dConstantBuffers[CB_LightCalculationOptions].Get(),
            };
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

                // update perInstanceBuffer
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
        }*/
    }
}
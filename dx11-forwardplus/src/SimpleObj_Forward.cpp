#include "SimpleObj.h"

#include <set>
#include <functional>

using namespace Microsoft::WRL;
using namespace Yr;

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

    ComPtr<ID3D11SamplerState> samplerStates[] = { m_d3dSamplerState };
    m_d3dDeviceContext->PSSetSamplers(
        0,                                      // start slot
        1,                                      // number of sampler states
        samplerStates->GetAddressOf()           // array of sampler states
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
    m_d3dDeviceContext->OMSetBlendState(nullptr, nullptr, sampleMask);

    // for forward shading both light calculation share same bind texture functions
    std::function<void(ComPtr<ID3D11ShaderResourceView>[])> bindTextureDelegate = [&](ComPtr<ID3D11ShaderResourceView> textures[]) -> void {
        m_d3dDeviceContext->PSSetShaderResources(
            0,                                      // start slot
            MAX_TEXTURES,                           // number of resources
            textures->GetAddressOf()                // array of resources
        );
    };

    if (m_LightCalculationMode == LightCalculationMode::Loop)
    {
        // Draw Regular Entities
        {
            // Setup the vertex shader stage
            m_d3dDeviceContext->IASetInputLayout(m_d3dRegularInputLayout.Get());
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

            DrawRegularEntities(bindTextureDelegate);
        }

        // Draw Instanced Entities
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

            DrawInstancedEntities(bindTextureDelegate);
        }
    }
    
    else if (m_LightCalculationMode == LightCalculationMode::Single)
    {
        // Draw Regular Entities
        {
            bool hasDrawAnyModel = false;

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

            std::function<void(ComPtr<ID3D11ShaderResourceView>[])> bindTextureDelegate = [&](ComPtr<ID3D11ShaderResourceView> textures[]) -> void {
                m_d3dDeviceContext->PSSetShaderResources(
                    0,                                      // start slot
                    MAX_TEXTURES,                     // number of resources
                    textures->GetAddressOf()                // array of resources
                );
            };

            for (int i = -1; i < MAX_LIGHTS; ++i)
            {
                if (i != m_LightCalculationCount || (i != -1 && !m_Scene.Lights[i].Enabled))
                {
                    continue;
                }

                // i = -1 for first pass: ambient + emission
                if (!hasDrawAnyModel)
                {
                    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);
                    m_d3dDeviceContext->OMSetBlendState(nullptr, nullptr, sampleMask);
                    hasDrawAnyModel = true;
                }

                // other pass: additive blend diffuse and specular
                else
                {
                    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState_Overlay.Get(), 1);
                    m_d3dDeviceContext->OMSetBlendState(m_d3dBlendState_Add.Get(), nullptr, sampleMask);
                }

                m_LightingCalculationOptionsConstrantBuffer.LightIndex = i;
                m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_LightCalculationOptions].Get(), 0, nullptr, &m_LightingCalculationOptionsConstrantBuffer, 0, 0);

                DrawRegularEntities(bindTextureDelegate);
            }
        }

        // Draw Instanced Entities
        {
            m_d3dDeviceContext->IASetInputLayout(m_d3dInstancedInputLayout.Get());
            m_d3dDeviceContext->VSSetShader(m_d3dInstancedVertexShader.Get(), nullptr, 0);

            ID3D11Buffer* vertexShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Frame].Get()
            };
            m_d3dDeviceContext->VSSetConstantBuffers(0, _countof(vertexShaderConstantBuffers), vertexShaderConstantBuffers);

            m_d3dDeviceContext->PSSetShader(m_d3dForward_SingleLight_InstancedPixelShader.Get(), nullptr, 0);

            ID3D11Buffer* pixelShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Light].Get(),
                m_d3dConstantBuffers[CB_LightCalculationOptions].Get(),
            };
            m_d3dDeviceContext->PSSetConstantBuffers(0, _countof(pixelShaderConstantBuffers), pixelShaderConstantBuffers);

            bool hasDrawAnyModel = false;
            for (int i = -1; i < MAX_LIGHTS; ++i)
            {
                if (i != m_LightCalculationCount || (i != -1 && !m_Scene.Lights[i].Enabled))
                {
                    continue;
                }

                if (!hasDrawAnyModel)
                {
                    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);
                    m_d3dDeviceContext->OMSetBlendState(nullptr, nullptr, sampleMask);
                    hasDrawAnyModel = true;
                }
                else
                {
                    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState_Overlay.Get(), 1);
                    m_d3dDeviceContext->OMSetBlendState(m_d3dBlendState_Add.Get(), nullptr, sampleMask);
                }

                m_LightingCalculationOptionsConstrantBuffer.LightIndex = i;
                m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_LightCalculationOptions].Get(), 0, nullptr, &m_LightingCalculationOptionsConstrantBuffer, 0, 0);

                DrawInstancedEntities(bindTextureDelegate);
            }
        }
    }

    // Unbind SRVs
    ID3D11ShaderResourceView* const pSRV[MAX_TEXTURES] = { NULL };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(pSRV), pSRV);
}
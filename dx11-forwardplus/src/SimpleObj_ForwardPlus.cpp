#include "SimpleObj.h"
#include <set>

using namespace Microsoft::WRL;
using namespace Yr;

HRESULT SimpleObj::CreateStructuredBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut)
{
    D3D11_BUFFER_DESC descBuf = {};
    pBuffer->GetDesc(&descBuf);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;

    if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        desc.Buffer.NumElements = descBuf.ByteWidth / 4;
    }
    else
    {
        if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
        {
            desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
        }
        else
        {
            return E_INVALIDARG;
        }
    }

    return pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut);
};

HRESULT SimpleObj::CreateStructuredBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut)
{
    D3D11_BUFFER_DESC descBuf = {};
    pBuffer->GetDesc(&descBuf);

    D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;

    if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        // This is a Raw Buffer
        desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
        desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
        desc.Buffer.NumElements = descBuf.ByteWidth / 4;
    }
    else
    {
        if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
        {
            // This is a Structured Buffer
            desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
            desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
        }
        else
        {
            return E_INVALIDARG;
        }
    }

    return pDevice->CreateUnorderedAccessView(pBuffer, &desc, ppUAVOut);
};

HRESULT SimpleObj::CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut)
{
    *ppBufOut = nullptr;

    D3D11_BUFFER_DESC desc = {};
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.ByteWidth = uElementSize * uCount;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = uElementSize;

    if (pInitData)
    {
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pInitData;
        return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
    }
    else
        return pDevice->CreateBuffer(&desc, nullptr, ppBufOut);
}

void SimpleObj::ComputeFrustum()
{
    int screenWidth = max(m_ScreenDimensions.x, 1);
    int screenHeight = max(m_ScreenDimensions.y, 1);

    int numThreadsX = std::ceilf((float)screenWidth / (float)BLOCK_SIZE);
    int numThreadsY = std::ceilf((float)screenHeight / (float)BLOCK_SIZE);
    int numThreadsZ = 1;

    int numThreadGroupsX = std::ceilf((float)numThreadsX / (float)BLOCK_SIZE);
    int numThreadGroupsY = std::ceilf((float)numThreadsY / (float)BLOCK_SIZE);
    int numThreadGroupsZ = 1;

    m_ScreenToViewParamsConstantBuffer.InverseView = m_Camera.get_InverseViewMatrix();
    m_ScreenToViewParamsConstantBuffer.InverseProjection = m_Camera.get_InverseProjectionMatrix();
    m_ScreenToViewParamsConstantBuffer.ScreenDimensions = m_ScreenDimensions;
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_ScreenToViewParams].Get(), 0, nullptr, &m_ScreenToViewParamsConstantBuffer, 0, 0);

    m_DispatchParamsConstantBuffer.numThreads[0] = numThreadsX;
    m_DispatchParamsConstantBuffer.numThreads[1] = numThreadsY;
    m_DispatchParamsConstantBuffer.numThreads[2] = numThreadsZ;
    m_DispatchParamsConstantBuffer.numThreadGroups[0] = numThreadGroupsX;
    m_DispatchParamsConstantBuffer.numThreadGroups[1] = numThreadGroupsY;
    m_DispatchParamsConstantBuffer.numThreadGroups[2] = numThreadGroupsZ;

    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_DispatchParams].Get(), 0, nullptr, &m_DispatchParamsConstantBuffer, 0, 0);

    m_d3dDeviceContext->CSSetShader(m_d3dFowrardPlus_ComputeFrustumShader.Get(), nullptr, 0);

    ID3D11Buffer* computeShaderConstantBuffers[] =
    {
        m_d3dConstantBuffers[CB_DispatchParams].Get(),
        m_d3dConstantBuffers[CB_ScreenToViewParams].Get()
    };
    m_d3dDeviceContext->CSSetConstantBuffers(0, _countof(computeShaderConstantBuffers), computeShaderConstantBuffers);

    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, m_d3dFrustumBuffers_UAV.GetAddressOf(), nullptr);

    m_d3dDeviceContext->Dispatch(numThreadGroupsX, numThreadGroupsY, numThreadGroupsZ);

    // Clean up
    m_d3dDeviceContext->CSSetShader(nullptr, nullptr, 0);

    ID3D11UnorderedAccessView* nullUAVs[1] = { nullptr };
    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullptr);

    ID3D11Buffer* nullConstantBuffers[2] = { nullptr, nullptr };
    m_d3dDeviceContext->CSSetConstantBuffers(0, 1, nullConstantBuffers);

#ifdef _DEBUG
    {
        if (m_ForwardPlusPrintDebugInfo)
        {
            // copy result back to m_d3dFrustumBuffers
            auto tempBuffer = ReadBuffer(m_d3dDevice.Get(), m_d3dDeviceContext.Get(), m_d3dFrustumBuffers.Get());

            D3D11_MAPPED_SUBRESOURCE MappedResource;
            m_d3dDeviceContext->Map(tempBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
            std::copy_n((struct Frustum*)MappedResource.pData, m_frustums.size(), m_frustums.data());

            int zeroCount = 0;
            for (auto& frustum : m_frustums) {
                for (int i = 0; i < 4; ++i)
                {
                    if (frustum.plane[i].d < 0.00001) {
                        zeroCount += 1;
                    }
                }
            }
            std::cout << "Zero Count = " << zeroCount << " / " << m_frustums.size() * 4.0 << std::endl;

            // Clean up
            m_d3dDeviceContext->Unmap(tempBuffer, 0);
            SafeRelease(tempBuffer);
        }
    }
#endif
}

void SimpleObj::RenderScene_FowardPlus(RenderEventArgs& e)
{
    if (!m_frustumComputed)
    {
        ComputeFrustum();
        
        m_frustumComputed = true;
    }

    RenderScene_FowardPlus_DepthPrePass();

    if (m_ForwardPlusDebugMode == ForwardPlus_DebugMode::DepthTex)
    {
        m_DebugPropertiesConstantBuffer.DebugMode = (int)Deferred_DebugMode::Depth;
        m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Debug].Get(), 0, nullptr, &m_DebugPropertiesConstantBuffer, 0, 0);
        
        RenderScene_Deferred_DebugPass();
        return;
    }
    
    RenderScene_FowardPlus_CullLightPass();

    if (m_ForwardPlusDebugMode != ForwardPlus_DebugMode::None)
    {
        RenderScene_Deferred_DebugLightMapPass();

        return;
    }

    RenderScene_FowardPlus_FinalPass();
}

void SimpleObj::RenderScene_FowardPlus_FinalPass()
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

    const int textureStartOffset = 2;
    ComPtr<ID3D11ShaderResourceView> texturesWithStartOffset[MAX_TEXTURES + textureStartOffset] = {
        m_d3dOpaqueLightIndexListBuffers_SRV.Get(),
        m_d3dOpaqueLightGridBuffers_SRV.Get()
    };

    std::function<void(ComPtr<ID3D11ShaderResourceView>[])> bindTextureDelegate = [&](ComPtr<ID3D11ShaderResourceView> textures[]) -> void {
        for (int i = 0; i < MAX_TEXTURES; ++i) // memcpy not suitable for ComPtr!
        {
            texturesWithStartOffset[textureStartOffset + i] = textures[i];
        }
        
        m_d3dDeviceContext->PSSetShaderResources(
            0,                                      // start slot
            MAX_TEXTURES + textureStartOffset,      // number of resources
            texturesWithStartOffset->GetAddressOf() // array of resources
        );
    };

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
            m_d3dDeviceContext->PSSetShader(m_d3dFowrardPlus_FinalPass_RegularPixelShader.Get(), nullptr, 0);
            ID3D11Buffer* pixelShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Material].Get(),
                m_d3dConstantBuffers[CB_Light].Get(),
                m_d3dConstantBuffers[CB_LightCalculationOptions].Get(),
                m_d3dConstantBuffers[CB_DispatchParams].Get(),
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

            m_d3dDeviceContext->PSSetShader(m_d3dFowrardPlus_FinalPass_InstancedPixelShader.Get(), nullptr, 0);

            ID3D11Buffer* pixelShaderConstantBuffers[] =
            {
                m_d3dConstantBuffers[CB_Light].Get(),
                m_d3dConstantBuffers[CB_LightCalculationOptions].Get(),
                m_d3dConstantBuffers[CB_DispatchParams].Get(),
            };
            m_d3dDeviceContext->PSSetConstantBuffers(
                0,                                      // start slot
                _countof(pixelShaderConstantBuffers),   // number of buffers
                pixelShaderConstantBuffers              // array of constant buffers
            );

            DrawInstancedEntities(bindTextureDelegate);
        }
    } 

    // Unbind SRVs
    ID3D11ShaderResourceView* const pSRV[MAX_TEXTURES + 2] = { NULL };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(pSRV), pSRV);
}

void SimpleObj::RenderScene_FowardPlus_DepthPrePass()
{
    AssertIfNull(m_d3dDevice, "Render Scene", "Device is null");
    AssertIfNull(m_d3dDeviceContext, "Render Scene", "Device Context is null");

    // Setup the input assembler stage
    m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Setup the pixel stage stage
    m_d3dDeviceContext->PSSetShader(nullptr, nullptr, 0);
    m_d3dDeviceContext->PSSetConstantBuffers(0, 0, nullptr);
    m_d3dDeviceContext->PSSetSamplers(0, 0, nullptr);
    m_d3dDeviceContext->PSSetShaderResources(0, 0, nullptr);

    // Setup the output merger 
    m_d3dDeviceContext->OMSetRenderTargets(0, nullptr, m_d3dDepthStencilView_depth.Get());
    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);

    // set blend state to no blend
    m_d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);

    // Clear depth first
    m_d3dDeviceContext->ClearDepthStencilView(m_d3dDepthStencilView_depth.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    std::function<void(ComPtr<ID3D11ShaderResourceView>[])> bindTextureDelegate = [&](ComPtr<ID3D11ShaderResourceView> textures[]) -> void {
        // do nothing since we don't need texture in depth pre pass
    };

    // Draw Regular Entities
    {
        // Setup the vertex shader stage
        m_d3dDeviceContext->IASetInputLayout(m_d3dRegularInputLayout.Get());
        m_d3dDeviceContext->VSSetShader(m_d3dRegularVertexShader.Get(), nullptr, 0);
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

        DrawRegularEntities(bindTextureDelegate);
    }

    // Draw Instanced Entities
    {
        m_d3dDeviceContext->IASetInputLayout(m_d3dInstancedInputLayout.Get());
        m_d3dDeviceContext->VSSetShader(m_d3dInstancedVertexShader.Get(), nullptr, 0);
        
        ID3D11Buffer* vertexShaderConstantBuffers[] = { m_d3dConstantBuffers[CB_Frame].Get() };
        m_d3dDeviceContext->VSSetConstantBuffers(
            0,                                      // start slot
            _countof(vertexShaderConstantBuffers),  // number of buffers
            vertexShaderConstantBuffers             // array of constant buffers
        );

        DrawInstancedEntities(bindTextureDelegate);
    }

    m_d3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_d3dDeviceContext->OMSetDepthStencilState(nullptr, 0);
}

void SimpleObj::RenderScene_Deferred_DebugLightMapPass()
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
    m_d3dDeviceContext->PSSetShader(m_d3dFowrardPlus_DebugLightMap_PixelShader.Get(), nullptr, 0);

    // Setup pixel shader cb
    ID3D11Buffer* pixelShaderConstantBuffers[] = {
        m_d3dConstantBuffers[CB_DispatchParams].Get(),
        m_d3dConstantBuffers[CB_Debug].Get(),
        m_d3dConstantBuffers[CB_LightCalculationOptions].Get(),
    };
    m_d3dDeviceContext->PSSetConstantBuffers(
        0,
        _countof(pixelShaderConstantBuffers),
        pixelShaderConstantBuffers
    );

    // Setup the input assembler stage
    m_d3dDeviceContext->IASetInputLayout(nullptr);
    m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // setup textures
    m_d3dDeviceContext->PSSetSamplers(0, 0, nullptr);
    ComPtr<ID3D11ShaderResourceView> textures[] = { m_d3dOpaqueLightGridBuffers_SRV };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(textures), textures->GetAddressOf());

    Draw(4, 0);

    // Unbind SRVs
    ID3D11ShaderResourceView* const pSRV[1] = { NULL };
    m_d3dDeviceContext->PSSetShaderResources(0, _countof(pSRV), pSRV);
}

void SimpleObj::RenderScene_FowardPlus_CullLightPass()
{
    int screenWidth = max(m_ScreenDimensions.x, 1);
    int screenHeight = max(m_ScreenDimensions.y, 1);

    int numThreadsX = std::ceilf((float)screenWidth / (float)BLOCK_SIZE);
    int numThreadsY = std::ceilf((float)screenHeight / (float)BLOCK_SIZE);
    int numThreadsZ = 1;

    int numThreadGroupsX = std::ceilf((float)screenWidth / (float)BLOCK_SIZE);
    int numThreadGroupsY = std::ceilf((float)screenHeight / (float)BLOCK_SIZE);
    int numThreadGroupsZ = 1;

    m_ScreenToViewParamsConstantBuffer.InverseView = m_Camera.get_InverseViewMatrix();
    m_ScreenToViewParamsConstantBuffer.InverseProjection = m_Camera.get_InverseProjectionMatrix();
    m_ScreenToViewParamsConstantBuffer.ScreenDimensions = m_ScreenDimensions;
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_ScreenToViewParams].Get(), 0, nullptr, &m_ScreenToViewParamsConstantBuffer, 0, 0);

    m_DispatchParamsConstantBuffer.numThreadGroups[0] = numThreadGroupsX;
    m_DispatchParamsConstantBuffer.numThreadGroups[1] = numThreadGroupsY;
    m_DispatchParamsConstantBuffer.numThreadGroups[2] = numThreadGroupsZ;
    m_DispatchParamsConstantBuffer.numThreads[0] = m_ScreenDimensions.x;
    m_DispatchParamsConstantBuffer.numThreads[1] = m_ScreenDimensions.y;
    m_DispatchParamsConstantBuffer.numThreads[2] = 1;
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_DispatchParams].Get(), 0, nullptr, &m_DispatchParamsConstantBuffer, 0, 0);

    // clear light index counter, other structure buffer remains dirty after calculations
    m_opaqueLightIndexCounter[0] = 0;
    m_d3dDeviceContext->UpdateSubresource(m_d3dOpaqueLightIndexCounterBuffers.Get(), 0, nullptr, m_opaqueLightIndexCounter.data(), 0, 0);

    m_d3dDeviceContext->CSSetShader(m_d3dFowrardPlus_CullLightShader.Get(), nullptr, 0);

    ID3D11Buffer* computeShaderConstantBuffers[] =
    {
        m_d3dConstantBuffers[CB_DispatchParams].Get(),
        m_d3dConstantBuffers[CB_ScreenToViewParams].Get(),
        m_d3dConstantBuffers[CB_Light].Get(),
        m_d3dConstantBuffers[CB_Debug].Get(),
        m_d3dConstantBuffers[CB_LightCalculationOptions].Get(),
    };

    ComPtr<ID3D11ShaderResourceView> textures[] =
    {
        m_d3dDepthStencilView_depth_SRV,
        m_d3dFrustumBuffers_SRV,
    };

    ComPtr<ID3D11UnorderedAccessView> buffers[] =
    {
        m_d3dOpaqueLightIndexCounterBuffers_UAV.Get(),
        m_d3dOpaqueLightIndexListBuffers_UAV.Get(),
        m_d3dOpaqueLightGrid_UAV.Get(),
        m_d3dDebugRWListBuffers_UAV.Get(),
    };

    // bind input
    m_d3dDeviceContext->CSSetConstantBuffers(0, _countof(computeShaderConstantBuffers), computeShaderConstantBuffers);
    m_d3dDeviceContext->CSSetShaderResources(0, _countof(textures), textures->GetAddressOf());

    // bind output
    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, _countof(buffers), buffers->GetAddressOf(), nullptr);

    // dispatch
    Dispatch(numThreadGroupsX, numThreadGroupsY, numThreadGroupsZ);

    // clean up
    m_d3dDeviceContext->CSSetShader(nullptr, nullptr, 0);

    ID3D11UnorderedAccessView* nullUAVs[] = { nullptr, nullptr, nullptr, nullptr };
    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, _countof(nullUAVs), nullUAVs, nullptr);

    ID3D11Buffer* nullConstantBuffers[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
    m_d3dDeviceContext->CSSetConstantBuffers(0, _countof(nullConstantBuffers), nullConstantBuffers);

    ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr };
    m_d3dDeviceContext->CSSetShaderResources(0, _countof(nullSRVs), nullSRVs);

#ifdef _DEBUG
    if (m_ForwardPlusPrintDebugInfo) 
    {
        {
            // copy result back to m_opaqueLightIndexCounter
            auto tempBuffer = ReadBuffer(m_d3dDevice.Get(), m_d3dDeviceContext.Get(), m_d3dDebugRWListBuffers.Get());

            D3D11_MAPPED_SUBRESOURCE MappedResource;
            m_d3dDeviceContext->Map(tempBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
            std::copy_n((float*)MappedResource.pData, m_debugRWList.size(), m_debugRWList.data());

            // Clean up
            m_d3dDeviceContext->Unmap(tempBuffer, 0);
            SafeRelease(tempBuffer);
        }

        {
            // copy result back to m_opaqueLightIndexCounter
            auto tempBuffer = ReadBuffer(m_d3dDevice.Get(), m_d3dDeviceContext.Get(), m_d3dOpaqueLightIndexCounterBuffers.Get());

            D3D11_MAPPED_SUBRESOURCE MappedResource;
            m_d3dDeviceContext->Map(tempBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
            std::copy_n((int*)MappedResource.pData, m_opaqueLightIndexCounter.size(), m_opaqueLightIndexCounter.data());

            // Clean up
            m_d3dDeviceContext->Unmap(tempBuffer, 0);
            SafeRelease(tempBuffer);
        }

        {
            // copy result back to m_opaqueLightIndexList
            auto tempBuffer = ReadBuffer(m_d3dDevice.Get(), m_d3dDeviceContext.Get(), m_d3dOpaqueLightIndexListBuffers.Get());

            D3D11_MAPPED_SUBRESOURCE MappedResource;
            m_d3dDeviceContext->Map(tempBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
            std::copy_n((uint32_t*)MappedResource.pData, m_opaqueLightIndexList.size(), m_opaqueLightIndexList.data());

            // Clean up
            m_d3dDeviceContext->Unmap(tempBuffer, 0);
            SafeRelease(tempBuffer);
        }

        {
            // copy result back to m_d3dOpaqueLightGrid
            auto tempBuffer = ReadTexture2D(m_d3dDevice.Get(), m_d3dDeviceContext.Get(), m_d3dOpaqueLightGridBuffers.Get());

            D3D11_MAPPED_SUBRESOURCE MappedResource;
            m_d3dDeviceContext->Map(tempBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
            std::copy_n((uint2*)MappedResource.pData, m_d3dOpaqueLightGrid.size(), m_d3dOpaqueLightGrid.data());

            for (int i = 0; i < numThreadGroupsX; ++i)
            {
                for (int j = 0; j < numThreadGroupsY; ++j)
                {
                    auto startIndex = m_d3dOpaqueLightGrid[i * numThreadGroupsY + j].x;
                    auto lightCount = m_d3dOpaqueLightGrid[i * numThreadGroupsY + j].y;
                    
                    std::cout << "(" << i << ", " << j << ") = [ ";
                    for (int k = 0; k < lightCount; ++k)
                    {
                        std::cout << m_opaqueLightIndexList[startIndex + k] << ", ";
                    }
                    std::cout << "]" << std::endl;
                }
            
                std::cout << std::endl;
            }
            
            // Clean up
            m_d3dDeviceContext->Unmap(tempBuffer, 0);
            SafeRelease(tempBuffer);
        }
    }
#endif
}

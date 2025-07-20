#include "SimpleObj.h"

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
};

void SimpleObj::ComputeFrustum(int width, int height, int blockSize)
{
    int threadGroupCountX = std::ceilf((float)width / (float)blockSize);
    int threadGroupCountY = std::ceilf((float)height / (float)blockSize);
    int threadGroupCountZ = 1;
    int totalGroupCounts = threadGroupCountX * threadGroupCountY * threadGroupCountZ;
    
    m_ScreenToViewParamsConstantBuffer.InverseView = m_Camera.get_InverseViewMatrix();
    m_ScreenToViewParamsConstantBuffer.InverseProjection = m_Camera.get_InverseProjectionMatrix();
    m_ScreenToViewParamsConstantBuffer.ScreenDimensions = Vector2(width, height);
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_ScreenToViewParams].Get(), 0, nullptr, &m_ScreenToViewParamsConstantBuffer, 0, 0);

    m_DispatchParamsConstantBuffer.numThreads[0] = width;
    m_DispatchParamsConstantBuffer.numThreads[1] = height;
    m_DispatchParamsConstantBuffer.numThreads[2] = 1;
    m_DispatchParamsConstantBuffer.numThreadGroups[0] = threadGroupCountX;
    m_DispatchParamsConstantBuffer.numThreadGroups[1] = threadGroupCountY;
    m_DispatchParamsConstantBuffer.numThreadGroups[2] = threadGroupCountZ;
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_DispatchParams].Get(), 0, nullptr, &m_DispatchParamsConstantBuffer, 0, 0);

    m_d3dDeviceContext->CSSetShader(m_d3dFowrardPlus_ComputeFrustumShader.Get(), nullptr, 0);

    ID3D11Buffer* computeShaderConstantBuffers[] =
    {
        m_d3dConstantBuffers[CB_DispatchParams].Get(),
        m_d3dConstantBuffers[CB_ScreenToViewParams].Get()
    };
    m_d3dDeviceContext->CSSetConstantBuffers(0, _countof(computeShaderConstantBuffers), computeShaderConstantBuffers);

    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, m_d3dFrustumBuffers_UAV.GetAddressOf(), nullptr);

    m_d3dDeviceContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

    // Clean up
    m_d3dDeviceContext->CSSetShader(nullptr, nullptr, 0);

    ID3D11UnorderedAccessView* nullUAVs[1] = { nullptr };
    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullptr);

    ID3D11Buffer* nullConstantBuffers[1] = { nullptr };
    m_d3dDeviceContext->CSSetConstantBuffers(0, 1, nullConstantBuffers);

#ifdef _DEBUG
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
#endif
}

void SimpleObj::RenderScene_FowardPlus(RenderEventArgs& e)
{
    // update subResource first
    int threadGroupCountX = std::ceilf((float)m_ScreenDimensions.x / (float)BLOCK_SIZE);
    int threadGroupCountY = std::ceilf((float)m_ScreenDimensions.y / (float)BLOCK_SIZE);
    int threadGroupCountZ = 1;
    int totalGroupCounts = threadGroupCountX * threadGroupCountY * threadGroupCountZ;

    m_ScreenToViewParamsConstantBuffer.InverseView = m_Camera.get_InverseViewMatrix();
    m_ScreenToViewParamsConstantBuffer.InverseProjection = m_Camera.get_InverseProjectionMatrix();
    m_ScreenToViewParamsConstantBuffer.ScreenDimensions = m_ScreenDimensions;
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_ScreenToViewParams].Get(), 0, nullptr, &m_ScreenToViewParamsConstantBuffer, 0, 0);

    m_DispatchParamsConstantBuffer.numThreads[0] = m_ScreenDimensions.x;
    m_DispatchParamsConstantBuffer.numThreads[1] = m_ScreenDimensions.y;
    m_DispatchParamsConstantBuffer.numThreads[2] = 1;
    m_DispatchParamsConstantBuffer.numThreadGroups[0] = threadGroupCountX;
    m_DispatchParamsConstantBuffer.numThreadGroups[1] = threadGroupCountY;
    m_DispatchParamsConstantBuffer.numThreadGroups[2] = threadGroupCountZ;
    m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_DispatchParams].Get(), 0, nullptr, &m_DispatchParamsConstantBuffer, 0, 0);

    RenderScene_FowardPlus_DepthPrePass();

    if (m_ForwardPlusDebugMode == ForwardPlus_DebugMode::DepthTex)
    {
        m_DebugPropertiesConstantBuffer.DebugMode = (int)Deferred_DebugMode::Depth;
        m_d3dDeviceContext->UpdateSubresource(m_d3dConstantBuffers[CB_Debug].Get(), 0, nullptr, &m_DebugPropertiesConstantBuffer, 0, 0);
        
        RenderScene_Deferred_DebugPass();
        return;
    }
    
    RenderScene_FowardPlus_CullLightPass(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

    if (m_ForwardPlusDebugMode != ForwardPlus_DebugMode::None)
    {
        RenderScene_Deferred_DebugLightMapPass();

        return;
    }
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

        UINT vertexStride = sizeof(VertexData);
        UINT offset = 0;
        for (auto entity : m_Scene.Entities)
        {
            if (entity->Instanced)
                continue;

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

            Draw(
                entity->Model->VertexCount(),
                0
            );
        }
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

            DrawInstanced(
                verticesCount,
                size,
                0,
                0
            );
        }
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
        m_d3dConstantBuffers[CB_ScreenToViewParams].Get(), 
        m_d3dConstantBuffers[CB_Debug].Get(),
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

void SimpleObj::RenderScene_FowardPlus_CullLightPass(int threadGroupCountX, int threadGroupCountY, int threadGroupCountZ)
{
    int totalGroupCounts = threadGroupCountX * threadGroupCountY * threadGroupCountZ;

    m_d3dDeviceContext->CSSetShader(m_d3dFowrardPlus_CullLightShader.Get(), nullptr, 0);

    ID3D11Buffer* computeShaderConstantBuffers[] =
    {
        m_d3dConstantBuffers[CB_DispatchParams].Get(),
        m_d3dConstantBuffers[CB_ScreenToViewParams].Get(),
        m_d3dConstantBuffers[CB_Light].Get(),
        m_d3dConstantBuffers[CB_Debug].Get(),
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
    m_d3dDeviceContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

    // clean up
    m_d3dDeviceContext->CSSetShader(nullptr, nullptr, 0);

    ID3D11UnorderedAccessView* nullUAVs[3] = { nullptr, nullptr, nullptr };
    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, _countof(nullUAVs), nullUAVs, nullptr);

    ID3D11Buffer* nullConstantBuffers[3] = { nullptr, nullptr, nullptr };
    m_d3dDeviceContext->CSSetConstantBuffers(0, _countof(nullConstantBuffers), nullConstantBuffers);

    ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
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
            std::copy_n((uint*)MappedResource.pData, m_opaqueLightIndexList.size(), m_opaqueLightIndexList.data());

            int zeroCount = 0;
            for (auto i : m_opaqueLightIndexList) {
                if (i == 0) {
                    zeroCount += 1;
                }
                else {
                    std::cout << i << ",";
                }
            }
            std::cout << "Zero Count = " << zeroCount << " / " << m_opaqueLightIndexList.size() << std::endl;

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

            for (int j = 0; j < threadGroupCountY; ++j)
            {
                for (int i = 0; i < threadGroupCountX; ++i)
                {
                    std::cout << m_d3dOpaqueLightGrid[i * threadGroupCountY + j].y << ",";
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

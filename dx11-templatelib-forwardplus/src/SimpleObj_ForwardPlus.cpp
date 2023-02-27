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

HRESULT SimpleObj::CreateStructuredBuffer (ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut)
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

    RenderScene_FowardPlus_CullLightPass(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
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
    {
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
            std::copy_n((int*)MappedResource.pData, m_opaqueLightIndexList.size(), m_opaqueLightIndexList.data());

            // Clean up
            m_d3dDeviceContext->Unmap(tempBuffer, 0);
            SafeRelease(tempBuffer);
        }
    }
#endif
}

#include "SimpleObj.h"

using namespace Yr;

HRESULT SimpleObj::CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut)
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
    HRESULT hr;

    int threadGroupCountX = std::ceilf((float)width / (float)blockSize);
    int threadGroupCountY = std::ceilf((float)height / (float)blockSize);
    int threadGroupCountZ = 1;
    int totalGroupCounts = threadGroupCountX * threadGroupCountY * threadGroupCountZ;
    
    m_Frustums.resize(totalGroupCounts);

    hr = CreateStructuredBuffer(m_d3dDevice.Get(), sizeof(struct Frustum), totalGroupCounts, NULL, m_d3dFrustumBuffers.GetAddressOf());
    AssertIfFailed(hr, "Create Buffer", "Unable to create m_d3dFrustumBuffers");

    hr = CreateBufferUAV(m_d3dDevice.Get(), m_d3dFrustumBuffers.Get(), m_d3dFrustumBuffersUAV.GetAddressOf());
    AssertIfFailed(hr, "Create Buffer UAV", "Unable to create m_d3dFrustumBuffersUAV");

    m_ScreenToViewParamsConstantBuffer.InverseView = m_Camera.get_InverseViewMatrix();
    m_ScreenToViewParamsConstantBuffer.InverseProjection = m_Camera.get_InverseProjectionMatrix();
    m_ScreenToViewParamsConstantBuffer.ScreenDimensions = m_ScreenDimensions;
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

    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, m_d3dFrustumBuffersUAV.GetAddressOf(), nullptr);

    m_d3dDeviceContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

    // Clean up
    m_d3dDeviceContext->CSSetShader(nullptr, nullptr, 0);

    ID3D11UnorderedAccessView* nullUAVs[1] = { nullptr };
    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullptr);
    
    ID3D11Buffer* nullConstantBuffers[1] = { nullptr };
    m_d3dDeviceContext->CSSetConstantBuffers(0, 1, nullConstantBuffers);
}
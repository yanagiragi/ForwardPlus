#include "Entity.h"
using namespace Yr;

void Entity::Setup()
{
    if (Instanced)
    {
        if (InstancedReference == nullptr)
        {
            return;
        }

        if (InstancedReference->batchedVertices.size() > 1)
        {
            DisplayError("Only one submesh is supported for instancing");
            return;
        }
        
        if (InstancedReference->InstanceDataBuffer != nullptr)
        {
            return;
        }
        
        std::vector<InstancedObjectConstantBuffer> instanceData;
        for (int i = 0; i < InstancedReference->InstancedCount; ++i)
        {
            instanceData.push_back({
                InstancedReference->WorldMatrix,
                InstancedReference->InverseTransposeWorldMatrix,
                InstancedReference->InverseTransposeWorldViewMatrix,
                InstancedReference->InstancedMaterial
                });
        }

        // Create the per-instance vertex buffer.
        D3D11_BUFFER_DESC instanceBufferDesc;
        ZeroMemory(&instanceBufferDesc, sizeof(D3D11_BUFFER_DESC));

        instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        instanceBufferDesc.ByteWidth = sizeof(InstancedObjectConstantBuffer) * instanceData.size();
        instanceBufferDesc.CPUAccessFlags = 0;
        instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = instanceData.data();
        resourceData.SysMemPitch = 0;
        resourceData.SysMemSlicePitch = 0;

        HRESULT hr = Model::CreateBuffer(&instanceBufferDesc, &resourceData, &InstancedReference->InstanceDataBuffer);
        if (FAILED(hr)) {
            DisplayError("Failed to create buffer");
        }
    }
    else
    {
        Model::Load(ModelPath.c_str(), Scale, batchedVertices);
    }
}

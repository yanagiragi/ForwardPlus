# DebugDraw

> This readme is a backup from the original source.
>
> Original Source:
>
> https://github.com/microsoft/DirectXTK/wiki/DebugDraw

This provides a set of helpers that use [[PrimitiveBatch]] to draw common debug shapes like boxes, spheres, frustums, etc. They are drawn as simple lines which makes them ideally suited for visualizing bounding volumes, collision data, etc.

![Collision Sample](https://github.com/Microsoft/DirectXTK/wiki/images/DebugDraw.PNG)

[DebugDraw.h](https://github.com/Microsoft/DirectXTK/wiki/DebugDraw.h)

 ```cpp
#include <DirectXColors.h>
#include <DirectXCollision.h>

void XM_CALLCONV Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    const DirectX::BoundingSphere& sphere,
    DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    const DirectX::BoundingBox& box,
    DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    const DirectX::BoundingOrientedBox& obb,
    DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    const DirectX::BoundingFrustum& frustum,
    DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawGrid(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis,
    DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs,
    DirectX::GXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawRing(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR origin, DirectX::FXMVECTOR majorAxis, DirectX::FXMVECTOR minorAxis,
    DirectX::GXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawRay(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR origin, DirectX::FXMVECTOR direction, bool normalize = true,
    DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawTriangle(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR pointA, DirectX::FXMVECTOR pointB, DirectX::FXMVECTOR pointC,
    DirectX::GXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawQuad(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR pointA, DirectX::FXMVECTOR pointB,
    DirectX::FXMVECTOR pointC, DirectX::GXMVECTOR pointD,
    DirectX::HXMVECTOR color = DirectX::Colors::White);    
```

[DebugDraw.cpp](https://github.com/Microsoft/DirectXTK/wiki/DebugDraw.cpp)

```cpp
#include "DebugDraw.h"

using namespace DirectX;

namespace
{
    inline void XM_CALLCONV DrawCube(PrimitiveBatch<VertexPositionColor>* batch,
        CXMMATRIX matWorld,
        FXMVECTOR color)
    {
        static const XMVECTORF32 s_verts[8] =
        {
            { { { -1.f, -1.f, -1.f, 0.f } } },
            { { {  1.f, -1.f, -1.f, 0.f } } },
            { { {  1.f, -1.f,  1.f, 0.f } } },
            { { { -1.f, -1.f,  1.f, 0.f } } },
            { { { -1.f,  1.f, -1.f, 0.f } } },
            { { {  1.f,  1.f, -1.f, 0.f } } },
            { { {  1.f,  1.f,  1.f, 0.f } } },
            { { { -1.f,  1.f,  1.f, 0.f } } }
        };

        static const WORD s_indices[] =
        {
            0, 1,
            1, 2,
            2, 3,
            3, 0,
            4, 5,
            5, 6,
            6, 7,
            7, 4,
            0, 4,
            1, 5,
            2, 6,
            3, 7
        };

        VertexPositionColor verts[8];
        for (size_t i = 0; i < 8; ++i)
        {
            XMVECTOR v = XMVector3Transform(s_verts[i], matWorld);
            XMStoreFloat3(&verts[i].position, v);
            XMStoreFloat4(&verts[i].color, color);
        }

        batch->DrawIndexed(D3D_PRIMITIVE_TOPOLOGY_LINELIST, s_indices, static_cast<UINT>(std::size(s_indices)), verts, 8);
    }
}


void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
    const BoundingSphere& sphere,
    FXMVECTOR color)
{
    XMVECTOR origin = XMLoadFloat3(&sphere.Center);

    const float radius = sphere.Radius;

    XMVECTOR xaxis = g_XMIdentityR0 * radius;
    XMVECTOR yaxis = g_XMIdentityR1 * radius;
    XMVECTOR zaxis = g_XMIdentityR2 * radius;

    DrawRing(batch, origin, xaxis, zaxis, color);
    DrawRing(batch, origin, xaxis, yaxis, color);
    DrawRing(batch, origin, yaxis, zaxis, color);
}


void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
    const BoundingBox& box,
    FXMVECTOR color)
{
    XMMATRIX matWorld = XMMatrixScaling(box.Extents.x, box.Extents.y, box.Extents.z);
    XMVECTOR position = XMLoadFloat3(&box.Center);
    matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

    DrawCube(batch, matWorld, color);
}


void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
    const BoundingOrientedBox& obb,
    FXMVECTOR color)
{
    XMMATRIX matWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
    XMMATRIX matScale = XMMatrixScaling(obb.Extents.x, obb.Extents.y, obb.Extents.z);
    matWorld = XMMatrixMultiply(matScale, matWorld);
    XMVECTOR position = XMLoadFloat3(&obb.Center);
    matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

    DrawCube(batch, matWorld, color);
}


void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
    const BoundingFrustum& frustum,
    FXMVECTOR color)
{
    XMFLOAT3 corners[BoundingFrustum::CORNER_COUNT];
    frustum.GetCorners(corners);

    VertexPositionColor verts[24] = {};
    verts[0].position = corners[0];
    verts[1].position = corners[1];
    verts[2].position = corners[1];
    verts[3].position = corners[2];
    verts[4].position = corners[2];
    verts[5].position = corners[3];
    verts[6].position = corners[3];
    verts[7].position = corners[0];

    verts[8].position = corners[0];
    verts[9].position = corners[4];
    verts[10].position = corners[1];
    verts[11].position = corners[5];
    verts[12].position = corners[2];
    verts[13].position = corners[6];
    verts[14].position = corners[3];
    verts[15].position = corners[7];

    verts[16].position = corners[4];
    verts[17].position = corners[5];
    verts[18].position = corners[5];
    verts[19].position = corners[6];
    verts[20].position = corners[6];
    verts[21].position = corners[7];
    verts[22].position = corners[7];
    verts[23].position = corners[4];

    for (size_t j = 0; j < std::size(verts); ++j)
    {
        XMStoreFloat4(&verts[j].color, color);
    }

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, verts, static_cast<UINT>(std::size(verts)));
}


void XM_CALLCONV DrawGrid(PrimitiveBatch<VertexPositionColor>* batch,
    FXMVECTOR xAxis,
    FXMVECTOR yAxis,
    FXMVECTOR origin,
    size_t xdivs,
    size_t ydivs,
    GXMVECTOR color)
{
    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float percent = float(i) / float(xdivs);
        percent = (percent * 2.f) - 1.f;
        XMVECTOR scale = XMVectorScale(xAxis, percent);
        scale = XMVectorAdd(scale, origin);

        VertexPositionColor v1(XMVectorSubtract(scale, yAxis), color);
        VertexPositionColor v2(XMVectorAdd(scale, yAxis), color);
        batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        FLOAT percent = float(i) / float(ydivs);
        percent = (percent * 2.f) - 1.f;
        XMVECTOR scale = XMVectorScale(yAxis, percent);
        scale = XMVectorAdd(scale, origin);

        VertexPositionColor v1(XMVectorSubtract(scale, xAxis), color);
        VertexPositionColor v2(XMVectorAdd(scale, xAxis), color);
        batch->DrawLine(v1, v2);
    }
}


void XM_CALLCONV DrawRing(PrimitiveBatch<VertexPositionColor>* batch,
    FXMVECTOR origin,
    FXMVECTOR majorAxis,
    FXMVECTOR minorAxis,
    GXMVECTOR color)
{
    static const size_t c_ringSegments = 32;

    VertexPositionColor verts[c_ringSegments + 1];

    FLOAT fAngleDelta = XM_2PI / float(c_ringSegments);
    // Instead of calling cos/sin for each segment we calculate
    // the sign of the angle delta and then incrementally calculate sin
    // and cosine from then on.
    XMVECTOR cosDelta = XMVectorReplicate(cosf(fAngleDelta));
    XMVECTOR sinDelta = XMVectorReplicate(sinf(fAngleDelta));
    XMVECTOR incrementalSin = XMVectorZero();
    static const XMVECTORF32 s_initialCos =
    {
        1.f, 1.f, 1.f, 1.f
    };
    XMVECTOR incrementalCos = s_initialCos.v;
    for (size_t i = 0; i < c_ringSegments; i++)
    {
        XMVECTOR pos = XMVectorMultiplyAdd(majorAxis, incrementalCos, origin);
        pos = XMVectorMultiplyAdd(minorAxis, incrementalSin, pos);
        XMStoreFloat3(&verts[i].position, pos);
        XMStoreFloat4(&verts[i].color, color);
        // Standard formula to rotate a vector.
        XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
        XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
        incrementalCos = newCos;
        incrementalSin = newSin;
    }
    verts[c_ringSegments] = verts[0];

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, c_ringSegments + 1);
}


void XM_CALLCONV DrawRay(PrimitiveBatch<VertexPositionColor>* batch,
    FXMVECTOR origin,
    FXMVECTOR direction,
    bool normalize,
    FXMVECTOR color)
{
    VertexPositionColor verts[3];
    XMStoreFloat3(&verts[0].position, origin);

    XMVECTOR normDirection = XMVector3Normalize(direction);
    XMVECTOR rayDirection = (normalize) ? normDirection : direction;

    XMVECTOR perpVector = XMVector3Cross(normDirection, g_XMIdentityR1);

    if (XMVector3Equal(XMVector3LengthSq(perpVector), g_XMZero))
    {
        perpVector = XMVector3Cross(normDirection, g_XMIdentityR2);
    }
    perpVector = XMVector3Normalize(perpVector);

    XMStoreFloat3(&verts[1].position, XMVectorAdd(rayDirection, origin));
    perpVector = XMVectorScale(perpVector, 0.0625f);
    normDirection = XMVectorScale(normDirection, -0.25f);
    rayDirection = XMVectorAdd(perpVector, rayDirection);
    rayDirection = XMVectorAdd(normDirection, rayDirection);
    XMStoreFloat3(&verts[2].position, XMVectorAdd(rayDirection, origin));

    XMStoreFloat4(&verts[0].color, color);
    XMStoreFloat4(&verts[1].color, color);
    XMStoreFloat4(&verts[2].color, color);

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 2);
}


void XM_CALLCONV DrawTriangle(PrimitiveBatch<VertexPositionColor>* batch,
    FXMVECTOR pointA,
    FXMVECTOR pointB,
    FXMVECTOR pointC,
    GXMVECTOR color)
{
    VertexPositionColor verts[4];
    XMStoreFloat3(&verts[0].position, pointA);
    XMStoreFloat3(&verts[1].position, pointB);
    XMStoreFloat3(&verts[2].position, pointC);
    XMStoreFloat3(&verts[3].position, pointA);

    XMStoreFloat4(&verts[0].color, color);
    XMStoreFloat4(&verts[1].color, color);
    XMStoreFloat4(&verts[2].color, color);
    XMStoreFloat4(&verts[3].color, color);

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 4);
}


void XM_CALLCONV DX::DrawQuad(PrimitiveBatch<VertexPositionColor>* batch,
    FXMVECTOR pointA,
    FXMVECTOR pointB,
    FXMVECTOR pointC,
    GXMVECTOR pointD,
    HXMVECTOR color)
{
    VertexPositionColor verts[5];
    XMStoreFloat3(&verts[0].position, pointA);
    XMStoreFloat3(&verts[1].position, pointB);
    XMStoreFloat3(&verts[2].position, pointC);
    XMStoreFloat3(&verts[3].position, pointD);
    XMStoreFloat3(&verts[4].position, pointA);

    XMStoreFloat4(&verts[0].color, color);
    XMStoreFloat4(&verts[1].color, color);
    XMStoreFloat4(&verts[2].color, color);
    XMStoreFloat4(&verts[3].color, color);
    XMStoreFloat4(&verts[4].color, color);

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 5);
}
```

# Example (DirectX 11)

To use the debug draw routines in your application, set up drawing with ``PrimitiveBatch`` per the usual setup (see [[Simple rendering]] for more details).

```cpp
m_states = std::make_unique<CommonStates>(device);
m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

m_effect = std::make_unique<BasicEffect>(device);
m_effect->SetVertexColorEnabled(true);
m_effect->SetView(...);
m_effect->SetProjection(...);

{
    void const* shaderByteCode;
    size_t byteCodeLength;

    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    DX::ThrowIfFailed(
        device->CreateInputLayout(
            VertexPositionColor::InputElements, VertexPositionColor::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_layout.ReleaseAndGetAddressOf()));
}
```

To render , call ``Begin`` and then use the debug draw functions on the 'open' ``PrimitiveBatch``, then call ``End``.

```cpp
context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
context->OMSetDepthStencilState(m_states->DepthNone(), 0);
context->RSSetState(m_states->CullNone());

m_effect->Apply(context);

context->IASetInputLayout(m_layout.Get());

m_batch->Begin();

Draw(m_batch.get(), frustum, Colors::Blue); // BoundingFrustum
Draw(m_batch.get(), box, Colors::Blue); // BoundingBox
Draw(m_batch.get(), orientedBox, Colors::Blue); // BoundingOrientedBox
Draw(m_batch.get(), sphere, Colors::Blue); // BoundingSphere

m_batch->End();
```

# Example (DirectX 12)

To use the debug draw routines in your application, set up drawing with ``PrimitiveBatch`` per the usual setup (see [Simple rendering](https://github.com/Microsoft/DirectXTK12/wiki/Simple-rendering) for more details).

```cpp
m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(device);

EffectPipelineStateDescription pd(
    &VertexPositionColor::InputLayout,
    CommonStates::Opaque,
    CommonStates::DepthDefault,
    CommonStates::CullNone,
    rtState);

m_effect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, pd);
m_effect->SetView(...);
m_effect->SetProjection(...);
```

To render , call ``Begin`` and then use the debug draw functions on the 'open' ``PrimitiveBatch``, then call ``End``.

```cpp
m_effect->Apply(commandList);

m_batch->Begin(commandList);

Draw(m_batch.get(), frustum, Colors::Blue); // BoundingFrustum
Draw(m_batch.get(), box, Colors::Blue); // BoundingBox
Draw(m_batch.get(), orientedBox, Colors::Blue); // BoundingOrientedBox
Draw(m_batch.get(), sphere, Colors::Blue); // BoundingSphere

m_batch->End();
```

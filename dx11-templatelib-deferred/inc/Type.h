#pragma once

#include "Light.h"
#include "Material.h"

#include "SimpleMath.h"
using namespace DirectX::SimpleMath;

#pragma region Enumeration

enum ConstantBuffer
{
    CB_Frame,
    CB_Object,
    CB_Material,
    CB_Light,
    CB_Debug,
    CB_ScreenToViewParams,
    NumConstantBuffers
};

enum class RenderMode
{
    Forward,
    Deferred,
    LEN_RENDER_MODE
};

enum class Deferred_DebugMode
{
    None,
    LightAccumulation,
    Diffuse,
    Specular,
    Normal,
    Depth,
    LEN_DEFERRED_DEBUGMODE
};

#pragma endregion

#pragma region Structures

struct FrameConstantBuffer
{
    Matrix ViewMatrix;
    Matrix ProjectionMatrix;
};

struct ObjectConstantBuffer
{
    Matrix WorldMatrix;
    Matrix InverseTransposeWorldMatrix;
    Matrix WorldViewProjectionMatrix;
};

struct InstancedObjectConstantBuffer
{
    Matrix WorldMatrix;
    Matrix InverseTransposeWorldMatrix;
    struct Material Material;
};


struct ScreenToViewParams
{
    Matrix InverseView;
    Matrix InverseProjection;
    Vector2 ScreenDimensions;
    float padding[2];
};

struct DebugProperties
{
    int DeferredDebugMode = 0;
    float DeferredDepthPower = 0;
    float padding[2];
};

#pragma endregion
#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

#define MAX_TEXTURES 64

struct Material
{
    Vector4  Emissive = Vector4::Zero;
    Vector4  Ambient = Vector4::One;
    Vector4  Diffuse = Vector4::One;
    Vector4  Specular = Vector4::One;
    int TextureId = -1;
    float SpecularPower = 128.0f;
    Vector2 Padding;
};

struct MaterialProperties
{
    struct Material Material;
};
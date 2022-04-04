// ==============================================================
//
// Constant Buffers
// 
// ==============================================================

cbuffer PerApplication : register(b0)
{
    matrix projectionMatrix;
}

cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    float4 EyePosition;
    struct Light Lights[MAX_LIGHTS];
}

cbuffer PerObject : register(b2)
{
    matrix modelMatrix;
    matrix normalMatrix;
    struct _Material Material;
}
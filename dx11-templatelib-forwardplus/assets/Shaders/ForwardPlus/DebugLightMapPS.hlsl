#include "../Structures.hlsli"

cbuffer ScreenToViewParams : register(b0)
{
    float4x4 InverseView;
    float4x4 InverseProjection;
    float2 ScreenDimensions;
    float2 ThreadGroups;
}

Texture2D<uint2> Lightmap : register(t0);

sampler Sampler : register(s0);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    float4 color;

    uint2 texValue = Lightmap.Load(int3(IN.uv * ThreadGroups, 0));
    float lightCount = texValue.y / ThreadGroups.y;

    // float lightCount = texValue.y / MAX_LIGHTS;
    color = float4(texValue.x / ThreadGroups.x, lightCount, 0, 1);

    return color;
}
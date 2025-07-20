#include "../Structures.hlsli"

cbuffer ScreenToViewParams : register(b0)
{
    float4x4 InverseView;
    float4x4 InverseProjection;
    float2 ScreenDimensions;
    float2 ThreadGroups;
}

cbuffer DebugProperties : register(b1)
{
    int DebugMode;                 // 4 bytes
    float DepthPower;         // 4 bytes
    float padding[2];         // 8 bytes
                              //----------(16 byte boundary)
}; // Total:                  // 16 bytes (1 * 16 byte boundary)

Texture2D<uint2> Lightmap : register(t0);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    float4 color;
    uint2 texValue = Lightmap.Load(int3(IN.uv * ThreadGroups, 0));

    if (DebugMode == FP_DEBUG_MODE_UV)
    {
        color = float4(texValue.x / ThreadGroups.x, texValue.y / ThreadGroups.y, 0, 1);
    }

    else if (DebugMode == FP_DEBUG_MODE_LIGHT_MAP)
    {
        float lightCount = texValue.y / (float)MAX_LIGHTS;
        color = float4(lightCount, lightCount, lightCount, 1);
    }

    else if (DebugMode == FP_DEBUG_MODE_DEPTH)
    {
        color = float4(texValue.yyy / DepthPower, 1);
    }

    return color;
}
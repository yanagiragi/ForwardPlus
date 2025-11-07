#include "../Structures.hlsli"

cbuffer DispatchParams : register(b0)
{
    // Number of groups dispatched
    uint3 numThreadGroups;
    uint padding1;

    // Total number of threads dispatched
    // Note this value may be less than the actual number of threads executed
    // if the screen size is not divisible by the block size
    uint3 numThreads;
    uint padding2;
}

cbuffer DebugProperties : register(b1)
{
    int DebugMode;                 // 4 bytes
    float DepthPower;         // 4 bytes
    float padding3[2];         // 8 bytes
                              //----------(16 byte boundary)
}; // Total:                  // 16 bytes (1 * 16 byte boundary)

cbuffer LightingCalculationOptions : register(b2)
{
    int lightingSpace;        // 4 bytes
    int lightCount;           // 4 bytes
    int lightIndex;           // 4 bytes
    float padding4;            // 4 bytes
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
    uint2 texValue = Lightmap.Load(int3(IN.uv * numThreadGroups, 0));

    if (DebugMode == FP_DEBUG_MODE_UV)
    {
        color = float4(texValue.x / (float)numThreadGroups.x, texValue.y / (float)numThreadGroups.y, 0, 1);
    }

    else if (DebugMode == FP_DEBUG_MODE_LIGHT_MAP)
    {
        float t = texValue.y / (float)lightCount;
        color = float4(t.xxx, 1);
    }

    else if (DebugMode == FP_DEBUG_MODE_DEPTH)
    {
        color = float4(texValue.yyy / DepthPower, 1);
    }

    else if (DebugMode == FP_DEBUG_MODE_DEPTH_DIFF)
    {
        color = float4(texValue.xy / 20.0, 0.0, 1);
    }

    return color;
}
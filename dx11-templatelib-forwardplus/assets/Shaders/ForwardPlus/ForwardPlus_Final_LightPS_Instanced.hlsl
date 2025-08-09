#include "../Structures.hlsli"
#include "../Lighting.hlsli"
#include "Common.hlsli"

cbuffer LightProperties : register(b0)
{
    float4 EyePosition;                 // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 GlobalAmbient;               // 16 bytes
    //----------------------------------- (16 byte boundary)
    struct LightProperties Lights[MAX_LIGHTS];    // 80 * 8 = 640 bytes
};  // Total:                           // 672 bytes (42 * 16 byte boundary)

cbuffer LightingCalculationOptions : register(b1)
{
    int lightingSpace;        // 4 bytes
    int lightCount;           // 4 bytes
    int lightIndex;           // 4 bytes
    float padding;            // 4 bytes
                              //----------(16 byte boundary)
}; // Total:                  // 16 bytes (1 * 16 byte boundary)

cbuffer DispatchParams : register(b2)
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

Texture2D Texture : register(t0);
StructuredBuffer<uint> LightIndexList : register( t1 );
Texture2D<uint2> LightGrid : register( t2 );

sampler Sampler : register(s0);

#define WORLD_SPACE 0
#define VIEW_SPACE 1

// ==============================================================
//
// Main Function
// 
// ==============================================================

struct PixelShaderInput
{
    float4 PositionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 PositionWS : TEXCOORD1;
    float3 PositionVS : TEXCOORD2;
    float3 NormalWS : TEXCOORD3;
    float3 NormalVS : TEXCOORD4;
    struct MaterialProperties Material : MATERIAL;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    LightingResult lit = { {0, 0, 0}, {0, 0, 0}};

    // Get the index of the current pixel in the light grid.
    uint2 tileIndex = uint2( floor(IN.PositionCS.xy / BLOCK_SIZE) );

    // Get the start position and offset of the light in the light index list.
    uint startOffset = LightGrid[tileIndex].x;
    uint lightCount = LightGrid[tileIndex].y;

    LightingResult singleLightLit = { {0, 0, 0}, {0, 0, 0}};
    for ( uint i = 0; i < lightCount; i++ )
    {
        // TODO: light index seems wrong, might need to take a look at CullLight.hlsl
        uint lightIndex = LightIndexList[startOffset + i];

        if (lightingSpace == WORLD_SPACE)
        {
            singleLightLit = ComputeLightingWS_Single(Lights[lightIndex], IN.PositionWS, normalize(IN.NormalWS), IN.Material.SpecularPower, EyePosition);
        }
        else if (lightingSpace == VIEW_SPACE)
        {
            singleLightLit = ComputeLightingVS_Single(Lights[lightIndex], IN.PositionVS, normalize(IN.NormalVS), IN.Material.SpecularPower);
        }

        lit.Diffuse += singleLightLit.Diffuse;
        lit.Specular += singleLightLit.Specular;
    }
    float3 emissive = IN.Material.Emissive;
    float3 ambient = IN.Material.Ambient * GlobalAmbient;
    float3 diffuse = IN.Material.Diffuse * lit.Diffuse;
    float3 specular = IN.Material.Specular * lit.Specular;

    float4 texColor = { 1, 1, 1, 1 };

    if (IN.Material.UseTexture)
    {
        texColor = Texture.Sample(Sampler, IN.uv);
    }

    return float4((emissive + ambient + diffuse) * texColor.rgb + specular, 1.0);
}
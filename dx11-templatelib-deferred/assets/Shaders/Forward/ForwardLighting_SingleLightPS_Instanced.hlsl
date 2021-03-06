#include "../Structures.hlsli"
#include "../Lighting.hlsli"

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

Texture2D Texture : register(t0);
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

    float3 emissive = IN.Material.Emissive;
    float3 ambient = IN.Material.Ambient * GlobalAmbient;

    float4 texColor = { 1, 1, 1, 1 };
    if (IN.Material.UseTexture)
    {
        texColor = Texture.Sample(Sampler, IN.uv);
    }

    if(lightIndex == -1)
    {
        return float4((emissive + ambient) * texColor.rgb, 1.0);
    }
    else
    {
        if (lightingSpace == WORLD_SPACE)
        {
            lit = ComputeLightingWS_Single(Lights[lightIndex], IN.PositionWS, normalize(IN.NormalWS), IN.Material.SpecularPower, EyePosition);
        }
        else if (lightingSpace == VIEW_SPACE)
        {
            lit = ComputeLightingVS_Single(Lights[lightIndex], IN.PositionVS, normalize(IN.NormalVS), IN.Material.SpecularPower);
        }
    }

    float3 diffuse = IN.Material.Diffuse * lit.Diffuse;
    float3 specular = IN.Material.Specular * lit.Specular;

    return float4(diffuse * texColor.rgb + specular, 1.0);
}
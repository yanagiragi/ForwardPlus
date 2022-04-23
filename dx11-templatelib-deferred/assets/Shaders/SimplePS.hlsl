#include "Lighting.hlsli"

cbuffer MaterialProperties : register(b0)
{
    struct _Material Material;
};

cbuffer LightProperties : register(b1)
{
    float4 EyePosition;                 // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 GlobalAmbient;               // 16 bytes
    //----------------------------------- (16 byte boundary)
    struct Light Lights[MAX_LIGHTS];    // 80 * 8 = 640 bytes
};  // Total:                           // 672 bytes (42 * 16 byte boundary)


Texture2D Texture : register(t0);
sampler Sampler : register(s0);

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
    float3 NormalWS : TEXCOORD2;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    LightingResult lit = ComputeLighting(Lights, IN.PositionWS, normalize(IN.NormalWS), Material.SpecularPower, EyePosition.xyz);

    float3 emissive = Material.Emissive;
    float3 ambient = Material.Ambient * GlobalAmbient;
    float3 diffuse = Material.Diffuse * lit.Diffuse;
    float3 specular = Material.Specular * lit.Specular;

    float4 texColor = { 1, 1, 1, 1 };

    if (Material.UseTexture)
    {
        texColor = Texture.Sample(Sampler, IN.uv);
    }

    return float4((emissive + ambient + diffuse) * texColor.rgb + specular, 1.0);
}
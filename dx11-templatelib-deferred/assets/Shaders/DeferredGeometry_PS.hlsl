#include "Structures.hlsli"

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
    float3 emissive = Material.Emissive;
    float3 ambient = Material.Ambient * GlobalAmbient;
    float3 diffuse = Material.Diffuse;
    float4 specular = float4(Material.Specular.rgb, log2(Material.SpecularPower) / 10.5f);

    float4 texColor = { 1, 1, 1, 1 };

    if (Material.UseTexture)
    {
        texColor = Texture.Sample(Sampler, IN.uv);
    }

    return specular;
    return float4(diffuse, 1.0);
    return float4(texColor.rgb * diffuse, 1.0);
    return float4(emissive + ambient, 1.0);
    return float4(IN.PositionWS, 1.0);
    return float4(IN.NormalWS, 1.0);
}
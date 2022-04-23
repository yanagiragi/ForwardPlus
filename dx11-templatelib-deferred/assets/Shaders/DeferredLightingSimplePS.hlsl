#include "Lighting.hlsli"

cbuffer LightProperties : register(b0)
{
    float4 EyePosition;                 // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 GlobalAmbient;               // 16 bytes
    //----------------------------------- (16 byte boundary)
    struct Light Lights[MAX_LIGHTS];    // 80 * 8 = 640 bytes
};  // Total:                           // 672 bytes (42 * 16 byte boundary)

// ==============================================================
//
// Main Function
// 
// ==============================================================

Texture2D GBuffer_LightAccumulation : register(t0);
Texture2D GBuffer_Diffuse : register(t1);
Texture2D GBuffer_Specular : register(t2);
Texture2D GBuffer_Normal : register(t3);

sampler Sampler : register(s0);

struct PixelShaderInput
{
    float4 positionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    float4 accumulated = GBuffer_LightAccumulation.Sample(Sampler, IN.uv);
    float4 diffuse = GBuffer_Diffuse.Sample(Sampler, IN.uv);
    
    float4 specular = GBuffer_Specular.Sample(Sampler, IN.uv);
    float specularPower = exp2(specular.a * 10.5f);
    
    float4 normalRaw = GBuffer_Normal.Sample(Sampler, IN.uv);
    float3 normalWS = normalize(normalRaw.rgb * 2.0 - 1.0); // never normalize a vector4!

    // todo
    float3 positionWS = float3(0, 0, 0);

    LightingResult lit = ComputeLighting(Lights, positionWS, normalWS, specularPower, EyePosition.xyz);

    float3 color = accumulated.rgb + diffuse.rgb * lit.Diffuse + specular.rgb * lit.Specular;

    return float4(color, 1.0);
}
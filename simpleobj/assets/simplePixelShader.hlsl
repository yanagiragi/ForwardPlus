cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    float4 lightPosition;
    float4 lightDirection;
}

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldNormal : TEXCOORD1;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    float3 normal = normalize(IN.worldNormal);
    float NdotL = dot(lightDirection.xyz, normal);
    
    return float4(NdotL, NdotL, NdotL, 1);
}
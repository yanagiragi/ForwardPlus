cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    float4 lightParam1; // position, type
    float4 lightParam2; // direction, strength
}

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    // unpack light params
    float lightPosition = lightParam1.xyz;
    float lightType = lightParam1.w;
    float lightDirection = lightParam2.xyz;
    float lightStrengh = lightParam2.w;

    float attentuation;
    float3 lightVector;
    
    float3 normal = normalize(IN.worldNormal);

    // directional light
    if (lightType == 0)
    {
        lightVector = lightDirection;
        attentuation = 1.0f;
    }

    // point light
    else if (lightType == 1)
    {
        lightVector = lightPosition - IN.worldPosition;
        float distance = length(lightVector);
        attentuation = 1.0f / (distance * distance);
    }

    float NdotL = dot(lightVector, normal);
    float c = attentuation * NdotL * lightStrengh;
    
    return float4(c, c, c, 1);
}
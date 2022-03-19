#define LIGHT_COUNT 2

struct LightParam
{
    float4 Param1; // position, type
    float4 Param2; // direction, strength
};

cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    struct LightParam lightParams[LIGHT_COUNT];
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
    float3 normal = normalize(IN.worldNormal);
    float3 finalColor = float3(0, 0, 0);

    float attentuation;
    float3 lightVector;

    for (int i = 0; i < LIGHT_COUNT; ++i)
    {
        // unpack light params
        float lightPosition = lightParams[i].Param1.xyz;
        float lightType = lightParams[i].Param1.w;
        float lightDirection = lightParams[i].Param2.xyz;
        float lightStrengh = lightParams[i].Param2.w;

        // None
        if (lightType == 0)
        {
            attentuation = 0;
        }

        // directional light
        else if (lightType == 1)
        {
            lightVector = lightDirection;
            attentuation = 1.0f;
        }

        // point light
        else if (lightType == 2)
        {
            lightVector = lightPosition - IN.worldPosition;
            float distance = length(lightVector);
            attentuation = 1.0f / (distance * distance);
        }

        float NdotL = dot(lightVector, normal);
        float c = attentuation * NdotL * lightStrengh;

        finalColor += float3(c, c, c);
    }

    return float4(finalColor, 1);
}
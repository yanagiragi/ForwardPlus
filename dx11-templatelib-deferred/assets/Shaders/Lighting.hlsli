#include "Structures.hlsli"

float4 DoDiffuse(Light light, float3 L, float3 N)
{
    float NdotL = max(0, dot(N, L));
    return light.Color * NdotL;
}

float4 DoSpecular(Light light, float3 V, float3 L, float3 N, float specularPower)
{
    // Phong lighting.
    float3 R = normalize(reflect(-L, N));
    float RdotV = max(0, dot(R, V));

    // Blinn-Phong lighting
    // float3 H = normalize(L + V);
    // float NdotH = max(0, dot(N, H));

    return light.Color * pow(RdotV, specularPower);
}

float DoAttenuation(Light light, float d)
{
    return 1.0f / (light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d);
}

float DoSpotCone(Light light, float3 L)
{
    float minCos = cos(light.SpotAngle);
    float maxCos = (minCos + 1.0f) / 2.0f;
    float cosAngle = dot(light.Direction.xyz, -L);
    return smoothstep(minCos, maxCos, cosAngle);
}

LightingResult DoDirectionalLight(Light light, float3 V, float3 N, float specularPower)
{
    LightingResult result;

    float3 L = light.Direction.xyz;

    result.Diffuse = DoDiffuse(light, L, N);
    result.Specular = DoSpecular(light, V, L, N, specularPower);

    return result;
}

LightingResult DoPointLight(Light light, float3 V, float3 P, float3 N, float specularPower)
{
    LightingResult result;
    
    float3 L = (light.Position - P).xyz;
    float distance = length(L);
    L = L / distance;

    float attenuation = DoAttenuation(light, distance);

    result.Diffuse = DoDiffuse(light, L, N) * attenuation;
    result.Specular = DoSpecular(light, V, L, N, specularPower) * attenuation;

    return result;
}

LightingResult DoSpotLight(Light light, float3 V, float3 P, float3 N, float specularPower)
{
    LightingResult result;

    float3 L = (light.Position - P).xyz;
    float distance = length(L);
    L = L / distance;

    float attenuation = DoAttenuation(light, distance);
    float spotIntensity = DoSpotCone(light, -L);

    result.Diffuse = DoDiffuse(light, L, N) * attenuation * spotIntensity;
    result.Specular = DoSpecular(light, V, L, N, specularPower) * attenuation * spotIntensity;

    return result;
}

LightingResult ComputeLighting(Light Lights[MAX_LIGHTS], float3 position, float3 normal, float specularPower, float3 eyePosition)
{
    float3 view = normalize(eyePosition - position).xyz;
    
    LightingResult totalResult = { {0, 0, 0}, {0, 0, 0} };
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        if (!Lights[i].Enabled)
        {
            continue;
        }

        LightingResult result = { {0, 0, 0}, {0, 0, 0} };

        switch ((int)Lights[i].LightType)
        {
        case DIRECTIONAL_LIGHT:
            result = DoDirectionalLight(Lights[i], view, normal, specularPower);
            break;
        case POINT_LIGHT:
            result = DoPointLight(Lights[i], view, position, normal, specularPower);
            break;
        case SPOT_LIGHT:
            result = DoSpotLight(Lights[i], view, position, normal, specularPower);
            break;
        }

        totalResult.Diffuse += result.Diffuse * Lights[i].Strength;
        totalResult.Specular += result.Specular * Lights[i].Strength;
    }

    return totalResult;
}

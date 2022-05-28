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

float DoSpotConeWS(Light light, float3 L)
{
    float minCos = cos(light.SpotAngle);
    float maxCos = (minCos + 1.0f) / 2.0f;
    float cosAngle = dot(light.DirectionWS.xyz, -L);
    return smoothstep(minCos, maxCos, cosAngle);
}

float DoSpotConeVS(Light light, float3 L)
{
    float minCos = cos(light.SpotAngle);
    float maxCos = (minCos + 1.0f) / 2.0f;
    float cosAngle = dot(light.DirectionVS.xyz, -L);
    return smoothstep(minCos, maxCos, cosAngle);
}

LightingResult _DoDirectionalLight(Light light, float3 V, float3 N, float specularPower, float3 L)
{
    LightingResult result;
    result.Diffuse = DoDiffuse(light, L, N);
    result.Specular = DoSpecular(light, V, L, N, specularPower);
    return result;
}

LightingResult DoDirectionalLightWS(Light light, float3 V, float3 N, float specularPower)
{
    float3 L = normalize(light.DirectionWS.xyz);
    return _DoDirectionalLight(light, V, N, specularPower, L);
}

LightingResult DoDirectionalLightVS(Light light, float3 V, float3 N, float specularPower)
{
    float3 L = normalize(light.DirectionVS.xyz); // buggy! Note: negative direction gets lit but still wrong
    return _DoDirectionalLight(light, V, N, specularPower, L);
}

LightingResult _DoPointLight(Light light, float3 V, float3 P, float3 N, float specularPower, float3 L)
{
    LightingResult result;
    
    float distance = length(L);
    L = L / distance;

    float attenuation = DoAttenuation(light, distance);

    result.Diffuse = DoDiffuse(light, L, N) * attenuation;
    result.Specular = DoSpecular(light, V, L, N, specularPower) * attenuation;

    return result;
}

LightingResult DoPointLightWS(Light light, float3 V, float3 P, float3 N, float specularPower)
{
    float3 L = (light.PositionWS - P).xyz;
    return _DoPointLight(light, V, P, N, specularPower, L);
}

LightingResult DoPointLightVS(Light light, float3 V, float3 P, float3 N, float specularPower)
{
    float3 L = (light.PositionVS - P).xyz;
    return _DoPointLight(light, V, P, N, specularPower, L);
}

LightingResult _DoSpotLight(Light light, float3 V, float3 P, float3 N, float specularPower, float3 L, float spotIntensity)
{
    LightingResult result;

    float distance = length(L);
    L = L / distance;

    float attenuation = DoAttenuation(light, distance);

    result.Diffuse = DoDiffuse(light, L, N) * attenuation * spotIntensity;
    result.Specular = DoSpecular(light, V, L, N, specularPower) * attenuation * spotIntensity;

    return result;
}

LightingResult DoSpotLightWS(Light light, float3 V, float3 P, float3 N, float specularPower)
{
    float3 L = (light.PositionWS - P).xyz;
    float spotIntensity = DoSpotConeWS(light, -normalize(L));
    return _DoSpotLight(light, V, P, N, specularPower, L, spotIntensity);
}

LightingResult DoSpotLightVS(Light light, float3 V, float3 P, float3 N, float specularPower)
{
    float3 L = (light.PositionVS - P).xyz;
    float spotIntensity = DoSpotConeVS(light, -normalize(L));
    return _DoSpotLight(light, V, P, N, specularPower, L, spotIntensity);
}

LightingResult ComputeLightingVS(Light Lights[MAX_LIGHTS], int lightCount, float3 positionVS, float3 normalVS, float specularPower)
{
    // view space calculation is still buggy!

    float3 view = normalize(positionVS);
    
    LightingResult totalResult = { {0, 0, 0}, {0, 0, 0} };
    
    [unroll]
    for (int i = 0; i < lightCount; ++i)
    {
        if (!Lights[i].Enabled)
        {
            continue;
        }

        LightingResult result = { {0, 0, 0}, {0, 0, 0} };

        switch ((int)Lights[i].LightType)
        {
        case DIRECTIONAL_LIGHT:
            result = DoDirectionalLightVS(Lights[i], view, normalVS, specularPower);
            break;
        case POINT_LIGHT:
            result = DoPointLightVS(Lights[i], view, positionVS, normalVS, specularPower);
            break;
        case SPOT_LIGHT:
            result = DoSpotLightVS(Lights[i], view, positionVS, normalVS, specularPower);
            break;
        }

        totalResult.Diffuse += result.Diffuse * Lights[i].Strength;
        totalResult.Specular += result.Specular * Lights[i].Strength;
    }

    return totalResult;
}

LightingResult ComputeLightingVS_Single(Light light, float3 positionVS, float3 normalVS, float specularPower)
{
    // view space calculation is still buggy!

    float3 view = normalize(positionVS);
    
    LightingResult totalResult = { {0, 0, 0}, {0, 0, 0} };
    
    if (!light.Enabled)
    {
        return totalResult;
    }

    LightingResult result = { {0, 0, 0}, {0, 0, 0} };

    switch ((int)light.LightType)
    {
    case DIRECTIONAL_LIGHT:
        result = DoDirectionalLightVS(light, view, normalVS, specularPower);
        break;
    case POINT_LIGHT:
        result = DoPointLightVS(light, view, positionVS, normalVS, specularPower);
        break;
    case SPOT_LIGHT:
        result = DoSpotLightVS(light, view, positionVS, normalVS, specularPower);
        break;
    }

    totalResult.Diffuse += result.Diffuse * light.Strength;
    totalResult.Specular += result.Specular * light.Strength;

    return totalResult;
}

LightingResult ComputeLightingWS(Light Lights[MAX_LIGHTS], int lightCount, float3 positionWS, float3 normalWS, float specularPower, float3 eyePosition)
{
    float3 view = normalize(eyePosition - positionWS);
    
    LightingResult totalResult = { {0, 0, 0}, {0, 0, 0} };
    
    [unroll]
    for (int i = 0; i < lightCount; ++i)
    {
        if (!Lights[i].Enabled)
        {
            continue;
        }

        LightingResult result = { {0, 0, 0}, {0, 0, 0} };

        switch ((int)Lights[i].LightType)
        {
        case DIRECTIONAL_LIGHT:
            result = DoDirectionalLightWS(Lights[i], view, normalWS, specularPower);
            break;
        case POINT_LIGHT:
            result = DoPointLightWS(Lights[i], view, positionWS, normalWS, specularPower);
            break;
        case SPOT_LIGHT:
            result = DoSpotLightWS(Lights[i], view, positionWS, normalWS, specularPower);
            break;
        }

        totalResult.Diffuse += result.Diffuse * Lights[i].Strength;
        totalResult.Specular += result.Specular * Lights[i].Strength;
    }

    return totalResult;
}

LightingResult ComputeLightingWS_Single(Light light, float3 positionWS, float3 normalWS, float specularPower, float3 eyePosition)
{
    float3 view = normalize(eyePosition - positionWS);
    
    LightingResult totalResult = { {0, 0, 0}, {0, 0, 0} };
    
    if (!light.Enabled)
    {
        return totalResult;
    }

    LightingResult result = { {0, 0, 0}, {0, 0, 0} };

    switch ((int)light.LightType)
    {
    case DIRECTIONAL_LIGHT:
        result = DoDirectionalLightWS(light, view, normalWS, specularPower);
        break;
    case POINT_LIGHT:
        result = DoPointLightWS(light, view, positionWS, normalWS, specularPower);
        break;
    case SPOT_LIGHT:
        result = DoSpotLightWS(light, view, positionWS, normalWS, specularPower);
        break;
    }

    totalResult.Diffuse += result.Diffuse * light.Strength;
    totalResult.Specular += result.Specular * light.Strength;

    return totalResult;
}
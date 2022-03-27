#define MAX_LIGHTS 8

// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

// ==============================================================
//
// Structures
// 
// ==============================================================
struct Light
{
    float4      Position;               // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4      Direction;              // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4      Color;                  // 16 bytes
    //----------------------------------- (16 byte boundary)
    float       SpotAngle;              // 4 bytes
    float       ConstantAttenuation;    // 4 bytes
    float       LinearAttenuation;      // 4 bytes
    float       QuadraticAttenuation;   // 4 bytes
    //----------------------------------- (16 byte boundary)
    int         LightType;              // 4 bytes
    bool        Enabled;                // 4 bytes
    float       Strength;               // 4 bytes
    int         Padding;                // 4 bytes
    //----------------------------------- (16 byte boundary)
};  // Total:                           // 80 bytes (5 * 16)

struct _Material
{
    float4  Emissive;       // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4  Ambient;        // 16 bytes
    //------------------------------------(16 byte boundary)
    float4  Diffuse;        // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4  Specular;       // 16 bytes
    //----------------------------------- (16 byte boundary)
    float   SpecularPower;  // 4 bytes
    bool    UseTexture;     // 4 bytes
    float2  Padding;        // 8 bytes
    //----------------------------------- (16 byte boundary)
};  // Total:               // 80 bytes ( 5 * 16 )

struct LightingResult
{
    float3 Diffuse;
    float3 Specular;
};

// ==============================================================
//
// Constant Buffers
// 
// ==============================================================

cbuffer PerApplication : register(b0)
{
    matrix projectionMatrix;
}

cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    float4 EyePosition;
    struct Light Lights[MAX_LIGHTS];
}

cbuffer PerObject : register(b2)
{
    matrix modelMatrix;
    matrix normalMatrix;
    struct _Material Material;
}

// ==============================================================
//
// Functions
// 
// ==============================================================

float4 DoDiffuse(Light light, float3 L, float3 N)
{
    float NdotL = max(0, dot(N, L));
    return light.Color * NdotL;
}

float4 DoSpecular(Light light, float3 V, float3 L, float3 N)
{
    // Phong lighting.
    float3 R = normalize(reflect(-L, N));
    float RdotV = max(0, dot(R, V));

    // Blinn-Phong lighting
    // float3 H = normalize(L + V);
    // float NdotH = max(0, dot(N, H));

    return light.Color * pow(RdotV, Material.SpecularPower);
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

LightingResult DoDirectionalLight(Light light, float3 V, float3 P, float3 N)
{
    LightingResult result;

    float3 L = light.Direction.xyz;

    result.Diffuse = DoDiffuse(light, L, N);
    result.Specular = DoSpecular(light, V, L, N);

    return result;
}

LightingResult DoPointLight(Light light, float3 V, float3 P, float3 N)
{
    LightingResult result;
    
    float3 L = (light.Position - P).xyz;
    float distance = length(L);
    L = L / distance;

    float attenuation = DoAttenuation(light, distance);

    result.Diffuse = DoDiffuse(light, L, N) * attenuation;
    result.Specular = DoSpecular(light, V, L, N) * attenuation;

    return result;
}

LightingResult DoSpotLight(Light light, float3 V, float3 P, float3 N)
{
    LightingResult result;

    float3 L = (light.Position - P).xyz;
    float distance = length(L);
    L = L / distance;

    float attenuation = DoAttenuation(light, distance);
    float spotIntensity = DoSpotCone(light, -L);

    result.Diffuse = DoDiffuse(light, L, N) * attenuation * spotIntensity;
    result.Specular = DoSpecular(light, V, L, N) * attenuation * spotIntensity;

    return result;
}

LightingResult ComputeLighting(float3 position, float3 normal)
{
    float3 view = normalize(EyePosition - position).xyz;
    
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
            result = DoDirectionalLight(Lights[i], view, position, normal);
            break;
        case POINT_LIGHT:
            result = DoPointLight(Lights[i], view, position, normal);
            break;
        case SPOT_LIGHT:
            result = DoSpotLight(Lights[i], view, position, normal);
            break;
        }

        totalResult.Diffuse += result.Diffuse * Lights[i].Strength;
        totalResult.Specular += result.Specular * Lights[i].Strength;
    }

    return totalResult;
}

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
    float GlobalAmbient = 0.05f;

    LightingResult lit = ComputeLighting(IN.PositionWS, normalize(IN.NormalWS));

    float3 emissive = Material.Emissive;
    float3 ambient = Material.Ambient * GlobalAmbient;
    float3 diffuse = Material.Diffuse * lit.Diffuse;
    float3 specular = Material.Specular * lit.Specular;

    float4 texColor = { 1, 1, 1, 1 };

    if (Material.UseTexture)
    {
        // texColor = Texture.Sample(Sampler, IN.uv);
    }

    return float4((emissive + ambient + diffuse + specular) * texColor.rgb, 1.0);
}
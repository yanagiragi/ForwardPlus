#include "../Structures.hlsli"
#include "../Lighting.hlsli"

cbuffer MaterialProperties : register(b0)
{
    struct MaterialProperties Material;
};

cbuffer LightProperties : register(b1)
{
    float4 EyePosition;                 // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 GlobalAmbient;               // 16 bytes
    //----------------------------------- (16 byte boundary)
    struct LightProperties Lights[MAX_LIGHTS];    // 80 * 8 = 640 bytes
};  // Total:                           // 672 bytes (42 * 16 byte boundary)

cbuffer LightingCalculationOptions : register(b2)
{
    int lightingSpace;        // 4 bytes
    int lightCount;           // 4 bytes
    int lightIndex;           // 4 bytes
    float padding;            // 4 bytes
                              //----------(16 byte boundary)
}; // Total:                  // 16 bytes (1 * 16 byte boundary)

sampler Sampler : register(s0);

// Since only DX12 supports bindless texture, manual declare potential need textures here
Texture2D Texture0 : register(t0);
Texture2D Texture1 : register(t1);
Texture2D Texture2 : register(t2);
Texture2D Texture3 : register(t3);
Texture2D Texture4 : register(t4);
Texture2D Texture5 : register(t5);
Texture2D Texture6 : register(t6);
Texture2D Texture7 : register(t7);
Texture2D Texture8 : register(t8);
Texture2D Texture9 : register(t9);
Texture2D Texture10 : register(t10);
Texture2D Texture11 : register(t11);
Texture2D Texture12 : register(t12);
Texture2D Texture13 : register(t13);
Texture2D Texture14 : register(t14);
Texture2D Texture15 : register(t15);
Texture2D Texture16 : register(t16);
Texture2D Texture17 : register(t17);
Texture2D Texture18 : register(t18);
Texture2D Texture19 : register(t19);
Texture2D Texture20 : register(t20);
Texture2D Texture21 : register(t21);
Texture2D Texture22 : register(t22);
Texture2D Texture23 : register(t23);
Texture2D Texture24 : register(t24);
Texture2D Texture25 : register(t25);
Texture2D Texture26 : register(t26);
Texture2D Texture27 : register(t27);
Texture2D Texture28 : register(t28);
Texture2D Texture29 : register(t29);
Texture2D Texture30 : register(t30);
Texture2D Texture31 : register(t31);
Texture2D Texture32 : register(t32);
Texture2D Texture33 : register(t33);
Texture2D Texture34 : register(t34);
Texture2D Texture35 : register(t35);
Texture2D Texture36 : register(t36);
Texture2D Texture37 : register(t37);
Texture2D Texture38 : register(t38);
Texture2D Texture39 : register(t39);
Texture2D Texture40 : register(t40);
Texture2D Texture41 : register(t41);
Texture2D Texture42 : register(t42);
Texture2D Texture43 : register(t43);
Texture2D Texture44 : register(t44);
Texture2D Texture45 : register(t45);
Texture2D Texture46 : register(t46);
Texture2D Texture47 : register(t47);
Texture2D Texture48 : register(t48);
Texture2D Texture49 : register(t49);
Texture2D Texture50 : register(t50);
Texture2D Texture51 : register(t51);
Texture2D Texture52 : register(t52);
Texture2D Texture53 : register(t53);
Texture2D Texture54 : register(t54);
Texture2D Texture55 : register(t55);
Texture2D Texture56 : register(t56);
Texture2D Texture57 : register(t57);
Texture2D Texture58 : register(t58);
Texture2D Texture59 : register(t59);
Texture2D Texture60 : register(t60);
Texture2D Texture61 : register(t61);
Texture2D Texture62 : register(t62);
Texture2D Texture63 : register(t63);

float4 SampleTexture(sampler Sampler, float2 uv, int textureId)
{
    if (textureId == 0) return Texture0.Sample(Sampler, uv);
    else if (textureId == 1) return Texture1.Sample(Sampler, uv);
    else if (textureId == 2) return Texture2.Sample(Sampler, uv);
    else if (textureId == 3) return Texture3.Sample(Sampler, uv);
    else if (textureId == 4) return Texture4.Sample(Sampler, uv);
    else if (textureId == 5) return Texture5.Sample(Sampler, uv);
    else if (textureId == 6) return Texture6.Sample(Sampler, uv);
    else if (textureId == 7) return Texture7.Sample(Sampler, uv);
    else if (textureId == 8) return Texture8.Sample(Sampler, uv);
    else if (textureId == 9) return Texture9.Sample(Sampler, uv);
    else if (textureId == 10) return Texture10.Sample(Sampler, uv);
    else if (textureId == 11) return Texture11.Sample(Sampler, uv);
    else if (textureId == 12) return Texture12.Sample(Sampler, uv);
    else if (textureId == 13) return Texture13.Sample(Sampler, uv);
    else if (textureId == 14) return Texture14.Sample(Sampler, uv);
    else if (textureId == 15) return Texture15.Sample(Sampler, uv);
    else if (textureId == 16) return Texture16.Sample(Sampler, uv);
    else if (textureId == 17) return Texture17.Sample(Sampler, uv);
    else if (textureId == 18) return Texture18.Sample(Sampler, uv);
    else if (textureId == 19) return Texture19.Sample(Sampler, uv);
    else if (textureId == 20) return Texture20.Sample(Sampler, uv);
    else if (textureId == 21) return Texture21.Sample(Sampler, uv);
    else if (textureId == 22) return Texture22.Sample(Sampler, uv);
    else if (textureId == 23) return Texture23.Sample(Sampler, uv);
    else if (textureId == 24) return Texture24.Sample(Sampler, uv);
    else if (textureId == 25) return Texture25.Sample(Sampler, uv);
    else if (textureId == 26) return Texture26.Sample(Sampler, uv);
    else if (textureId == 27) return Texture27.Sample(Sampler, uv);
    else if (textureId == 28) return Texture28.Sample(Sampler, uv);
    else if (textureId == 29) return Texture29.Sample(Sampler, uv);
    else if (textureId == 30) return Texture30.Sample(Sampler, uv);
    else if (textureId == 31) return Texture31.Sample(Sampler, uv);
    else if (textureId == 32) return Texture32.Sample(Sampler, uv);
    else if (textureId == 33) return Texture33.Sample(Sampler, uv);
    else if (textureId == 34) return Texture34.Sample(Sampler, uv);
    else if (textureId == 35) return Texture35.Sample(Sampler, uv);
    else if (textureId == 36) return Texture36.Sample(Sampler, uv);
    else if (textureId == 37) return Texture37.Sample(Sampler, uv);
    else if (textureId == 38) return Texture38.Sample(Sampler, uv);
    else if (textureId == 39) return Texture39.Sample(Sampler, uv);
    else if (textureId == 40) return Texture40.Sample(Sampler, uv);
    else if (textureId == 41) return Texture41.Sample(Sampler, uv);
    else if (textureId == 42) return Texture42.Sample(Sampler, uv);
    else if (textureId == 43) return Texture43.Sample(Sampler, uv);
    else if (textureId == 44) return Texture44.Sample(Sampler, uv);
    else if (textureId == 45) return Texture45.Sample(Sampler, uv);
    else if (textureId == 46) return Texture46.Sample(Sampler, uv);
    else if (textureId == 47) return Texture47.Sample(Sampler, uv);
    else if (textureId == 48) return Texture48.Sample(Sampler, uv);
    else if (textureId == 49) return Texture49.Sample(Sampler, uv);
    else if (textureId == 50) return Texture50.Sample(Sampler, uv);
    else if (textureId == 51) return Texture51.Sample(Sampler, uv);
    else if (textureId == 52) return Texture52.Sample(Sampler, uv);
    else if (textureId == 53) return Texture53.Sample(Sampler, uv);
    else if (textureId == 54) return Texture54.Sample(Sampler, uv);
    else if (textureId == 55) return Texture55.Sample(Sampler, uv);
    else if (textureId == 56) return Texture56.Sample(Sampler, uv);
    else if (textureId == 57) return Texture57.Sample(Sampler, uv);
    else if (textureId == 58) return Texture58.Sample(Sampler, uv);
    else if (textureId == 59) return Texture59.Sample(Sampler, uv);
    else if (textureId == 60) return Texture60.Sample(Sampler, uv);
    else if (textureId == 61) return Texture61.Sample(Sampler, uv);
    else if (textureId == 62) return Texture62.Sample(Sampler, uv);
    else if (textureId == 63) return Texture63.Sample(Sampler, uv);
    return float4(0, 0, 0, 0);
}

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
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    LightingResult lit = { {0, 0, 0}, {0, 0, 0}};

    float3 emissive = Material.Emissive;
    float3 ambient = Material.Ambient * GlobalAmbient;

    float4 texColor = { 1, 1, 1, 1 };
    if (Material.TextureId >= 0)
    {
        texColor = SampleTexture(Sampler, IN.uv, Material.TextureId);
    }

    if(lightIndex == -1)
    {
        return float4((emissive + ambient) * texColor.rgb, 1.0);
    }
    else
    {
        if (lightingSpace == WORLD_SPACE)
        {
            lit = ComputeLightingWS_Single(Lights[lightIndex], IN.PositionWS, normalize(IN.NormalWS), Material.SpecularPower, EyePosition);
        }
        else if (lightingSpace == VIEW_SPACE)
        {
            lit = ComputeLightingVS_Single(Lights[lightIndex], IN.PositionVS, normalize(IN.NormalVS), Material.SpecularPower);
        }
    }

    float3 diffuse = Material.Diffuse * lit.Diffuse;
    float3 specular = Material.Specular * lit.Specular;

    return float4(diffuse * texColor.rgb + specular, 1.0);
}
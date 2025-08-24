#include "../Structures.hlsli"
#include "../Lighting.hlsli"
#include "Common.hlsli"

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

cbuffer DispatchParams : register(b3)
{
    // Number of groups dispatched
    uint3 numThreadGroups;
    uint padding1;

    // Total number of threads dispatched
    // Note this value may be less than the actual number of threads executed
    // if the screen size is not divisible by the block size
    uint3 numThreads;
    uint padding2;
}

StructuredBuffer<uint> LightIndexList : register( t0 );
Texture2D<uint2> LightGrid : register( t1 );

// Since only DX12 supports bindless texture, manual declare potential need textures here
Texture2D Texture0  : register(t2);
Texture2D Texture1  : register(t3);
Texture2D Texture2  : register(t4);
Texture2D Texture3  : register(t5);
Texture2D Texture4  : register(t6);
Texture2D Texture5  : register(t7);
Texture2D Texture6  : register(t8);
Texture2D Texture7  : register(t9);
Texture2D Texture8  : register(t10);
Texture2D Texture9  : register(t11);
Texture2D Texture10 : register(t12);
Texture2D Texture11 : register(t13);
Texture2D Texture12 : register(t14);
Texture2D Texture13 : register(t15);
Texture2D Texture14 : register(t16);
Texture2D Texture15 : register(t17);
Texture2D Texture16 : register(t18);
Texture2D Texture17 : register(t19);
Texture2D Texture18 : register(t20);
Texture2D Texture19 : register(t21);
Texture2D Texture20 : register(t22);
Texture2D Texture21 : register(t23);
Texture2D Texture22 : register(t24);
Texture2D Texture23 : register(t25);
Texture2D Texture24 : register(t26);
Texture2D Texture25 : register(t27);
Texture2D Texture26 : register(t28);
Texture2D Texture27 : register(t29);
Texture2D Texture28 : register(t30);
Texture2D Texture29 : register(t31);
Texture2D Texture30 : register(t32);
Texture2D Texture31 : register(t33);
Texture2D Texture32 : register(t34);
Texture2D Texture33 : register(t35);
Texture2D Texture34 : register(t36);
Texture2D Texture35 : register(t37);
Texture2D Texture36 : register(t38);
Texture2D Texture37 : register(t39);
Texture2D Texture38 : register(t40);
Texture2D Texture39 : register(t41);
Texture2D Texture40 : register(t42);
Texture2D Texture41 : register(t43);
Texture2D Texture42 : register(t44);
Texture2D Texture43 : register(t45);
Texture2D Texture44 : register(t46);
Texture2D Texture45 : register(t47);
Texture2D Texture46 : register(t48);
Texture2D Texture47 : register(t49);
Texture2D Texture48 : register(t50);
Texture2D Texture49 : register(t51);
Texture2D Texture50 : register(t52);
Texture2D Texture51 : register(t53);
Texture2D Texture52 : register(t54);
Texture2D Texture53 : register(t55);
Texture2D Texture54 : register(t56);
Texture2D Texture55 : register(t57);
Texture2D Texture56 : register(t58);
Texture2D Texture57 : register(t59);
Texture2D Texture58 : register(t60);
Texture2D Texture59 : register(t61);
Texture2D Texture60 : register(t62);
Texture2D Texture61 : register(t63);
Texture2D Texture62 : register(t64);
Texture2D Texture63 : register(t65);

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

sampler Sampler : register(s0);

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

    // Get the index of the current pixel in the light grid.
    uint2 tileIndex = uint2( floor(IN.PositionCS.xy / BLOCK_SIZE) );

    // Get the start position and offset of the light in the light index list.
    uint startOffset = LightGrid[tileIndex].x;
    uint lightCount = LightGrid[tileIndex].y;

    LightingResult singleLightLit = { {0, 0, 0}, {0, 0, 0} };
    for ( uint i = 0; i < lightCount; i++ )
    {
        uint index = LightIndexList[startOffset + i];

        if (lightingSpace == WORLD_SPACE)
        {
            singleLightLit = ComputeLightingWS_Single(Lights[index], IN.PositionWS, normalize(IN.NormalWS), Material.SpecularPower, EyePosition);
        }
        else if (lightingSpace == VIEW_SPACE)
        {
            singleLightLit = ComputeLightingVS_Single(Lights[index], IN.PositionVS, normalize(IN.NormalVS), Material.SpecularPower);
        }

        lit.Diffuse += singleLightLit.Diffuse;
        lit.Specular += singleLightLit.Specular;
    }

    float3 emissive = Material.Emissive;
    float3 ambient = Material.Ambient * GlobalAmbient;
    float3 diffuse = Material.Diffuse * lit.Diffuse;
    float3 specular = Material.Specular * lit.Specular;

    float4 texColor = { 1, 1, 1, 1 };
    if (Material.TextureId >= 0)
    {
        texColor = SampleTexture(Sampler, IN.uv, Material.TextureId);
    }

    return float4((emissive + ambient + diffuse) * texColor.rgb + specular, 1.0);
}
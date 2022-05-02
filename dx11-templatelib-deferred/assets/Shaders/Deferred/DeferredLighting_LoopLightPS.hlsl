#include "../Structures.hlsli"
#include "../Lighting.hlsli"

cbuffer LightProperties : register(b0)
{
    float4 EyePosition;                 // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 GlobalAmbient;               // 16 bytes
    //----------------------------------- (16 byte boundary)
    struct Light Lights[MAX_LIGHTS];    // 80 * 8 = 640 bytes
};  // Total:                           // 672 bytes (42 * 16 byte boundary)

cbuffer ScreenToViewParams : register( b1 )
{
    float4x4 InverseView;
    float4x4 InverseProjection;
    float2 ScreenDimensions;
}

// ==============================================================
//
// Main Function
// 
// ==============================================================

Texture2D GBuffer_LightAccumulation : register(t0);
Texture2D GBuffer_Diffuse : register(t1);
Texture2D GBuffer_Specular : register(t2);
Texture2D GBuffer_Normal : register(t3);
Texture2D GBuffer_Depth : register(t4);

sampler Sampler : register(s0);

struct PixelShaderInput
{
    float4 positionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 ViewToWorld( float4 view )
{
    float4 world = mul( InverseView, view );
    return world;
}

// Convert clip space coordinates to view space
float4 ClipToView( float4 clip )
{
    float4 view = mul( InverseProjection, clip );
    view = view / view.w; 
    return view;
}
 
// Convert screen space coordinates to view space.
float4 ScreenToView( float4 screen )
{
    float2 texCoord = screen.xy / ScreenDimensions;
    float4 clip = float4( float2( texCoord.x, 1.0f - texCoord.y ) * 2.0f - 1.0f, screen.z, screen.w); 
    return ClipToView( clip );
}

float4 main(PixelShaderInput IN) : SV_TARGET
{
    int2 texCoord = IN.positionCS.xy;
    float depth = GBuffer_Depth.Load( int3( texCoord, 0 ) ).r;
    float4 positionVS = ScreenToView( float4( texCoord, depth, 1.0f ) );
    float4 positionWS = ViewToWorld(positionVS);

    float4 accumulated = GBuffer_LightAccumulation.Sample(Sampler, IN.uv);
    float4 diffuse = GBuffer_Diffuse.Sample(Sampler, IN.uv);
    
    float4 specular = GBuffer_Specular.Sample(Sampler, IN.uv);
    float specularPower = exp2(specular.a * 10.5f);
    
    float4 normalRaw = GBuffer_Normal.Sample(Sampler, IN.uv);
    float3 normalWS = normalize(normalRaw.rgb * 2.0 - 1.0); // never normalize a vector4!

    LightingResult lit = ComputeLightingWS(Lights, positionWS.xyz, normalWS, specularPower, EyePosition.xyz);

    float3 color = accumulated.rgb + diffuse.rgb * lit.Diffuse + specular.rgb * lit.Specular;

    return float4(color, 1.0);
}
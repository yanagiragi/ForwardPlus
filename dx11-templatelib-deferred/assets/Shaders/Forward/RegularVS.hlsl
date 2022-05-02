cbuffer PerFrame : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}

cbuffer PerObject : register(b1)
{
    matrix WorldMatrix;
    matrix InverseTransposeWorldMatrix;
    matrix InverseTransposeWorldViewMatrix;
    matrix WorldViewProjectionMatrix;
}

// ==============================================================
//
// Main Functions
// 
// ==============================================================

struct AppData
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 PositionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 PositionWS : TEXCOORD1;
    float3 PositionVS : TEXCOORD2;
    float3 NormalWS : TEXCOORD3;
    float3 NormalVS : TEXCOORD4;
};

VertexShaderOutput main(AppData IN)
{
    VertexShaderOutput OUT;
    OUT.PositionCS = mul(WorldViewProjectionMatrix, float4(IN.position, 1.0f));
    OUT.PositionWS = mul(WorldMatrix, float4(IN.position, 1.0f));
    OUT.PositionVS = mul(ViewMatrix, float4(OUT.PositionWS, 1.0f));
    OUT.NormalWS = mul(InverseTransposeWorldMatrix, float4(IN.normal, 1.0f));
    OUT.NormalVS = mul(InverseTransposeWorldViewMatrix, float4(IN.normal, 1.0f));
    OUT.uv = IN.uv;
    return OUT;
}
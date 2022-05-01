#include "Structures.hlsli"

cbuffer PerFrame : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}

// ==============================================================
//
// Main Functions
// 
// ==============================================================

struct AppData
{
    // Per-vertex data
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    // Per-instance data
    matrix WorldMatrix : WORLDMATRIX;
    matrix InverseTransposeWorldMatrix : NORMALWORLDMATRIX;
    matrix InverseTransposeWorldViewMatrix : NORMALVIEWMATRIX;
    struct _Material Material : MATERIAL;
};

struct VertexShaderOutput
{
    float4 PositionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 PositionWS : TEXCOORD1;
    float3 PositionVS : TEXCOORD2;
    float3 NormalWS : TEXCOORD3;
    float3 NormalVS : TEXCOORD4;
    struct _Material Material : MATERIAL;
};

VertexShaderOutput main(AppData IN)
{
    VertexShaderOutput OUT;
    matrix mvp = mul(ProjectionMatrix, mul(ViewMatrix, IN.WorldMatrix));
    OUT.PositionCS = mul(mvp, float4(IN.position, 1.0f));
    OUT.PositionWS = mul(IN.WorldMatrix, float4(IN.position, 1.0f));
    OUT.PositionVS = mul(ViewMatrix, float4(OUT.PositionWS, 1.0f));
    OUT.NormalWS = mul(IN.InverseTransposeWorldMatrix, float4(IN.normal, 1.0f));
    OUT.NormalVS = mul(IN.InverseTransposeWorldViewMatrix, float4(IN.normal, 1.0f));
    OUT.Material = IN.Material;
    OUT.uv = IN.uv;
    return OUT;
}
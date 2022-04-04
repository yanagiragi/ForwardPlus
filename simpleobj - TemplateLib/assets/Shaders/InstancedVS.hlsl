#include "Structures.hlsli"
#include "ConstrantBuffers.hlsli"

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
    matrix NormalMatrix : NORMALMATRIX;
    struct _Material Material : MATERIAL;
};

struct VertexShaderOutput
{
    float4 PositionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 PositionWS : TEXCOORD1;
    float3 NormalWS : TEXCOORD2;
    struct _Material Material : MATERIAL;
};

VertexShaderOutput main(AppData IN)
{
    VertexShaderOutput OUT;
    matrix mvp = mul(projectionMatrix, mul(viewMatrix, IN.WorldMatrix));
    OUT.PositionCS = mul(mvp, float4(IN.position, 1.0f));
    OUT.PositionWS = mul(IN.WorldMatrix, float4(IN.position, 1.0f));
    OUT.NormalWS = mul(IN.NormalMatrix, float4(IN.normal, 1.0f));
    OUT.Material = IN.Material;
    OUT.uv = IN.uv;
    return OUT;
}
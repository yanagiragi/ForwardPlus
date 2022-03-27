#include "Structures.hlsli"
#include "ConstrantBuffers.hlsli"

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
    float3 NormalWS : TEXCOORD2;
};

VertexShaderOutput main(AppData IN)
{
    VertexShaderOutput OUT;
    matrix mvp = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
    OUT.PositionCS = mul(mvp, float4(IN.position, 1.0f));
    OUT.PositionWS = mul(modelMatrix, float4(IN.position, 1.0f));
    OUT.NormalWS = mul(normalMatrix, float4(IN.normal, 1.0f));
    OUT.uv = IN.uv;
    return OUT;
}
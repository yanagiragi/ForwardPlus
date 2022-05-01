cbuffer PerObject : register(b0)
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
    float3 NormalWS : TEXCOORD1;
};

VertexShaderOutput main(AppData IN)
{
    VertexShaderOutput OUT;
    OUT.PositionCS = mul(WorldViewProjectionMatrix, float4(IN.position, 1.0f));
    OUT.NormalWS = mul(InverseTransposeWorldMatrix, float4(IN.normal, 1.0f));
    OUT.uv = IN.uv;
    return OUT;
}
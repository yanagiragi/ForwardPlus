cbuffer PerApplication : register(b0)
{
    matrix projectionMatrix;
}

#define LIGHT_COUNT 2

struct LightParam
{
    float4 Param1; // position, type
    float4 Param2; // direction, strength
};

cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    struct LightParam lightParams[LIGHT_COUNT];
}

cbuffer PerObject : register(b2)
{
    matrix modelMatrix;
    matrix normalMatrix;
}

struct AppData
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
};

VertexShaderOutput main(AppData IN)
{
    VertexShaderOutput OUT;
    matrix mvp = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
    OUT.position = mul(mvp, float4(IN.position, 1.0f));
    OUT.worldPosition = mul(modelMatrix, float4(IN.position, 1.0f));
    OUT.worldNormal = mul(normalMatrix, float4(IN.normal, 1.0f));
    OUT.uv = IN.uv;
    return OUT;
}
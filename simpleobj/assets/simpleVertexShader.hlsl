cbuffer PerApplication : register(b0)
{
    matrix projectionMatrix;
}

cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    float4 lightPosition;
    float4 lightDirection;
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
    float3 worldNormal : TEXCOORD1;
};

VertexShaderOutput main(AppData IN)
{
    VertexShaderOutput OUT;
    matrix mvp = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
    OUT.position = mul(mvp, float4(IN.position, 1.0f));
    OUT.worldNormal = mul(normalMatrix, float4(IN.normal, 1.0f));
    OUT.uv = IN.uv;
    return OUT;
}
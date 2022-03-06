cbuffer PerApplication : register(b0)
{
    matrix projectionMatrix;
}

cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    float one;
}

cbuffer PerObject : register(b2)
{
    matrix modelMatrix;
}

struct AppData
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 uv : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VertexShaderOutput main(AppData IN)
{
    VertexShaderOutput OUT;

    matrix mvp = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
    OUT.position = mul(mvp, float4(IN.position, 1.0f));
    OUT.color = float4(IN.normal, 1.0f);

    return OUT;
}
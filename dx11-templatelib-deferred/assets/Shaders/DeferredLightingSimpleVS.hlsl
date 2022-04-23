struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VertexShaderOutput main(uint vI : SV_VERTEXID)
{
    VertexShaderOutput OUT;
    float2 uv = float2(vI&1, vI>>1);
    float4 position = float4((uv.x - 0.5f) * 2, -(uv.y - 0.5f) * 2, 0, 1);

    OUT.uv = uv;
    OUT.position = position;
    
    return OUT;
}
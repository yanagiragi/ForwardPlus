cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    float one;
}

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    return float4(one, 0, 0, 1);
    return IN.color;
}
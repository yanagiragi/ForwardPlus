struct PixelShaderInput
{
    float4 PositionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    return float4(1.0, 0.0, 0.0, 1.0);
}
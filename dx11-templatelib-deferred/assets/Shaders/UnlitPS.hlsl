struct PixelShaderInput
{
    float4 PositionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 PositionWS : TEXCOORD1;
    float3 PositionVS : TEXCOORD2;
    float3 NormalWS : TEXCOORD3;
    float3 NormalVS : TEXCOORD4;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    return float4(1.0, 0.0, 0.0, 1.0);
}
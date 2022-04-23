Texture2D Texture : register(t0);
sampler Sampler : register(s0);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    float4 color = Texture.Sample(Sampler, IN.uv);
    return color;
    
    return float4(IN.uv ,0, 1);
}
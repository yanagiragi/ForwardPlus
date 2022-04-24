#define LIGHT_ACCUMULATION 1
#define DIFFUSE 2
#define SPECULAR 3
#define NORMAL 4
#define DEPTH 5

cbuffer DebugProperties : register(b0)
{
    int mode;                 // 4 bytes
    float depthPower;         // 4 bytes
    float padding[2];         // 12 bytes
                              //----------(16 byte boundary)
}; // Total:                  // 16 bytes (1 * 16 byte boundary)

Texture2D GBuffer_LightAccumulation : register(t0);
Texture2D GBuffer_Diffuse : register(t1);
Texture2D GBuffer_Specular : register(t2);
Texture2D GBuffer_Normal : register(t3);
Texture2D GBuffer_Depth : register(t4);

sampler Sampler : register(s0);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    float4 color;

    switch(mode)
    {
    case LIGHT_ACCUMULATION:
        color = GBuffer_LightAccumulation.Sample(Sampler, IN.uv);
        break;

    case DIFFUSE:
        color = GBuffer_Diffuse.Sample(Sampler, IN.uv);
        break;

    case SPECULAR:
        color = GBuffer_Specular.Sample(Sampler, IN.uv);
        color.rgb *= color.a; // modify by specularPower
        break;

    case NORMAL:
        color = GBuffer_Normal.Sample(Sampler, IN.uv) * 2.0 - 1.0;
        break;

    case DEPTH:
        // correct way should use Load with int3 as param (positionCS, 0)
        // however only for visualization simple sample is enough
        float d = GBuffer_Depth.Sample(Sampler, IN.uv);
        d = pow(d, depthPower);
        color = float4(d, d, d, 1);
        break;
    }

    return color;
}
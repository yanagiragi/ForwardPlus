#define LIGHT_ACCUMULATION 0
#define DIFFUSE 1
#define SPECULAR 2
#define NORMAL 3

cbuffer DebugProperties : register(b0)
{
    int mode;                 // 4 bytes
    float padding[3];         // 12 bytes
                              //----------(16 byte boundary)
}; // Total:                  // 16 bytes (1 * 16 byte boundary)

Texture2D GBuffer_LightAccumulation : register(t0);
Texture2D GBuffer_Diffuse : register(t1);
Texture2D GBuffer_Specular : register(t2);
Texture2D GBuffer_Normal : register(t3);

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
        break;

    case NORMAL:
        color = GBuffer_Normal.Sample(Sampler, IN.uv); 
        break;
    }

    return color;
    // return float4(IN.uv ,0, 1);
}
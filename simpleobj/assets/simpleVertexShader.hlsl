#define MAX_LIGHTS 8

// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

// ==============================================================
//
// Structures
// 
// ==============================================================
struct Light
{
    float4      Position;               // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4      Direction;              // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4      Color;                  // 16 bytes
    //----------------------------------- (16 byte boundary)
    float       SpotAngle;              // 4 bytes
    float       ConstantAttenuation;    // 4 bytes
    float       LinearAttenuation;      // 4 bytes
    float       QuadraticAttenuation;   // 4 bytes
    //----------------------------------- (16 byte boundary)
    int         LightType;              // 4 bytes
    bool        Enabled;                // 4 bytes
    float       Strength;               // 4 bytes
    int         Padding;                // 4 bytes
    //----------------------------------- (16 byte boundary)
};  // Total:                           // 80 bytes (5 * 16)

struct _Material
{
    float4  Emissive;       // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4  Ambient;        // 16 bytes
    //------------------------------------(16 byte boundary)
    float4  Diffuse;        // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4  Specular;       // 16 bytes
    //----------------------------------- (16 byte boundary)
    float   SpecularPower;  // 4 bytes
    bool    UseTexture;     // 4 bytes
    float2  Padding;        // 8 bytes
    //----------------------------------- (16 byte boundary)
};  // Total:               // 80 bytes ( 5 * 16 )

struct LightingResult
{
    float3 Diffuse;
    float3 Specular;
};

// ==============================================================
//
// Constant Buffers
// 
// ==============================================================

cbuffer PerApplication : register(b0)
{
    matrix projectionMatrix;
}

cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    float4 EyePosition;
    struct Light Lights[MAX_LIGHTS];
}

cbuffer PerObject : register(b2)
{
    matrix modelMatrix;
    matrix normalMatrix;
    struct _Material Material;
}

// ==============================================================
//
// Main Functions
// 
// ==============================================================

struct AppData
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 PositionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 PositionWS : TEXCOORD1;
    float3 NormalWS : TEXCOORD2;
};

VertexShaderOutput main(AppData IN)
{
    VertexShaderOutput OUT;
    matrix mvp = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
    OUT.PositionCS = mul(mvp, float4(IN.position, 1.0f));
    OUT.PositionWS = mul(modelMatrix, float4(IN.position, 1.0f));
    OUT.NormalWS = mul(normalMatrix, float4(IN.normal, 1.0f));
    OUT.uv = IN.uv;
    return OUT;
}
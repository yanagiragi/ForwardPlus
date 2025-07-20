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
struct LightProperties
{
    float4      PositionWS;             // 16 bytes
    float4      PositionVS;             // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4      DirectionWS;            // 16 bytes
    float4      DirectionVS;            // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4      Color;                  // 16 bytes
    //----------------------------------- (16 byte boundary)
    float       SpotAngle;              // 4 bytes
    float       Range;                  // 4 bytes
    int         LightType;              // 4 bytes
    bool        Enabled;                // 4 bytes
    //----------------------------------- (16 byte boundary)
    float       Strength;               // 4 bytes
    float3      Padding;                // 4 bytes
    //----------------------------------- (16 byte boundary)
};  // Total:                           // 80 bytes (5 * 16)

struct MaterialProperties
{
    float4  Emissive;       // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4  Ambient;        // 16 bytes
    //------------------------------------(16 byte boundary)
    float4  Diffuse;        // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4  Specular;       // 16 bytes
    //----------------------------------- (16 byte boundary)
    bool    UseTexture;     // 4 bytes
    float   SpecularPower;  // 4 bytes
    float2  Padding;        // 8 bytes
    //----------------------------------- (16 byte boundary)
};  // Total:               // 80 bytes ( 5 * 16 )

struct LightingResult
{
    float3 Diffuse;
    float3 Specular;
};

#define FP_DEBUG_MODE_NONE 0
#define FP_DEBUG_MODE_UV 1
#define FP_DEBUG_MODE_DEPTH_TEX 2
#define FP_DEBUG_MODE_DEPTH 3
#define FP_DEBUG_MODE_LIGHT_MAP 4
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

#define MAX_LIGHTS 8

enum class LightType
{
    Directional,
    Point,
    SPOTLIGHT,
    NumLightType
};

struct Light
{
    Vector4     Position;                           // 16 bytes
    //--------------------------------------------------------- (16 byte boundary)
    Vector4     Direction;                          // 16 bytes
    //--------------------------------------------------------- (16 byte boundary)
    Vector4     Color = Vector4(1, 1, 1, 1);        // 16 bytes
    //--------------------------------------------------------- (16 byte boundary)
    float       SpotAngle = 0.0f;                   // 4 bytes
    float       ConstantAttenuation = 1.0f;         // 4 bytes
    float       LinearAttenuation = 0.08f;          // 4 bytes
    float       QuadraticAttenuation = 0.0f;        // 4 bytes
    //--------------------------------------------------------- (16 byte boundary)
    int         LightType = 0;                      // 4 bytes
    int         Enabled = false;                    // 4 bytes
    float       Strength = 0.0f;                    // 4 bytes
    int         Padding = 0;                            // 4 bytes
    //--------------------------------------------------------- (16 byte boundary)
};  // Total:                                       // 80 bytes (5 * 16)

#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

#define MAX_LIGHTS 512
#define LIGHT_EPSILON 0.01

enum class LightType
{
    Directional,
    Point,
    Spotlight,
    NumLightType
};

struct Light
{
    Vector4     PositionWS;                         // 16 bytes
    Vector4     PositionVS;                         // 16 bytes
    Vector4     DirectionWS;                        // 16 bytes
    Vector4     DirectionVS;                        // 16 bytes
    Vector4     Color = Vector4(1, 1, 1, 1);        // 16 bytes
    //--------------------------------------------------------- (16 byte boundary)
    float       SpotAngle = 0.0f;                   // 4 bytes. Angle in radians.
    float       Range = 1.0f;                       // 4 bytes
    int         LightType = 0;                      // 4 bytes
    float       Strength = 0.0f;                    // 4 bytes. Strength = 0 equals not enabled
    //--------------------------------------------------------- (16 byte boundary)
};  // Total:                                       // 96 bytes (6 * 16)

struct LightProperties
{
    LightProperties()
        : EyePosition(0.0f, 0.0f, 0.0f, 1.0f)
        , GlobalAmbient(0.2f, 0.2f, 0.8f, 1.0f)
    {}

    Vector4   EyePosition;
    Vector4   GlobalAmbient;
    //----------------------------------- (16 byte boundary)
    Light     Lights[MAX_LIGHTS];
};  // Total:                                  672 bytes (42 * 16)
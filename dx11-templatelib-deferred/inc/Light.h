#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

#define MAX_LIGHTS 8

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
    //--------------------------------------------------------- (16 byte boundary)
    Vector4     DirectionWS;                        // 16 bytes
    Vector4     DirectionVS;                        // 16 bytes
    //--------------------------------------------------------- (16 byte boundary)
    Vector4     Color = Vector4(1, 1, 1, 1);        // 16 bytes
    //--------------------------------------------------------- (16 byte boundary)
    float       SpotAngle = 0.0f;                   // 4 bytes. Angle in radians.
    float       ConstantAttenuation = 1.0f;         // 4 bytes
    float       LinearAttenuation = 0.7f;          // 4 bytes
    float       QuadraticAttenuation = 1.8f;        // 4 bytes
    //--------------------------------------------------------- (16 byte boundary)
    int         LightType = 0;                      // 4 bytes
    int         Enabled = false;                    // 4 bytes
    float       Strength = 0.0f;                    // 4 bytes
    int         Padding = 0;                            // 4 bytes
    //--------------------------------------------------------- (16 byte boundary)

    static float GetRadius(Light* light)
    {
        auto constant = light->ConstantAttenuation;
        auto linear = light->LinearAttenuation;
        auto quadratic = light->QuadraticAttenuation;
        auto strength = light->Strength;
        auto lightMax = std::fmaxf(std::fmaxf(light->Color.x, light->Color.y), light->Color.z) * strength;

        // Reference: https://learnopengl.com/Advanced-Lighting/Deferred-Shading, we use 10/256 as dark threshold
        float darkThreshold = (256.0f / 2.5f);
        return (-linear + std::sqrtf(linear * linear - 4.0f * quadratic * (constant - darkThreshold * lightMax))) / (2.0f * quadratic);
    }

};  // Total:                                       // 112 bytes (7 * 16)

struct LightProperties
{
    LightProperties()
        : EyePosition(0.0f, 0.0f, 0.0f, 1.0f)
        , GlobalAmbient(0.2f, 0.2f, 0.8f, 1.0f)
    {}

    Vector4   EyePosition;
    //----------------------------------- (16 byte boundary)
    Vector4   GlobalAmbient;
    //----------------------------------- (16 byte boundary)
    Light     Lights[MAX_LIGHTS]; // 80 * 8 bytes
};  // Total:                                  672 bytes (42 * 16)
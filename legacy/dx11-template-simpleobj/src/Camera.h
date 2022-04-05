#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

class Camera
{
public:

    inline float ToRadians(float degree)
    {
        return degree * DirectX::XM_PI / 180.0f;
    }

    inline Vector3 GetPosition()
    {
        return m_position;
    }

    inline Vector4 GetPosition_V4()
    {
        return Vector4(m_position.x, m_position.y, m_position.z, 0.0f);
    }

    inline Vector3 GetForward()
    {
        auto cosTheta = cos(ToRadians(m_theta));
        auto sinTheta = sin(ToRadians(m_theta));
        auto cosPhi = cos(ToRadians(m_phi));
        auto sinPhi = sin(ToRadians(m_phi));
        auto result = Vector3(
            cosTheta * cosPhi,
            sinTheta,
            cosTheta * sinPhi
        );
        return result;
    }

    inline Matrix GetViewMatrix()
    {
        // use LH coordinate instead of RH coordinate from DirectXTK
        return XMMatrixLookAtLH(m_position, m_position + GetForward(), Vector3::UnitY);
    }

    inline void Translate(Vector3 translation)
    {
        auto forward = GetForward();
        forward.Normalize();
        
        auto right = forward.Cross(Vector3::UnitY);
        right.Normalize();
        
        auto up = right.Cross(forward);
        up.Normalize();

        m_position += right * translation.x;
        m_position += up * translation.y;
        m_position += forward * translation.z;
        
        PrintInfo();
    }

    inline void Rotate(float theta, float phi)
    {
        auto intergerPart = 0;

        m_theta += theta;
        m_phi += phi;

        if (m_theta > 89.f)
        {
            m_theta = 89.f;
        }
        
        if (m_theta < -89.f)
        {
            m_theta = -89.f;
        }

        if (m_phi > 360.f || m_phi < -360.f)
        {
            m_phi = fmod(m_phi, 360.0f);
        }

        PrintInfo();
    }

    inline void PrintInfo()
    {
        printf("Camera position: (%f, %f, %f), Camera angle: (theta = %f, phi = %f)\n", m_position.x, m_position.y, m_position.z, m_theta, m_phi);
    }

    Camera(Vector3 position, float theta, float phi) :
        m_position(position),
        m_theta(theta),
        m_phi(phi)
    {
    }

private:
    Vector3 m_position;
    float m_theta, m_phi;
};
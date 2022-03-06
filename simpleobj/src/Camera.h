#pragma once

#define GLM_FORCE_SSE42 1
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES 1
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace glm;

class Camera
{
public:
    inline vec3 GetPosition()
    {
        return m_position;
    }

    inline vec3 GetForward()
    {
        return  glm::normalize(vec3(
            cos(radians(m_theta)) * cos(radians(m_phi)),
            sin(radians(m_theta)),
            cos(radians(m_theta)) * sin(radians(m_phi))
        ));
    }

    inline mat4 GetViewMatrix()
    {
        return lookAt(m_position, m_position + GetForward(), m_worldUp);
    }

    inline void Translate(vec3 translation)
    {
        auto forward = GetForward();
        auto right = glm::normalize(glm::cross(forward, m_worldUp));
        auto up = glm::normalize(glm::cross(right, forward));

        m_position += translation.x * right;
        m_position += translation.y * up;
        m_position += translation.z * forward;
        
        printf("Camera position: (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
    }

    inline void Rotate(float theta, float phi)
    {
        m_theta += theta;
        m_phi += phi;

        if (m_theta > 89.f)
            m_theta = 89.f;
        if (m_theta < -89.f)
            m_theta = -89.f;

        if (m_phi > 360.f || m_phi < -360.f)
            m_phi = glm::mod(m_phi, 360.f);

        printf("Camera angle: (theta = %f, phi = %f)\n", m_theta, m_phi);
    }

    Camera(vec3 position, float theta, float phi) :
        m_position(position),
        m_theta(theta),
        m_phi(phi)
    {
    }

private:
    vec3 m_position;
    float m_theta, m_phi;
    vec3 m_worldUp = glm::vec3(0, 1, 0);
};
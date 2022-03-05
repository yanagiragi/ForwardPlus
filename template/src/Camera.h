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
    inline vec3 Position()
    {
        return m_position;
    }

    inline vec3 Forward()
    {
        return vec3(
            cos(radians(m_theta)) * cos(radians(m_phi)),
            sin(radians(m_theta)),
            -cos(radians(m_theta)) * sin(radians(m_phi))
        );
    }

    inline vec3 Up()
    {
        return m_up;
    }

    inline mat4 GetViewMatrix()
    {
        return lookAt(m_position, m_position + Forward(), m_up);
    }

    inline void Translate(vec3 translation)
    {
        m_position.x += translation.z * cos(radians(m_theta)) * cos(radians(m_phi)) + translation.x * sin(radians(m_phi));
        m_position.y += translation.z * sin(radians(m_theta));
        m_position.z += translation.z * (-cos(radians(m_theta)) * sin(radians(m_phi))) + translation.x * cos(radians(m_phi));

        m_position.y += translation.y;

        // m_position += translation;
        printf("Camera position: (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
    }

    inline void Rotate(float theta, float phi)
    {
        m_theta += theta;
        m_phi += phi;
        printf("Camera angle: (theta = %f, phi = %f)\n", m_theta, m_phi);
    }

    Camera(vec3 position, float theta, float phi) :
        m_position(position),
        m_theta(theta),
        m_phi(phi),
        m_up(glm::vec3(0, 1, 0))
    {
    }

private:
    vec3 m_position;
    float m_theta, m_phi;
    vec3 m_up;
};
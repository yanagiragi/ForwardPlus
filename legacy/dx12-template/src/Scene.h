#pragma once

#define GLM_FORCE_SSE42 1
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES 1
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// TODO: Remove this?
#include <direct.h>

struct RenderInfo
{
    unsigned width;    
    unsigned height;
    float elapsedTime;
};

class Scene
{
    public:
    Scene();
    void InitializeReources();
    void Update(const RenderInfo);
    void Release();

    private:
};
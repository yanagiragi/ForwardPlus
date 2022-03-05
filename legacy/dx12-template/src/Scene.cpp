#include "Scene.h"

#include "CrossWindow/CrossWindow.h"
#include "CrossWindow/Graphics.h"

#include <chrono>

#define GLM_FORCE_SSE42 1
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES 1
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <direct.h>
#include "Common.h"
#include "Renderer.h"

using namespace glm;

Scene::Scene() 
{
    /*mUniformBuffer = nullptr;
    mUniformBufferHeap = nullptr;
    mMappedUniformBuffer = nullptr;*/
}

void Scene::Update(const RenderInfo renderInfo)
{
    /*
    // Update Uniforms
    float zoom = 2.5f;

    // Update matrices
    uboVS.projectionMatrix = glm::perspective(
        45.0f, (float)renderInfo.width / (float)renderInfo.height, 0.01f, 1024.0f);

    uboVS.viewMatrix =
        glm::translate(glm::identity<mat4>(), vec3(0.0f, 0.0f, zoom));

    uboVS.modelMatrix = glm::identity<mat4>();

    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, 0.001f * renderInfo.elapsedTime, vec3(0.0f, 1.0f, 0.0f));
    */
}

void Scene::Release() 
{
    /*if (mUniformBuffer)
    {
        mUniformBuffer->Release();
        mUniformBuffer = nullptr;
    }

    if (mUniformBufferHeap)
    {
        mUniformBufferHeap->Release();
        mUniformBufferHeap = nullptr;
    }*/
}

void Scene::InitializeReources() 
{
    // Create the vertex buffer.
    // Create the index buffer.
    // Define the vertex input layout.
    // Create the UBO.// Describe and create the graphics pipeline state object (PSO).
}
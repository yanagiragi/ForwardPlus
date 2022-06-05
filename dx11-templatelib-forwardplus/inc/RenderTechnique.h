#pragma once

#include "Scene.h"

class RenderTechnique
{
    virtual void Render(Scene & scene) = 0;
};
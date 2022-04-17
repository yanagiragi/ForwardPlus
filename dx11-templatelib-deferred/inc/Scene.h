#pragma once

#include <string>
#include <vector>
#include <map>

#include "Entity.h"
#include "Light.h"
#include "Common.h"

class Scene
{
public:
    inline void Add(Entity* entity)
    {
        if (entity->Instanced)
        {
            auto key = entity->ModelPath;
            if (InstancedEntity.find(key) != InstancedEntity.end())
            {
                InstancedEntity[key].emplace_back(entity);
            }
            else
            {
                InstancedEntity.insert({ key, { entity } });
            }
        }
        
        Entities.emplace_back(entity);
    }

    inline int Count() 
    {
        return Entities.size();
    }

    Vector4 GlobalAmbient = Vector4(0.05, 0.05, 0.05, 1.0);
    Light Lights[MAX_LIGHTS];
    std::vector<Entity*> Entities;
    std::map<std::string, std::vector<Entity*>> InstancedEntity;
};
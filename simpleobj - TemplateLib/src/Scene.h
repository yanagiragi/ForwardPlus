#pragma once

#include <string>
#include <vector>
#include <map>

#include "Entity.h"

struct InstancedEntity
{
    Entity* Entity;
    ID3D11Buffer* InstancedBuffer;
};

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

    std::vector<Entity*> Entities;
    std::map<std::string, std::vector<Entity*>> InstancedEntity;
};
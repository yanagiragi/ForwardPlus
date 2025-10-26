#pragma once

#include <string>

#include "Type.h"
#include "Model.h"

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

// Entities
class Entity
{
public:
    Entity(std::string name, std::string path, Vector3 position, Quaternion rotation, Vector3 scale = Vector3::One, bool instanced = false, Entity* instancedReference = nullptr, int instancedCount = 0) :
        Name(name),
        ModelPath(path),
        PositionWS(position),
        Rotation(rotation),
        Scale(scale),
        Instanced(instanced),
        InstancedReference(instancedReference),
        InstancedCount(instancedCount)
    {
        Setup();
    }

    void Setup();

    std::string Name;
    std::string ModelPath;

    std::vector<struct BatchedVertices> batchedVertices;
    
    bool Instanced = false;
    int InstancedCount = 0;
    Entity* InstancedReference = nullptr;
    ID3D11Buffer* InstanceDataBuffer = nullptr;
    struct Material InstancedMaterial;
    
    Vector3 PositionWS = Vector3::Zero;
    Quaternion Rotation = Quaternion::Identity;
    Vector3 Scale = Vector3::One;
    Vector3 RotateAxisSpeed = Vector3::Zero;
    
    Matrix WorldMatrix = Matrix::Identity;
    Matrix InverseTransposeWorldMatrix = Matrix::Identity;
    Matrix InverseTransposeWorldViewMatrix = Matrix::Identity;
    Matrix WorldViewProjectionMatrix = Matrix::Identity;
};
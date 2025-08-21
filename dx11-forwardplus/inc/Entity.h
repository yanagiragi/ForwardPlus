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
    Entity(std::string name, std::string path, Vector3 position, Quaternion rotation, struct Material material, bool instanced = false) :
        Name(name),
        ModelPath(path),
        PositionWS(position),
        Rotation(rotation),
        Material(material),
        Instanced(instanced)
    {
    }

    std::string Name;
    std::string ModelPath;
    
    bool Instanced = false;
    Model* Model = nullptr;
    Vector3 PositionWS = Vector3::Zero;
    Quaternion Rotation = Quaternion::Identity;
    Vector3 RotateAxisSpeed = Vector3::Zero;;
    
    struct Material Material;
    Matrix WorldMatrix = Matrix::Identity;
    Matrix InverseTransposeWorldMatrix = Matrix::Identity;
    Matrix InverseTransposeWorldViewMatrix = Matrix::Identity;
    Matrix WorldViewProjectionMatrix = Matrix::Identity;
};
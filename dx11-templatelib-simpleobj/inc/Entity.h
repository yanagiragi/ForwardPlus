#pragma once

#include <string>

#include <d3d11.h>

#include "SimpleMath.h"

#include "Material.h"
#include "Model.h"

using namespace DirectX::SimpleMath;

enum ConstantBuffer
{
    CB_Frame,
    CB_Object,
    CB_Material,
    CB_Light,
    NumConstantBuffers
};

struct FrameConstantBuffer
{
    Matrix ViewMatrix;
    Matrix ProjectionMatrix;
};

struct ObjectConstantBuffer
{
    Matrix WorldMatrix;
    Matrix InverseTransposeWorldMatrix;
    Matrix WorldViewProjectionMatrix;
};

struct InstancedObjectConstantBuffer
{
    Matrix WorldMatrix;
    Matrix InverseTransposeWorldMatrix;
    struct Material Material;
};

// Entities
class Entity
{
public:
    Entity(std::string name, std::string path, Vector3 position, Quaternion rotation, struct Material material, bool instanced = false) :
        Name(name),
        ModelPath(path),
        Position(position),
        Rotation(rotation),
        Material(material),
        Instanced(instanced)
    {
    }

    std::string Name;
    std::string ModelPath;
    
    bool Instanced = false;
    Model* Model = nullptr;
    Vector3 Position = Vector3::Zero;
    Quaternion Rotation = Quaternion::Identity;
    Vector3 RotateAxisSpeed = Vector3::Zero;;
    
    struct Material Material;
    Matrix WorldMatrix = Matrix::Identity;
    Matrix InverseTransposeWorldMatrix = Matrix::Identity;
    Matrix WorldViewProjectionMatrix = Matrix::Identity;
};
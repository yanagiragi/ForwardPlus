#pragma once

#include <string>

#include <d3d11.h>

#include "SimpleMath.h"
#include "Material.h"
#include "Model.h"

#include "Light.h"

using namespace DirectX::SimpleMath;

#define MAX_LIGHTS 8

enum ConstantBuffer
{
    CB_Application,
    CB_Frame,
    CB_Object,
    NumConstantBuffers
};

struct ApplicationConstantBuffer
{
    DirectX::SimpleMath::Matrix projectionMatrix;
};

struct FrameConstantBuffer
{
    DirectX::SimpleMath::Matrix viewMatrix;
    DirectX::SimpleMath::Vector4 eyePosition;
    struct Light lights[MAX_LIGHTS];
};

struct ObjectConstantBuffer
{
    DirectX::SimpleMath::Matrix WorldMatrix;
    DirectX::SimpleMath::Matrix NormalMatrix;
    struct Material Material;
};

// Entities
class Entity
{
public:
    Entity(std::string name, std::string path) : Name(name), ModelPath(path) {}

    Entity(std::string name, std::string path, Vector3 position, Quaternion rotation, struct Material material, bool instanced = false) :
        Name(name),
        ModelPath(path),
        Position(position),
        Rotation(rotation),
        Instanced(instanced)
    {
        ConstantBuffer.Material = material;
    }

    bool Instanced;
    std::string Name;
    std::string ModelPath;
    Model* Model = nullptr;
    Vector3 Position = Vector3::Zero;
    Quaternion Rotation = Quaternion::Identity;
    Vector3 RotateAxisSpeed;
    //ID3D11Buffer* VertexBuffer = nullptr;

    struct ObjectConstantBuffer ConstantBuffer;

    /*struct Material Material;
    Matrix WorldMatrix;
    Matrix NormalMatrix;*/
};
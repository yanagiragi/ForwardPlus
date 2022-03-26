#include <string>

#include <d3d11.h>

#include "SimpleMath.h"
#include "Model.h"

using namespace DirectX::SimpleMath;

struct Material
{
    Vector4  Emissive = Vector4::One;
    Vector4  Ambient = Vector4::One;
    Vector4  Diffuse = Vector4::One;
    Vector4  Specular = Vector4::One;
    float SpecularPower = 128.0f;
    bool UseTexture;
    Vector2 Padding;
};

struct ObjectConstantBuffer
{
    Matrix modelMatrix;
    Matrix normalMatrix;
    struct Material material;
};

// Entities
class Entity
{
public:
    Entity(std::string name, std::string path) : Name(name), ModelPath(path) {}

    Entity(std::string name, std::string path, Vector3 position, Quaternion rotation, struct Material material) :
        Name(name),
        ModelPath(path),
        Position(position),
        Rotation(rotation),
        Material(material)
    {}

    std::string Name;
    std::string ModelPath;
    struct ObjectConstantBuffer ConstantBuffer;
    Model* Model = nullptr;
    Vector3 Position = Vector3::Zero;
    Quaternion Rotation = Quaternion::Identity;
    Vector3 RotateAxisSpeed;
    ID3D11Buffer* VertexBuffer = nullptr;
    struct Material Material;
};
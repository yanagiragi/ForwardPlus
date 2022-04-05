#include <string>

#include <d3d11.h>

#include "SimpleMath.h"
#include "Material.h"
#include "Model.h"

using namespace DirectX::SimpleMath;

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
    Model* Model = nullptr;
    Vector3 Position = Vector3::Zero;
    Quaternion Rotation = Quaternion::Identity;
    Vector3 RotateAxisSpeed;
    ID3D11Buffer* VertexBuffer = nullptr;
    struct Material Material;
    Matrix WorldMatrix;
    Matrix NormalMatrix;
};
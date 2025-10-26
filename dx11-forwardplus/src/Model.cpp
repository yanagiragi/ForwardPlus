#include "Model.h"

#include "Material.h"

#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

#include <Effects.h>
#include <DirectXTex.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace Microsoft::WRL;

std::vector<ComPtr<ID3D11ShaderResourceView>> Model::m_Textures;

ID3D11Device* Model::m_d3dDevice;
ID3D11DeviceContext* Model::m_d3dDeviceContext;
DirectX::EffectFactory* Model::m_d3dEffectFactory;

int Model::LoadTexture(const wchar_t* filepath)
{
    ID3D11ShaderResourceView* newTextureView = nullptr;

    if (HasExtension(filepath, L".tga"))
    {
        DirectX::ScratchImage image;
        HRESULT hr = DirectX::LoadFromTGAFile(filepath, nullptr, image);
        if (FAILED(hr)) {
            DisplayError("Failed to load tga texture");
        }

        ComPtr<ID3D11ShaderResourceView> textureSRV;
        hr = DirectX::CreateShaderResourceView(m_d3dDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), &newTextureView);
        if (FAILED(hr)) {
            DisplayError("Failed to create tga texture");
        }

        m_Textures.push_back(newTextureView);
        return m_Textures.size() - 1;
    }
    else
    {
        try
        {
            m_d3dEffectFactory->CreateTexture(filepath, m_d3dDeviceContext, &newTextureView);
            m_Textures.push_back(newTextureView);
            return m_Textures.size() - 1;
        }
        catch (std::exception&)
        {
            DisplayError("Failed to load texture");
        }
    }

    return -1;
}

bool Model::HasExtension(const wchar_t* path, const std::wstring& ext)
{
    std::wstring filename(path);
    if (filename.length() >= ext.length()) {
        return (0 == filename.compare(filename.length() - ext.length(), ext.length(), ext));
    }
    return false;
}

bool Model::Load(const char* filepath, Vector3 scale, std::vector<struct BatchedVertices>& outBatchedVertices)
{
    tinyobj::ObjReaderConfig reader_config;

    std::string path(filepath);
    size_t pos = path.find_last_of("/\\");
    std::string folder = (pos == std::string::npos) ? "./" : path.substr(0, pos);

    reader_config.mtl_search_path = folder; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, reader_config)) {
        std::string error = reader.Error();
        if (!error.empty()) {
            DisplayError(error.c_str());
        }
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& raw_materials = reader.GetMaterials();

    std::vector<struct Material> materials;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    for (const auto& metadata : raw_materials)
    {
        struct Material material;
        material.Ambient        = Vector4( metadata.ambient[0],  metadata.ambient[1],  metadata.ambient[2], 1.0f);
        material.Diffuse        = Vector4( metadata.diffuse[0],  metadata.diffuse[1],  metadata.diffuse[2], 1.0f);
        material.Emissive       = Vector4(metadata.emission[0], metadata.emission[1], metadata.emission[2], 1.0f);
        material.Specular       = Vector4(metadata.specular[0], metadata.specular[1], metadata.specular[2], 1.0f);
        material.SpecularPower  = metadata.shininess;
        
        if (metadata.diffuse_texname.size() > 0)
        {
            auto diffuseTexture = folder + std::string("/") + metadata.diffuse_texname;

            std::wstring diffuseTexture_ws = converter.from_bytes(diffuseTexture);
            const wchar_t* diffuseTexture_wptr = diffuseTexture_ws.c_str();
            material.TextureId = LoadTexture(diffuseTexture_wptr);
        }

        materials.push_back(material);
    }

    // Loop over shapes
    std::map<int, std::vector<struct VertexData>> batchedVertices;
    std::vector<struct VertexData> vertices;
    for (size_t s = 0; s < shapes.size(); s++) {
        vertices.clear();

        auto materialId = shapes[s].mesh.material_ids[0];
        auto key = std::string(filepath) + std::string("-") + std::to_string(materialId);

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {

                struct VertexData vertex;

                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                vertex.vertex[0] = vx * scale.x;
                vertex.vertex[1] = vy * scale.y;
                vertex.vertex[2] = vz * scale.z;

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                    vertex.normal[0] = nx;
                    vertex.normal[1] = ny;
                    vertex.normal[2] = nz;
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    vertex.uv[0] = tx;
                    vertex.uv[1] = ty;
                }

                if (f >= shapes[s].mesh.material_ids.size())
                {
                    break;
                }

                vertices.push_back(vertex);
            }

            index_offset += fv;
        }

        if (batchedVertices.find(materialId) != batchedVertices.end())
        {
            batchedVertices[materialId].insert(batchedVertices[materialId].end(), vertices.begin(), vertices.end());
        }
        else
        {
            batchedVertices.insert({ materialId, vertices });
        }
    }

    for (auto const& pair : batchedVertices)
    {
        // Create an initialize the vertex buffer.
        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
        vertexBufferDesc.ByteWidth = sizeof(VertexData) * pair.second.size();                      // size of the buffer in bytes
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;                                           // how the buffer is expected to be read from and written to
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;                                  // how the buffer will be bound to the pipeline
        vertexBufferDesc.CPUAccessFlags = 0;                                                    // no CPI access is necessary

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        resourceData.pSysMem = pair.second.data();                                        // pointer to the data to initialize the buffer with
        resourceData.SysMemPitch = 0;                                                   // distance from the beginning of one line of a texture to the nextline. No used for now.
        resourceData.SysMemSlicePitch = 0;                                              // distance from the beginning of one depth level to the next. No used for now.

        ID3D11Buffer* buffer = nullptr;
        HRESULT hr = m_d3dDevice->CreateBuffer(
            &vertexBufferDesc,                                                          // buffer description
            &resourceData,                                                              // pointer to the initialization data
            &buffer                                                                     // pointer to the created buffer object
        );
        std::string message = "Unable to create vertex buffer";
        AssertIfFailed(hr, "Load Content", message.c_str());

        struct BatchedVertices newBatchedVertices;
        newBatchedVertices.materialName = std::string(filepath) + std::string("-") + std::to_string(pair.first);
        newBatchedVertices.buffer = buffer;
        newBatchedVertices.count = pair.second.size();
        if (pair.first >= 0)
        {
            newBatchedVertices.material = materials[pair.first];
        }
        outBatchedVertices.push_back(newBatchedVertices);
    }

    return true;
}

HRESULT Model::CreateBuffer(D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* data, ID3D11Buffer** buffer)
{
    return m_d3dDevice->CreateBuffer(desc, data, buffer);
}
#include "Model.h"

#include <iostream>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

int Model::m_ResourceCount = 0;
std::map<std::string, std::vector<struct VertexData>> Model::m_Resources;
std::map<std::string, int> Model::m_RefCount;
std::map<std::string, ID3D11Buffer*> Model::m_VertexBuffers;
std::map<std::string, ID3D11Buffer*> Model::m_PerInstanceVertexBuffers;

Model::~Model()
{
    for (auto key : keys)
    {
        m_RefCount[key] -= 1;
        if (m_RefCount[key] == 0) {
            m_Resources.erase(key);
            m_RefCount.erase(key);
        }
    }
}

bool Model::Load(const char* filepath)
{
    tinyobj::ObjReaderConfig reader_config;

    std::string path(filepath);
    size_t pos = path.find_last_of("/\\");
    std::string folder = (pos == std::string::npos) ? "./" : path.substr(0, pos);

    reader_config.mtl_search_path = folder; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    std::vector<struct VertexData> vertices;

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    const bool batched = true;

    if (batched)
    {
        std::map<int, std::vector<struct VertexData>> batchedVertices;
        
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {

            vertices.clear();

            auto materialId = shapes[s].mesh.material_ids[0];
            auto key = std::string(filepath) + std::string("-") + std::to_string(materialId);

            if (m_Resources.find(key) != m_Resources.end())
            {
                m_RefCount[key] += 1;
                continue;
            }

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

                    vertex.vertex[0] = vx;
                    vertex.vertex[1] = vy;
                    vertex.vertex[2] = vz;

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

                    vertex.material = shapes[s].mesh.material_ids[f];

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

        for (const auto& pair : batchedVertices)
        {
            auto key = std::string(filepath) + std::string("-") + std::to_string(pair.first);
            keys.push_back(key);
            m_Resources.insert({ key, pair.second });
            m_RefCount.insert({ key, 1 });
        }
    }
    else
    {
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {

            vertices.clear();
            auto key = std::string(filepath) + std::string("-") + std::string(shapes[s].name);

            if (m_Resources.find(key) != m_Resources.end())
            {
                keys.push_back(key);
                m_RefCount[key] += 1;
                continue;
            }

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

                    vertex.vertex[0] = vx;
                    vertex.vertex[1] = vy;
                    vertex.vertex[2] = vz;

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

                    vertex.material = shapes[s].mesh.material_ids[f];

                    vertices.push_back(vertex);
                }

                index_offset += fv;
            }

            keys.push_back(key);
            m_Resources.insert({ key, vertices });
            m_RefCount.insert({ key, 1 });
        }
    }

    return true;
}
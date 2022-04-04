#include "Model.h"

#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

int Model::m_ResourceCount = 0;
std::map<std::string, std::vector<struct VertexData>> Model::m_Resources;
std::map<std::string, int> Model::m_RefCount;
std::map<std::string, ID3D11Buffer*> Model::m_VertexBuffers;
std::map<std::string, ID3D11Buffer*> Model::m_PerInstanceVertexBuffers;

Model::~Model()
{
    m_RefCount[m_Key] -= 1;
    if (m_RefCount[m_Key] == 0) {
        m_Resources.erase(m_Key);
        m_RefCount.erase(m_Key);
    }
}

bool Model::Load(const char* filepath)
{
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

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

    m_Key = std::string(filepath);

    if (m_Resources.find(m_Key) != m_Resources.end())
    {
        m_RefCount[m_Key] += 1;
        m_Id = m_RefCount[m_Key];
        return true;
    }

    std::vector<struct VertexData> vertices;

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {

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

                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

                vertices.push_back(vertex);
            }

            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

    m_Resources.insert({ m_Key, vertices });
    m_RefCount.insert({ m_Key, 1 });
    m_Id = 1;

    return true;
}
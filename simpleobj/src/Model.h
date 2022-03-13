#pragma once

#include <vector>

struct VertexData
{
    float vertex[3];
    float normal[2];
    float uv[2];
};

class Model
{
public:
    ~Model();
    bool Load(const char* filepath);

    std::vector<VertexData>* Vertices()
    {
        return &m_Vertices;
    }

    struct VertexData* Head()
    {
        return &m_Vertices[0];
    }

private:
    std::vector<VertexData> m_Vertices;
};
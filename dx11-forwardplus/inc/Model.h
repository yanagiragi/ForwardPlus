#pragma once

#include <string>
#include <vector>
#include <map>
#include <d3d11.h>

#include "Common.h"

struct VertexData
{
    float vertex[3];
    float normal[3];
    float uv[2];
};

class Model
{
public:
    ~Model();
    bool Load(const char* filepath);

    int VertexCount()
    {
        return GetVertexCount(m_Key);
    }

    struct VertexData* Head()
    {
        return m_Resources[m_Key].data();
    }

    std::string Key()
    {
        return m_Key;
    }

    ID3D11Buffer* VertexBuffer()
    {
        return GetVertexBuffer(m_Key);
    }

    // static methods

    static bool ContainsVertexBuffer(std::string key)
    {
        return m_VertexBuffers.find(key) != m_VertexBuffers.end();
    }

    static void AddVertexBuffer(std::string key, ID3D11Buffer* buffer)
    {
        m_VertexBuffers.insert({ key, buffer });
    }

    static ID3D11Buffer* GetVertexBuffer(std::string key)
    {
        if (m_VertexBuffers.find(key) != m_VertexBuffers.end())
        {
            return m_VertexBuffers[key];
        }
        return nullptr;
    }

    static void AddInstancedVertexBuffer(std::string key, ID3D11Buffer* buffer)
    {
        m_PerInstanceVertexBuffers.insert({ key, buffer });
    }

    static ID3D11Buffer* GetInstancedVertexBuffer(std::string key)
    {
        if (m_PerInstanceVertexBuffers.find(key) != m_PerInstanceVertexBuffers.end())
        {
            return m_PerInstanceVertexBuffers[key];
        }
        return nullptr;
    }

    static int GetVertexCount(std::string key)
    {
        if (m_Resources.find(key) != m_Resources.end())
        {
            return m_Resources[key].size();
        }
        return -1;
    }

    static void UnloadStaticResources()
    {
        m_Resources.clear();
        m_RefCount.clear();
        
        for (auto& pair : m_VertexBuffers) {
            SafeRelease(pair.second);
        }
        m_VertexBuffers.clear();

        for (auto& pair : m_PerInstanceVertexBuffers) {
            SafeRelease(pair.second);
        }
        m_PerInstanceVertexBuffers.clear();
    }

private:
    std::string m_Key;
    int m_Id;

    static int m_ResourceCount;
    static std::map<std::string, std::vector<struct VertexData>> m_Resources;
    static std::map<std::string, ID3D11Buffer*> m_VertexBuffers;
    static std::map<std::string, ID3D11Buffer*> m_PerInstanceVertexBuffers;
    static std::map<std::string, int> m_RefCount;
};
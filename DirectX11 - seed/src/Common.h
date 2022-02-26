#pragma once

#include <Windows.h>

#include <fstream>
#include <iostream>
#include <vector>

#include <direct.h>

inline std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    bool exists = (bool)file;

    if (!exists || !file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}

// Safely release a COM object.
template<typename T>
inline void SafeRelease(T& ptr)
{
    if (ptr != NULL)
    {
        ptr->Release();
        ptr = NULL;
    }
}

inline std::string GetCurrentWorkingDirectory(bool withTrailingSeperator)
{
    char pBuf[1024];
    _getcwd(pBuf, 1024);
    auto path = std::string(pBuf);
    if (withTrailingSeperator)
    {
        path += "\\";
    }

    return path;
}
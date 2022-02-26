#pragma once

#include <Windows.h>
#include <strsafe.h>
#include <direct.h>

#include <fstream>
#include <iostream>
#include <vector>

/// <summary>
/// Read file into lines of string
/// </summary>
/// <param name="filename"></param>
/// <returns></returns>
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

/// <summary>
/// Display error message box and throw exception if HRESULT fails
/// </summary>
/// <param name="hr"></param>
/// <param name="title"></param>
/// <param name="description"></param>
/// <param name="exception"></param>
inline void AssertIfFailed(HRESULT hr, LPTSTR title, LPTSTR description, LPTSTR exception = nullptr)
{
    if (FAILED(hr))
    {
        MessageBox(0, TEXT(title), TEXT(description), MB_OK);
        if (exception != nullptr) {
            throw new std::exception(exception);
        }
        else {
            throw new std::exception(description);
        }
    }
}

/// <summary>
/// Safely release a COM object
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="ptr"></param>
template<typename T>
inline void SafeRelease(T& ptr)
{
    if (ptr != NULL)
    {
        ptr->Release();
        ptr = NULL;
    }
}

/// <summary>
/// Display last error from Win32api in message box
/// </summary>
/// <param name="messagePrefix"></param>
inline void DisplayLastError(LPTSTR messagePrefix)
{
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process
    lpDisplayBuf = (LPVOID)LocalAlloc(
        LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)messagePrefix) + 50) * sizeof(TCHAR)
    );
    StringCchPrintf(
        (LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s, error code = [%d], error message = %s"),
        messagePrefix,
        dw,
        lpMsgBuf
    );
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

/// <summary>
/// Get current working directory
/// </summary>
/// <param name="withTrailingSeperator"></param>
/// <returns></returns>
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
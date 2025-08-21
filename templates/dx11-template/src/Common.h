#pragma once

#include <Windows.h>
#include <strsafe.h>
#include <direct.h>

#include <fstream>
#include <iostream>
#include <vector>

// For redirect console outputs
#include <fcntl.h>
#include <io.h>

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

/// <summary>
/// Transform lparam to virtual key code
/// </summary>
/// <param name="lparam"></param>
/// <returns></returns>
inline int GetVKcode(int lparam) {
    // pick scan code and extended key bit
    lparam = (lparam >> 16) & 0x1ff;

    // Check ctrl and alt, if pressed do XOR to get the code
    if (lparam & 0x100) {
        lparam ^= 0xe100;
    }
    int vkCode = MapVirtualKey(lparam, MAPVK_VSC_TO_VK_EX);
    return vkCode;
}

/// <summary>
/// To allow redirect stdout to consoles for Win32 GUI App
/// Reference: https://stackoverflow.com/a/46050762
/// </summary>
inline void RedirectIOToConsole() 
{
    //Create a console for this application
    AllocConsole();

    // Get STDOUT handle
    HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
    FILE* COutputHandle = _fdopen(SystemOutput, "w");

    // Get STDERR handle
    HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
    int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
    FILE* CErrorHandle = _fdopen(SystemError, "w");

    // Get STDIN handle
    HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
    FILE* CInputHandle = _fdopen(SystemInput, "r");

    //make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
    std::ios::sync_with_stdio(true);

    // Redirect the CRT standard input, output, and error handles to the console
    freopen_s(&CInputHandle, "CONIN$", "r", stdin);
    freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
    freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);

    //Clear the error state for each of the C++ standard stream objects. We need to do this, as
    //attempts to access the standard streams before they refer to a valid target will cause the
    //iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
    //to always occur during startup regardless of whether anything has been read from or written to
    //the console or not.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
}
#include "DirectXTemplateLibPCH.h"

#include "Application.h"

#include "Window.h"

#include "SimpleObj.h"
#include "Common.h"

const char* g_WindowName = "Forward+ Test";
int g_WindowWidth = 1280;
int g_WindowHeight = 720;
bool g_VSync = true;
bool g_Windowed = true;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);

#ifdef _DEBUG
    RedirectIOToConsole();

    // Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
    // This may happen if the application is launched through the PIX UI. 
    if (GetModuleHandle("WinPixGpuCapturer.dll") == 0)
    {
        LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
    }
#endif

    Application::Create(hInstance);
    Application& app = Application::Get();

    Window& window = app.CreateRenderWindow(g_WindowName, g_WindowWidth, g_WindowHeight, g_VSync, g_Windowed);

    Yr::SimpleObj* pDemo = new Yr::SimpleObj(window);

    if (!pDemo->Initialize())
    {
        return -1;
    }

    if (!pDemo->LoadContent())
    {
        return -1;
    }

    int exitCode = app.Run();

    pDemo->UnloadContent();
    pDemo->Cleanup();

    delete pDemo;

    return exitCode;
}
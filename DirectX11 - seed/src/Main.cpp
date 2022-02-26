#include <Windows.h>
#include <strsafe.h>

// DirectX includes
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

// STL includes
#include <iostream>
#include <string>

// For DirectX Math
using namespace DirectX;

const LONG g_WindowWidth = 1280;
const LONG g_WindowHeight = 720;
LPCSTR g_WindowClassName = "DirectXWindowClass";
LPCSTR g_WindowName = "DirectX Template";
HWND g_WindowHandle = 0;

const BOOL g_EnableVSync = TRUE;

// Direct3D device and swap chain.
ID3D11Device* g_d3dDevice = nullptr;
ID3D11DeviceContext* g_d3dDeviceContext = nullptr;
IDXGISwapChain* g_d3dSwapChain = nullptr;

// Render target view for the back buffer of the swap chain.
ID3D11RenderTargetView* g_d3dRenderTargetView = nullptr;
// Depth/stencil view for use as a depth buffer.
ID3D11DepthStencilView* g_d3dDepthStencilView = nullptr;
// A texture to associate to the depth stencil view.
ID3D11Texture2D* g_d3dDepthStencilBuffer = nullptr;

// Define the functionality of the depth/stencil stages.
ID3D11DepthStencilState* g_d3dDepthStencilState = nullptr;
// Define the functionality of the rasterizer stage.
ID3D11RasterizerState* g_d3dRasterizerState = nullptr;
D3D11_VIEWPORT g_Viewport = { 0 };

// Vertex buffer data
ID3D11InputLayout* g_d3dInputLayout = nullptr;
ID3D11Buffer* g_d3dVertexBuffer = nullptr;
ID3D11Buffer* g_d3dIndexBuffer = nullptr;

// Shader data
ID3D11VertexShader* g_d3dVertexShader = nullptr;
ID3D11PixelShader* g_d3dPixelShader = nullptr;

// Shader resources
enum ConstantBuffer
{
    CB_Application,
    CB_Frame,
    CB_Object,
    NumConstantBuffers
};

ID3D11Buffer* g_d3dConstantBuffers[NumConstantBuffers];

// Demo parameters
XMMATRIX g_WorldMatrix;
XMMATRIX g_ViewMatrix;
XMMATRIX g_ProjectionMatrix;

// Vertex data for a colored cube.
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

VertexPosColor g_Vertices[8] =
{
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};

WORD g_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

// Forward declarations.
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

template<class ShaderClass>
ShaderClass* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile);

bool LoadContent();
void UnloadContent();

void Update(float deltaTime);
void Render();
void Cleanup();

/**
 * Initialize the application window.
 */
int InitApplication(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASSEX wndClass = { 0 };
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;               // window class style
    wndClass.lpfnWndProc = &WndProc;                        // pointer to window procedure that will handle window events, e.g. mouse & keyboard input, etc.
    wndClass.cbClsExtra = 0;                                // extra bytes need to allocate following the window class structure
    wndClass.cbWndExtra = 0;                                // extra bytes need to allocate following the window instance
    wndClass.hInstance = hInstance;                         // handle instance of the window
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);      // cursor handle, use default cursor for here
    wndClass.hIcon = nullptr;                               // icon handle
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);    // handle to the background flush
    wndClass.lpszMenuName = nullptr;                        // resource name of the class menu
    wndClass.lpszClassName = g_WindowClassName;             // name of the class
    wndClass.hIconSm = nullptr;                             // handle to the small icon

    if (!RegisterClassEx(&wndClass))
    {
        return -1;
    }

    RECT windowRect = { 0, 0, g_WindowWidth, g_WindowHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    g_WindowHandle = CreateWindowA(
        g_WindowClassName,                                  // name of the window class to use as a template to create the window instance
        g_WindowName,                                       // name of the window
        WS_OVERLAPPEDWINDOW,                                // the style of the window being created
        CW_USEDEFAULT,                                      // horizontal position of the window
        CW_USEDEFAULT,                                      // vertical position of the window
        windowRect.right - windowRect.left,                 // width of the window
        windowRect.bottom - windowRect.top,                 // height of the window
        nullptr,                                            // handle to the parent window
        nullptr,                                            // handle to a window to use the menu
        hInstance,                                          // handle to the instance of the module to associated with the window
        nullptr                                             // point to the value to be passed to the window
    );

    if (!g_WindowHandle)
    {
        return -2;
    }

    ShowWindow(g_WindowHandle, nCmdShow);
    UpdateWindow(g_WindowHandle);

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paintStruct;
    HDC hDC;

    switch (message)
    {
    case WM_PAINT:
        {
            hDC = BeginPaint(hwnd, &paintStruct);
            EndPaint(hwnd, &paintStruct);
        }
        break;
    
    case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;
    
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

/**
 * The main application loop.
 */
int Run()
{
    MSG msg = { 0 };

    static DWORD previousTime = timeGetTime();
    static const float targetFramerate = 30.0f;
    static const float maxTimeStep = 1.0f / targetFramerate;

    while (msg.message != WM_QUIT)
    {
        // Peek next message from the message queue, return false if there are no messages
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);     // translate virtual-key messages into character messages
            DispatchMessage(&msg);      // dispatch the message to window's procedure function            
        }
        else
        {
            DWORD currentTime = timeGetTime();
            float deltaTime = (currentTime - previousTime) / 1000.0f;
            previousTime = currentTime;

            // Cap the delta time to the max time step (useful if your 
            // debugging and you don't want the deltaTime value to explode.
            deltaTime = std::min<float>(deltaTime, maxTimeStep);

            //            Update( deltaTime );
            //            Render();
        }
    }

    return static_cast<int>(msg.wParam);
}

void DisplayLastError(LPTSTR messagePrefix)
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, int nCmdShow)
{
    // remove warning C4100
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);

    // Check for DirectX Math library support.
    if (!XMVerifyCPUSupport())
    {
        DisplayLastError("Failed to verify DirectX Math library support.");
        return -1;
    }

    if (int ret = InitApplication(hInstance, nCmdShow) != 0)
    {
        DisplayLastError("Failed to create applicaiton window.");
        return ret;
    }

    int returnCode = Run();

    return returnCode;
}
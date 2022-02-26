#include <Windows.h>

// DirectX includes
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

// STL includes
#include <iostream>
#include <string>
#include "Common.h"

// Link library dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")

// For DirectX Math
using namespace DirectX;

#pragma region Forward declarations

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

template<class ShaderClass>
ShaderClass* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile);

int InitApplication(HINSTANCE hInstance, int nCmdShow);
int InitDirectX(HINSTANCE hInstance, BOOL vSync);

int Run();
bool LoadContent();
void UnloadContent();

void Update(float deltaTime);
void Render();
void Cleanup();

void Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil);
void Present(bool vSync);
DXGI_RATIONAL QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync);

#pragma endregion

#pragma region Global Variables

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

#pragma endregion

/// <summary>
/// Initialize the application window
/// </summary>
/// <param name="hInstance"></param>
/// <param name="nCmdShow"></param>
/// <returns></returns>
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

/// <summary>
/// Handles window event
/// </summary>
/// <param name="hwnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
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

/// <summary>
/// The main application loop
/// </summary>
/// <returns></returns>
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

            Clear(Colors::CornflowerBlue, 1.0f, 0);

            //            Update( deltaTime );
            //            Render();

            Present(g_EnableVSync);
        }
    }

    return static_cast<int>(msg.wParam);
}

/// <summary>
/// Query refresh rate of the device
/// </summary>
/// <param name="screenWidth"></param>
/// <param name="screenHeight"></param>
/// <param name="vsync"></param>
/// <returns></returns>
DXGI_RATIONAL QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync)
{
    // This function was inspired by:
    // http://www.rastertek.com/dx11tut03.html

    DXGI_RATIONAL refreshRate = { 0, 1 }; // unbounded refresh rate
    if (vsync)
    {
        IDXGIFactory* factory;
        IDXGIAdapter* adapter;
        IDXGIOutput* adapterOutput;
        DXGI_MODE_DESC* displayModeList;

        // Create a DirectX graphics interface factory.
        HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
        AssertIfFailed(hr, "Query Refresh Rate", "Could not create DXGIFactory instance.");

        hr = factory->EnumAdapters(0, &adapter);
        AssertIfFailed(hr, "Query Refresh Rate", "Failed to enumerate adapters.");
        
        hr = adapter->EnumOutputs(0, &adapterOutput);
        AssertIfFailed(hr, "Query Refresh Rate", "Failed to enumerate adapter outputs.");
        
        UINT numDisplayModes;
        hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr);
        AssertIfFailed(hr, "Query Refresh Rate", "Failed to query display mode list.");
        
        displayModeList = new DXGI_MODE_DESC[numDisplayModes];
        assert(displayModeList);

        hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList);
        AssertIfFailed(hr, "Query Refresh Rate", "Failed to query display mode list.");
        
        // Now store the refresh rate of the monitor that matches the width and height of the requested screen.
        for (UINT i = 0; i < numDisplayModes; ++i)
        {
            if (displayModeList[i].Width == screenWidth && displayModeList[i].Height == screenHeight)
            {
                refreshRate = displayModeList[i].RefreshRate;
            }
        }

        delete[] displayModeList;
        SafeRelease(adapterOutput);
        SafeRelease(adapter);
        SafeRelease(factory);
    }

    return refreshRate;
}

/// <summary>
/// Initialize the DirectX resources
/// </summary>
/// <param name="hInstance"></param>
/// <param name="vSync"></param>
/// <returns></returns>
int InitDirectX(HINSTANCE hInstance, BOOL vSync)
{
    // A window handle must have been created already.
    assert(g_WindowHandle != 0);

    RECT clientRect;
    GetClientRect(g_WindowHandle, &clientRect);

    // Compute the exact client dimensions. This will be used
    // to initialize the render targets for our swap chain.
    unsigned int clientWidth = clientRect.right - clientRect.left;
    unsigned int clientHeight = clientRect.bottom - clientRect.top;
    
    // Get screen refresh rate, for examaple 60hz would be like { Numerator: 60000, Denominator: 1000 }
    auto refreshRate = QueryRefreshRate(clientWidth, clientHeight, vSync);

    // TODO: Use double buffer
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    swapChainDesc.BufferCount = 1;                                  // Use single buffer for now
    swapChainDesc.BufferDesc.Width = clientWidth;                   // Resolution width
    swapChainDesc.BufferDesc.Height = clientHeight;                 // Resolution height
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;   // pixel format
    swapChainDesc.BufferDesc.RefreshRate = refreshRate;             // refresh rate in hertz
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // describes surface usage and CPU access options for the back buffer
    swapChainDesc.OutputWindow = g_WindowHandle;                    // handle to the output window
    swapChainDesc.SampleDesc.Count = 1;                             // number of multisamples per pixel
    swapChainDesc.SampleDesc.Quality = 0;                           // image quality level
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;            // options for handling the contents of the presentation buffer after presenting a surface
    swapChainDesc.Windowed = TRUE;                                  // is output in windowsed mode?

    UINT createDeviceFlags = 0;
#if _DEBUG
    createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

    // These are the feature levels that we will accept.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,         // Windows Display Driver Model (WDDM) version 1.2
        D3D_FEATURE_LEVEL_11_0,         // WDDM version 1.1, initial version of Windows 7 only supports it
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    // This will be the actual feature level that is used to create our device and swap chain.
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                        // point to the video adapter
        D3D_DRIVER_TYPE_HARDWARE,       // the Direct2D driver which implements the device
        nullptr,                        // handle to dll that implements a software rasterizer (for driver type D3D_DRIVER_TYPE_SOFTWARE)
        createDeviceFlags,              // device flags
        featureLevels,                  // supported feature levels
        _countof(featureLevels),        // count of supported feature levels
        D3D11_SDK_VERSION,              // SDK version
        &swapChainDesc,                 // address of a pointer to swapchain description
        &g_d3dSwapChain,                // address of a pointer to swapchain instance
        &g_d3dDevice,                   // address of a pointer to device instance
        &featureLevel,                  // address of a pointer to actual supported feature level
        &g_d3dDeviceContext             // address of a pointer to deviceContext instance
    );
    if (hr == E_INVALIDARG)
    {
        // if failed, re-create device with downgraded feature level (D3D_FEATURE_LEVEL_11_1 Removed)
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            &featureLevels[1],
            _countof(featureLevels) - 1,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &g_d3dSwapChain,
            &g_d3dDevice,
            &featureLevel,
            &g_d3dDeviceContext
        );
    }
    AssertIfFailed(hr, "InitDirectX", "Unable to create D3D11Device And D3D11SwapChain");

    // Next initialize the back buffer of the swap chain and associate it to a render target view.
    ID3D11Texture2D* backBuffer;
    hr = g_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    AssertIfFailed(hr, "InitDirectX", "Unable to get buffer from SwapChain");

    hr = g_d3dDevice->CreateRenderTargetView(
        backBuffer,                     // pointer to ID3D11Resource that represents a render target
        nullptr,                        // pointer to render-target view description, NULL means it can access all of the subresources in mipmap level 0
        &g_d3dRenderTargetView          // address of a pointer to a ID3D11RenderTargetView
    );
    AssertIfFailed(hr, "InitDirectX", "Unable to create Render Target View from backBuffer");

    // After RTV if created, back buffer texture can be released.
    SafeRelease(backBuffer);

    // Create the depth buffer for use with the depth/stencil view.
    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
    depthStencilBufferDesc.Width = clientWidth;                     // texture width, should be same to width of swap chain's back buffer
    depthStencilBufferDesc.Height = clientHeight;                   // texture height, should be same to height of swap chain's back buffer
    depthStencilBufferDesc.MipLevels = 1;                           // mip level, use 1 for a multisampled texture
    depthStencilBufferDesc.ArraySize = 1;                           // number of textures in the texture array
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;  // texture format, in here we use a 32-bit z-buffer that supports 24 bits for depth and 8 bits for stencil
    depthStencilBufferDesc.SampleDesc.Count = 1;                    // multisamping settings
    depthStencilBufferDesc.SampleDesc.Quality = 0;                  // multisamping settings
    depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;             // how the texture is to read from and written to, DEFAULT means it is a resource that requires read and write access by the GPU.
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;    // how a resource is binded to the pipeline
    depthStencilBufferDesc.CPUAccessFlags = 0;                      // the types of CPU access allowed for a resource. 0 means no CPU access required.

    hr = g_d3dDevice->CreateTexture2D(
        &depthStencilBufferDesc,        // resource description
        nullptr,                        // a pointer to an array of f D3D11_SUBRESOURCE_DATA represent as initial data.
                                        // The param has to be NULL for multiple sampled texture since it cannot be initialized with data when they are created.
        &g_d3dDepthStencilBuffer        // pointer to a buffer that receives the texture2D interface of the created texture
    );
    AssertIfFailed(hr, "InitDirectX", "Unable to create Depth and Stencil Texture");
    
    hr = g_d3dDevice->CreateDepthStencilView(
        g_d3dDepthStencilBuffer,        // pointer to the resource that will serve as the depth-stencil surface
        nullptr,                        // pointer to depth-stencil-view description. NULL means it can access all of the subresources in mipmap level 0
        &g_d3dDepthStencilView          // address of a pointer to an ID3D11DepthStencilView
    );
    AssertIfFailed(hr, "InitDirectX", "Unable to create Depth Stencil View");

    // Setup depth/stencil state.
    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
    ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    depthStencilStateDesc.DepthEnable = TRUE;                                   // is depth test enabled?
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;          // is depth write enabled?
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;                    // depth comparsion func
    depthStencilStateDesc.StencilEnable = FALSE;                                // is stencil test enabled?
    
    hr = g_d3dDevice->CreateDepthStencilState(&depthStencilStateDesc, &g_d3dDepthStencilState);
    AssertIfFailed(hr, "InitDirectX", "Unable to create Depth Stencil State");

    // Setup rasterizer state.
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;                 // fill mode to use when rendering. alternative: D3D11_FILL_WIREFRAME
    rasterizerDesc.CullMode = D3D11_CULL_BACK;                  // cull mode
    rasterizerDesc.FrontCounterClockwise = FALSE;               // determines if a triangle is front or back facing
    rasterizerDesc.DepthBias = 0;                               // depth value added to a given pixel
    rasterizerDesc.DepthBiasClamp = 0.0f;                       // maximum depth bias of a pixel
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;                 // scalar on a given pixel's slope
    rasterizerDesc.DepthClipEnable = TRUE;                      // z-clipping based on distance
    rasterizerDesc.ScissorEnable = FALSE;                       // scissor-rectangle culling
    rasterizerDesc.MultisampleEnable = FALSE;                   // Use the quadrilateral or alpha line anti-aliasing algorithm on MSAA render targets?
    rasterizerDesc.AntialiasedLineEnable = FALSE;               // Is line antialiasing enabled?

    // Create the rasterizer state object.
    hr = g_d3dDevice->CreateRasterizerState(&rasterizerDesc, &g_d3dRasterizerState);
    AssertIfFailed(hr, "InitDirectX", "Unable to create Rasterizer State");

    // Initialize the viewport to occupy the entire client area.
    g_Viewport.TopLeftX = 0.0f;
    g_Viewport.TopLeftY = 0.0f;
    g_Viewport.Width = static_cast<float>(clientWidth);
    g_Viewport.Height = static_cast<float>(clientHeight);
    g_Viewport.MinDepth = 0.0f;                             // Minimum depth of the viewport, Range: [0, 1]
    g_Viewport.MaxDepth = 1.0f;

    return 0;
}

/// <summary>
/// Clear the color and depth buffers.
/// </summary>
/// <param name="clearColor"></param>
/// <param name="clearDepth"></param>
/// <param name="clearStencil"></param>
void Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil)
{
    g_d3dDeviceContext->ClearRenderTargetView(g_d3dRenderTargetView, clearColor);
    g_d3dDeviceContext->ClearDepthStencilView(g_d3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
}

/// <summary>
/// Flush the frame buffer to screen
/// </summary>
/// <param name="vSync"></param>
void Present(bool vSync)
{
    if (vSync)
    {
        g_d3dSwapChain->Present(1, 0);
    }
    else
    {
        g_d3dSwapChain->Present(0, 0);
    }
}

/// <summary>
/// Entry point of the program
/// </summary>
/// <param name="hInstance"></param>
/// <param name="hPrevInstance"></param>
/// <param name="pCmdLine"></param>
/// <param name="nCmdShow"></param>
/// <returns></returns>
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

    int returnCode = InitApplication(hInstance, nCmdShow);
    if (returnCode != 0)
    {
        DisplayLastError("Failed to create applicaiton window.");
        return returnCode;
    }

    returnCode = InitDirectX(hInstance, g_EnableVSync);
    if (returnCode != 0)
    {
        DisplayLastError("Failed to create DirectX device and swap chain.");
        return returnCode;
    }

    returnCode = Run();
    if (returnCode != 0)
    {
        DisplayLastError("Failed to perform Run().");
    }

    return returnCode;
}
// Windows includes
#include <Windows.h>

// DirectX includes
#include <d3d11.h>
#include <d3dcompiler.h>

// STL includes
#include <iostream>
#include <string>

// GLM includes
#define GLM_FORCE_SSE42 1
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES 1
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// Project includes
#include "Common.h"
#include "Camera.h"

// Link library dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")

#pragma region Forward declarations

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

template<class ShaderClass>
std::string GetLatestProfile(ID3D11Device* device);

template<>
std::string GetLatestProfile<ID3D11VertexShader>(ID3D11Device* device);

template<>
std::string GetLatestProfile<ID3D11PixelShader>(ID3D11Device* device);

template<class ShaderClass>
ShaderClass* CreateShader(ID3D11Device* device, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage);

template<>
ID3D11VertexShader* CreateShader<ID3D11VertexShader>(ID3D11Device* device, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage);

template<>
ID3D11PixelShader* CreateShader<ID3D11PixelShader>(ID3D11Device* device, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage);

template<class ShaderClass>
ID3DBlob* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& _profile);

void Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil);
void Present(bool vSync);
DXGI_RATIONAL QueryRefreshRate(UINT screenWidth, UINT screenHeight, BOOL vsync);

int InitApplication(HINSTANCE hInstance, int nCmdShow);
int InitDirectX(HINSTANCE hInstance, BOOL vSync);

int Run();
void LoadContent();
void UnloadContent();
void Render();
void Cleanup();
void Update(float deltaTime);

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

// Matrixs
glm::mat4 g_ModelMatrix;
glm::mat4 g_ViewMatrix;
glm::mat4 g_ProjectionMatrix;

// Camera Params
Camera* g_Camera = new Camera(
    glm::vec3(0, 0, -10),
    0, // theta in degree
    -90  // phi in degree
);

// Input Params
float g_CameraTranslateStep = .5f;
float g_CameraRotateStep = 1.0f; // in degrees

// Vertex data for a colored cube.
struct VertexPosColor
{
    glm::vec3 Position;
    glm::vec3 Color;
};

VertexPosColor g_Vertices[8] =
{
    { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f) }, // 0
    { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) }, // 1
    { glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 0.0f) }, // 2
    { glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f) }, // 3
    { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f) }, // 4
    { glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 1.0f, 1.0f) }, // 5
    { glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(1.0f, 1.0f, 1.0f) }, // 6
    { glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(1.0f, 0.0f, 1.0f) }  // 7
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

void HandleKeyDown(int keycode)
{
    printf("keydown :%4x\n", keycode);

    switch (keycode)
    {
    case 0x41: // A
        g_Camera->Translate(glm::vec3(1, 0, 0) * g_CameraTranslateStep);
        break;
    case 0x44: // D
        g_Camera->Translate(glm::vec3(-1, 0, 0) * g_CameraTranslateStep);
        break;
    case 0x57: // W
        g_Camera->Translate(glm::vec3(0, 0, 1) * g_CameraTranslateStep);
        break;
    case 0x53: // S
        g_Camera->Translate(glm::vec3(0, 0, -1) * g_CameraTranslateStep);
        break;
    case 0x51: // Q
        g_Camera->Translate(glm::vec3(0, 1, 0) * g_CameraTranslateStep);
        
        break;
    case 0x45: // E
        g_Camera->Translate(glm::vec3(0, -1, 0) * g_CameraTranslateStep);
        break;
    case 0x49: // I
        g_Camera->Rotate(g_CameraRotateStep, 0);
        break;
    case 0x4a: // J
        g_Camera->Rotate(0, -g_CameraRotateStep);
        break;
    case 0x4c: // L
        g_Camera->Rotate(0, g_CameraRotateStep);
        break;
    case 0x4b: // K
        g_Camera->Rotate(-g_CameraRotateStep, 0);

    default:
        break;
    }
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
    case WM_KEYDOWN:
        HandleKeyDown(GetVKcode(lParam));
        break;
    case WM_KEYUP:
        {
            printf("keyup    wparam:%4x  lparam:%8x  virtual-key code:%4x\n", wParam, lParam, GetVKcode(lParam));
        }
        break;
    case WM_SYSKEYDOWN:
        {
            printf("syskeydown    wparam:%4x  lparam:%8x  virtual-key code:%4x\n", wParam, lParam, GetVKcode(lParam));
        }
        break;
    case WM_SYSKEYUP:
        {
            printf("syskeyup    wparam:%4x  lparam:%8x  virtual-key code:%4x\n", wParam, lParam, GetVKcode(lParam));
        }
        break;

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

            Update( deltaTime );
            Render();
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
        g_d3dSwapChain->Present(
            1,                      // synchronize presentation of a frame with the vertical black
                                    // 0 for no synchronization, 1,2,3,4 means synchronize presentation after the nth vertical blank
            0                       // flags of swap-chain presentation options
        );
    }
    else
    {
        g_d3dSwapChain->Present(0, 0);
    }
}

/// <summary>
/// Get latest d3d profile of vertex shader
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
template<>
std::string GetLatestProfile<ID3D11VertexShader>(ID3D11Device* device)
{
    assert(device);

    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = device->GetFeatureLevel();

    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "vs_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "vs_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "vs_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "vs_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "vs_4_0_level_9_1";
    }
    break;
    } // switch( featureLevel )

    return "";
}

/// <summary>
/// Get latest d3d profile of pixel shader
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
template<>
std::string GetLatestProfile<ID3D11PixelShader>(ID3D11Device* device)
{
    assert(device);

    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = device->GetFeatureLevel();
    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "ps_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "ps_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "ps_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "ps_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "ps_4_0_level_9_1";
    }
    break;
    }
    return "";
}

/// <summary>
/// Create vertex shader
/// </summary>
/// <param name="device"></param>
/// <param name="pShaderBlob"></param>
/// <param name="pClassLinkage"></param>
/// <returns>pointer to ID3D11VertexShader instance</returns>
template<>
ID3D11VertexShader* CreateShader<ID3D11VertexShader>(ID3D11Device* device, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage)
{
    assert(device);
    assert(pShaderBlob);

    ID3D11VertexShader* pVertexShader = nullptr;
    device->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pVertexShader);

    return pVertexShader;
}

/// <summary>
/// Create pixel shader
/// </summary>
/// <param name="device"></param>
/// <param name="pShaderBlob"></param>
/// <param name="pClassLinkage"></param>
/// <returns>pointer to ID3D11VertexShader instance</returns>
template<>
ID3D11PixelShader* CreateShader<ID3D11PixelShader>(ID3D11Device* device, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage)
{
    assert(device);
    assert(pShaderBlob);

    ID3D11PixelShader* pPixelShader = nullptr;
    device->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pPixelShader);

    return pPixelShader;
}

/// <summary>
/// Load Shader and compile it
/// </summary>
/// <typeparam name="ShaderClass">ID3D11VertexShader or ID3D11PixelShader</typeparam>
/// <param name="fileName"></param>
/// <param name="entryPoint"></param>
/// <param name="_profile"></param>
/// <returns>pointer to ID3DBlob instance</returns>
template<class ShaderClass>
ID3DBlob* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& _profile)
{
    ID3DBlob* pShaderBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;

    std::string profile = _profile;
    if (profile == "latest")
    {
        profile = GetLatestProfile<ShaderClass>(g_d3dDevice);
    }

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompileFromFile(
        fileName.c_str(),                       // name of the shader file
        nullptr,                                // optional array of shader macros
        D3D_COMPILE_STANDARD_FILE_INCLUDE,      // optional pointer to include files, D3D_COMPILE_STANDARD_FILE_INCLUDE implies it include files that are relative to the current directory
        entryPoint.c_str(),                     // entry point of the shader
        profile.c_str(),                        // shader target
        flags,                                  // shader compile options of a HLSL code
        0,                                      // shader compile options of a effect
        &pShaderBlob,                           // pointer to the compiled code
        &pErrorBlob                             // pointer to the error messages
    );

    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            std::string errorMessage = (char*)pErrorBlob->GetBufferPointer();
            OutputDebugStringA(errorMessage.c_str());

            SafeRelease(pShaderBlob);
            SafeRelease(pErrorBlob);
        }

        return nullptr;
    }
    
    SafeRelease(pErrorBlob);

    return pShaderBlob;
}

/// <summary>
/// Load resources of the application
/// </summary>
void LoadContent()
{
    assert(g_d3dDevice);

    // Create an initialize the vertex buffer.
    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
    vertexBufferDesc.ByteWidth = sizeof(VertexPosColor) * _countof(g_Vertices);     // size of the buffer in bytes
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;                                   // how the buffer is expected to be read from and written to
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;                          // how the buffer will be bound to the pipeline
    vertexBufferDesc.CPUAccessFlags = 0;                                            // no CPI access is necessary

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    resourceData.pSysMem = g_Vertices;                                              // pointer to the data to initialize the buffer with
    resourceData.SysMemPitch = 0;                                                   // distance from the beginning of one line of a texture to the nextline.
                                                                                    // No used for now.
    resourceData.SysMemSlicePitch = 0;                                              // distance from the beginning of one depth level to the next. 
                                                                                    // no used for now.

    HRESULT hr = g_d3dDevice->CreateBuffer(
        &vertexBufferDesc,                                                          // buffer description
        &resourceData,                                                              // pointer to the initialization data
        &g_d3dVertexBuffer                                                          // pointer to the created buffer object
    );
    AssertIfFailed(hr, "Load Content", "Unable to create vertex buffer");
    
    // Create and initialize the index buffer.
    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
    indexBufferDesc.ByteWidth = sizeof(WORD) * _countof(g_Indicies);
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    
    //re-use resourceData
    resourceData.pSysMem = g_Indicies;

    hr = g_d3dDevice->CreateBuffer(&indexBufferDesc, &resourceData, &g_d3dIndexBuffer);
    AssertIfFailed(hr, "Load Content", "Unable to create index buffer");
    
    // Create the constant buffers for the variables defined in the vertex shader.
    D3D11_BUFFER_DESC constantBufferDesc;
    ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
    constantBufferDesc.ByteWidth = sizeof(glm::mat4);
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = 0;

    hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_d3dConstantBuffers[CB_Application]);
    AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_Application");
    
    hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_d3dConstantBuffers[CB_Frame]);
    AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_Frame");
    
    hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_d3dConstantBuffers[CB_Object]);
    AssertIfFailed(hr, "Load Content", "Unable to create constant buffer: CB_Object");
    
    // Note:
    // Since we will need to update the contents of the constant buffer in the application,
    // Instead of set the buffer¡¦s Usage property to D3D11_USAGE_DYNAMIC and the CPU AccessFlags to D3D11_CPU_ACCESS_WRITE,
    // we'll be using ID3D11DeviceContext::UpdateSubresource method,
    // which it expects constant buffers to be initialized with D3D11_USAGE_DEFAULT usage flag
    // and buffers that are created with the D3D11_USAGE_DEFAULT flag must have their CPU AccessFlags set to 0.

    // Load and compile the vertex shader
    ID3DBlob* vertexShaderBlob;
    vertexShaderBlob = LoadShader<ID3D11VertexShader>(L"assets/simpleVertexShader.hlsl", "main", "latest");
    g_d3dVertexShader = CreateShader<ID3D11VertexShader>(g_d3dDevice, vertexShaderBlob, nullptr);
    
    // Create the input layout for the vertex shader.
    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
    {
        {
            "POSITION",                             // semantic name
            0,                                      // semantic index
            DXGI_FORMAT_R32G32B32_FLOAT,            // format
            0,                                      // input slot (used for packed vertex buffers)
            offsetof(VertexPosColor, Position),     // aligned byte offset
            D3D11_INPUT_PER_VERTEX_DATA,            // input slot class
            0                                       // additional param for slot class: D3D11_INPUT_PER_INSTANCE_DATA
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            offsetof(VertexPosColor, Color),
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    hr = g_d3dDevice->CreateInputLayout(
        vertexLayoutDesc,                           // input layout description
        _countof(vertexLayoutDesc),                 // amount of the elements
        vertexShaderBlob->GetBufferPointer(),       // pointer to the compiled shader
        vertexShaderBlob->GetBufferSize(),          // size in bytes of the compiled shader
        &g_d3dInputLayout                           // pointer to the input-layout object
    );
    AssertIfFailed(hr, "Load Content", "Unable to create input layout");

    // After creating input layouy, the shader blob is no longer needed
    SafeRelease(vertexShaderBlob);

    // Load and compile the pixel shader
    ID3DBlob* pixelShaderBlob;
    pixelShaderBlob = LoadShader<ID3D11PixelShader>(L"assets/simplePixelShader.hlsl", "main", "latest");
    g_d3dPixelShader = CreateShader<ID3D11PixelShader>(g_d3dDevice, pixelShaderBlob, nullptr);
    SafeRelease(pixelShaderBlob);

    // Setup the projection matrix.
    RECT clientRect;
    GetClientRect(g_WindowHandle, &clientRect);

    // Compute the exact client dimensions, which is required for a correct projection matrix.
    float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
    float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

    // Setup projection matrix
    g_ProjectionMatrix = glm::perspective(glm::radians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f);
    g_d3dDeviceContext->UpdateSubresource(
        g_d3dConstantBuffers[CB_Application],           // pointer to the destination resource
        0,                                              // zero-based index that identifies the destination subresource
        nullptr,                                        // pointer to the box that defines the portion of the destination subresource
                                                        // to copy the resource data into
        glm::value_ptr(g_ProjectionMatrix),             // pointer to the source data in memory
        0,                                              // size of one row of the source data
        0                                               // size of one depth slice of source data
    );
}

/// <summary>
/// The render loop
/// </summary>
void Render()
{
    assert(g_d3dDevice);
    assert(g_d3dDeviceContext);

    // CornflowerBlue = #6495ED
    Clear(glm::value_ptr(glm::vec3(0.392f, 0.584f, 0.929f)), 1.0f, 0);

    // Setup the input assembler stage
    const UINT vertexStride = sizeof(VertexPosColor);
    const UINT offset = 0;
    g_d3dDeviceContext->IASetVertexBuffers(
        0,                                      // start slot, should equal to slot we use when CreateInputLayout in LoadContent()
        1,                                      // number of vertex buffers in the array
        &g_d3dVertexBuffer,                     // pointer to an array of vertex buffers
        &vertexStride,                          // pointer to stride values
        &offset                                 // pointer to offset values
    );
    g_d3dDeviceContext->IASetInputLayout(g_d3dInputLayout);
    g_d3dDeviceContext->IASetIndexBuffer(
        g_d3dIndexBuffer,                       // pointer to indicies
        DXGI_FORMAT_R16_UINT,                   // indicies format
        0                                       // offset
    );
    g_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Setup the vertex shader stage
    g_d3dDeviceContext->VSSetShader(
        g_d3dVertexShader,                      // pointer to vertex shader
        nullptr,                                // pointer to an array of class-instance interfaces, NULL means shader does not use any interface
        0                                       // number of class-instance interfaces of previous param
    );
    g_d3dDeviceContext->VSSetConstantBuffers(
        0,                                      // start slot
        3,                                      // number of buffers
        g_d3dConstantBuffers                    // array of constant buffers
    );

    // Setup the rasterizer stage
    g_d3dDeviceContext->RSSetState(g_d3dRasterizerState);
    g_d3dDeviceContext->RSSetViewports(
        1,                                      // numbers of the viewport to bind
        &g_Viewport                             // array of viewport
    );

    // Setup the pixel stage stage
    g_d3dDeviceContext->PSSetShader(g_d3dPixelShader, nullptr, 0);

    // Setup the output merger stage
    g_d3dDeviceContext->OMSetRenderTargets(
        1,                                      // number of render target to bind
        &g_d3dRenderTargetView,                 // pointer to an array of render-target view
        g_d3dDepthStencilView                   // pointer to depth-stencil view
    );
    g_d3dDeviceContext->OMSetDepthStencilState(
        g_d3dDepthStencilState,                 // depth stencil state
        1                                       // stencil reference
    );

    // Draw the cube
    g_d3dDeviceContext->DrawIndexed(
        _countof(g_Indicies),                   // number of the indicies to draw
        0,                                      // start index location of index buffer
        0                                       // a value to added to each index before reading a vertex from vertex buffer
    );

    // present swap chain's back buffer to the screen
    Present(g_EnableVSync);
}

/// <summary>
/// Unload resources of the application
/// </summary>
void UnloadContent()
{
    SafeRelease(g_d3dConstantBuffers[CB_Application]);
    SafeRelease(g_d3dConstantBuffers[CB_Frame]);
    SafeRelease(g_d3dConstantBuffers[CB_Object]);
    SafeRelease(g_d3dIndexBuffer);
    SafeRelease(g_d3dVertexBuffer);
    SafeRelease(g_d3dInputLayout);
    SafeRelease(g_d3dVertexShader);
    SafeRelease(g_d3dPixelShader);
}

/// <summary>
/// Unload all resources
/// </summary>
void Cleanup()
{
    SafeRelease(g_d3dDepthStencilView);
    SafeRelease(g_d3dRenderTargetView);
    SafeRelease(g_d3dDepthStencilBuffer);
    SafeRelease(g_d3dDepthStencilState);
    SafeRelease(g_d3dRasterizerState);
    SafeRelease(g_d3dSwapChain);
    SafeRelease(g_d3dDeviceContext);
    SafeRelease(g_d3dDevice);

    delete g_Camera;
}

/// <summary>
/// The Logic loop
/// </summary>
/// <param name="deltaTime"></param>
void Update(float deltaTime)
{
    g_ViewMatrix = g_Camera->GetViewMatrix();
    g_d3dDeviceContext->UpdateSubresource(g_d3dConstantBuffers[CB_Frame], 0, nullptr, glm::value_ptr(g_ViewMatrix), 0, 0);

    static float angle = 0.0;
    angle += 90.0f * deltaTime;
    glm::vec3 rotationAxis = glm::vec3(0, 1, 1);

    g_ModelMatrix = glm::mat4(1);
    g_ModelMatrix = glm::rotate(g_ModelMatrix, glm::radians(angle), rotationAxis);

    g_d3dDeviceContext->UpdateSubresource(g_d3dConstantBuffers[CB_Object], 0, nullptr, glm::value_ptr(g_ModelMatrix), 0, 0);
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

#ifdef _DEBUG
    RedirectIOToConsole();
#endif

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

    LoadContent();

    returnCode = Run();
    if (returnCode != 0)
    {
        DisplayLastError("Failed to perform Run().");
    }

    UnloadContent();
    Cleanup();

    return returnCode;
}
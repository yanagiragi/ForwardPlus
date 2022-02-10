#pragma once

#include "CrossWindow/CrossWindow.h"
#include "CrossWindow/Graphics.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

#include <direct.h>

#include "Scene.h"

inline void InitializeVertexBuffer(const void* pVertexBufferData, const int stride)
{
    // Create the vertex buffer.
    /*const UINT vertexBufferSize = sizeof(pVertexBufferData);

    // Note: using upload heaps to transfer static data like vert buffers is
    // not recommended. Every time the GPU needs it, the upload heap will be
    // marshalled over. Please read up on Default Heap usage. An upload heap
    // is used here for code simplicity and because there are very few verts
    // to actually transfer.
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC vertexBufferResourceDesc;
    vertexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferResourceDesc.Alignment = 0;
    vertexBufferResourceDesc.Width = vertexBufferSize;
    vertexBufferResourceDesc.Height = 1;
    vertexBufferResourceDesc.DepthOrArraySize = 1;
    vertexBufferResourceDesc.MipLevels = 1;
    vertexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexBufferResourceDesc.SampleDesc.Count = 1;
    vertexBufferResourceDesc.SampleDesc.Quality = 0;
    vertexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ThrowIfFailed(mDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &vertexBufferResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&mVertexBuffer)));

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;

    // We do not intend to read from this resource on the CPU.
    D3D12_RANGE readRange;
    readRange.Begin = 0;
    readRange.End = 0;

    ThrowIfFailed(mVertexBuffer->Map(
        0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, pVertexBufferData, sizeof(pVertexBufferData));
    mVertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = stride;
    mVertexBufferView.SizeInBytes = vertexBufferSize;*/
}

class Renderer
{
public:
    Renderer(xwin::Window& window);

    ~Renderer();

    // Render onto the render target
    void render();

    // Resize the window and internal data structures
    void resize(unsigned width, unsigned height);

protected:
    // Initialize your Graphics API
    void initializeAPI(xwin::Window& window);

    // Destroy any Graphics API data structures used in this example
    void destroyAPI();

    // Initialize any resources such as VBOs, IBOs, used in this example
    void initializeResources();

    // Destroy any resources used in this example
    void destroyResources();

    // Create graphics API specific data structures to send commands to the GPU
    void createCommands();

    // Set up commands used when rendering frame by this app
    void setupCommands();

    // Destroy all commands
    void destroyCommands();

    // Set up the FrameBuffer
    void initFrameBuffer();

    void destroyFrameBuffer();

    // Set up the RenderPass
    void createRenderPass();

    void createSynchronization();

    UINT8* InitializeVertexBuffer(const void* pVertexBufferData, const int stride);

    // Set up the swapchain
    void setupSwapchain(unsigned width, unsigned height);

    std::chrono::time_point<std::chrono::steady_clock> tStart, tEnd;

    static const UINT backbufferCount = 2;

    xwin::Window* mWindow;
    unsigned mWidth, mHeight;

    // Initialization
    IDXGIFactory4* mFactory;
    IDXGIAdapter1* mAdapter;
#if defined(_DEBUG)
    ID3D12Debug1* mDebugController;
    ID3D12DebugDevice* mDebugDevice;
#endif
    ID3D12Device* mDevice;
    ID3D12CommandQueue* mCommandQueue;
    ID3D12CommandAllocator* mCommandAllocator;
    ID3D12GraphicsCommandList* mCommandList;

    // Current Frame
    UINT mCurrentBuffer;
    ID3D12DescriptorHeap* mRtvHeap;
    ID3D12Resource* mRenderTargets[backbufferCount];
    IDXGISwapChain3* mSwapchain;

    // Resources
    D3D12_VIEWPORT mViewport;
    D3D12_RECT mSurfaceSize;

    ID3D12Resource* mVertexBuffer;
    ID3D12Resource* mIndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
    D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

    UINT mRtvDescriptorSize;
    ID3D12RootSignature* mRootSignature;
    ID3D12PipelineState* mPipelineState;

    // Sync
    UINT mFrameIndex;
    HANDLE mFenceEvent;
    ID3D12Fence* mFence;
    UINT64 mFenceValue;

    float mElapsedTime = 0.0f;
    Scene* mScene;
    RenderInfo mRenderInfo;

    
    struct Vertex
    {
        float position[3];
        float color[3];
    };

    Vertex mVertexBufferData[3] = {{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                   {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                   {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

    uint32_t mIndexBufferData[3] = {0, 1, 2};

    ID3D12Resource* mUniformBuffer;
    ID3D12DescriptorHeap* mUniformBufferHeap;
    UINT8* mMappedUniformBuffer;

    // Uniform data
    struct
    {
        glm::mat4 projectionMatrix;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
    } uboVS;
};
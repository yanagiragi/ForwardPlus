#include "Common.hlsli"

//  =========================
//      Constant Buffers
//  =========================
cbuffer DispatchParams : register(b0)
{
    // Number of groups dispatched
    uint3 numThreadGroups;
    uint padding1;

    // Total number of threads dispatched
    // Note this value may be less than the actual number of threads executed
    // if the screen size is not divisible by the block size
    uint3 numThreads;
    uint padding2;
}

cbuffer ScreenToViewParams : register(b1)
{
    float4x4 InverseView;
    float4x4 InverseProjection;
    float2 ScreenDimensions;
    float2 ThreadGroups;
}

//  =========================
//      Main Functions
//  =========================

RWStructuredBuffer<Frustum> out_Frustums : register( u0 );

struct ComputeShaderInput
{
    uint3 groupID           : SV_GroupID;           // 3D index of the thread group in the dispatch.
    uint3 groupThreadID     : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.
    uint3 dispatchThreadID  : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.
    uint  groupIndex        : SV_GroupIndex;        // Flattened local index of the thread within a thread g
};

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(ComputeShaderInput IN)
{
    const float3 eyePos = float3(0, 0, 0);

    float4 screenSpace[4];
    // Since we're using left hand coorindate, use 1.0 for z value since camera looking at +Z axis in view space
    // For right hand coordintate, use -1.0 instead
    screenSpace[0] = float4(float2(IN.dispatchThreadID.x    , IN.dispatchThreadID.y    ) * BLOCK_SIZE, 1.0, 1.0f); // top-left point
    screenSpace[1] = float4(float2(IN.dispatchThreadID.x + 1, IN.dispatchThreadID.y    ) * BLOCK_SIZE, 1.0, 1.0f); // top-right point
    screenSpace[2] = float4(float2(IN.dispatchThreadID.x    , IN.dispatchThreadID.y + 1) * BLOCK_SIZE, 1.0, 1.0f); // bottom-left point
    screenSpace[3] = float4(float2(IN.dispatchThreadID.x + 1, IN.dispatchThreadID.y + 1) * BLOCK_SIZE, 1.0, 1.0f); // bottom-right point

    float3 viewSpace[4];
    for(int i = 0; i < 4; ++i)
    {
        viewSpace[i] = ScreenToView(screenSpace[i], InverseProjection, ScreenDimensions).xyz;
    }

    Frustum frustum;
    frustum.planes[0] = ComputePlane(eyePos, viewSpace[2], viewSpace[0]); // left plane
    frustum.planes[1] = ComputePlane(eyePos, viewSpace[1], viewSpace[3]); // right plane
    frustum.planes[2] = ComputePlane(eyePos, viewSpace[0], viewSpace[1]); // top plane
    frustum.planes[3] = ComputePlane(eyePos, viewSpace[3], viewSpace[2]); // bottom plane

    // first check current thread ID is bound of the grid
    if (IN.dispatchThreadID.x < numThreads.x && IN.dispatchThreadID.y < numThreads.y)
    {
        uint index = IN.dispatchThreadID.x + (IN.dispatchThreadID.y * numThreads.x);
        out_Frustums[index] = frustum;
    }
}
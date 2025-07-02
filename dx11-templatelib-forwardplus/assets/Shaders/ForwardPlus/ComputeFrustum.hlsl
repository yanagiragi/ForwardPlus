#include "Common.hlsli"

#define BLOCK_SIZE 16

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
}

//  =========================
//        Functions
//  =========================

// Convert clip space coordinates to view space
float4 ClipToView( float4 clip )
{
    float4 view = mul( InverseProjection, clip );
    view = view / view.w; 
    return view;
}

// Convert screen space coordinates to view space.
float4 ScreenToView( float4 screen )
{
    float2 texCoord = screen.xy / ScreenDimensions;
    float4 clip = float4( float2( texCoord.x, 1.0f - texCoord.y ) * 2.0f - 1.0f, screen.z, screen.w); 
    return ClipToView( clip );
}

// right-handed (counter-clockwise winding order), so the normal points to inside of the frustum
Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
    Plane plane;

    float3 v1 = p1 - p0;
    float3 v2 = p2 - p0;

    plane.N = normalize(cross(v1, v2));
    plane.d = dot(plane.N, p0);

    return plane;
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
    int z = 1; // z = 1 for left-hand coodinate system
    screenSpace[0] = float4(float2(IN.dispatchThreadID.x    , IN.dispatchThreadID.y    ) * BLOCK_SIZE, z, 1.0f); // top-left point
    screenSpace[1] = float4(float2(IN.dispatchThreadID.x + 1, IN.dispatchThreadID.y    ) * BLOCK_SIZE, z, 1.0f); // top-right point
    screenSpace[2] = float4(float2(IN.dispatchThreadID.x    , IN.dispatchThreadID.y + 1) * BLOCK_SIZE, z, 1.0f); // bottom-left point
    screenSpace[3] = float4(float2(IN.dispatchThreadID.x + 1, IN.dispatchThreadID.y + 1) * BLOCK_SIZE, z, 1.0f); // bottom-right point

    float3 viewSpace[4];
    for(int i = 0; i < 4; ++i)
    {
        viewSpace[i] = ScreenToView(screenSpace[i]).xyz;
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
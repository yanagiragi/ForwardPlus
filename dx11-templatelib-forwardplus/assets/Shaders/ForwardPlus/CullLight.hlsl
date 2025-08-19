#include "../Structures.hlsli"
#include "Common.hlsli"

//  =========================
//        Input  Buffers
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
    float ScreenToViewParams_padding[2];
}

cbuffer LightProperties : register(b2)
{
    float4 EyePosition;                 // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 GlobalAmbient;               // 16 bytes
    //----------------------------------- (16 byte boundary)
    struct LightProperties Lights[MAX_LIGHTS];    // 80 * 8 = 640 bytes
};  // Total:  

cbuffer DebugProperties : register(b3)
{
    int DebugMode;                  // 4 bytes
    float DepthPower;               // 4 bytes
    float DebugPadding[2];          // 8 bytes
                                    //----------(16 byte boundary)
}; // Total:                        // 16 bytes (1 * 16 byte boundary)

// The depth from the screen space texture.
Texture2D DepthTextureVS : register( t0 );

// Precomputed frustums for the grid.
StructuredBuffer<Frustum> in_Frustums : register( t1 );

// "o_" prefix indicates light lists for opaque geometry while 
// "t_" prefix indicates light lists for transparent geometry.
RWStructuredBuffer<uint> o_LightIndexCounter : register( u0 );
RWStructuredBuffer<uint> o_LightIndexList : register( u1 );
RWTexture2D<uint2> o_LightGrid : register( u2 );
RWStructuredBuffer<float> debugList : register( u3 );

// RWStructuredBuffer<uint> t_LightIndexCounter : register( u2 );
// RWStructuredBuffer<uint> t_LightIndexList : register( u4 );
// RWTexture2D<uint2> t_LightGrid : register( u6 );

// shader model 5.0 does not provide atomic functions for floating point values,
// so we use unsigned int to atomically compared and updated per thread.
groupshared uint uMinDepth;
groupshared uint uMaxDepth;

groupshared Frustum GroupFrustum;

// Opaque geometry light lists.
groupshared uint o_LightCount;
groupshared uint o_LightIndexStartOffset;
groupshared uint o_LightList[MAX_LIGHTS];
 
// Transparent geometry light lists.
// groupshared uint t_LightCount;
// groupshared uint t_LightIndexStartOffset;
// groupshared uint t_LightList[1024];

//  =========================
//        Functions
//  =========================

// Add the light to the visible light list for opaque geometry.
void o_AppendLight( uint lightIndex )
{
    uint index; // Index into the visible lights array.
    InterlockedAdd( o_LightCount, 1, index ); // atomic add
    if ( index < MAX_LIGHTS )
    {
        o_LightList[index] = lightIndex;
    }
}
 
// // Add the light to the visible light list for transparent geometry.
// void t_AppendLight( uint lightIndex )
// {
//     uint index; // Index into the visible lights array.
//     InterlockedAdd( t_LightCount, 1, index );
//     if ( index < MAX_LIGHTS )
//     {
//         t_LightList[index] = lightIndex;
//     }
// }

//  =========================
//      Main Functions
//  =========================

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
    // Calculate min & max depth in threadgroup / tile.
    int2 texCoord = IN.dispatchThreadID.xy;
    float fDepth = DepthTextureVS.Load( int3( texCoord, 0 ) ).r;
 
    // as long as we don’t try to preform any arithmetic operations on 
    // the unsigned integer depth values, we should get the correct minimum and maximum values.
    uint uDepth = asuint( fDepth );

    // Step 1: initialize uMinDepth and uMaxDepth to default value

    if ( IN.groupIndex == 0 ) // Avoid contention by other threads in the group.
    {
        uMinDepth = 0xffffffff;
        uMaxDepth = 0;
        o_LightCount = 0;
        // t_LightCount = 0;
        GroupFrustum = in_Frustums[IN.groupID.x + ( IN.groupID.y * numThreadGroups.x )];
    }
 
    GroupMemoryBarrierWithGroupSync();

    // Step 2: set uMinDepth and uMaxDepth to proper value

    InterlockedMin( uMinDepth, uDepth );
    InterlockedMax( uMaxDepth, uDepth );
 
    GroupMemoryBarrierWithGroupSync();

    // covert back to float
    float fMinDepth = asfloat( uMinDepth );
    float fMaxDepth = asfloat( uMaxDepth );
 
    // Convert depth values to view space.
    float minDepthVS = ScreenToView( float4( 0, 0, fMinDepth, 1 ), InverseProjection, ScreenDimensions ).z; // for opaque geometry
    float maxDepthVS = ScreenToView( float4( 0, 0, fMaxDepth, 1 ), InverseProjection, ScreenDimensions ).z;
    float nearClipVS = ScreenToView( float4( 0, 0, 0,         1 ), InverseProjection, ScreenDimensions ).z; // for transparent geometry
 
    // Clipping plane for minimum depth value 
    // (used for testing lights within the bounds of opaque geometry).
    Plane minPlane = { float3( 0, 0, -1 ), -minDepthVS };

    // Cull lights
    // Each thread in a group will cull 1 light until all lights have been culled.
    for ( uint i = IN.groupIndex; i < MAX_LIGHTS; i += BLOCK_SIZE * BLOCK_SIZE )
    {
        if ( Lights[i].Enabled )
        {
            LightProperties light = Lights[i];

            switch ( light.LightType )
            {
                case POINT_LIGHT:
                {
                    Sphere sphere = { light.PositionVS.xyz, light.Range + light.Bias };
                    if ( SphereInsideFrustum( sphere, GroupFrustum, nearClipVS, maxDepthVS) )
                    {
                        // Add light to light list for transparent geometry.
                        // t_AppendLight( i );
    
                        if ( !SphereInsidePlane( sphere, minPlane ) )
                        {
                            // Add light to light list for opaque geometry.
                            o_AppendLight( i );
                        }
                    }
                }
                break;

                case SPOT_LIGHT:
                {
                    // SpotAngle is already in radian unit
                    float coneRadius = tan( light.SpotAngle + light.Bias ) * light.Range;

                    // Since we treat light direction as vector starts from point to light,
                    // we need to negate the direction to get correct cone direction
                    Cone cone = { light.PositionVS.xyz, light.Range, -light.DirectionVS.xyz, coneRadius };
                    
                    if ( ConeInsideFrustum( cone, GroupFrustum, nearClipVS, maxDepthVS ) )
                    {
                        // Add light to light list for transparent geometry.
                        // t_AppendLight( i );
    
                        if ( !ConeInsidePlane( cone, minPlane ) )
                        {
                            // Add light to light list for opaque geometry.
                            o_AppendLight( i );
                        }
                    }
                }
                break;

                case DIRECTIONAL_LIGHT:
                {
                    // Directional lights always get added to our light list.
                    // (Hopefully there are not too many directional lights!)
                    // t_AppendLight( i );
                    o_AppendLight( i );
                }
                break;
            }
        }
    }
 
    // Wait till all threads in group have caught up.
    GroupMemoryBarrierWithGroupSync();

    // Update global memory with visible light buffer.
    // First update the light grid (only thread 0 in group needs to do this)
    if ( IN.groupIndex == 0 )
    {
        // Update light grid for opaque geometry.
        InterlockedAdd( o_LightIndexCounter[0], o_LightCount, o_LightIndexStartOffset );
        o_LightGrid[IN.groupID.xy] = uint2( o_LightIndexStartOffset, o_LightCount );

        if (DebugMode == FP_DEBUG_MODE_UV)
        {
            o_LightGrid[IN.groupID.xy] = uint2( IN.groupID.x, IN.groupID.y );
        }

        else if (DebugMode == FP_DEBUG_MODE_DEPTH)
        {
            fDepth = pow(fDepth, DepthPower);
            o_LightGrid[IN.groupID.xy] = uint2(fDepth * DepthPower, fDepth * DepthPower); // scale to integer range
        }
        
        // Update light grid for transparent geometry.
        // InterlockedAdd( t_LightIndexCounter[0], t_LightCount, t_LightIndexStartOffset );
        // t_LightGrid[IN.groupID.xy] = uint2( t_LightIndexStartOffset, t_LightCount );
    }
 
    GroupMemoryBarrierWithGroupSync();

    // Now update the light index list (all threads).    
    // For opaque geometry.
    for ( i = IN.groupIndex; i < o_LightCount; i += BLOCK_SIZE * BLOCK_SIZE )
    {
        o_LightIndexList[o_LightIndexStartOffset + i] = o_LightList[i];
    }

    // For transparent geometry.
    // for ( i = IN.groupIndex; i < t_LightCount; i += BLOCK_SIZE * BLOCK_SIZE )
    // {
    //     t_LightIndexList[t_LightIndexStartOffset + i] = t_LightList[i];
    // }
}
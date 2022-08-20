#include "../Structures.hlsli"
#include "Common.hlsli"

#define BLOCK_SIZE 16

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
}

cbuffer LightProperties : register(b2)
{
    float4 EyePosition;                 // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 GlobalAmbient;               // 16 bytes
    //----------------------------------- (16 byte boundary)
    struct LightProperties Lights[MAX_LIGHTS];    // 80 * 8 = 640 bytes
};  // Total:  

// The depth from the screen space texture.
Texture2D DepthTextureVS : register( t3 );

// Precomputed frustums for the grid.
StructuredBuffer<Frustum> in_Frustums : register( t9 );

// "o_" prefix indicates light lists for opaque geometry while 
// "t_" prefix indicates light lists for transparent geometry.
RWStructuredBuffer<uint> o_LightIndexCounter : register( u1 );
RWStructuredBuffer<uint> o_LightIndexList : register( u3 );
RWTexture2D<uint2> o_LightGrid : register( u5 );

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

bool SphereInsidePlane( Sphere sphere, Plane plane )
{
    return dot( plane.N, sphere.c ) - plane.d < -sphere.r;
}

// Check to see of a light is partially contained within the frustum.
// Assumes a right-handed coordinate system with the camera looking towards the negative z axis
bool SphereInsideFrustum( Sphere sphere, Frustum frustum, float zNear, float zFar )
{
    bool result = true;
 
    // TODO: check z axis should be reversed?
    // if ( (sphere.c.z - sphere.r) > zFar || (sphere.c.z + sphere.r) < zNear )

    // First check depth, note the sphere is in view space
    // Also, the view vector points in the -Z axis so the far depth value will be approaching -infinity.
    if ( (sphere.c.z - sphere.r) > zNear || (sphere.c.z + sphere.r) < zFar )
    {
        result = false;
    }
 
    // Then check frustum planes
    for ( int i = 0; i < 4 && result; i++ )
    {
        if ( SphereInsidePlane( sphere, frustum.planes[i] ) )
        {
            result = false;
        }
    }
 
    return result;
}

// Check to see if a point is fully behind (inside the negative halfspace of) a plane.
bool PointInsidePlane( float3 p, Plane plane )
{
    return dot( plane.N, p ) - plane.d < 0;
}

// Check to see if a cone if fully behind (inside the negative halfspace of) a plane.
bool ConeInsidePlane( Cone cone, Plane plane )
{
    // Compute the farthest point on the end of the cone to the positive space of the plane.
    float3 m = cross( cross( plane.N, cone.d ), cone.d );
    float3 Q = cone.T + cone.d * cone.h - m * cone.r;
 
    // The cone is in the negative halfspace of the plane if both
    // the tip of the cone and the farthest point on the end of the cone to the 
    // positive halfspace of the plane are both inside the negative halfspace 
    // of the plane.
    return PointInsidePlane( cone.T, plane ) && PointInsidePlane( Q, plane );
}

bool ConeInsideFrustum( Cone cone, Frustum frustum, float zNear, float zFar )
{
    bool result = true;
 
    Plane nearPlane = { float3( 0, 0, -1 ), -zNear };
    Plane farPlane = { float3( 0, 0, 1 ), zFar };
 
    // First check the near and far clipping planes.
    if ( ConeInsidePlane( cone, nearPlane ) || ConeInsidePlane( cone, farPlane ) )
    {
        result = false;
    }
 
    // Then check frustum planes
    for ( int i = 0; i < 4 && result; i++ )
    {
        if ( ConeInsidePlane( cone, frustum.planes[i] ) )
        {
            result = false;
        }
    }
 
    return result;
}

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

// Convert clip space coordinates to view space
float4 ClipToView( float4 clip )
{
    float4 view = mul( InverseProjection, clip );
    view = view / view.w; 
    return view;
}

float GetRadius(LightProperties light)
{
    float lightMax = max(max(light.Color.x, light.Color.y), light.Color.z) * light.Strength;

    // Reference: https://learnopengl.com/Advanced-Lighting/Deferred-Shading, we use 10/256 as dark threshold
    float darkThreshold = (256.0f / 2.5f);
    return (-light.LinearAttenuation + sqrt(light.LinearAttenuation * light.LinearAttenuation - 4.0f * light.QuadraticAttenuation * (light.ConstantAttenuation - darkThreshold * lightMax))) / (2.0f * light.QuadraticAttenuation);
}

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
    float minDepthVS = ClipToView( float4( 0, 0, fMinDepth, 1 ) ).z; // for opaque geometry
    float maxDepthVS = ClipToView( float4( 0, 0, fMaxDepth, 1 ) ).z;
    
    float nearClipVS = ClipToView( float4( 0, 0, 0, 1 ) ).z; // for transparent geometry
 
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
                    float range = GetRadius(light);
                    Sphere sphere = { light.PositionVS.xyz, range };
                    if ( SphereInsideFrustum( sphere, GroupFrustum, nearClipVS, maxDepthVS ) )
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
                    float range = GetRadius(light);
                    float coneRadius = tan( radians( light.SpotAngle ) ) * range;
                    Cone cone = { light.PositionVS.xyz, range, light.DirectionVS.xyz, coneRadius };
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
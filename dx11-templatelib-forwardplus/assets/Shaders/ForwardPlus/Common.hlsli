#define BLOCK_SIZE 16

struct Plane
{
    float3 N;   // Plane Normal
    float d;    // Distance to origin
};

struct Frustum
{
    Plane planes[4];    // left, right, top, bottom
};

struct Sphere
{
    float3 c;   // Center point.
    float  r;   // Radius.
};

struct Cone
{
    float3 T;   // Cone tip.
    float  h;   // Height of the cone.
    float3 d;   // Direction of the cone.
    float  r;   // bottom radius of the cone.
};

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
 
    // First check depth, note the sphere is in view space
    // Since we use left hand coordintate, the view vector points in the +Z axis
    if ( (sphere.c.z - sphere.r) < zNear || (sphere.c.z + sphere.r) > zFar)
    {
        result = false;
    }

    // Right hand coordinate version, the view vector points in the -Z axis so the far depth value will be approaching -infinity.
    // if ( sphere.c.z - sphere.r > zNear || sphere.c.z + sphere.r < zFar )
    // {
    //     result = false;
    // }

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

// Convert clip space coordinates to view space
float4 ClipToView( matrix InverseProjection, float4 clip )
{
    float4 view = mul( InverseProjection, clip );
    view = view / view.w; 
    return view;
}

// Convert screen space coordinates to view space.
float4 ScreenToView(float4 screen, matrix InverseProjection, float2 ScreenDimensions)
{
    // Convert to normalized texture coordinates
    float2 texCoord = screen.xy / ScreenDimensions;

    // Convert to clip space
    float4 clip = float4( float2( texCoord.x, 1.0f - texCoord.y ) * 2.0f - 1.0f, screen.z, screen.w );

    return ClipToView(InverseProjection, clip );
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
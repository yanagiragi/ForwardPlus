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
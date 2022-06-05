struct Plane
{
    float3 N;   // Plane Normal
    float d;    // Distance to origin
};

struct Frustum
{
    Plane planes[4];    // left, right, top, bottom
};

Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
    Plane plane;

    float3 v1 = p1 - p0;
    float3 v2 = p2 - p0;

    plane.N = normalize(cross(v1, v2));
    plane.d = dot(plane.N, p0);

    return plane;
}

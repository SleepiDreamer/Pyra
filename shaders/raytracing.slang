uniform RWTexture2D outputTexture;
uniform RaytracingAccelerationStructure sceneBVH;

struct [raypayload] Payload
{
    float3 color : read(caller) : write(caller, closesthit, miss);
};

[shader("raygeneration")]
void RayGen()
{
    uint2 idx = DispatchRaysIndex().xy;

    RayDesc ray;
    ray.Origin = float3(0.0, 0.0, 0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1000.0;

    Payload payload;
    payload.color = float3(0.0, 0.0, 0.0, 1.0);

    TraceRay(sceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    outputTexture[idx] = float4(payload.color, 1.0);
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = float3(1.0, 0.0, 0.0);
}

[shader("miss")]
void Miss(inout Payload payload)
{
    payload.color = float3(0.0, 0.0, 1.0);
}
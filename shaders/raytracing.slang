uniform RWTexture2D outputTexture;
uniform RaytracingAccelerationStructure sceneBVH : register(t0, space0);

struct Camera
{
    float3 position;
    float fov;
    float3 forward;
    float _pad0;
    float3 right;
    float _pad1;
    float3 up;
    float _pad2;
}

struct [raypayload] Payload
{
    float3 color : read(caller) : write(caller, closesthit, miss);
};

float3 PinholeCamera(float2 uv, float3 cameraForward, float3 cameraRight, float3 cameraUp)
{
    float2 ndc = (uv * 2.0f) - 1.0f;
    float2 resolution = DispatchRaysDimensions().xy;
    float aspectRatio = resolution.x / resolution.y;

    float x = ndc.x * aspectRatio * tan(radians(70) * 0.5);
    float y = -ndc.y * tan(radians(70) * 0.5);

    float3 rayDir = normalize(float3(x, y, 1.0));

    float3 rotatedRayDir = rayDir.x * cameraRight + rayDir.y * cameraUp + rayDir.z * cameraForward;

    return normalize(rotatedRayDir);
}

[shader("raygeneration")]
void RayGen()
{
    uint2 idx = DispatchRaysIndex().xy;
    float2 uv = (float2(idx) + 0.5) / DispatchRaysDimensions().xy;

    RayDesc ray;
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = float3(1.0, 0.0, 0.0);
    float3 forward = float3(0.0, 0.0, 1.0);
    ray.Origin = PinholeCamera(uv, forward, right, up);
    //ray.Direction = normalize(float3(0.0, 0.0, 1.0));
    ray.Direction = normalize(-ray.Origin);
    ray.TMin = 0.0;
    ray.TMax = 1000.0;

    Payload payload;
    payload.color = float3(0.0, 0.0, 0.0);

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
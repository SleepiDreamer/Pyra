uniform RWTexture2D outputTexture : register(u0, space0);
uniform RaytracingAccelerationStructure sceneBVH : register(t0, space0);
StructuredBuffer<Material> materials : register(t1, space0);
SamplerState linearSampler : register(s0, space0);
uniform Camera camera : register(b0, space0);

StructuredBuffer<Vertex> vertices : register(t0, space1);
StructuredBuffer<uint> indices : register(t1, space1);

struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
}

struct Camera
{
    float3 position;
    float fov;
    float3 forward;
    uint _pad0;
    float3 right;
    uint _pad1;
    float3 up;
    uint _pad2;
}

struct Material
{
    float3 albedoFactor;
    int albedoIndex;
    float metallicFactor;
    float roughnessFactor;
    int metallicRoughnessIndex;
    int normalIndex;
    int emissiveIndex;
    int _pad0;
};

struct [raypayload] Payload
{
    float3 color : read(caller) : write(caller, closesthit, miss);
};

float3 PinholeCamera(float2 uv, Camera camera)
{
    float2 ndc = (uv * 2.0f) - 1.0f;
    float2 resolution = DispatchRaysDimensions().xy;
    float aspectRatio = resolution.x / resolution.y;

    float x = ndc.x * aspectRatio * tan(radians(camera.fov) * 0.5);
    float y = -ndc.y * tan(radians(camera.fov) * 0.5);

    float3 rayDir = normalize(float3(x, y, 1.0));

    float3 rotatedRayDir = rayDir.x * camera.right + rayDir.y * camera.up + rayDir.z * camera.forward;

    return normalize(rotatedRayDir);
}

[shader("raygeneration")]
void RayGen()
{
    uint2 idx = DispatchRaysIndex().xy;
    float2 uv = (float2(idx) + 0.5) / DispatchRaysDimensions().xy;

    RayDesc ray;
    ray.Origin = camera.position;
    ray.Direction = PinholeCamera(uv, camera);
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
    uint baseIdx = PrimitiveIndex() * 3;
    uint i0 = indices[baseIdx];
    uint i1 = indices[baseIdx + 1];
    uint i2 = indices[baseIdx + 2];

    float w = 1 - attr.barycentrics.x - attr.barycentrics.y;
    float2 uv = vertices[i0].uv * w
              + vertices[i1].uv * attr.barycentrics.x
              + vertices[i2].uv * attr.barycentrics.y;

    Material mat = materials[InstanceID()];

    DescriptorHandle<Texture2D> albedoHandle = DescriptorHandle<Texture2D>(uint2(mat.albedoIndex, 0));
    Texture2D albedoTex = albedoHandle;
    float3 albedo = albedoTex.SampleLevel(linearSampler, uv, 0).rgb;

    payload.color = albedo;
}

[shader("miss")]
void Miss(inout Payload payload)
{
    payload.color = (WorldRayDirection() + 1.0) / 2.0;
}
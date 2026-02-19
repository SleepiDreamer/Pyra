import structs;
import helpers;
import camera;

uniform RWTexture2D outputTexture : register(u0, space0);
uniform RaytracingAccelerationStructure sceneBVH : register(t0, space0);
StructuredBuffer<Material> materials : register(t1, space0);
SamplerState linearSampler : register(s0, space0);
uniform CameraData camera : register(b0, space0);

StructuredBuffer<Vertex> vertices : register(t0, space1);
StructuredBuffer<uint> indices : register(t1, space1);
cbuffer HitGroupConstants : register(b0, space1)
{
    uint materialIndex;
};

[shader("raygeneration")]
void RayGen()
{
    uint2 idx = DispatchRaysIndex().xy;
    float2 uv = (float2(idx) + 0.5) / DispatchRaysDimensions().xy;

    Camera cam;

    RayDesc ray;
    ray.Origin = camera.position;
    ray.Direction = cam.PinholeCamera(uv, camera);
    ray.TMin = 0.0;
    ray.TMax = 1000.0;

    Payload payload;
    payload.color = float3(0.0, 0.0, 0.0);

    TraceRay(sceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    outputTexture[idx] = float4(LinearToSrgb(payload.color), 1.0);
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

    Material material = materials[materialIndex];

    DescriptorHandle<Texture2D> albedoHandle = DescriptorHandle<Texture2D>(uint2(material.albedoIndex, 0));
    Texture2D albedoTex = albedoHandle;
    float3 albedo;
    if (material.albedoIndex != -1)
    {
        albedo = albedoTex.SampleLevel(linearSampler, uv, 0).rgb * material.albedoFactor;
    }
    else
    {
        albedo = material.albedoFactor;
    }

    payload.color = albedo;
}

[shader("miss")]
void Miss(inout Payload payload)
{
    payload.color = (WorldRayDirection() + 1.0) / 2.0;
}
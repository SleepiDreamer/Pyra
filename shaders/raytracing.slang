import structs;
import helpers;
import camera;

uniform RWTexture2D outputTexture : register(u0, space0);
uniform RaytracingAccelerationStructure sceneBVH : register(t0, space0);
StructuredBuffer<Material> materials : register(t1, space0);
SamplerState linearSampler : register(s0, space0);
ConstantBuffer<CameraData> camera : register(b0, space0);
ConstantBuffer<RenderSettings> settings : register(b1, space0);

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

    float3 albedo = 0;
    if (material.albedoIndex != -1)
    {
        DescriptorHandle<Texture2D> albedoHandle = DescriptorHandle<Texture2D>(uint2(material.albedoIndex, 0));
        Texture2D albedoTex = albedoHandle;
        albedo = albedoTex.SampleLevel(linearSampler, uv, 0).rgb * material.albedoFactor;
    }
    else
    {
        albedo = material.albedoFactor;
    }
    float3 emissive = 0;
    if (material.emissiveIndex != -1)
    {
        DescriptorHandle<Texture2D> emissiveHandle = DescriptorHandle<Texture2D>(uint2(material.emissiveIndex, 0));
        Texture2D emissiveTex = emissiveHandle;
        emissive = emissiveTex.SampleLevel(linearSampler, uv, 0).rgb; // TODO: no emissive factor?
    }
    float roughness = 0;
    float metallic = 0;
    if (material.metallicRoughnessIndex != -1)
    {
        DescriptorHandle<Texture2D> metallicRoughnessHandle = DescriptorHandle<Texture2D>(uint2(material.metallicRoughnessIndex, 0));
        Texture2D metallicRoughnessTex = metallicRoughnessHandle;
        float2 metallicRoughness = metallicRoughnessTex.SampleLevel(linearSampler, uv, 0).gb;
        roughness = metallicRoughness.x * material.roughnessFactor;
        metallic = metallicRoughness.y * material.metallicFactor;
    }
    else
    {
        roughness = material.roughnessFactor;
        metallic = material.metallicFactor;
    }


    if (settings.whiteFurnace) albedo = float3(1);
    payload.color = albedo;
    
    switch (settings.debugMode)
    {
    case DebugMode::None:
    {
        break;
    }
    case DebugMode::Albedo:
    {
        payload.color = albedo;
        return;
    }
    case DebugMode::Emissive:
    {
        payload.color = emissive;
        return;
    }
    case DebugMode::Metallic:
    {
        payload.color = metallic;
        return;
    }
    case DebugMode::Roughness:
    {
        payload.color = roughness;
        return;
    }
    case DebugMode::Normal:
    {
        payload.color = 1;
        return;
        break;
    }
    }

}

[shader("miss")]
void Miss(inout Payload payload)
{
    payload.color = (WorldRayDirection() + 1.0) / 2.0;
}
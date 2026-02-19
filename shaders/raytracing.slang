import structs;
import helpers;
import settings;
import rng;
import random.sampling;
import camera;
import tonemapping;

uniform RWTexture2D<float4> outputBuffer : register(u0, space0);
uniform RWTexture2D<float4> accumulationBuffer : register(u1, space0);
uniform RaytracingAccelerationStructure sceneBVH : register(t0, space0);
StructuredBuffer<Material> materials : register(t1, space0);
SamplerState linearSampler : register(s0, space0);
ConstantBuffer<CameraData> camera : register(b0, space0);
ConstantBuffer<RenderSettings> renderSettings : register(b1, space0);

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
    float2 size = DispatchRaysDimensions().xy;
    float2 uv = idx / size;

    Payload payload;
    payload.radiance = float3(0.0, 0.0, 0.0);
    payload.throughput = float3(1.0, 1.0, 1.0);
    payload.done = false;
    payload.depth = 0;
    payload.rng = PathTracingRNG.create(idx, uint(size.x), renderSettings.frame);
    uv += payload.rng.nextFloat2() / size; // TODO: pass jitter from CPU for DLSS

    Camera cam;

    RayDesc ray;
    ray.Origin = camera.position;
    ray.Direction = cam.PinholeCamera(uv, camera);
    ray.TMin = 0.0;
    ray.TMax = MAX_RAY_DEPTH;

    while (!payload.done && payload.depth < renderSettings.bounces + 1)
    {
        TraceRay(sceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

        if (payload.done)
        {
            break;
        }

        // from Nvidia's Zorah talk: https://www.youtube.com/watch?v=8AnVpcIczyk&t=1107s
        if (USE_RUSSIAN_ROULETTE)
        {
            if (payload.depth >= RUSSIAN_ROULETTE_START_BOUNCE)
            {
                float luminance = pow(dot(payload.throughput, float3(0.2126, 0.7152, 0.0722)), 0.5);
                float continuationProb = min(1.0, luminance);

                uint activeLaneCount = WaveActiveCountBits(true);
                uint totalLaneCount = WaveGetLaneCount();
                float activeLaneRatio = (float)activeLaneCount / (float)totalLaneCount;
                static const float activeLaneRatioThreshold = 0.1;
                float groupContinuationProb = continuationProb * saturate(activeLaneRatio / activeLaneRatioThreshold);

                if (payload.rng.nextFloat() >= groupContinuationProb)
                {
                    payload.done = true;
                }
                else
                {
                    payload.throughput /= groupContinuationProb;
                }
            }
        }

        ray.Origin = payload.nextOrigin;
        ray.Direction = payload.nextDirection;
        payload.depth++;
    }

    float3 radiance = max(sanitize(payload.radiance), 0.0);

    if (renderSettings.frame == 0)
        accumulationBuffer[idx].rgb = radiance;
    else
        accumulationBuffer[idx].rgb += radiance;

    float3 result = renderSettings.upscaling
        ? radiance
        : accumulationBuffer[idx].rgb / (renderSettings.frame + 1);

    outputBuffer[idx] = float4(tonemap(result, renderSettings.tonemapper, renderSettings.exposure), 1.0);
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    uint baseIdx = PrimitiveIndex() * 3;
    uint i0 = indices[baseIdx];
    uint i1 = indices[baseIdx + 1];
    uint i2 = indices[baseIdx + 2];

    float2 uv = sampleTriangle(vertices[i0].uv, vertices[i1].uv, vertices[i2].uv, attr.barycentrics);
    float3 normal = sampleTriangle(vertices[i0].normal, vertices[i1].normal, vertices[i2].normal, attr.barycentrics) .xyz;
    normal = normalize(mul(normal, (float3x3)WorldToObject3x4()));
    float3 hitPoint = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    Material material = materials[materialIndex];
    float3 albedo = sampleOrDefault(material.albedoIndex, linearSampler, uv, float4(1)).rgb * material.albedoFactor;
    if (renderSettings.whiteFurnace) albedo = float3(1);
    float3 emission = sampleOrDefault(material.emissiveIndex, linearSampler, uv, float4(0)).rgb * material.emissiveFactor;
    float2 metallicRoughness = sampleOrDefault(material.metallicRoughnessIndex, linearSampler, uv, float4(1)).gb;
    float roughness = metallicRoughness.x * material.roughnessFactor;
    float metallic = metallicRoughness.y * material.metallicFactor;

    switch (renderSettings.debugMode)
    {
        case DebugMode::None: break;
        case DebugMode::Albedo:     payload.done = true; payload.radiance = albedo; return;
        case DebugMode::Emissive:   payload.done = true; payload.radiance = emission; return;
        case DebugMode::Metallic:   payload.done = true; payload.radiance = metallic; return;
        case DebugMode::Roughness:  payload.done = true; payload.radiance = roughness; return;
        case DebugMode::Normal:     payload.done = true; payload.radiance = normal; return;
    }

    payload.radiance += emission * payload.throughput * renderSettings.lightExposure;

    payload.throughput *= albedo; // TODO: implement PBR
    payload.nextDirection = tangentToWorld(sampleCosineHemisphere(payload.rng.nextFloat2()), normal);
    payload.nextOrigin = OffsetRay(hitPoint, normal);
}

[shader("miss")]
void Miss(inout Payload payload)
{
    payload.done = true;

    if (renderSettings.debugMode != 0)
    {
        return;
    }

    if (renderSettings.whiteFurnace == 1)
    {
        payload.radiance += float3(1) * payload.throughput;
    }

    payload.radiance += float3(1) * renderSettings.skyExposure * payload.throughput;
}
import structs;
import helpers;
import settings;
import rng;
import random.sampling;
import pbr;
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
    payload.rng = RNG.create(idx, uint(size.x), renderSettings.frame);
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

        if (all(payload.throughput == 0.0)) payload.done = true;

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

    if (renderSettings.debugMode == DebugMode::None) result = tonemap(result, renderSettings.tonemapper, renderSettings.exposure);
    outputBuffer[idx] = float4(result, 1.0);
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    uint baseIdx = PrimitiveIndex() * 3;
    uint i0 = indices[baseIdx];
    uint i1 = indices[baseIdx + 1];
    uint i2 = indices[baseIdx + 2];

    Material material = materials[materialIndex];
    float2 uv = sampleTriangle(vertices[i0].uv, vertices[i1].uv, vertices[i2].uv, attr.barycentrics);

    float3 geoNormal = sampleTriangle(vertices[i0].normal, vertices[i1].normal, vertices[i2].normal, attr.barycentrics);
    geoNormal = normalize(mul(geoNormal, (float3x3)WorldToObject3x4()));
    float tangentW = vertices[i0].tangent.w * -1;
    float3 tangent = sampleTriangle(vertices[i0].tangent.xyz, vertices[i1].tangent.xyz, vertices[i2].tangent.xyz, attr.barycentrics);
    tangent = normalize(mul((float3x3)ObjectToWorld3x4(), tangent));
    float3 bitangent = cross(geoNormal, tangent) * tangentW;

    float3 normal = geoNormal;
    float3 normalMap = float3(0, 0, 1);
    if (material.normalIndex != -1)
    {
         Texture2D normalTex = DescriptorHandle<Texture2D>(uint2(material.normalIndex, 0));
         normalMap = normalTex.SampleLevel(linearSampler, uv, 0).rgb * 2 - 1;
    }
    float3x3 TBN = float3x3(tangent, bitangent, geoNormal);
    normal = normalize(mul(normalMap, TBN));
    if (HitKind() == HIT_KIND_TRIANGLE_BACK_FACE) 
    {
        normal = -normal;
        geoNormal = -geoNormal;
    }

    float3 hitPoint = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    float3 albedo = sampleOrDefault(material.albedoIndex, linearSampler, uv, material.albedoFactor);
    if (renderSettings.whiteFurnace) albedo = float3(1);
    float3 emission = sampleOrDefault(material.emissiveIndex, linearSampler, uv, material.emissiveFactor);
    float2 metallicRoughness = sampleOrDefault(material.metallicRoughnessIndex, linearSampler, uv, float3(1, material.metallicFactor, material.roughnessFactor)).gb;
    float roughness = max(metallicRoughness.x, 0.0001);
    float metallic = metallicRoughness.y;

    switch (renderSettings.debugMode)
    {
    case DebugMode::None: break;
        case DebugMode::Albedo:     payload.done = true; payload.radiance = albedo; return;
        case DebugMode::Emissive:   payload.done = true; payload.radiance = emission; return;
        case DebugMode::Metallic:   payload.done = true; payload.radiance = metallic; return;
        case DebugMode::Roughness:  payload.done = true; payload.radiance = roughness; return;
        case DebugMode::NormalMap:  payload.done = true; payload.radiance = (normalMap + 1) / 2; return;
        case DebugMode::Normal:     payload.done = true; payload.radiance = (normal + 1) / 2; return;
        case DebugMode::GeoNormal:  payload.done = true; payload.radiance = (geoNormal + 1) / 2; return;
        case DebugMode::Tangent:    payload.done = true; payload.radiance = (tangent + 1) / 2; return;
        case DebugMode::Bitangent:  payload.done = true; payload.radiance = (bitangent + 1) / 2; return;
        case DebugMode::TangentW:   payload.done = true; payload.radiance = tangentW; return;
    }

    payload.radiance += emission * payload.throughput * renderSettings.lightExposure;

    float3 wo;
    payload.throughput *= PBRIndirect(payload.rng, normal, geoNormal, -WorldRayDirection(), albedo, roughness, metallic, wo);
    payload.nextDirection = wo;
    payload.nextOrigin = OffsetRay(hitPoint, geoNormal);
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
        return;
    }

    if (renderSettings.hdriIndex != -1)
    {
        Texture2D hdri = DescriptorHandle<Texture2D>(uint2(renderSettings.hdriIndex, 0));
        float2 uv = DirectionToEquirectangular(WorldRayDirection());
        float3 env = min(hdri.SampleLevel(linearSampler, uv, 0).rgb, 30.0);
        payload.radiance += env * renderSettings.skyExposure * payload.throughput;
    }
    else
    {
        payload.radiance += float3(0.6, 0.8, 1.0) * renderSettings.skyExposure * payload.throughput * (WorldRayDirection().y + 1.1) / 2.1;
    }
}
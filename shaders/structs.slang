module structs;
import rng;

public struct [raypayload] Payload
{
    public float3 radiance : write(caller, closesthit, miss) : read(caller, closesthit, miss);
    public float depth : write(caller) : read(caller, closesthit, miss);
    public float3 throughput : write(caller, closesthit) : read(caller, closesthit, miss);
    public bool done : write(caller, closesthit, miss) : read(caller, closesthit, miss);
    public float3 nextOrigin : write(caller, closesthit) : read(caller);
    public float3 nextDirection : write(caller, closesthit) : read(caller);
    public RNG rng : write(caller, closesthit) : read(caller, closesthit);
};

public struct Vertex
{
    public float3 position;
    public float3 normal;
    public float2 uv;
    public float4 tangent;
};

public struct CameraData
{
    public float3 position;
    public float fov;
    public float3 forward;
    uint _pad0;
    public float3 right;
    uint _pad1;
    public float3 up;
    uint _pad2;
};

public enum DebugMode
{
    None = 0,
    Albedo,
    Emissive,
    Metallic,
    Roughness,
    NormalMap,
    Normal,
    GeoNormal,
    Tangent,
    Bitangent,
    TangentW,
};

public enum TonemapOperator
{
    Linear = 0,
    Aces,
    Reinhard,
    AgX,
    GT7,
};

public struct RenderSettings
{
    public DebugMode debugMode;
    public uint bounces;
    public float exposure;
    public float skyExposure;
    public float lightExposure;
    public int hdriIndex;
    public uint frame;
    public TonemapOperator tonemapper;
    public bool whiteFurnace;
    public bool upscaling;
};

public struct Material
{
    public float3 albedoFactor;
    public int albedoIndex;
    public float3 emissiveFactor;
    public int emissiveIndex;
    public float metallicFactor;
    public float roughnessFactor;
    public int metallicRoughnessIndex;
    public int normalIndex;
    int _pad0;
    int _pad1;
};
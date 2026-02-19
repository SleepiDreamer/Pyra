module structs;

public struct Vertex
{
    public float3 position;
    public float3 normal;
    public float2 uv;
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

public struct Material
{
    public float3 albedoFactor;
    public int albedoIndex;
    public float metallicFactor;
    public float roughnessFactor;
    public int metallicRoughnessIndex;
    public int normalIndex;
    public int emissiveIndex;
    int _pad0;
};

public struct [raypayload] Payload
{
    public float3 color : read(caller) : write(caller, closesthit, miss);
};
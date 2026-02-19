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
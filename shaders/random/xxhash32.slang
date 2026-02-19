// Spec: https://github.com/Cyan4973/xxHash/blob/f9155bd4c57e/doc/xxhash_spec.md
module xxhash32;

namespace xxHash32
{
    static const uint PRIME1 = 2654435761u;
    static const uint PRIME2 = 2246822519u;
    static const uint PRIME3 = 3266489917u;
    static const uint PRIME4 = 668265263u;
    static const uint PRIME5 = 374761393u;

    uint rol1 (uint v) { return (v <<  1) | (v >> 31); }
    uint rol7 (uint v) { return (v <<  7) | (v >> 25); }
    uint rol12(uint v) { return (v << 12) | (v >> 20); }
    uint rol13(uint v) { return (v << 13) | (v >> 19); }
    uint rol17(uint v) { return (v << 17) | (v >> 15); }
    uint rol18(uint v) { return (v << 18) | (v >> 14); }

    uint round(uint acc, uint lane)
    {
        return rol13(acc + lane * PRIME2) * PRIME1;
    }

    uint consumeUint(uint h, uint val)
    {
        h += val * PRIME3;
        return rol17(h) * PRIME4;
    }

    uint avalanche(uint h)
    {
        h ^= h >> 15; h *= PRIME2;
        h ^= h >> 13; h *= PRIME3;
        h ^= h >> 16;
        return h;
    }

    uint mergeAccumulators(uint v1, uint v2, uint v3, uint v4)
    {
        return rol1(v1) + rol7(v2) + rol12(v3) + rol18(v4);
    }

    public uint hash(uint p, uint seed = 0)
    {
        uint h = seed + PRIME5 + 4u;
        h = consumeUint(h, p);
        return avalanche(h);
    }

    public uint hash(uint2 p, uint seed = 0)
    {
        uint h = seed + PRIME5 + 8u;
        h = consumeUint(h, p.x);
        h = consumeUint(h, p.y);
        return avalanche(h);
    }

    public uint hash(uint3 p, uint seed = 0)
    {
        uint h = seed + PRIME5 + 12u;
        h = consumeUint(h, p.x);
        h = consumeUint(h, p.y);
        h = consumeUint(h, p.z);
        return avalanche(h);
    }

    public uint hash(uint4 p, uint seed = 0)
    {
        uint v1 = round(seed + PRIME1 + PRIME2, p.x);
        uint v2 = round(seed + PRIME2,           p.y);
        uint v3 = round(seed,                    p.z);
        uint v4 = round(seed - PRIME1,           p.w);
        uint h = mergeAccumulators(v1, v2, v3, v4) + 16u;
        return avalanche(h);
    }
}
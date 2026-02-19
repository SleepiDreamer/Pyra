module IRandom;

interface IRandom
{
    [mutating] uint nextUint();

    [mutating] float nextFloat();

    [mutating] float2 nextFloat2();

    [mutating] float3 nextFloat3();
}

extension<T : IRandom> T
{
    [mutating] float nextFloat()
    {
        return nextUint() * (1.0f / 4294967296.0f);
    }

    [mutating] float2 nextFloat2()
    {
        return float2(nextFloat(), nextFloat());
    }

    [mutating] float3 nextFloat3()
    {
        return float3(nextFloat(), nextFloat(), nextFloat());
    }
}


// ============================================================
// File: XxHash32.slang
// ============================================================
module XxHash32;

/// Spec-compliant xxHash32 core (stateless, pure functions).
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

    // --- Overloads by input size ---

    uint hash(uint p, uint seed = 0)
    {
        uint h = seed + PRIME5 + 4u;
        h = consumeUint(h, p);
        return avalanche(h);
    }

    uint hash(uint2 p, uint seed = 0)
    {
        uint h = seed + PRIME5 + 8u;
        h = consumeUint(h, p.x);
        h = consumeUint(h, p.y);
        return avalanche(h);
    }

    uint hash(uint3 p, uint seed = 0)
    {
        uint h = seed + PRIME5 + 12u;
        h = consumeUint(h, p.x);
        h = consumeUint(h, p.y);
        h = consumeUint(h, p.z);
        return avalanche(h);
    }

    uint hash(uint4 p, uint seed = 0)
    {
        uint v1 = round(seed + PRIME1 + PRIME2, p.x);
        uint v2 = round(seed + PRIME2,           p.y);
        uint v3 = round(seed,                    p.z);
        uint v4 = round(seed - PRIME1,           p.w);
        uint h = mergeAccumulators(v1, v2, v3, v4) + 16u;
        return avalanche(h);
    }
}


// ============================================================
// File: XxHashRandom.slang
// ============================================================
module XxHashRandom;

import IRandom;
import XxHash32;

/// Stateless hashing strategy: every nextUint() call is an
/// independent hash of (state, dimension).
/// Best decorrelation, slightly more ALU per sample.
struct XxHashStateless : IRandom
{
    uint state;
    uint dimension;

    /// Construct from pixel index + sample index.
    /// Hashes the pair to decouple from spatial coherence.
    __init(uint pixelIndex, uint sampleIndex)
    {
        state     = xxHash32::hash(uint2(pixelIndex, sampleIndex));
        dimension = 0;
    }

    /// Construct from pixel coords + sample index.
    __init(uint2 pixel, uint sampleIndex)
    {
        state     = xxHash32::hash(uint3(pixel, sampleIndex));
        dimension = 0;
    }

    [mutating] uint nextUint()
    {
        return xxHash32::hash(uint2(state, dimension++));
    }
}

/// Hybrid strategy: xxHash32 to seed, then a fast sequential
/// PCG for subsequent draws. Cheaper per-sample after the first,
/// but slightly more correlated across dimensions.
struct XxHashSeeded : IRandom
{
    uint state;

    __init(uint pixelIndex, uint sampleIndex)
    {
        state = xxHash32::hash(uint2(pixelIndex, sampleIndex));
    }

    __init(uint2 pixel, uint sampleIndex)
    {
        state = xxHash32::hash(uint3(pixel, sampleIndex));
    }

    [mutating] uint nextUint()
    {
        // PCG-RXS-M-XS (32-bit state, 32-bit output)
        state = state * 747796405u + 2891336453u;
        uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
    }
}
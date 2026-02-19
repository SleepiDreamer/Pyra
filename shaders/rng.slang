module rng;
import random.xxhash32;

public struct PathTracingRNG
{
    uint pixel;
    uint sample;
    uint dimension;

    [mutating]
    public static PathTracingRNG create(uint2 pixelCoord, uint screenWidth, uint sampleIndex)
    {
        PathTracingRNG rng;
        rng.pixel = pixelCoord.y * screenWidth + pixelCoord.x;
        rng.sample = sampleIndex;
        rng.dimension = 0;
        return rng;
    }

    public static PathTracingRNG create(uint pixelIndex, uint sampleIndex)
    {
        PathTracingRNG rng;
        rng.pixel = pixelIndex;
        rng.sample = sampleIndex;
        rng.dimension = 0;
        return rng;
    }

    // Sequential
    [mutating]
    public uint nextUint()
    {
        return xxHash32::hash(uint3(pixel, sample, dimension++));
    }

    [mutating]
    public float nextFloat()
    {
        return nextUint() * 0x1p-32f;
    }

    [mutating]
    public float2 nextFloat2()
    {
        return float2(nextFloat(), nextFloat());
    }

    // Stateless
    public uint hashDimension(uint dim)
    {
        return xxHash32::hash(uint3(pixel, sample, dim));
    }

    public float hashDimensionFloat(uint dim)
    {
        return hashDimension(dim) * 0x1p-32f;
    }
}
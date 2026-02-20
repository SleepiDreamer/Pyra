module rng;
import random.xxhash32;

public struct RNG
{
    public uint pixel;
    public uint sample;
    public uint dimension;

    [mutating]
    public static RNG create(uint2 pixelCoord, uint screenWidth, uint sampleIndex)
    {
        RNG rng;
        rng.pixel = pixelCoord.y * screenWidth + pixelCoord.x;
        rng.sample = sampleIndex;
        rng.dimension = 0;
        return rng;
    }

    public static RNG create(uint pixelIndex, uint sampleIndex)
    {
        RNG rng;
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

    public float2 hashDimensionFloat2(uint dim)
    {
        return float2(hashDimensionFloat(dim), hashDimensionFloat(dim + 1));
    }
}
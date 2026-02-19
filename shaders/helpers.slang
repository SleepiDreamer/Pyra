module helpers;

public float LinearToSrgb(float linear)
{
    if (linear <= 0.0031308)
        return linear * 12.92;
    else
        return 1.055 * pow(linear, 1.0 / 2.4) - 0.055;
}

public float3 LinearToSrgb(float3 linear)
{
    return float3(LinearToSrgb(linear.r), LinearToSrgb(linear.g), LinearToSrgb(linear.b));
}

public float SrgbToLinear(float srgb)
{
    if (srgb <= 0.04045)
        return srgb / 12.92;
    else
        return pow((srgb + 0.055) / 1.055, 2.4);
}

public float2 sampleTriangle(float2 x0, float2 x1, float2 x2, float2 bary)
{
    float w = 1 - bary.x - bary.y;
    return (x0 * w) + (x1 * bary.x) + (x2 * bary.y);
}

public float3 sampleTriangle(float3 x0, float3 x1, float3 x2, float2 bary)
{
    float w = 1 - bary.x - bary.y;
    return (x0 * w) + (x1 * bary.x) + (x2 * bary.y);
}

public float3 SrgbToLinear(float3 srgb)
{
    return float3(SrgbToLinear(srgb.r), SrgbToLinear(srgb.g), SrgbToLinear(srgb.b));
}

[ForceInline]
public float4 sampleOrDefault(int texIndex, SamplerState s, float2 uv, float4 fallback)
{
    if (texIndex != -1)
    {
        Texture2D tex = DescriptorHandle<Texture2D>(uint2(texIndex, 0));
        return tex.SampleLevel(s, uv, 0);
    }
    return fallback;
}

public float3 OffsetRay(float3 p, float3 n)
{
    const float origin = 1.0f / 32.0f;
    const float float_scale = 1.0f / 65536.0f;
    const float int_scale = 256.0f;

    int3 of_i = int3(int_scale * n);

    float3 p_i = float3(
        asfloat(asint(p.x) + ((p.x < 0) ? -of_i.x : of_i.x)),
                        asfloat(asint(p.y) + ((p.y < 0) ? -of_i.y : of_i.y)),
                        asfloat(asint(p.z) + ((p.z < 0) ? -of_i.z : of_i.z)));

    return float3(
        abs(p.x) < origin ? p.x + float_scale * n.x : p_i.x,
                  abs(p.y) < origin ? p.y + float_scale * n.y : p_i.y,
                  abs(p.z) < origin ? p.z + float_scale * n.z : p_i.z);
}
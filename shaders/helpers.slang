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

public float3 SrgbToLinear(float3 srgb)
{
    return float3(SrgbToLinear(srgb.r), SrgbToLinear(srgb.g), SrgbToLinear(srgb.b));
}
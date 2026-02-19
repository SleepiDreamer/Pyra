module tonemapping;

import structs;
import tonemapping.aces;
import tonemapping.reinhard;
import tonemapping.agx;

public float3 tonemap(float3 color, TonemapOperator op, float exposure)
{
    color *= exposure;

    switch (op)
    {
        case TonemapOperator::Linear:   return color;
        case TonemapOperator::Aces:     return tonemapACES(color);
        case TonemapOperator::Reinhard: return tonemapReinhard(color);
        case TonemapOperator::AgX:      return tonemapAgX(color);
        default:                        return color;
    }
}
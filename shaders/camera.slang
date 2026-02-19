module camera;
import structs;

public struct Camera
{
    public float3 PinholeCamera(float2 uv, CameraData camera)
    {
        float2 ndc = (uv * 2.0f) - 1.0f;
        float2 resolution = DispatchRaysDimensions().xy;
        float aspectRatio = resolution.x / resolution.y;

        float x = ndc.x * aspectRatio * tan(radians(camera.fov) * 0.5);
        float y = -ndc.y * tan(radians(camera.fov) * 0.5);

        float3 rayDir = normalize(float3(x, y, 1.0));

        float3 rotatedRayDir = rayDir.x * camera.right + rayDir.y * camera.up + rayDir.z * camera.forward;

        return normalize(rotatedRayDir);
    }
}

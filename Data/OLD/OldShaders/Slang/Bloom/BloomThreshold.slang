import "Modules/Constants";

struct Push
{
    float Threshold;
    float SoftThreshold;
}

[vk::push_constant] ConstantBuffer<Push> push;

[vk::binding(0)] RWTexture2D<float4> inputImage;
[vk::binding(1)] RWTexture2D<float4> outputImage;

[shader("compute")]
[numthreads(16, 16, 1)]
func main(uint3 dispatchID: SV_DispatchThreadID)
{
    float3 color = inputImage[dispatchID.xy].rgb;

    // From: https://catlikecoding.com/unity/tutorials/advanced-rendering/bloom/#3.4
    float brightness = max(max(color.r, color.g), color.b);
    float knee = push.Threshold * push.SoftThreshold;
    float soft = brightness - push.Threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / max(4.0 * knee, Constants::EPSILON);
    float contribution = max(soft, brightness - push.Threshold);
    contribution /= max(brightness, Constants::EPSILON);

    color *= contribution;
    outputImage[dispatchID.xy] = float4(color, 1.0);
}
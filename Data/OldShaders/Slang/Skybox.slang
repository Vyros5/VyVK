struct VSIn
{
    [vk::location(0)] float3 Position;
};

struct VSOut
{
    float4 Position : SV_Position;
    float3 LocalPos;
};

struct FSOut
{
    [vk::location(0)] float4 Color;
};

struct Push
{
    float4x4 ViewProjection;
};

[vk::push_constant] ConstantBuffer<Push> push;
[vk::binding(0)] SamplerCube skyboxMap;

// Vertex Shader ---------------------------

[shader("vertex")]
func vertexMain(VSIn input, out VSOut output)
{
    float4 clipPos = mul(push.ViewProjection, float4(input.Position, 1.0));
    output.Position = clipPos.xyww; // Ensure maximum depth (z = 1.0)
    output.LocalPos = input.Position;
}

// Fragment Shader ---------------------------

[shader("fragment")]
func fragmentMain(VSOut input, out FSOut output)
{
    output.Color = float4(skyboxMap.Sample(input.LocalPos).rgb, 1.0);
}
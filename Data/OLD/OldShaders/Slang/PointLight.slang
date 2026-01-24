import "Modules/Constants";

static const float LIGHT_RADIUS = 0.1;

static const float2 OFFSETS[6] = float2[](
    float2(-1.0, -1.0),
    float2( 1.0, -1.0),
    float2(-1.0,  1.0),
    float2(-1.0,  1.0),
    float2( 1.0, -1.0),
    float2( 1.0,  1.0)
);

struct VSOut
{
    float4 Position : SV_Position;
    float2 FragOffset;
}

struct FSOut
{
    [vk::location(0)] float4 Color;
}

struct Push
{
    float4 Position;
    float4 Color;
    float  Radius;
}

struct GlobalUbo
{
    float4x4 View;
    float4x4 Projection;
}

[vk::push_constant] ConstantBuffer<Push> push;
[vk::binding(0)] ConstantBuffer<GlobalUbo> ubo;

// Vertex shader --------------------

[shader("vertex")]
func vertexMain(uint vertexID: SV_VertexID, out VSOut output)
{
    output.FragOffset = OFFSETS[vertexID];

    float4 lightCameraSpace    = mul(ubo.View, push.Position);
    float4 positionCameraSpace = lightCameraSpace + float4(output.FragOffset, 0.0, 0.0) * push.Radius;

    output.Position = mul(ubo.Projection, positionCameraSpace);
}

// Fragment shader ------------------

[shader("fragment")]
func fragmentMain(VSOut input, out FSOut output)
{
    float offsetDistance = dot(input.FragOffset, input.FragOffset);
    if (offsetDistance > 1.0)
        discard;

    float cosDistance = 0.5 * (cos(offsetDistance * Constants::PI) + 1.0);

    output.Color = float4(push.Color.xyz + cosDistance, cosDistance);
}
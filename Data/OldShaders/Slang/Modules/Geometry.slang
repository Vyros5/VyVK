module Geometry;

namespace Geometry
{
    public struct Global
    {
        public float4x4 Projection;
        public float4x4 View;
        public float4x4 InverseView;
    };

    public struct Push
    {
        public float4x4 Model;
        public float4x4 Normal;
    };

    public struct Vertex
    {
        public float3 Position;
        public float3 Normal;
        public float2 UV;
        public float3 Color;
    };

    public struct Meshlet
    {
        public float3  Center;
        public float   Radius;
        public int8_t  ConeAxis[3];
        public int8_t  ConeCutoff;
        public uint    VertexOffset;
        public uint    PrimitiveOffset;
        public uint8_t VertexCount;
        public uint8_t PrimitiveCount;
    };

    public struct VertexIn
    {
        [vk::location(0)] public float3 Position;
        [vk::location(1)] public float3 Normal;
        [vk::location(2)] public float2 UV;
        [vk::location(3)] public float3 Color;
    };

    public struct GBuffer
    {
        [vk::location(0)] public float4 Position;
        [vk::location(1)] public float4 Normal;
        [vk::location(2)] public float4 Albedo;
        [vk::location(3)] public float4 ARM;
        [vk::location(4)] public float4 Emissive;
    };
}
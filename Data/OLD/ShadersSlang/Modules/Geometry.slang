module Geometry;

namespace Geometry
{
    public struct CameraData
    {
        public float4x4 Projection;
        public float4x4 View;
        public float4x4 InverseView;
    };

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

    public struct VertexIn
    {
        [[vk::location(0)]] public float3 Position : POSITION0;
        [[vk::location(1)]] public float3 Normal   : NORMAL0;
        [[vk::location(2)]] public float2 UV       : TEXCOORD0;
        [[vk::location(3)]] public float4 Tangent  : TEXCOORD1;
    };


}
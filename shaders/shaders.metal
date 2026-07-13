#include <metal_stdlib>

using namespace metal;

struct VertexIn{
    packed_float2 position;
    packed_float4 color;
};

struct VertexOut{
    float4 position [[position]];
    float4 color;
};

vertex VertexOut vertexMain(
    const device VertexIn* vertices [[buffer(0)]],
    uint vertexID [[vertex_id]])
{
    VertexOut out;

    out.position = float4(
        vertices[vertexID].position,
        0.0,
        1.0);

    out.color = vertices[vertexID].color;

    return out;
}

fragment float4 fragmentMain(
    VertexOut in [[stage_in]])
{
    return in.color;
}
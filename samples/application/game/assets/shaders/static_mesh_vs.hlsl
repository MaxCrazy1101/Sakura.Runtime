#pragma pack_matrix(row_major)

struct VSIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    centroid float3 normal : NORMAL;
#ifdef VERTEX_HAS_TANGENT
    centroid float4 tangent : TANGENT;
#endif
#ifdef VERTEX_HAS_SKIN
    centroid uint4 joints : JOINTS;
    centroid float4 weights : WEIGHTS;
#endif
};

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    centroid float4 normal : NORMAL;
    float4 tangent : TANGENT;
};

cbuffer ForwardRenderConstants
{
    float4x4 view_proj;
};

struct RootConstants
{
    float4x4 model;
};
[[vk::push_constant]]
ConstantBuffer<RootConstants> push_constants;

VSOut main(const VSIn input)
{
    VSOut output;
    float4 posW = mul(float4(input.position, 1.0f), push_constants.model);
    float4 posH = mul(posW, view_proj);
    output.position = posH;
    output.uv = input.uv;
    output.normal = float4(input.normal, 0.f);
#ifdef VERTEX_HAS_TANGENT // if (VERTEX_HAS_TANGENT.on)
    output.tangent = input.tangent;
#endif
#ifdef SM_5_0
    output.uv = input.uv.yx;
#endif
#ifdef SM_6_3
    output.uv = output.uv.yx;
#endif
#ifdef SM_6_6
    output.uv = output.uv.yx;
#endif
    return output;
}
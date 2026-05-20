// Shaders.hlsl
// Simple position+color shader — no lighting

cbuffer cbCameraInfo : register(b0)
{
    matrix gmtxView;
    matrix gmtxProjection;
    float3 gvCameraPosition;
    float  padding;
};

cbuffer cbWorldMatrix : register(b1)
{
    matrix gmtxWorld;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 posW = mul(float4(input.position, 1.0f), gmtxWorld);
    float4 posV = mul(posW, gmtxView);
    output.position = mul(posV, gmtxProjection);
    output.color = input.color;
    return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return input.color;
}

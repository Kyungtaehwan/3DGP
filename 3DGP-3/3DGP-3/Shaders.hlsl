// Shaders.hlsl
// Position-only vertex stream + per-object color from CB (no lighting).

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
    float4 gvObjectColor;
};

struct VS_INPUT
{
    float3 position : POSITION;
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
    output.color    = gvObjectColor;
    return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return input.color;
}

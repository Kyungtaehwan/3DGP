// Shaders.hlsl — per-pixel lighting (matches Sample LabProject07-9-1-3).

struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular; // a = power
    float4 m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
    matrix gmtxView         : packoffset(c0);
    matrix gmtxProjection   : packoffset(c4);
    float3 gvCameraPosition : packoffset(c8);
};

#define MATERIALS_IN_HIERARCHY 8

cbuffer cbGameObjectInfo : register(b2)
{
    matrix   gmtxGameObject : packoffset(c0);
    MATERIAL gMaterials[MATERIALS_IN_HIERARCHY] : packoffset(c4);
};

cbuffer cbFrameworkConstantInfo : register(b3)
{
    int gnMaterial : packoffset(c0);
};

#include "Light.hlsl"

struct VS_LIGHTING_INPUT
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
    float4 position  : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW   : NORMAL;
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
    VS_LIGHTING_OUTPUT output;
    output.normalW   = mul(input.normal, (float3x3)gmtxGameObject);
    output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
    output.position  = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    return output;
}

float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
    input.normalW = normalize(input.normalW);
    float4 color  = Lighting(input.positionW, input.normalW);
    return color;
}

// ------------------------------------------------------------
// Terrain pixel shader: lit terrain + a route line painted onto the
// surface. The line is the segment (gLineP0P1.xy) -> (gLineP0P1.zw) in
// world XZ; pixels within gLineHalfWidth of it are tinted gLineColor.
// Only the terrain uses this PS, so the humvee/trees driving over the
// line are NOT tinted.
// ------------------------------------------------------------
cbuffer cbTerrainLine : register(b5)
{
    float4 gLineP0P1;       // xy = start(x,z), zw = end(x,z)
    float4 gLineColor;      // rgb = color, a = opacity
    float  gLineHalfWidth;  // world units
    float3 gLinePad;
};

float4 PSTerrainLine(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
    input.normalW = normalize(input.normalW);
    float4 color  = Lighting(input.positionW, input.normalW);

    float2 p  = input.positionW.xz;
    float2 a  = gLineP0P1.xy;
    float2 b  = gLineP0P1.zw;
    float2 ab = b - a;
    float  t  = saturate(dot(p - a, ab) / max(dot(ab, ab), 1e-5f));
    float  d  = distance(p, a + t * ab);

    // 1 inside the band, smoothly fading to 0 at the edge.
    float k = 1.0f - smoothstep(gLineHalfWidth * 0.6f, gLineHalfWidth, d);
    color.rgb = lerp(color.rgb, gLineColor.rgb, k * gLineColor.a);
    return color;
}

// ------------------------------------------------------------
// Unlit vertex-color shader (for bitmap cube text on the
// LOGO / MENU screens). Reuses cbCameraInfo (b1) + the world
// matrix in cbGameObjectInfo (b2); ignores materials and lights.
// ------------------------------------------------------------
struct VS_COLOR_INPUT
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct VS_COLOR_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

VS_COLOR_OUTPUT VSColor(VS_COLOR_INPUT input)
{
    VS_COLOR_OUTPUT output;
    float4 posW    = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.position = mul(mul(posW, gmtxView), gmtxProjection);
    output.color    = input.color;
    return output;
}

float4 PSColor(VS_COLOR_OUTPUT input) : SV_TARGET
{
    return input.color;
}

#include "header.hlsl"

Texture2D g_texture         : register(t0);
Texture2D g_detailTexture   : register(t1);
SamplerState g_sampler      : register(s0);

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projMatrix);
    output.color = input.color;
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return input.color;
}

// --------------------------------------

VSTextureOutput VSTextureMain(VSTextureInput input)
{
    VSTextureOutput output;
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projMatrix);
    output.uv = input.uv;
    return output;
}

float4 PSTextureMain(VSTextureOutput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}

// --------------------------------------

VSTerrainOutput VSTerrainMain(VSTerrainInput input)
{
    VSTerrainOutput output;
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projMatrix);
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
    return output;
}

float4 PSTerrainMain(VSTerrainOutput input) : SV_TARGET
{
    float4 baseTextureColor = g_texture.Sample(g_sampler, input.uv0);
    float4 detailTextureColor = g_detailTexture.Sample(g_sampler, input.uv1);
    return lerp(baseTextureColor, detailTextureColor, 0.4f);
}

// --------------------------------------

VSTerrainOutput VSTerrainTessMain(VSTerrainInput input)
{
    VSTerrainOutput output;
    output.position = mul(input.position, worldMatrix);
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
    return output;
}

float CalculateTessFactor(float3 f3Position)
{   
    float fDistToCamera = distance(f3Position, cameraPosition);
    float s = saturate(fDistToCamera / 75.0f);
    return lerp(64.0f, 3.0f, s);
}

PatchTess TerrainTessConstantHS(InputPatch<VSTerrainOutput, 25> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess output;
    output.EdgeTess[0] = CalculateTessFactor(0.5f * (patch[0].position.xyz + patch[4].position.xyz));
    output.EdgeTess[1] = CalculateTessFactor(0.5f * (patch[0].position.xyz + patch[20].position.xyz));
    output.EdgeTess[2] = CalculateTessFactor(0.5f * (patch[4].position.xyz + patch[24].position.xyz));
    output.EdgeTess[3] = CalculateTessFactor(0.5f * (patch[20].position.xyz + patch[24].position.xyz));
    
    float3 center = float3(0, 0, 0);
    for (int i = 0; i < 25; ++i)
        center += patch[i].position.xyz;
    center /= 25.0f;
    output.InsideTess[0] = output.InsideTess[1] = CalculateTessFactor(center);
    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(25)]
[patchconstantfunc("TerrainTessConstantHS")]
[maxtessfactor(64.0f)]
HSOutput HSTerrainTessMain(InputPatch<VSTerrainOutput, 25> p, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    HSOutput result;
    result.position = p[i].position;
    result.uv0 = p[i].uv0;
    result.uv1 = p[i].uv1;
    return result;
}

void BernsteinCoeffcient5x5(float t, out float fBernstein[5])
{
    float tInv = 1.0f - t;
    fBernstein[0] = tInv * tInv * tInv * tInv;
    fBernstein[1] = 4.0f * t * tInv * tInv * tInv;
    fBernstein[2] = 6.0f * t * t * tInv * tInv;
    fBernstein[3] = 4.0f * t * t * t * tInv;
    fBernstein[4] = t * t * t * t;
}

float3 CubicBezierSum5x5(OutputPatch<HSOutput, 25> patch, float2 uv)
{
    // 4차 베지에 곡선 계수 계산
    float uB[5], vB[5];
    BernsteinCoeffcient5x5(uv.x, uB);
    BernsteinCoeffcient5x5(uv.y, vB);
    
    // 4차 베지에 곡면 계산
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 5; ++i)
    {
        float3 subSum = float3(0.0f, 0.0f, 0.0f);
        for (int j = 0; j < 5; ++j)
        {
            subSum += uB[j] * patch[i * 5 + j].position;
        }
        sum += vB[i] * subSum;
    }
    return sum;
    
    /*
    float3 f3Sum = float3(0.0f, 0.0f, 0.0f);
    f3Sum =  vB[0] * (uB[0] * patch[0].position +  uB[1] * patch[1].position +  uB[2] * patch[2].position +  uB[3] * patch[3].position +  uB[4] * patch[4].position);
    f3Sum += vB[1] * (uB[0] * patch[5].position +  uB[1] * patch[6].position +  uB[2] * patch[7].position +  uB[3] * patch[8].position +  uB[4] * patch[9].position);
    f3Sum += vB[2] * (uB[0] * patch[10].position + uB[1] * patch[11].position + uB[2] * patch[12].position + uB[3] * patch[13].position + uB[4] * patch[14].position);
    f3Sum += vB[3] * (uB[0] * patch[15].position + uB[1] * patch[16].position + uB[2] * patch[17].position + uB[3] * patch[18].position + uB[4] * patch[19].position);
    f3Sum += vB[4] * (uB[0] * patch[20].position + uB[1] * patch[21].position + uB[2] * patch[22].position + uB[3] * patch[23].position + uB[4] * patch[24].position);
    return f3Sum;
    */
}

[domain("quad")]
DSOutput DSTerrainTessMain(PatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HSOutput, 25> patch)
{
    DSOutput result;
    
    // 위치 좌표(베지에 곡면)
    result.position = float4(CubicBezierSum5x5(patch, uv), 1.0f);
    result.position = mul(result.position, viewMatrix);
    result.position = mul(result.position, projMatrix);
    
    // 텍스쳐 좌표
    result.uv0 = lerp(lerp(patch[0].uv0, patch[4].uv0, uv.x), lerp(patch[20].uv0, patch[24].uv0, uv.x), uv.y);
    result.uv1 = lerp(lerp(patch[0].uv1, patch[4].uv1, uv.x), lerp(patch[20].uv1, patch[24].uv1, uv.x), uv.y);
    
    return result;
}

float4 PSTerrainTessMain(DSOutput pin) : SV_TARGET
{
    float4 baseTextureColor = g_texture.Sample(g_sampler, pin.uv0);
    float4 detailTextureColor = g_detailTexture.Sample(g_sampler, pin.uv1);
    return lerp(baseTextureColor, detailTextureColor, 0.4f);
}

float4 PSTerrainTessWireMain(DSOutput pin) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
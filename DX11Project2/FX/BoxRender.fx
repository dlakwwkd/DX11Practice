//--------------------------------------------------------------------------------------
// File: BoxRender.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
    float4x4 WVP;
    float4x4 World;

    float4 lightDir;
    float4 lightColor;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 Pos : POSITION;
    float4 Color : COLOR;

    float3 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;

    float4 Normal : TEXCOORD;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(float4(input.Pos, 1.0f), WVP);
    output.Color = input.Color;

    output.Normal = mul(float4(input.Normal, 0.0f), World);
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
    float4 finalColor = 0;

    finalColor = saturate(dot((float3) - lightDir, input.Normal) * lightColor);
    finalColor.a = 1.0f;

    return finalColor;
}




cbuffer EnvConstantBuffer : register(b0)
{
    float4		LightDirection;
    float4		LightColor;
};


cbuffer CameraConstantBuffer : register(b1)
{
    float4x4    View;
    float4x4    Projection;
	float4      CameraPosition;
    float       Exposure;
    float       Vignette;
    float       Gamma;
    float       Bloom;
    float       Tone;
    float       Chromatic;
    float       Monokuro;
};


cbuffer ObjectConstantBuffer : register(b2)
{
    float4x4    World;
};


cbuffer SubsetConstantBuffer : register(b3)
{
    struct MATERIAL
    {
        float4 BaseColor;
        float4 EmissionColor;
        float Metallic;
        float Specular;
        float Roughness;
        float NormalWeight;
    } Material;
};






struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Color : COLOR;
};


struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float4 WorldPosition : POSITION;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Color : COLOR;
};

struct PS_OUTPUT_GEOMETRY
{
	float4 Color    : SV_TARGET0;
	float4 Normal   : SV_TARGET1;
	float4 Position : SV_TARGET2;
    float4 ARM      : SV_TARGET3;
    float4 Emission : SV_TARGET4;
};



struct PS_OUTPUT
{
    float4 Color : SV_TARGET0;
};

float3 ASESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

Texture2D<float4> TextureBaseColor   : register(t0);
Texture2D<float4> TextureNormal      : register(t1);
Texture2D<float4> TexturePosition    : register(t2);
Texture2D<float4> TextureARM         : register(t3);
Texture2D<float4> TextureEmission    : register(t4);

Texture2D<float4> TextureEnvironment : register(t5);

Texture2D<float4> TextureBloom[5]    : register(t1);

SamplerState Sampler : register(s0);

static float PI = 3.141592653589;


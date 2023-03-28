#include "Common.hlsl"

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;

	float4x4 wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);

	float4 position = float4(input.Position, 1.0f);
	output.Position = mul(position, wvp);

	float4 normal = float4(input.Normal, 0.0f);
	output.Normal = mul(normal, World);

	output.TexCoord = input.TexCoord;

	output.Color = input.Color;

	return output;

}
#include "Common.hlsl"

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;

	output.Position = float4(input.Position, 1.0);
	output.Normal = float4(input.Normal, 1.0);
	output.TexCoord = input.TexCoord;
	output.Color = input.Color;

	return output;

}
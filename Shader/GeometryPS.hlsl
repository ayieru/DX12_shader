#include "Common.hlsl"

PS_OUTPUT_GEOMETRY main(PS_INPUT input)
{
	PS_OUTPUT_GEOMETRY output;

	float4 baseColor;
	baseColor = TextureBaseColor.Sample(Sampler, input.TexCoord);
	output.Color = baseColor * Material.BaseColor;

	float3 normal = input.Normal;
	normal = normalize(normal);
	output.Normal.rgb = normal;
	output.Normal.a = 1.0f;

	float3 position = input.WorldPosition.xyz;
	output.Position.rgb = position;
	output.Position.a = 1.0f;

	float4 arm;
	arm = TextureARM.Sample(Sampler, input.TexCoord);
	output.ARM = arm;

	output.Emission = baseColor * Material.EmissionColor;
	return output;

}
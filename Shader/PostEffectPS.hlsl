#include "Common.hlsl"

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output;

	output.Color = TextureBaseColor.Sample(Sampler, input.TexCoord);
	
	//�u���[��
	for (int i = 0; i < 5; i++)
	{
		output.Color += TextureBloom[i].Sample(Sampler, input.TexCoord) * Bloom;
	}

	//�F����
	if (Chromatic) {
		output.Color.r += TextureBaseColor.Sample(Sampler, input.TexCoord).r;
		output.Color.g += TextureBaseColor.Sample(Sampler, input.TexCoord) * 1.0;
		output.Color.b += TextureBaseColor.Sample(Sampler, input.TexCoord) * 1.0;
	}

	//�r�l�b�g
	{
		float2 pos;
		pos.x = input.TexCoord.x * 2.0f - 1.0f;
		pos.y = input.TexCoord.y * 2.0f - 1.0f;

		float len = length(pos);

		output.Color.rgb *= (1.0f + (saturate(1.0 - len * 0.5) * Vignette));
	}

	//�I�o�␳
	output.Color.rgb *= 1.0f + (5 * Exposure);

	//�g�[���}�b�s���O
	if(Tone) output.Color.rgb = ASESFilm(output.Color.rgb) * Tone;

	//�K���}�␳
	if(Gamma) output.Color.rgb = pow(output.Color.rgb, 1.0 / 1.5) * Gamma;

	//���m�N��
	if(Monokuro) output.Color.rgb = ((output.Color.r + output.Color.b + output.Color.g) / 3.0) * Monokuro;

	return output;

}
#include "Common.hlsl"

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output;

	float4 baseColor = TextureBaseColor.Sample(Sampler, input.TexCoord).rgba;
	float4 normal = TextureNormal.Sample(Sampler, input.TexCoord).rgba;
	float4 position = TexturePosition.Sample(Sampler, input.TexCoord).rgba;
	float4 arm = TextureARM.Sample(Sampler, input.TexCoord).rgba;
	float4 emission = TextureEmission.Sample(Sampler, input.TexCoord).rgba;

	float3 eye = CameraPosition.xyz - position.xyz;
	eye = normalize(eye);

	float3 ref = reflect(-eye, normal.xyz);

	float3 halfVector = normalize(LightDirection.xyz + eye);

	float ao = arm.r;
	float metallic = arm.b;
	float spec = 0.8;
	float roughness = arm.g;

	float3 diffuse;
	{
		//ランバート
		diffuse = saturate(dot(LightDirection.xyz, normal.xyz));
		diffuse *= baseColor.rgb;
		diffuse *= LightColor.rgb;

		float ndv = saturate(dot(normal.xyz, eye));
		float ndl = saturate(dot(normal.xyz, LightDirection.xyz));
		float hdv = saturate(dot(halfVector, eye));
		float3 f90 = 0.5 + 2.0 * roughness * hdv * hdv;

		float fl = 1.0 + (f90 - 1.0) * pow(1.0 - ndl, 5);
		float fv = 1.0 + (f90 - 1.0) * pow(1.0 - ndv, 5);

		diffuse *= fl * fv;

		//正規化ランバート
		diffuse /= PI;
	}


	{
		//IBL
		float2 iblTexCoord;
		iblTexCoord.x = atan2(normal.x, normal.z) / (PI * 2);
		iblTexCoord.y = acos(normal.y) / PI;

		float3 light = TextureEnvironment.SampleLevel(Sampler, iblTexCoord, 8).rgb;

		diffuse += baseColor.rgb * light * 1.0 * ao;
	}


	float3 specular;
	{
		//GGX
		float ndv = saturate(dot(normal.xyz, eye));
		float ndl = saturate(dot(normal.xyz, LightDirection.xyz));
		float hdv = saturate(dot(halfVector, eye));
		float hdn = saturate(dot(halfVector, normal.xyz));

		float d;
		{
			float alpha = roughness * roughness;
			float alpha2 = alpha * alpha;
			float t = ((hdn * hdn) * (alpha2 - 1.0) + 1.0);
			d = alpha2 / (PI * t * t);
		}

		float g;
		{
			float k = (roughness + 1) * (roughness + 1) / 8.0;
			float gl = ndl / (ndl * (1.0 - k) + k);
			float gv = ndv / (ndv * (1.0 - k) + k);
			g = gl * gv;
		}

		float3 f;
		{
			float3 f0 = lerp(0.08 * spec, baseColor.rgb, metallic);
			f = f0 + (1.0 - f0) * pow(1.0 - hdv, 5);
		}

		specular = (d * f * g) / (4.0 * ndv * ndl);
		specular = saturate(specular);

		//環境マッピング
		float2 envTexCoord;
		envTexCoord.x = atan2(ref.x, ref.z) / (PI * 2);
		envTexCoord.y = acos(ref.y) / PI;

		specular += f * TextureEnvironment.SampleBias(
			Sampler, envTexCoord, roughness * 10).rgb * (1.0 - roughness) * ao;
	}

	output.Color.rgb = diffuse.rgb + specular.rgb + emission.rgb;
	output.Color.a = 1.0;

	return output;

}
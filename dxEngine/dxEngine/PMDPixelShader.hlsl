#include "PMDShaderHeader.hlsli"

float4 PSmain(Output input) : SV_TARGET
{
	//光源
	float3 light = normalize(float3( 1, -1, 1)); 
	//ライトのカラー 0~1
	float3 lightColor = float3(1, 1, 1);

	//ディフューズ
	float diffLight = dot(-light, input.normal);

	float2 normalUV = (input.normal.xy + float2(1, -1))
		* float2(0.5, -0.5);
	//スフィアマップ用のuv
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5); 

	//テクスチャカラー
	float4 texColor = tex.Sample(smp, input.uv); 

	//反射ベクトル
	float3 ref = normalize(reflect(light, input.normal.xyz)); 
	//スペキュラー
	float specLight = pow(
		saturate(dot(ref, -input.ray)),
		specular.a
	); 

	return max(
		diffLight * diffuse * texColor               //ディフューズ
		* sphere.Sample(smp, sphereMapUV)            //スフィアマップ
		+ addSph.Sample(smp, sphereMapUV) * texColor //加算スフィアマップ
		+ float4(specLight * specular.rgb, 1)        //スペキュラー
		, float4(ambient * texColor, 1));            //アンビエント
	//return addSph.Sample(smp, sphereMapUV) * texColor;
}
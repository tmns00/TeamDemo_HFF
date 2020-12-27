#include "OBJShaderHeader.hlsli"

Output VSmain(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD
){
	//ライト
	float3 lightDir = float3(1, -1, 1);
	lightDir = normalize(lightDir);

	//ライトの色
	float3 lightColor = float3(1, 1, 1);
	
	//環境反射光
	float3 ambient = m_ambient;

	//拡散反射光
	float3 diffuse = dot(-lightDir, normal) * m_diffuse;

	//視点座標
	const float3 eye = float3(0, 0, -20);
	//光沢度
	const float shininess = 4.0f;
	//頂点から視点への方向ベクトル
	float3 eyeDir = normalize(eye - pos.xyz);
	//反射光ベクトル
	float3 reflect = normalize(lightDir + 2 * dot(-lightDir, normal) * normal);
	//鏡面反射光
	float3 specular = pow(saturate(dot(reflect, eyeDir)), shininess) * m_specular;

	//ピクセルシェーダーに渡す値
	Output output; 
	output.svpos = mul(mat, pos);
	//Lambert反射の計算
	output.color.rgb = (ambient + diffuse + specular) * lightColor;
	output.color.a = m_alpha;
	output.uv = uv;
	return output;
}
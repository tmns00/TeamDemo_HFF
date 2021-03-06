#include "OBJShaderHeader.hlsli"

Output VSmain(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD
){
	//Cg
	float3 lightDir = float3(1, -1, 1);
	lightDir = normalize(lightDir);

	//CgΜF
	float3 lightColor = float3(1, 1, 1);
	
	//Β«½Λυ
	float3 ambient = m_ambient;

	//gU½Λυ
	float3 diffuse = dot(-lightDir, normal) * m_diffuse;

	//_ΐW
	const float3 eye = float3(0, 0, -20);
	//υςx
	const float shininess = 4.0f;
	//Έ_©η_ΦΜϋόxNg
	float3 eyeDir = normalize(eye - pos.xyz);
	//½ΛυxNg
	float3 reflect = normalize(lightDir + 2 * dot(-lightDir, normal) * normal);
	//ΎΚ½Λυ
	float3 specular = pow(saturate(dot(reflect, eyeDir)), shininess) * m_specular;

	//sNZVF[_[Ιn·l
	Output output; 
	output.svpos = mul(mat, pos);
	//Lambert½ΛΜvZ
	output.color.rgb = (ambient + diffuse + specular) * lightColor;
	output.color.a = m_alpha;
	output.uv = uv;
	return output;
}
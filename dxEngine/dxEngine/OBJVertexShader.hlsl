#include "OBJShaderHeader.hlsli"

Output VSmain(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD
){
	//���C�g
	float3 lightDir = float3(1, -1, 1);
	lightDir = normalize(lightDir);

	//���C�g�̐F
	float3 lightColor = float3(1, 1, 1);
	
	//�����ˌ�
	float3 ambient = m_ambient;

	//�g�U���ˌ�
	float3 diffuse = dot(-lightDir, normal) * m_diffuse;

	//���_���W
	const float3 eye = float3(0, 0, -20);
	//����x
	const float shininess = 4.0f;
	//���_���王�_�ւ̕����x�N�g��
	float3 eyeDir = normalize(eye - pos.xyz);
	//���ˌ��x�N�g��
	float3 reflect = normalize(lightDir + 2 * dot(-lightDir, normal) * normal);
	//���ʔ��ˌ�
	float3 specular = pow(saturate(dot(reflect, eyeDir)), shininess) * m_specular;

	//�s�N�Z���V�F�[�_�[�ɓn���l
	Output output; 
	output.svpos = mul(mat, pos);
	//Lambert���˂̌v�Z
	output.color.rgb = (ambient + diffuse + specular) * lightColor;
	output.color.a = m_alpha;
	output.uv = uv;
	return output;
}
#include "FBXShaderHeader.hlsli"

Output VSmain(
	float4 pos : POSITION,
	float4 normal : NORMAL
	//,float2 uv : TEXCOORD
) /*: SV_POSITION*/
{
	Output output; //�s�N�Z���V�F�[�_�[�ɓn���l
	output.svpos = mul(mat, pos);
	output.normal = normal; //�@���Ƀ��[���h�ϊ�
	//output.uv = uv;
	return output;
}
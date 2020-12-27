#include "PMDShaderHeader.hlsli"

Output VSmain(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 bone_no : BONE_NO,
	min16uint weight : WEIGHT
) /*: SV_POSITION*/
{
	Output output; //�s�N�Z���V�F�[�_�[�ɓn���l
	float w = weight / 100.0f;
	matrix bm = bones[bone_no[0]] * w +
		bones[bone_no[1]] * (1 - w);
	pos = mul(bm, pos);
	output.svpos = mul(mul(viewproj, world), pos);
	output.pos = mul(mul(view, world), pos);
	normal.w = 0; //���s�ړ������𖳌�
	output.normal = mul(world, normal); //�@���Ƀ��[���h�ϊ�
	output.vnormal = mul(view, output.normal); //���[���h�ϊ���̖@���x�N�g���Ƀr���[�ϊ���������
	output.uv = uv;
	output.ray = normalize(pos.xyz - mul(view, eye)); //�����x�N�g��
	return output;
}
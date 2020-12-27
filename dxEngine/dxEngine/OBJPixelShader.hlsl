#include "OBJShaderHeader.hlsli"

Texture2D<float4> tex:register(t0); //0�ԃX���b�g�@�e�N�X�`��
SamplerState smp:register(s0);     //0�ԃX���b�g�@�T���v���[

float4 PSmain(Output input) : SV_TARGET
{
	float4 texcolor = tex.Sample(smp, input.uv);
	return input.color * texcolor;
}
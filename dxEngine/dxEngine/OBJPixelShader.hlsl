#include "OBJShaderHeader.hlsli"

Texture2D<float4> tex:register(t0); //0番スロット　テクスチャ
SamplerState smp:register(s0);     //0番スロット　サンプラー

float4 PSmain(Output input) : SV_TARGET
{
	float4 texcolor = tex.Sample(smp, input.uv);
	return input.color * texcolor;
}
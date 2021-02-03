#include "FBXShaderHeader.hlsli"

float4 PSmain(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1,-1,1)); // 右下奥　向きのライト
	float diffuse = saturate(dot(-light, input.normal));
	float brightness = diffuse + 0.3f;
	//float4 texcolor = tex.Sample(smp, input.uv) * color;
	//return float4(texcolor.rgb * brightness, texcolor.a);
	return float4(brightness, brightness, brightness,1);
}
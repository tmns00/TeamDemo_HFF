#include "FBXShaderHeader.hlsli"

Output VSmain(
	float4 pos : POSITION,
	float4 normal : NORMAL
	//,float2 uv : TEXCOORD
) /*: SV_POSITION*/
{
	Output output; //ピクセルシェーダーに渡す値
	output.svpos = mul(mat, pos);
	output.normal = normal; //法線にワールド変換
	//output.uv = uv;
	return output;
}
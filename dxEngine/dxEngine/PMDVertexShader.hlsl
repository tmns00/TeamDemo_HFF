#include "PMDShaderHeader.hlsli"

Output VSmain(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 bone_no : BONE_NO,
	min16uint weight : WEIGHT
) /*: SV_POSITION*/
{
	Output output; //ピクセルシェーダーに渡す値
	float w = weight / 100.0f;
	matrix bm = bones[bone_no[0]] * w +
		bones[bone_no[1]] * (1 - w);
	pos = mul(bm, pos);
	output.svpos = mul(mul(viewproj, world), pos);
	output.pos = mul(mul(view, world), pos);
	normal.w = 0; //平行移動成分を無効
	output.normal = mul(world, normal); //法線にワールド変換
	output.vnormal = mul(view, output.normal); //ワールド変換後の法線ベクトルにビュー変換をかける
	output.uv = uv;
	output.ray = normalize(pos.xyz - mul(view, eye)); //視線ベクトル
	return output;
}
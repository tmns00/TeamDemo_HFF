//定数バッファ
cbuffer cbuff0 : register(b0)
{
	matrix mat;
};

cbuffer cbuff1:register(b1)
{
	float3 m_ambient:packoffset(c0);  //アンビエント係数
	float3 m_diffuse:packoffset(c1);  //ディフューズ係数
	float3 m_specular:packoffset(c2); //スペキュラー係数
	float m_alpha : packoffset(c2.w); //アルファ
}

//頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct Output
{
	float4 svpos : SV_POSITION; // システム用頂点座標
	float4 color : COLOR;       //色
	float2 uv  :TEXCOORD;       // uv値
};
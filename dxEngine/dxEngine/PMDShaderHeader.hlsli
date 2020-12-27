Texture2D<float4> tex : register(t0); //0番スロットに設定されたテクスチャ
Texture2D<float4> sphere : register(t1); //スフィアマップ 1番スロット
Texture2D<float4> addSph : register(t2); //加算スフィアマップ 2番スロット
SamplerState smp : register(s0); //0番スロットに設定されたサンプラー
//定数バッファ
cbuffer cbuff0 : register(b0)
{
	matrix world;    //ワールド変換行列
	matrix view;     //ビュー行列
	matrix proj;     //プロジェクション行列
	matrix viewproj; //ビュー・プロジェクション行列
	float3 eye;      //視点
	matrix bones[256]; //ボーン行列
};
//定数バッファ マテリアル用
cbuffer Material : register(b1)
{
	float4 diffuse;  //ディフューズ
	float4 specular; //スペキュラー
	float3 ambient;  //アンビエント
}

//頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体
struct Output
{
	float4 svpos : SV_POSITION; //システム用頂点座標
	float4 pos : POSITION;      //頂点座標
	float4 normal : NORMAL0;    //法線ベクトル
	float4 vnormal : NORMAL1;   //ビュー変換後の法線ベクトル
	float2 uv : TEXCOORD;       //uv値
	float3 ray : VECTOR;        //視線ベクトル
};
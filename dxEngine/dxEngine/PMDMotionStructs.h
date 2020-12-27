#pragma once

#include<unordered_map>
#include<DirectXMath.h>

//モーションデータ構造体
struct Motion
{
	unsigned int frameNo; //アニメーション開始からのフレーム数
	DirectX::XMVECTOR quaternion;  //クォータニオン
	DirectX::XMFLOAT3 offset;      //IKの初期座標からのオフセット情報
	DirectX::XMFLOAT2 p1, p2;      //ベジェ曲線の中間コントロールポイント
};

//IKオン/オフデータ
struct VMDIKEnable
{
	uint32_t frameNo;                                   //キーフレームがあるフレーム番号
	std::unordered_map<std::string, bool>ikEnableTable; //名前とオン/オフフラグのマップ
};

//ロードから描画クラスに必要なデータをまとめたもの
struct MotionDatas
{
	std::unordered_map<std::string, std::vector<Motion>> motion;
	unsigned int duration;
	std::vector<VMDIKEnable> ikEnableData;
};
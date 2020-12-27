#pragma once

#include"PMDMotionStructs.h"

#include<unordered_map>
#include<DirectXMath.h>

class VMDLoader
{
private:
	using XMMATRIX = DirectX::XMMATRIX;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT2;

private:
	//モーションデータロード用構造体
	struct VMDMotionData
	{
		char boneName[15];        //ボーン名
		unsigned int frameNo;     //フレーム番号
		XMFLOAT3 location;        //位置
		XMFLOAT4 quaternion;      //クォータニオン
		unsigned char bezier[64]; //ベジェ補間パラメーター
	};

#pragma pack(1)
	//表情データ
	struct VMDMorph
	{
		char name[15]; //名前
		uint32_t frameNo; //フレーム番号
		float weight; //ウェイト
	};
#pragma pack()

#pragma pack(1)
	//カメラ
	struct VMDCamera
	{
		uint32_t frameNo;          //フレーム番号
		float distance;            //距離
		XMFLOAT3 pos;              //座標
		XMFLOAT3 eulerAngle;       //オイラー角
		uint8_t Interpolation[24]; //補間
		uint32_t fov;              //視界角
		uint8_t persFlag;          //パースフラグ
	};
#pragma pack()

	//ライト照明データ
	struct VMDLight
	{
		uint32_t frameNo; //フレーム番号
		XMFLOAT3 rgb;     //ライト色
		XMFLOAT3 vec;     //光線ベクトル(平行光線)
	};

#pragma pack(1)
	//セルフ影データ
	struct VMDSelfShadow
	{
		uint32_t frameNo; //フレーム番号
		uint8_t mode;     //影モード
		float distance;   //距離
	};
#pragma pack()

private:
	static VMDLoader* instance;
	std::unordered_map<std::string,MotionDatas> motionDatasList;

private:
	VMDLoader() {};

	VMDLoader(const VMDLoader&) = delete;
	void operator=(const VMDLoader&) = delete;

public:
	~VMDLoader();

	MotionDatas GetMotion(
		std::string key
	);

public:
	static void Create();

	static VMDLoader* GetInstance();

private:
	void LoadVMDFile(
		std::string path
	);
};
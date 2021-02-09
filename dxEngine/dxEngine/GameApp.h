#pragma once

#include"WindowsApp.h"
#include"DirectXSystem.h"
#include"Input.h"
#include"Sound.h"
#include"Camera.h"
#include"GameObjectManager.h"
#include"DeltaCone.h"
#include"Sphere.h"
#include"Plane.h"
#include"Cylinder.h"
#include"Cube.h"
#include"Capsule.h"
#include"PMDObject.h"
#include"OBJObject.h"
#include"FbxObj2.h"
#include"SafeDelete.h"

#include"Vector3.h"
#include"PhisicMath.h"

enum Scene
{
	Title,
	Game,
	End,
};

class GameApp
{
private:
	//ウィンドウ表示クラス
	WindowsApp* winApp = nullptr;
	//DirectXの初期化クラス
	DirectXSystem* dxSystem = nullptr;
	//キー入力クラス
	Input* input = nullptr;
	//サウンドクラス
	Sound* sound = nullptr;
	//カメラクラス
	Camera* camera = nullptr;


	//ゲームオブジェクト管理クラス
	GameObjectManager* objManager = nullptr;
	
	//板ポリ
	Plane* plane = nullptr;
	//立方体
	Cube* cube = nullptr;
	
	//OBJモデル
	OBJObject* objObj = nullptr;
	//FBX
	FbxObj2* fbxObj = nullptr;

	//Sphereコンテナ
	std::vector<Sphere*> spheres;

	//シーン管理
	Scene sceneState = Title;
	//タイトル板ポリ
	Plane* title = nullptr;

	//物理
	bool isJamp;
	float t;
	Vector3 a;
	Vector3 vec;
	Vector3 a2;
	Vector3 vec2;
	Vector3 a3;
	float g = 0;
	Vector3 vec3;
	Vector3 r = { 0,0,0 };
	float radxx = 0;
	float radzx = 0;
	float radxy = 0;
	float radzy = 0;
	float rady = 0;
	float omgZ = 0;
	float omgY = 0;
	float omgX = 0;
	float VecZ = 0;
	float VecX = 0;
	float VecY = 0;
	Col_Sphere rectangle;
	Col_Plane col_plane;
	Vector3 Fx;
	Vector3 Fz;
	Vector3 Fy;
	Vector3 position;
	Vector3 inter2;
	bool isStopX = false;
	bool isStopZ = false;
	int count = 0;
	int cnt1[8] = { 0,1,2,3,//前			
				 4,5,6,7,//右
	};
	int cnt2[8]={ 
		4,5,6,7,//後ろ		 
		4,5,6,7,//左
	};
	float theat = 0;
	float phai = 0;
	float len = 0;
	float R1 = 0;
	float R2 = 0;
	float Theat = 0;
	
	DirectX::XMFLOAT3 Len;
	DirectX::XMFLOAT3 VertPos[24];
	DirectX::XMFLOAT3 v[8];
	
	float omg;
	DirectX::XMFLOAT3 POS;
	DirectX::XMFLOAT3 INTER;
	
	bool isHit = false;
	bool hit = false;
	bool PosStop = false;
	bool Grav = false;
	Vector3 F;
	bool GetPos = true;

	DirectX::XMFLOAT3 cube_rot;


private:
	//コンストラクタ
	GameApp();

	//コピー・代入禁止
	GameApp(const GameApp&) = delete;
	void operator=(const GameApp&) = delete;

public:
	//シングルトンインスタンスを得る
	static GameApp& Instance();

	//初期化
	bool Initialize();

	//ループ開始
	void Run();

	//終了時処理
	void Delete();

	//デストラクタ
	~GameApp();
};


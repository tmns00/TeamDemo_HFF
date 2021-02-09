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
	//�E�B���h�E�\���N���X
	WindowsApp* winApp = nullptr;
	//DirectX�̏������N���X
	DirectXSystem* dxSystem = nullptr;
	//�L�[���̓N���X
	Input* input = nullptr;
	//�T�E���h�N���X
	Sound* sound = nullptr;
	//�J�����N���X
	Camera* camera = nullptr;


	//�Q�[���I�u�W�F�N�g�Ǘ��N���X
	GameObjectManager* objManager = nullptr;
	
	//�|��
	Plane* plane = nullptr;
	//������
	Cube* cube = nullptr;
	
	//OBJ���f��
	OBJObject* objObj = nullptr;
	//FBX
	FbxObj2* fbxObj = nullptr;

	//Sphere�R���e�i
	std::vector<Sphere*> spheres;

	//�V�[���Ǘ�
	Scene sceneState = Title;
	//�^�C�g���|��
	Plane* title = nullptr;

	//����
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
	int cnt1[8] = { 0,1,2,3,//�O			
				 4,5,6,7,//�E
	};
	int cnt2[8]={ 
		4,5,6,7,//���		 
		4,5,6,7,//��
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
	//�R���X�g���N�^
	GameApp();

	//�R�s�[�E����֎~
	GameApp(const GameApp&) = delete;
	void operator=(const GameApp&) = delete;

public:
	//�V���O���g���C���X�^���X�𓾂�
	static GameApp& Instance();

	//������
	bool Initialize();

	//���[�v�J�n
	void Run();

	//�I��������
	void Delete();

	//�f�X�g���N�^
	~GameApp();
};


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
	//���[�V�����f�[�^���[�h�p�\����
	struct VMDMotionData
	{
		char boneName[15];        //�{�[����
		unsigned int frameNo;     //�t���[���ԍ�
		XMFLOAT3 location;        //�ʒu
		XMFLOAT4 quaternion;      //�N�H�[�^�j�I��
		unsigned char bezier[64]; //�x�W�F��ԃp�����[�^�[
	};

#pragma pack(1)
	//�\��f�[�^
	struct VMDMorph
	{
		char name[15]; //���O
		uint32_t frameNo; //�t���[���ԍ�
		float weight; //�E�F�C�g
	};
#pragma pack()

#pragma pack(1)
	//�J����
	struct VMDCamera
	{
		uint32_t frameNo;          //�t���[���ԍ�
		float distance;            //����
		XMFLOAT3 pos;              //���W
		XMFLOAT3 eulerAngle;       //�I�C���[�p
		uint8_t Interpolation[24]; //���
		uint32_t fov;              //���E�p
		uint8_t persFlag;          //�p�[�X�t���O
	};
#pragma pack()

	//���C�g�Ɩ��f�[�^
	struct VMDLight
	{
		uint32_t frameNo; //�t���[���ԍ�
		XMFLOAT3 rgb;     //���C�g�F
		XMFLOAT3 vec;     //�����x�N�g��(���s����)
	};

#pragma pack(1)
	//�Z���t�e�f�[�^
	struct VMDSelfShadow
	{
		uint32_t frameNo; //�t���[���ԍ�
		uint8_t mode;     //�e���[�h
		float distance;   //����
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
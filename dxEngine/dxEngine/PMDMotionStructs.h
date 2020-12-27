#pragma once

#include<unordered_map>
#include<DirectXMath.h>

//���[�V�����f�[�^�\����
struct Motion
{
	unsigned int frameNo; //�A�j���[�V�����J�n����̃t���[����
	DirectX::XMVECTOR quaternion;  //�N�H�[�^�j�I��
	DirectX::XMFLOAT3 offset;      //IK�̏������W����̃I�t�Z�b�g���
	DirectX::XMFLOAT2 p1, p2;      //�x�W�F�Ȑ��̒��ԃR���g���[���|�C���g
};

//IK�I��/�I�t�f�[�^
struct VMDIKEnable
{
	uint32_t frameNo;                                   //�L�[�t���[��������t���[���ԍ�
	std::unordered_map<std::string, bool>ikEnableTable; //���O�ƃI��/�I�t�t���O�̃}�b�v
};

//���[�h����`��N���X�ɕK�v�ȃf�[�^���܂Ƃ߂�����
struct MotionDatas
{
	std::unordered_map<std::string, std::vector<Motion>> motion;
	unsigned int duration;
	std::vector<VMDIKEnable> ikEnableData;
};
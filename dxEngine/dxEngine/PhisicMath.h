#pragma once
#include<math.h>
#include"Vector3.h"
#include"quaternion.h"
#include <DirectXTex.h>
//�͂�������x�����߂�v�Z
float& Acc(float& F, const float& m);
//���x�����]���x���v�Z
float& Omg(float& V, const float& R);
//���W����p�x�����߂�v�Z
float& Rad(Vector3& pos, Vector3& inter);
float& theta( DirectX::XMFLOAT3& VertPos);
float& Phi( DirectX::XMFLOAT3& VertPos);
//�����x���瑬�x�����߂�
float& Vec(float& vec, const float& a, const float& t);
//�x�_�𒆐S�ɉ�]�^������v�Z
float& roty(float& Pos, float& R,const float& omg);
float& rotx(float& Pos, float& R,const float& omg);
float& rotz(float& Pos, float& R,const float& omg);
//��C��R�̌v�Z
float& Airres(float& Vec, float& k);
//�΂˂̌v�Z
float& Spring(float& Pos, float& k);
//�����̌v�Z
float& Len(Vector3 Pos);
//�R�̂悤�ȋ����̌v�Z�i�΂ˎg�p�j
float& Rope(float& spring,float& Airres);

//�N�I�[�^�j�I��
//���W�ix,y,z�j�̃N�I�[�^�j�I���쐬
Quaternion pc(float& x, float& y, float& z);
Quaternion q(float& th,const float& x,const float& y,const float& z);
//��]�N�I�[�^�j�I���̋����N�I�[�^�j�I���쐬
Quaternion qc(Quaternion& q);
Quaternion Quat(Quaternion& p,Quaternion& q,Quaternion& qc);

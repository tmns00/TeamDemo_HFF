#pragma once
#include<math.h>
#include"Vector3.h"
//�͂�������x�����߂�v�Z
float& Acc(float& F, const float& m);
//���x�����]���x���v�Z
float& Omg(float& V, const float& R);
//���W����p�x�����߂�v�Z
float& Rad(Vector3& pos, Vector3& inter);
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

#pragma once
//#include"Vector3D.h"
#ifndef _QUATERNION_H_
#define _QUATERNION_H_


struct Quaternion
{
	float x; //������
	float y; //������
	float z; //�������@
	float w; //������
};

Quaternion QuaternionMultiplication(Quaternion left, Quaternion right);
Quaternion Rotational2Quaternion(double angle, double x, double y, double z);
Quaternion QuaternionConjugate(Quaternion quat);
Quaternion Coordinate2Quaternion(double x, double y, double z);
#endif
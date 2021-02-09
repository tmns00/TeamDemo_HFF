#include "quaternion.h"
#include<cmath>
Quaternion quaternion(float x, float y, float z, float w)
{
	Quaternion result = { x,y,z,w };
	return result;
}

//テスト
Quaternion Coordinate2Quaternion(double x, double y, double z)
{
	Quaternion q;
	q.w = 0.0;
	q.x = x;
	q.y = y;
	q.z = z;
	return q;
}
Quaternion QuaternionMultiplication(Quaternion left, Quaternion right)
{
	Quaternion quat;
	double d0, d1, d2, d3;

	d0 = left.w * right.w;
	d1 = -left.x * right.x;
	d2 = -left.y * right.y;
	d3 = -left.z * right.z;
	quat.w = d0 + d1 + d2 + d3;

	d0 = left.w * right.x;
	d1 = right.w * left.x;
	d2 = left.y * right.z;
	d3 = -left.z * right.y;
	quat.x = d0 + d1 + d2 + d3;

	d0 = left.w * right.y;
	d1 = right.w * left.y;
	d2 = left.z * right.x;
	d3 = -left.x * right.z;
	quat.y = d0 + d1 + d2 + d3;

	d0 = left.w * right.z;
	d1 = right.w * left.z;
	d2 = left.x * right.y;
	d3 = -left.y * right.x;
	quat.z = d0 + d1 + d2 + d3;

	return quat;
}

Quaternion Rotational2Quaternion(double angle, double x, double y, double z)
{
	Quaternion quat = { 0.0,0.0,0.0,0.0 };
	double norm;

	norm = x * x + y * y + z * z;

	if (norm == 0.0)
	{
		return quat;
	}

	//単位ベクトルにする（重要）
	norm = 1.0 / sqrt(norm);
	x *= norm;
	y *= norm;
	z *= norm;

	//単位ベクトルからクオータニオンを作ると、単位四元数が作れるはず
	double sin_a = sin(angle / 2.0f);
	quat.w = cos(angle / 2.0f);
	quat.x = x * sin_a;
	quat.y = y * sin_a;
	quat.z = z * sin_a;
	return quat;
}

Quaternion QuaternionConjugate(Quaternion quat)
{
	Quaternion q;
	q.w = quat.w;
	q.x = quat.x;
	q.y = quat.y;
	q.z = quat.z;
	return q;

}
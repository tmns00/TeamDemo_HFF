#include "PhisicMath.h"

float& Acc(float& F,const float& m)
{
	//�͂�������x�����߂�v�Z
	float Acc = F / m;
	return Acc;
}
///���x�����]�͂̌v�Z
float& Omg(float& V,const float& R)
{
	float omg = V / R;
	return omg;
}

float& Rad(Vector3& pos, Vector3& inter)
{
	float x =inter.x-pos.x;
	float y = inter.y-pos.y;
	float z = inter.z-pos.z;
	float COS = sqrt(x * x + y * y) / sqrt(x * x + y * y + z * z);
	return COS;

}

float& Vec(float& vec,const float& a, const float& t)
{
	vec += a * t;
	return vec;
}

//��]�^���̌v�Z
float& roty(float& Pos, float& R,const float& omg)
{
	float fulcrum = Pos - R;
	float y = fulcrum + R * sin(omg);
	return y;
}

float& rotx(float& Pos, float& R,const float& omg)
{
	float fulcrum = Pos - R;
	float x = fulcrum + R * sin(omg);
	return x;
}

float& rotz(float& Pos, float& R,const float& omg)
{
	float fulcrum = Pos - R;
	float z = fulcrum + R * sin(omg);
	return z;
}
//��C��R�̌v�Z
float& Airres(float& Vec, float& k)
{
	float F = Vec * k;
	return F;
}
//�΂˂̌v�Z
float& Spring(float& Pos, float& k)
{
	float F = Pos * k;
	return F;
}
//�����̌v�Z
float& Len(Vector3 Pos)
{
	float len = sqrt(Pos.x * Pos.x + Pos.y * Pos.y + Pos.z * Pos.z);
	return len;
}
float& Rope(float& spring,float& Airres)
{
	
	float F = spring - Airres;
	return F;
}


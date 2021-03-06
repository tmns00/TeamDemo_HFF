#include "PhisicMath.h"

float& Acc(float& F,const float& m)
{
	//力から加速度を求める計算
	float Acc = F / m;
	return Acc;
}
///速度から回転力の計算
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

float& theta(DirectX::XMFLOAT3& VertPos)
{
	float x = VertPos.x;
	float y = VertPos.y;
	float z = VertPos.z;
		
	float COS = sqrt(z * z) / sqrt(x * x + z * z);
	return COS;
}

float& Phi( DirectX::XMFLOAT3& VertPos)
{
	float x = VertPos.x;
	float y = VertPos.y;
	float z = VertPos.z;

	float COS = sqrt(x * x + z * z) / sqrt(x * x + y * y + z * z);
	return COS;
}

float& Vec(float& vec,const float& a, const float& t)
{
	vec += a * t;
	return vec;
}

//回転運動の計算
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
//空気抵抗の計算
float& Airres(float& Vec, float& k)
{
	float F = Vec * k;
	return F;
}
//ばねの計算
float& Spring(float& Pos, float& k)
{
	float F = Pos * k;
	return F;
}
//長さの計算
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

Quaternion pc(float& x, float& y, float& z)
{
	Quaternion p = Coordinate2Quaternion(y, x, z);
	return p;
}

Quaternion q(float& th,const float& x,const float& y,const float& z)
{
	double d2r = atan(1.0) / 45.0f;
	Quaternion q = Rotational2Quaternion(th * d2r, x, y, z);	
	return q;
}

Quaternion qc(Quaternion& q)
{
	Quaternion qc = QuaternionConjugate(q);
	return qc;
}

//クオータニオン
Quaternion Quat(Quaternion& p, Quaternion& q, Quaternion& qc)
{
	
	//回転処理
	p = QuaternionMultiplication(q, p);
	p = QuaternionMultiplication(p, qc);
	//テスト終わり
	return p;
	
}


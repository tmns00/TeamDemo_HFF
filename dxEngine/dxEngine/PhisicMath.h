#pragma once
#include<math.h>
#include"Vector3.h"
#include"quaternion.h"
#include <DirectXTex.h>
//力から加速度を求める計算
float& Acc(float& F, const float& m);
//速度から回転速度を計算
float& Omg(float& V, const float& R);
//座標から角度を求める計算
float& Rad(Vector3& pos, Vector3& inter);
float& theta( DirectX::XMFLOAT3& VertPos);
float& Phi( DirectX::XMFLOAT3& VertPos);
//加速度から速度を求める
float& Vec(float& vec, const float& a, const float& t);
//支点を中心に回転運動する計算
float& roty(float& Pos, float& R,const float& omg);
float& rotx(float& Pos, float& R,const float& omg);
float& rotz(float& Pos, float& R,const float& omg);
//空気抵抗の計算
float& Airres(float& Vec, float& k);
//ばねの計算
float& Spring(float& Pos, float& k);
//長さの計算
float& Len(Vector3 Pos);
//紐のような挙動の計算（ばね使用）
float& Rope(float& spring,float& Airres);

//クオータニオン
//座標（x,y,z）のクオータニオン作成
Quaternion pc(float& x, float& y, float& z);
Quaternion q(float& th,const float& x,const float& y,const float& z);
//回転クオータニオンの共役クオータニオン作成
Quaternion qc(Quaternion& q);
Quaternion Quat(Quaternion& p,Quaternion& q,Quaternion& qc);

#pragma once
#include<math.h>
#include"Vector3.h"
//力から加速度を求める計算
float& Acc(float& F, const float& m);
//速度から回転速度を計算
float& Omg(float& V, const float& R);
//座標から角度を求める計算
float& Rad(Vector3& pos, Vector3& inter);
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

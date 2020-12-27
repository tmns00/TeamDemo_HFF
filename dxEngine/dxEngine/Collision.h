#pragma once

#include"CollisionPrimitive.h"

class Collision
{
public:
	/// <summary>
	/// 球体と球体の当たり判定
	/// </summary>
	/// <param name="sphere1">球体1</param>
	/// <param name="sphere2">球体2</param>
	/// <param name="inter">交点</param>
	/// <returns>交差判定</returns>
	static bool CheckSphere2Sphere(
		const Col_Sphere& sphere1,
		const Col_Sphere& sphere2,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// 球体と平面の当たり判定
	/// </summary>
	/// <param name="sphere">球体</param>
	/// <param name="plane">平面</param>
	/// <param name="inter">交点</param>
	/// <returns>交差判定</returns>
	static bool CheckSphere2Plane(
		const Col_Sphere& sphere,
		const Col_Plane& plane,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// 点と三角形の最近接点を求める
	/// </summary>
	/// <param name="point">点</param>
	/// <param name="triangle">三角形</param>
	/// <param name="closest">最近接点</param>
	static void ClosestPtPoint2Triangle(
		const DirectX::XMVECTOR& point,
		const Col_Triangle& triangle,
		DirectX::XMVECTOR* closest
	);

	/// <summary>
	/// 球と法線付き三角形の当たりチェック
	/// </summary>
	/// <param name="sphere">球</param>
	/// <param name="triangle">三角形</param>
	/// <param name="inter">交点(三角形上の最近接点)</param>
	/// <returns>交差してるかどうか</returns>
	static bool CheckSphere2Triangle(
		const Col_Sphere& sphere,
		const Col_Triangle& triangle,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// レイと平面の当たり判定
	/// </summary>
	/// <param name="lay">レイ</param>
	/// <param name="plane">平面</param>
	/// <param name="distance">距離</param>
	/// <param name="inter">交点</param>
	/// <returns>交差判定</returns>
	static bool CheckLay2Plane(
		const Col_Lay& lay,
		const Col_Plane& plane,
		float* distance = nullptr,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// レイと法線付き三角形の当たり判定
	/// </summary>
	/// <param name="lay">レイ</param>
	/// <param name="triangle">三角形</param>
	/// <param name="distance">距離</param>
	/// <param name="inter">交点</param>
	/// <returns>交差判定</returns>
	static bool CheckLay2Triangle(
		const Col_Lay& lay,
		const Col_Triangle& triangle,
		float* distance = nullptr,
		DirectX::XMVECTOR* inter = nullptr
	);

	/// <summary>
	/// レイと球の当たり判定
	/// </summary>
	/// <param name="lay">レイ</param>
	/// <param name="sphere">球</param>
	/// <param name="distance">距離</param>
	/// <param name="inter">交点</param>
	/// <returns>交差判定</returns>
	static bool CheckLay2Sphere(
		const Col_Lay& lay,
		const Col_Sphere& sphere,
		float* distance = nullptr,
		DirectX::XMVECTOR* inter = nullptr
	);
};


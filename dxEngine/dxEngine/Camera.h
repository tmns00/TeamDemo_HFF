#pragma once

#include<DirectXMath.h>

class Camera
{
private:
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMMATRIX = DirectX::XMMATRIX;

private:
	Camera();

	//コピー・代入禁止
	Camera(const Camera&) = delete;
	void operator=(const Camera&) = delete;

public:
	~Camera();

private:
	/// <summary>
	/// ビュー行列の更新
	/// </summary>
	static void UpdateViewMatrix();

	/// <summary>
	/// カメラのアップデート
	/// </summary>
	static void Update();

public:
	/// <summary>
	/// 生成
	/// </summary>
	static void Create();

	/// <summary>
	/// 削除
	/// </summary>
	static void Terminate();

	/// <summary>
	/// 視点座標を取得
	/// </summary>
	/// <returns>視点座標</returns>
	static const XMFLOAT3& GetEye() { return eye; }

	/// <summary>
	/// 視点座標を設定
	/// </summary>
	/// <param name="eye">設定する座標</param>
	static void SetEye(
		const XMFLOAT3& eye
	);

	/// <summary>
	/// 注視点座標を取得
	/// </summary>
	/// <returns>注視点座標</returns>
	static const XMFLOAT3& GetTarget() { return target; }

	/// <summary>
	/// 注視点座標を設定
	/// </summary>
	/// <param name="target">設定する座標</param>
	static void SetTarget(
		const XMFLOAT3& target
	);

	/// <summary>
	/// ベクトルを利用した移動
	/// </summary>
	/// <param name="move">移動量</param>
	static void MoveVector(
		const XMFLOAT3& move
	);

	/// <summary>
	/// ベクトルを利用した視点の移動
	/// </summary>
	/// <param name="move">移動量</param>
	static void MoveEyeVector(
		const XMFLOAT3& move
	);

	/// <summary>
	/// マウスを使ったカメラの回転
	/// </summary>
	/// <param name="theta">縦方向回転角</param>
	/// <param name="delta">横方向回転角</param>
	static void RotationCamForMouse(
		const float& theta,
		const float& delta
	);

	/// <summary>
	/// ビュー行列を取得
	/// </summary>
	/// <returns>ビュー行列</returns>
	static const XMMATRIX& GetViewMatrix() { return viewMat; }

	/// <summary>
	/// プロジェクション行列を取得
	/// </summary>
	/// <returns>プロジェクション行列</returns>
	static const XMMATRIX& GetProjectionMatrix() { return projectionMat; }

private:
	//インスタンス
	static Camera* instance;

	//カメラ視点座標
	static XMFLOAT3 eye;
	//注視点座標
	static XMFLOAT3 target;
	//上方向座標
	static XMFLOAT3 up;

	//

	//ビュー行列
	static XMMATRIX viewMat;
	//プロジェクション行列
	static XMMATRIX projectionMat;
};


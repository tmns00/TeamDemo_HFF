#pragma once

#include<DirectXMath.h>

#include "Input.h"

class Camera
{
private:
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;

private:
	//インスタンス
	static Camera* instance;

	//カメラ視点座標
	XMFLOAT3 eye = { 0, 0, -10 };
	//注視点座標
	XMFLOAT3 target = { 0, 0, 0 };
	//上方向座標
	XMFLOAT3 up = { 0, 1, 0 };

	//ビュー行列
	XMMATRIX viewMat = DirectX::XMMatrixIdentity();
	//プロジェクション行列
	XMMATRIX projectionMat = DirectX::XMMatrixIdentity();
	//ビュー・プロジェクション合成行列
	XMMATRIX viewProjMat = DirectX::XMMatrixIdentity();
	//ビュー行列ダーティフラグ
	bool viewDirty = false;
	//プロジェクション行列ダーティフラグ
	bool projDirty = false;

	//全方向ビルボード
	XMMATRIX billMat = DirectX::XMMatrixIdentity();
	//Y軸ビルボード
	XMMATRIX yBillMat = DirectX::XMMatrixIdentity();

	//アスペクト比
	float aspectRatio = 1.0f;

	//インプットクラスのポインタ
	Input* input = nullptr;

	//カメラ注視点までの距離
	float distance = 20;

	//スケーリング
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	//回転行列
	XMMATRIX matRot = DirectX::XMMatrixIdentity();

private:
	Camera() = default;

	//コピー・代入禁止
	Camera(const Camera&) = delete;
	void operator=(const Camera&) = delete;

	void UpdateViewMatrix();
	void UpdateProjectionMatrix();

	void DebugCameraUpdate();

public:
	/// <summary>
	/// 生成
	/// </summary>
	static void Create();

	static Camera* GetInstance();

public:
	~Camera() = default;

	void Update();
	void Terminate();

	/// <summary>
	/// 視点座標を取得
	/// </summary>
	/// <returns>視点座標</returns>
	const XMFLOAT3& GetEye() {
		return eye;
	}

	/// <summary>
	/// 視点座標を設定
	/// </summary>
	/// <param name="eye">設定する座標</param>
	void SetEye(
		const XMFLOAT3& eye
	) {
		this->eye = eye;
		viewDirty = true;
	}

	/// <summary>
	/// 注視点座標を取得
	/// </summary>
	/// <returns>注視点座標</returns>
	const XMFLOAT3& GetTarget() {
		return target;
	}

	/// <summary>
	/// 注視点座標を設定
	/// </summary>
	/// <param name="target">設定する座標</param>
	void SetTarget(
		const XMFLOAT3& target
	) {
		this->target = target;
		viewDirty = true;
	}

	const XMFLOAT3& GetUp() {
		return up;
	}

	void SetUp(
		const XMFLOAT3& up
	) {
		this->up = up;
		viewDirty = true;
	}

	// ベクトルを利用した移動
	void MoveVector(
		const XMFLOAT3& move
	);
	void MoveVector(
		const XMVECTOR& move
	);

	// ベクトルを利用した視点の移動
	void MoveEyeVector(
		const XMFLOAT3& move
	);
	void MoveEyeVector(
		const XMVECTOR& move
	);

	/// <summary>
	/// ビュー行列を取得
	/// </summary>
	/// <returns>ビュー行列</returns>
	const XMMATRIX& GetViewMatrix() {
		return viewMat;
	}

	/// <summary>
	/// プロジェクション行列を取得
	/// </summary>
	/// <returns>プロジェクション行列</returns>
	const XMMATRIX& GetProjectionMatrix() {
		return projectionMat;
	}


	const XMMATRIX& GetViewProjectionMatrix() {
		return viewProjMat;
	}

	const XMMATRIX& GetBillboardMatrix() {
		return billMat;
	}

	const XMMATRIX& GetYBillboardMatrix() {
		return yBillMat;
	}
};


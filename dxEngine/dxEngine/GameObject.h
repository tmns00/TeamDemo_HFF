#pragma once

#include<d3dx12.h>
#include<DirectXMath.h>
#include<wrl.h>

#include"CollisionPrimitive.h"

class GameObject abstract
{
private:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMMATRIX = DirectX::XMMATRIX;

protected:
	//当たり判定(仮　一旦全部球)
	Col_Sphere collision = { {0.0f,0.0f,0.0f,1.0f},0.5f };

	//定数バッファ
	ComPtr<ID3D12Resource> constBuff;
	// 色
	XMFLOAT4 color = { 1,1,1,1 };
	// ローカルスケール
	XMFLOAT3 scale = { 1,1,1 };
	// X,Y,Z軸回りのローカル回転角
	XMFLOAT3 rotation = { 0,0,0 };
	// ローカル座標
	XMFLOAT3 position = { 0,0,0 };
	// ローカルワールド変換行列
	XMMATRIX matWorld;

	//エラーバイナリコード
	ComPtr<ID3DBlob>errorBlob = nullptr;

public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw(
		ID3D12GraphicsCommandList* cmdList
	) = 0;
	virtual void Terminate() = 0;

	virtual HRESULT CreateVertBuff() = 0;
	virtual HRESULT CreateIndexBuff() = 0;
	virtual HRESULT ShaderCompile() = 0;
	virtual HRESULT CreateRootSignature() = 0;
	virtual HRESULT CreateGraphicsPipeline() = 0;
	virtual HRESULT CreateDescriptorHeap() = 0;
	virtual HRESULT LoadTexture(
		const std::string& textureName
	) = 0;
	virtual void CreateModel() = 0;
	virtual void InitMatrix() = 0;
	virtual HRESULT CreateConstBuff() = 0;
	virtual HRESULT CreateConstView() { return S_OK; }

public:
	/// <summary>
	/// 座標の取得
	/// </summary>
	/// <returns></returns>
	virtual XMFLOAT3& GetPosition() { return position; }
	/// <summary>
	/// 座標の設定
	/// </summary>
	/// <param name="position"></param>
	virtual void SetPosition(
		const XMFLOAT3& position
	) { this->position = position; }
	/// <summary>
	/// 回転の取得
	/// </summary>
	/// <returns></returns>
	virtual XMFLOAT3& GetRotation() { return rotation; }
	/// <summary>
	/// 回転の設定
	/// </summary>
	/// <param name="position"></param>
	virtual void SetRotation(
		const XMFLOAT3& rotation
	) {
		this->rotation = rotation;
	}
	/// <summary>
	/// スケールの取得
	/// </summary>
	/// <returns></returns>
	virtual XMFLOAT3& GetScale() { return scale; }
	/// <summary>
	/// スケールの設定
	/// </summary>
	/// <param name="position"></param>
	virtual void SetScale(
		const XMFLOAT3& scale
	) {
		this->scale = scale;
	}
	/// <summary>
	/// 色の取得
	/// </summary>
	/// <returns></returns>
	virtual XMFLOAT4& GetColor() { return color; }
	/// <summary>
	/// 色の設定
	/// </summary>
	/// <param name="position"></param>
	virtual void SetColor(
		const XMFLOAT4& color
	) {
		this->color = color;
	}

	/// <summary>
	/// 当たり判定のとき呼び出される
	/// </summary>
	/// <param name="obj"></param>
	virtual void Collision(
		const GameObject& obj
	);

	/// <summary>
	/// 当たり判定データの取得
	/// </summary>
	/// <returns></returns>
	virtual Col_Sphere& GetCollisionData() { return collision; }

protected:
	void DebugShader(
		const HRESULT& result
	);

	void UpdateColPos() { 
		collision.center = XMLoadFloat3(&position); 
	}
};

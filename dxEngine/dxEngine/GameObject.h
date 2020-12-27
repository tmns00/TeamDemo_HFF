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
	//�����蔻��(���@��U�S����)
	Col_Sphere collision = { {0.0f,0.0f,0.0f,1.0f},0.5f };

	//�萔�o�b�t�@
	ComPtr<ID3D12Resource> constBuff;
	// �F
	XMFLOAT4 color = { 1,1,1,1 };
	// ���[�J���X�P�[��
	XMFLOAT3 scale = { 1,1,1 };
	// X,Y,Z�����̃��[�J����]�p
	XMFLOAT3 rotation = { 0,0,0 };
	// ���[�J�����W
	XMFLOAT3 position = { 0,0,0 };
	// ���[�J�����[���h�ϊ��s��
	XMMATRIX matWorld;

	//�G���[�o�C�i���R�[�h
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
	/// ���W�̎擾
	/// </summary>
	/// <returns></returns>
	virtual XMFLOAT3& GetPosition() { return position; }
	/// <summary>
	/// ���W�̐ݒ�
	/// </summary>
	/// <param name="position"></param>
	virtual void SetPosition(
		const XMFLOAT3& position
	) { this->position = position; }
	/// <summary>
	/// ��]�̎擾
	/// </summary>
	/// <returns></returns>
	virtual XMFLOAT3& GetRotation() { return rotation; }
	/// <summary>
	/// ��]�̐ݒ�
	/// </summary>
	/// <param name="position"></param>
	virtual void SetRotation(
		const XMFLOAT3& rotation
	) {
		this->rotation = rotation;
	}
	/// <summary>
	/// �X�P�[���̎擾
	/// </summary>
	/// <returns></returns>
	virtual XMFLOAT3& GetScale() { return scale; }
	/// <summary>
	/// �X�P�[���̐ݒ�
	/// </summary>
	/// <param name="position"></param>
	virtual void SetScale(
		const XMFLOAT3& scale
	) {
		this->scale = scale;
	}
	/// <summary>
	/// �F�̎擾
	/// </summary>
	/// <returns></returns>
	virtual XMFLOAT4& GetColor() { return color; }
	/// <summary>
	/// �F�̐ݒ�
	/// </summary>
	/// <param name="position"></param>
	virtual void SetColor(
		const XMFLOAT4& color
	) {
		this->color = color;
	}

	/// <summary>
	/// �����蔻��̂Ƃ��Ăяo�����
	/// </summary>
	/// <param name="obj"></param>
	virtual void Collision(
		const GameObject& obj
	);

	/// <summary>
	/// �����蔻��f�[�^�̎擾
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

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
	//�C���X�^���X
	static Camera* instance;

	//�J�������_���W
	XMFLOAT3 eye = { 0, 0, -10 };
	//�����_���W
	XMFLOAT3 target = { 0, 0, 0 };
	//��������W
	XMFLOAT3 up = { 0, 1, 0 };

	//�r���[�s��
	XMMATRIX viewMat = DirectX::XMMatrixIdentity();
	//�v���W�F�N�V�����s��
	XMMATRIX projectionMat = DirectX::XMMatrixIdentity();
	//�r���[�E�v���W�F�N�V���������s��
	XMMATRIX viewProjMat = DirectX::XMMatrixIdentity();
	//�r���[�s��_�[�e�B�t���O
	bool viewDirty = false;
	//�v���W�F�N�V�����s��_�[�e�B�t���O
	bool projDirty = false;

	//�S�����r���{�[�h
	XMMATRIX billMat = DirectX::XMMatrixIdentity();
	//Y���r���{�[�h
	XMMATRIX yBillMat = DirectX::XMMatrixIdentity();

	//�A�X�y�N�g��
	float aspectRatio = 1.0f;

	//�C���v�b�g�N���X�̃|�C���^
	Input* input = nullptr;

	//�J���������_�܂ł̋���
	float distance = 20;

	//�X�P�[�����O
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	//��]�s��
	XMMATRIX matRot = DirectX::XMMatrixIdentity();

private:
	Camera() = default;

	//�R�s�[�E����֎~
	Camera(const Camera&) = delete;
	void operator=(const Camera&) = delete;

	void UpdateViewMatrix();
	void UpdateProjectionMatrix();

	void DebugCameraUpdate();

public:
	/// <summary>
	/// ����
	/// </summary>
	static void Create();

	static Camera* GetInstance();

public:
	~Camera() = default;

	void Update();
	void Terminate();

	/// <summary>
	/// ���_���W���擾
	/// </summary>
	/// <returns>���_���W</returns>
	const XMFLOAT3& GetEye() {
		return eye;
	}

	/// <summary>
	/// ���_���W��ݒ�
	/// </summary>
	/// <param name="eye">�ݒ肷����W</param>
	void SetEye(
		const XMFLOAT3& eye
	) {
		this->eye = eye;
		viewDirty = true;
	}

	/// <summary>
	/// �����_���W���擾
	/// </summary>
	/// <returns>�����_���W</returns>
	const XMFLOAT3& GetTarget() {
		return target;
	}

	/// <summary>
	/// �����_���W��ݒ�
	/// </summary>
	/// <param name="target">�ݒ肷����W</param>
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

	// �x�N�g���𗘗p�����ړ�
	void MoveVector(
		const XMFLOAT3& move
	);
	void MoveVector(
		const XMVECTOR& move
	);

	// �x�N�g���𗘗p�������_�̈ړ�
	void MoveEyeVector(
		const XMFLOAT3& move
	);
	void MoveEyeVector(
		const XMVECTOR& move
	);

	/// <summary>
	/// �r���[�s����擾
	/// </summary>
	/// <returns>�r���[�s��</returns>
	const XMMATRIX& GetViewMatrix() {
		return viewMat;
	}

	/// <summary>
	/// �v���W�F�N�V�����s����擾
	/// </summary>
	/// <returns>�v���W�F�N�V�����s��</returns>
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


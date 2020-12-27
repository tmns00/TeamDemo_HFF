#pragma once

#include<DirectXMath.h>

class Camera
{
private:
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMMATRIX = DirectX::XMMATRIX;

private:
	Camera();

	//�R�s�[�E����֎~
	Camera(const Camera&) = delete;
	void operator=(const Camera&) = delete;

public:
	~Camera();

private:
	/// <summary>
	/// �r���[�s��̍X�V
	/// </summary>
	static void UpdateViewMatrix();

	/// <summary>
	/// �J�����̃A�b�v�f�[�g
	/// </summary>
	static void Update();

public:
	/// <summary>
	/// ����
	/// </summary>
	static void Create();

	/// <summary>
	/// �폜
	/// </summary>
	static void Terminate();

	/// <summary>
	/// ���_���W���擾
	/// </summary>
	/// <returns>���_���W</returns>
	static const XMFLOAT3& GetEye() { return eye; }

	/// <summary>
	/// ���_���W��ݒ�
	/// </summary>
	/// <param name="eye">�ݒ肷����W</param>
	static void SetEye(
		const XMFLOAT3& eye
	);

	/// <summary>
	/// �����_���W���擾
	/// </summary>
	/// <returns>�����_���W</returns>
	static const XMFLOAT3& GetTarget() { return target; }

	/// <summary>
	/// �����_���W��ݒ�
	/// </summary>
	/// <param name="target">�ݒ肷����W</param>
	static void SetTarget(
		const XMFLOAT3& target
	);

	/// <summary>
	/// �x�N�g���𗘗p�����ړ�
	/// </summary>
	/// <param name="move">�ړ���</param>
	static void MoveVector(
		const XMFLOAT3& move
	);

	/// <summary>
	/// �x�N�g���𗘗p�������_�̈ړ�
	/// </summary>
	/// <param name="move">�ړ���</param>
	static void MoveEyeVector(
		const XMFLOAT3& move
	);

	/// <summary>
	/// �}�E�X���g�����J�����̉�]
	/// </summary>
	/// <param name="theta">�c������]�p</param>
	/// <param name="delta">��������]�p</param>
	static void RotationCamForMouse(
		const float& theta,
		const float& delta
	);

	/// <summary>
	/// �r���[�s����擾
	/// </summary>
	/// <returns>�r���[�s��</returns>
	static const XMMATRIX& GetViewMatrix() { return viewMat; }

	/// <summary>
	/// �v���W�F�N�V�����s����擾
	/// </summary>
	/// <returns>�v���W�F�N�V�����s��</returns>
	static const XMMATRIX& GetProjectionMatrix() { return projectionMat; }

private:
	//�C���X�^���X
	static Camera* instance;

	//�J�������_���W
	static XMFLOAT3 eye;
	//�����_���W
	static XMFLOAT3 target;
	//��������W
	static XMFLOAT3 up;

	//

	//�r���[�s��
	static XMMATRIX viewMat;
	//�v���W�F�N�V�����s��
	static XMMATRIX projectionMat;
};


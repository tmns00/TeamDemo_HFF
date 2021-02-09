#include "Camera.h"
#include"WindowsApp.h"

using namespace DirectX;

Camera* Camera::instance = nullptr;

const float CameraMoveScale = 0.1f;

void Camera::UpdateViewMatrix() {
	//���_���W
	XMVECTOR eyePos = XMLoadFloat3(&eye);
	//�����_���W
	XMVECTOR targetPos = XMLoadFloat3(&target);
	//���̏�����x�N�g��
	XMVECTOR upVec = XMLoadFloat3(&up);

	//�J����Z��(���������E���[���h���W�n)
	XMVECTOR cameraAxisZ = XMVectorSubtract(targetPos, eyePos);
	//0�x�N�g�������O
	assert(!XMVector3Equal(cameraAxisZ, XMVectorZero()));
	assert(!XMVector3IsInfinite(cameraAxisZ));
	assert(!XMVector3Equal(upVec, XMVectorZero()));
	assert(!XMVector3IsInfinite(upVec));
	//���K��
	cameraAxisZ = XMVector3Normalize(cameraAxisZ);

	//�J������X��(�E�����E���[���h���W�n)
	XMVECTOR cameraAxisX;
	cameraAxisX = XMVector3Cross(upVec, cameraAxisZ);
	//���K��
	cameraAxisX = XMVector3Normalize(cameraAxisX);

	//�J������Y��(������E���[���h���W�n)
	XMVECTOR cameraAxisY;
	cameraAxisY = XMVector3Cross(cameraAxisZ, cameraAxisX);

	//�J������]�s��
	XMMATRIX matCamRot;
	//�J�������W�n�����[���h���W�n
	matCamRot.r[0] = cameraAxisX;
	matCamRot.r[1] = cameraAxisY;
	matCamRot.r[2] = cameraAxisZ;
	matCamRot.r[3] = XMVectorSet(0, 0, 0, 1);
	//�]�u�ɂ��t�s��(�t��])���v�Z
	viewMat = XMMatrixTranspose(matCamRot);

	//���_���W��-1�����������W
	XMVECTOR reverseEyePos = XMVectorNegate(eyePos);
	//�J�����ʒu���烏�[���h���_�ւ̃x�N�g��(�J�������W�n)
	XMVECTOR tX = XMVector3Dot(cameraAxisX, reverseEyePos);
	XMVECTOR tY = XMVector3Dot(cameraAxisY, reverseEyePos);
	XMVECTOR tZ = XMVector3Dot(cameraAxisZ, reverseEyePos);
	//�܂Ƃ߂�
	XMVECTOR translation = XMVectorSet(
		tX.m128_f32[0],
		tY.m128_f32[1],
		tZ.m128_f32[2],
		1.0f
	);
	//�r���[�s��ɕ��s�ړ�������ݒ�
	viewMat.r[3] = translation;

#pragma region �S�����r���{�[�h�s��̌v�Z
	billMat.r[0] = cameraAxisX;
	billMat.r[1] = cameraAxisY;
	billMat.r[2] = cameraAxisZ;
	billMat.r[3] = XMVectorSet(0, 0, 0, 1);
#pragma endregion

#pragma region Y���r���{�[�h�̌v�Z
	XMVECTOR yBillCamAxisX, yBillCamAxisY, yBillCamAxisZ;

	//X���͋���
	yBillCamAxisX = cameraAxisX;
	//Y���̓��[���h���W�n��Y��
	yBillCamAxisY = XMVector3Normalize(upVec);
	//Z����X����Y���̊O��
	yBillCamAxisZ = XMVector3Cross(yBillCamAxisX, yBillCamAxisY);

	yBillMat.r[0] = yBillCamAxisX;
	yBillMat.r[1] = yBillCamAxisY;
	yBillMat.r[2] = yBillCamAxisZ;
	yBillMat.r[3] = XMVectorSet(0, 0, 0, 1);
#pragma endregion
}

void Camera::UpdateProjectionMatrix() {
	//�v���W�F�N�V�����s��
	projectionMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2, //��p90��
		aspectRatio, //�A�X�y�N�g��
		1.0f, //near
		5000.0f //far
	);
}

void Camera::DebugCameraUpdate() {
	bool dirty = false;
	float angleX = 0;
	float angleY = 0;

	//�}�E�X�̓��͂��擾
	Input::MouseMove mouseMove = input->GetMouseMove();

	if (input->TriggerMouseLeft()) {
		float dx = mouseMove.lenX * scaleX;
		float dy = mouseMove.lenY * scaleY;

		angleX = -dx * XM_PI;
		angleY = -dy * XM_PI;
		dirty = true;
	}

	if (input->TriggerMouseWheel()) {
		float dx = mouseMove.lenX * CameraMoveScale;
		float dy = mouseMove.lenY * CameraMoveScale;

		XMVECTOR move = { -dx,+dy,0,0 };
		move = XMVector3Transform(move, matRot);

		MoveVector(move);
		dirty = true;
	}

	if (mouseMove.lenZ != 0) {
		distance -= mouseMove.lenZ / 100.0f;
		distance = max(distance, 1.0f);
		dirty = true;
	}

	if (dirty || viewDirty) {
		XMMATRIX matRotNew = XMMatrixIdentity();
		matRotNew *= XMMatrixRotationX(-angleY);
		matRotNew *= XMMatrixRotationY(-angleX);

		matRot = matRotNew * matRot;

		XMVECTOR vTargetEye = { 0.0f,0.0f,-distance,1.0f };
		XMVECTOR vUp = { 0.0f,1.0f,0.0f,0.0f };

		vTargetEye = XMVector3Transform(vTargetEye, matRot);
		vUp = XMVector3Transform(vUp, matRot);

		const XMFLOAT3& target = GetTarget();
		SetEye({
			target.x + vTargetEye.m128_f32[0],
			target.y + vTargetEye.m128_f32[1],
			target.z + vTargetEye.m128_f32[2]
			});
		SetUp({
			vUp.m128_f32[0],
			vUp.m128_f32[1],
			vUp.m128_f32[2]
			});
	}
}

void Camera::Create() {
	instance = new Camera;
	if (!instance) {
		assert(0);
	}

	//�r���[�s��
	instance->UpdateViewMatrix();

	instance->aspectRatio
		= (float)WindowsApp::window_width / WindowsApp::window_height;
	instance->UpdateProjectionMatrix();

	instance->viewProjMat = instance->viewMat * instance->projectionMat;

	instance->input = Input::GetInstance();
	//��ʃT�C�Y�ɍ��킹���X�P�[��
	instance->scaleX = 1.0f / (float)WindowsApp::window_width;
	instance->scaleY = 1.0f / (float)WindowsApp::window_height;
}

Camera* Camera::GetInstance() {
	if (!instance) {
		return nullptr;
	}

	return instance;
}

void Camera::MoveVector(
	const XMFLOAT3& move
) {
	XMFLOAT3 eye_moved = GetEye();
	XMFLOAT3 target_moved = GetTarget();

	eye_moved.x += move.x;
	eye_moved.y += move.y;
	eye_moved.z += move.z;

	target_moved.x += move.x;
	target_moved.y += move.y;
	target_moved.z += move.z;

	SetEye(eye_moved);
	SetTarget(target_moved);
}

void Camera::MoveVector(
	const XMVECTOR& move
) {
	XMFLOAT3 eye_moved = GetEye();
	XMFLOAT3 target_moved = GetTarget();

	eye_moved.x += move.m128_f32[0];
	eye_moved.y += move.m128_f32[1];
	eye_moved.z += move.m128_f32[2];

	target_moved.x += move.m128_f32[0];
	target_moved.y += move.m128_f32[1];
	target_moved.z += move.m128_f32[2];

	SetEye(eye_moved);
	SetTarget(target_moved);
}

void Camera::MoveEyeVector(
	const XMFLOAT3& move
) {
	XMFLOAT3 eye_moved = GetEye();

	eye_moved.x += move.x;
	eye_moved.y += move.y;
	eye_moved.z += move.z;

	SetEye(eye_moved);
}

void Camera::MoveEyeVector(
	const XMVECTOR& move
) {
	XMFLOAT3 eye_moved = GetEye();

	eye_moved.x += move.m128_f32[0];
	eye_moved.y += move.m128_f32[1];
	eye_moved.z += move.m128_f32[2];

	SetEye(eye_moved);
}

void Camera::Update() {
	DebugCameraUpdate();

	if (viewDirty || projDirty) {
		if (viewDirty) {
			//�X�V
			UpdateViewMatrix();
			viewDirty = false;
		}

		if (projDirty) {
			//�X�V
			UpdateProjectionMatrix();
			projDirty = false;
		}
		//����
		viewProjMat = viewMat * projectionMat;
	}
}

void Camera::Terminate() {
	delete instance;
	instance = nullptr;
}

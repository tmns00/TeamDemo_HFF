#include "Camera.h"
#include"WindowsApp.h"

using namespace DirectX;

Camera* Camera::instance = nullptr;
XMFLOAT3 Camera::eye = { 0, 10, -20 };
XMFLOAT3 Camera::target = { 0, 0, 0 };
XMFLOAT3 Camera::up = { 0, 1, 0 };
XMMATRIX Camera::viewMat = {};
XMMATRIX Camera::projectionMat = {};

Camera::Camera(){
}

void Camera::UpdateViewMatrix(){
	viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));
}

void Camera::Update()
{
}

Camera::~Camera(){
}

void Camera::Create(){
	instance = new Camera();
	if (!instance) {
		assert(0);
	}

	//ビュー行列
	UpdateViewMatrix();
	//プロジェクション行列
	projectionMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2, //画角90°
		(float)WindowsApp::window_width / WindowsApp::window_height, //アスペクト比
		1.0f, //near
		5000.0f //far
	);
}

void Camera::Terminate(){
	delete instance;
	instance = nullptr;
}

void Camera::SetEye(const XMFLOAT3& eye){
	Camera::eye = eye;

	UpdateViewMatrix();
}

void Camera::SetTarget(const XMFLOAT3& target){
	Camera::target = target;

	UpdateViewMatrix();
}

void Camera::MoveVector(const XMFLOAT3& move){
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

void Camera::MoveEyeVector(const XMFLOAT3& move){
	XMFLOAT3 eye_moved = GetEye();

	eye_moved.x += move.x;
	eye_moved.y += move.y;
	eye_moved.z += move.z;

	SetEye(eye_moved);
}

void Camera::RotationCamForMouse(
	const float& theta,
	const float& delta
){
	XMFLOAT3 eye_moved = GetEye();
	XMFLOAT3 target_used = GetTarget();

	float dist = (float)sqrt(
		(target_used.x - eye_moved.x) * (target_used.x - eye_moved.x) +
		(target_used.y - eye_moved.y) * (target_used.y - eye_moved.y) +
		(target_used.z - eye_moved.z) * (target_used.z - eye_moved.z)
	);

	eye_moved.x = target_used.x + dist * cos(delta) * cos(theta);
	eye_moved.y = target_used.y + dist * sin(delta);
	eye_moved.z = target_used.z + dist * cos(delta) * sin(theta);

	SetEye(eye_moved);
}

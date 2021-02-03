#include "Input.h"

#include<assert.h>

Input* Input::instance = nullptr;

//������
void Input::Initialize(HWND hwnd) {
	HRESULT result;

	//DirectInput�I�u�W�F�N�g����
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	result = DirectInput8Create(
		hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&dInput,
		nullptr
	);
	assert(SUCCEEDED(result));

	//�L�[�{�[�h�f�o�C�X�̐���
	result = dInput->CreateDevice(
		GUID_SysKeyboard,
		&devkeyboard,
		NULL
	);
	assert(SUCCEEDED(result));

	//�}�E�X�f�o�C�X����
	result = dInput->CreateDevice(
		GUID_SysMouse,
		&devMouse,
		NULL
	);
	assert(SUCCEEDED(result));

	//���̓f�[�^�`���̃Z�b�g�E�L�[�{�[�h
	result = devkeyboard->SetDataFormat(
		&c_dfDIKeyboard
	); //�W���`��
	assert(SUCCEEDED(result));

	//�r�����䃌�x���̃Z�b�g�E�L�[�{�[�h
	result = devkeyboard->SetCooperativeLevel(
		hwnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));

	//���̓f�[�^�`���̃Z�b�g�E�}�E�X
	result = devMouse->SetDataFormat(
		&c_dfDIMouse2
	); //�W���`��
	assert(SUCCEEDED(result));

	//�r�����䃌�x���̃Z�b�g�E�}�E�X
	result = devMouse->SetCooperativeLevel(
		hwnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));
}

//���t���[������
void Input::Update() {
	HRESULT result;

	//�L�[�{�[�h
	result = devkeyboard->Acquire();
	//1�t���[���O�̃L�[���͏���ۑ�
	memcpy(preKey, key, sizeof(key));
	//�S�L�[�̓��͏�Ԃ��擾����
	result = devkeyboard->GetDeviceState(sizeof(key), key);


	//�}�E�X
	result = devMouse->Acquire();
	//�O�t���[���̓��͂�ۑ�
	preMouseStat = mouseStat;
	//�}�E�X�̓��͏�Ԃ��擾
	result = devMouse->GetDeviceState(sizeof(mouseStat), &mouseStat);
}

bool Input::IsKey(int keyNum) {
	assert(0x00 <= keyNum && keyNum <= 0xff);

	if (key[keyNum]) {
		return true;
	}

	return false;
}

bool Input::IsTrigger(int keyNum) {
	assert(0x00 <= keyNum && keyNum <= 0xff);

	if (!preKey[keyNum] && key[keyNum]) {
		return true;
	}

	return false;
}

bool Input::IsMouseLeft(){
	if (mouseStat.rgbButtons[0])
		return true;

	return false;
}

bool Input::IsMouseWheel(){
	if (mouseStat.rgbButtons[2])
		return true;

	return false;
}

bool Input::IsMouseRight(){
	if (mouseStat.rgbButtons[1])
		return true;

	return false;
}

bool Input::TriggerMouseLeft(){
	if (mouseStat.rgbButtons[0] 
		&& preMouseStat.rgbButtons[0])
		return true;

	return false;
}

bool Input::TriggerMouseWheel(){
	if (mouseStat.rgbButtons[2]
		&& preMouseStat.rgbButtons[2])
		return true;

	return false;
}

bool Input::TriggerMouseRight(){
	if (mouseStat.rgbButtons[1]
		&& preMouseStat.rgbButtons[1])
		return true;

	return false;
}

Input::MouseMove Input::GetMouseMove(){
	MouseMove mMove;
	mMove.lenX = mouseStat.lX;
	mMove.lenY = mouseStat.lY;
	mMove.lenZ = mouseStat.lZ;
	return mMove;
}

void Input::Terminate() {
	delete instance;
	instance = nullptr;
}

void Input::CreateInstance(
	HWND hwnd
) {
	instance = new Input;

	instance->Initialize(hwnd);
	if (!instance) {
		assert(0);
	}
}

Input* Input::GetInstance() {
	if (!instance) {
		return nullptr;
	}
	return instance;
}

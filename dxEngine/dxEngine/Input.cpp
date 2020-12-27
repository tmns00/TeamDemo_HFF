#include "Input.h"

#include<assert.h>

Input* Input::instance = nullptr;

Input::Input(){
}

Input::~Input(){
}

//������
void Input::Initialize(HWND hwnd){
	HRESULT result;
	//DirectInput�I�u�W�F�N�g�̐���
	IDirectInput8* dinput = nullptr;
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	//�L�[�{�[�h�f�o�C�X�̐���
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//���̓f�[�^�`���̃Z�b�g
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard); //�W���`��

	//�r�����䃌�x���̃Z�b�g
	result = devkeyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
}

//���t���[������
void Input::Update(){
	HRESULT result;
	result = devkeyboard->Acquire();

	//1�t���[���O�̃L�[���͏���ۑ�
	for (int i = 0; i < _countof(key); i++) {
		preKey[i] = key[i];
	}
	
	//�S�L�[�̓��͏�Ԃ��擾����
	result = devkeyboard->GetDeviceState(sizeof(key), key);
}

bool Input::isKey(int keyNum){
	if (keyNum < 0x00) return false;
	if (keyNum > 0xff) return false;

	if (key[keyNum]) {
		return true;
	}

	return false;
}

bool Input::isTrigger(int keyNum){
	if (keyNum < 0x00) return false;
	if (keyNum > 0xff) return false;

	if (!preKey[keyNum] && key[keyNum]) {
		return true;
	}

	return false;
}

void Input::Terminate(){
	delete instance;
	instance = nullptr;
}

void Input::CreateInstance(
	HWND hwnd
){
	instance = new Input;

	instance->Initialize(hwnd);
	if (!instance) {
		assert(0);
	}
}

Input* Input::GetInstance(){
	if (!instance) {
		return nullptr;
	}
	return instance;
}

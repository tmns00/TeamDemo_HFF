#include "Input.h"

#include<assert.h>

Input* Input::instance = nullptr;

//初期化
void Input::Initialize(HWND hwnd) {
	HRESULT result;

	//DirectInputオブジェクト生成
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	result = DirectInput8Create(
		hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&dInput,
		nullptr
	);
	assert(SUCCEEDED(result));

	//キーボードデバイスの生成
	result = dInput->CreateDevice(
		GUID_SysKeyboard,
		&devkeyboard,
		NULL
	);
	assert(SUCCEEDED(result));

	//マウスデバイス生成
	result = dInput->CreateDevice(
		GUID_SysMouse,
		&devMouse,
		NULL
	);
	assert(SUCCEEDED(result));

	//入力データ形式のセット・キーボード
	result = devkeyboard->SetDataFormat(
		&c_dfDIKeyboard
	); //標準形式
	assert(SUCCEEDED(result));

	//排他制御レベルのセット・キーボード
	result = devkeyboard->SetCooperativeLevel(
		hwnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));

	//入力データ形式のセット・マウス
	result = devMouse->SetDataFormat(
		&c_dfDIMouse2
	); //標準形式
	assert(SUCCEEDED(result));

	//排他制御レベルのセット・マウス
	result = devMouse->SetCooperativeLevel(
		hwnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));
}

//毎フレーム処理
void Input::Update() {
	HRESULT result;

	//キーボード
	result = devkeyboard->Acquire();
	//1フレーム前のキー入力情報を保存
	memcpy(preKey, key, sizeof(key));
	//全キーの入力状態を取得する
	result = devkeyboard->GetDeviceState(sizeof(key), key);


	//マウス
	result = devMouse->Acquire();
	//前フレームの入力を保存
	preMouseStat = mouseStat;
	//マウスの入力状態を取得
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

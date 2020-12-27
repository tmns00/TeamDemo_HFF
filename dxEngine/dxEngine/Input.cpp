#include "Input.h"

#include<assert.h>

Input* Input::instance = nullptr;

Input::Input(){
}

Input::~Input(){
}

//初期化
void Input::Initialize(HWND hwnd){
	HRESULT result;
	//DirectInputオブジェクトの生成
	IDirectInput8* dinput = nullptr;
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	//キーボードデバイスの生成
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//入力データ形式のセット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard); //標準形式

	//排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
}

//毎フレーム処理
void Input::Update(){
	HRESULT result;
	result = devkeyboard->Acquire();

	//1フレーム前のキー入力情報を保存
	for (int i = 0; i < _countof(key); i++) {
		preKey[i] = key[i];
	}
	
	//全キーの入力状態を取得する
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

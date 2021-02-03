#pragma once

#include <Windows.h>
#include <wrl.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input
{
private:
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	struct MouseMove {
		LONG lenX;
		LONG lenY;
		LONG lenZ;
	};

private:
	//インスタンス
	static Input* instance;
	//DirectInputオブジェクト
	ComPtr<IDirectInput8> dInput;
	//キーバードデバイス
	ComPtr<IDirectInputDevice8> devkeyboard;
	//キー入力情報
	BYTE key[256] = {};
	//1フレーム前のキー入力情報
	BYTE preKey[256] = {};

	//マウス入力
	ComPtr<IDirectInputDevice8> devMouse;
	DIMOUSESTATE2 mouseStat = {};
	DIMOUSESTATE2 preMouseStat = {};

private:
	Input() = default;
	~Input() = default;

	//コピー・代入禁止
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

public:
	/// <summary>
	/// インスタンス生成
	/// </summary>
	static void CreateInstance(
		HWND hwnd
	);

	/// <summary>
	/// インスタンス取得
	/// </summary>
	/// <returns>インスタンス</returns>
	static Input* GetInstance();

private:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="hwnd">ウィンドウハンドル</param>
	void Initialize(HWND hwnd);

public:
	//毎フレーム処理
	void Update();
	//キーが押されているか
	bool IsKey(int keyNum);
	//トリガー
	bool IsTrigger(int keyNum);
	//マウス左
	bool IsMouseLeft();
	//ホイールクリック
	bool IsMouseWheel();
	//マウス右
	bool IsMouseRight();
	//マウス左・トリガー
	bool TriggerMouseLeft();
	//ホイールクリック・トリガー
	bool TriggerMouseWheel();
	//マウス右・トリガー
	bool TriggerMouseRight();
	//マウス移動量
	MouseMove GetMouseMove();

	/// <summary>
	/// 削除
	/// </summary>
	void Terminate();

};


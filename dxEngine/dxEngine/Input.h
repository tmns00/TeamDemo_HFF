#pragma once

#include <Windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input
{
private:
	Input();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="hwnd">ウィンドウハンドル</param>
	void Initialize(HWND hwnd);

public:
	~Input();

	//毎フレーム処理
	void Update();
	//キーが押されているか
	bool isKey(int keyNum);
	//トリガー
	bool isTrigger(int keyNum);

	/// <summary>
	/// 削除
	/// </summary>
	void Terminate();

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
	//インスタンス
	static Input* instance;
	//キーバードデバイス
	IDirectInputDevice8* devkeyboard = nullptr;
	//キー入力情報
	BYTE key[256] = {};
	//1フレーム前のキー入力情報
	BYTE preKey[256] = {};
};


#pragma once

#include<Windows.h>

class WindowsApp
{
private:
	//コンストラクタ
	WindowsApp();

	//コピー・代入禁止
	WindowsApp(const WindowsApp&) = delete;
	void operator=(const WindowsApp&) = delete;

public:
	/// <summary>
	/// ウィンドウプロシージャ
	/// </summary>
	/// <param name="hwnd">ハンドル</param>
	/// <param name="msg">メッセージ</param>
	/// <param name="wparam"></param>
	/// <param name="lparam"></param>
	/// <returns></returns>
	static LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	/// <summary>
	/// インスタンス生成
	/// </summary>
	static void Create();

	/// <summary>
	/// インスタンス取得
	/// </summary>
	/// <returns></returns>
	static WindowsApp* GetInstance();

public:
	//デストラクタ
	~WindowsApp();

	/// <summary>
	/// ウィンドウの生成
	/// </summary>
	void CreateGameWindow();

	/// <summary>
	/// ウィンドウの削除
	/// </summary>
	void DeleteGameWindow();

	/// <summary>
	/// ハンドルの取得
	/// </summary>
	/// <returns></returns>
	HWND GetHWND() { return hwnd; }

	/// <summary>
	/// ウインドウがアクティブかどうか
	/// </summary>
	bool GetWindowActive();

private:
	//インスタンス
	static WindowsApp* instance;

	//ウィンドウ生成に必要な変数
	WNDCLASSEX wnd = {};
	HWND hwnd = nullptr;

public:
	static const int window_width = 1280; //画面幅
	static const int window_height = 720; //画面高
};


#include "WindowsApp.h"

WindowsApp* WindowsApp::instance = nullptr;

WindowsApp::WindowsApp(){
}

WindowsApp::~WindowsApp(){
}

LRESULT WindowsApp::WindowProcedure(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
){
	//ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OSに対して「このアプリは終わり」と伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WindowsApp::Create(){
	if (!instance) {
		instance = new WindowsApp;
	}
}

WindowsApp* WindowsApp::GetInstance(){
	return instance;
}

void WindowsApp::CreateGameWindow()
{
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.lpfnWndProc = (WNDPROC)WindowProcedure; //コールバック関数の指定
	wnd.lpszClassName = TEXT("DX12Sample"); //アプリケーションクラス名
	wnd.hInstance = GetModuleHandle(nullptr); //ハンドルの取得

	RegisterClassEx(&wnd); //アプリケーションクラス（ウィンドウクラスの指定をOSに伝える）

	RECT wrc = { 0,0,window_width,window_height }; //ウィンドウサイズを決める
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); //関数を使ってウィンドウのサイズを補正する

	//ウィンドウオブジェクトの生成
	hwnd = CreateWindow(
		wnd.lpszClassName,      //クラス名指定
		TEXT("DX12テスト"),   //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,  //タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,        //表示x座標はOSにお任せ
		CW_USEDEFAULT,        //表示y座標はOSにお任せ
		wrc.right - wrc.left, //ウィンドウ幅
		wrc.bottom - wrc.top, //ウィンドウ高
		nullptr,              //親ウィンドウハンドル
		nullptr,              //メニューハンドル
		wnd.hInstance,          //呼び出しアプリケーション
		nullptr);             //追加パラメーター

	//ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);
}

void WindowsApp::DeleteGameWindow()
{
	//もうクラスは使わないので登録解除する
	UnregisterClass(wnd.lpszClassName, wnd.hInstance);
}

bool WindowsApp::GetWindowActive(){
	if (hwnd == GetActiveWindow())
		return true;
	else
		return false;
}

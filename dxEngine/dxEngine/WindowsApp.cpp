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
	//�E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OS�ɑ΂��āu���̃A�v���͏I���v�Ɠ`����
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
	wnd.lpfnWndProc = (WNDPROC)WindowProcedure; //�R�[���o�b�N�֐��̎w��
	wnd.lpszClassName = TEXT("DX12Sample"); //�A�v���P�[�V�����N���X��
	wnd.hInstance = GetModuleHandle(nullptr); //�n���h���̎擾

	RegisterClassEx(&wnd); //�A�v���P�[�V�����N���X�i�E�B���h�E�N���X�̎w���OS�ɓ`����j

	RECT wrc = { 0,0,window_width,window_height }; //�E�B���h�E�T�C�Y�����߂�
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); //�֐����g���ăE�B���h�E�̃T�C�Y��␳����

	//�E�B���h�E�I�u�W�F�N�g�̐���
	hwnd = CreateWindow(
		wnd.lpszClassName,      //�N���X���w��
		TEXT("DX12�e�X�g"),   //�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,  //�^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,        //�\��x���W��OS�ɂ��C��
		CW_USEDEFAULT,        //�\��y���W��OS�ɂ��C��
		wrc.right - wrc.left, //�E�B���h�E��
		wrc.bottom - wrc.top, //�E�B���h�E��
		nullptr,              //�e�E�B���h�E�n���h��
		nullptr,              //���j���[�n���h��
		wnd.hInstance,          //�Ăяo���A�v���P�[�V����
		nullptr);             //�ǉ��p�����[�^�[

	//�E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);
}

void WindowsApp::DeleteGameWindow()
{
	//�����N���X�͎g��Ȃ��̂œo�^��������
	UnregisterClass(wnd.lpszClassName, wnd.hInstance);
}

bool WindowsApp::GetWindowActive(){
	if (hwnd == GetActiveWindow())
		return true;
	else
		return false;
}

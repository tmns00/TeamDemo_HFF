#pragma once

#include<Windows.h>

class WindowsApp
{
private:
	//�R���X�g���N�^
	WindowsApp();

	//�R�s�[�E����֎~
	WindowsApp(const WindowsApp&) = delete;
	void operator=(const WindowsApp&) = delete;

public:
	/// <summary>
	/// �E�B���h�E�v���V�[�W��
	/// </summary>
	/// <param name="hwnd">�n���h��</param>
	/// <param name="msg">���b�Z�[�W</param>
	/// <param name="wparam"></param>
	/// <param name="lparam"></param>
	/// <returns></returns>
	static LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	/// <summary>
	/// �C���X�^���X����
	/// </summary>
	static void Create();

	/// <summary>
	/// �C���X�^���X�擾
	/// </summary>
	/// <returns></returns>
	static WindowsApp* GetInstance();

public:
	//�f�X�g���N�^
	~WindowsApp();

	/// <summary>
	/// �E�B���h�E�̐���
	/// </summary>
	void CreateGameWindow();

	/// <summary>
	/// �E�B���h�E�̍폜
	/// </summary>
	void DeleteGameWindow();

	/// <summary>
	/// �n���h���̎擾
	/// </summary>
	/// <returns></returns>
	HWND GetHWND() { return hwnd; }

	/// <summary>
	/// �E�C���h�E���A�N�e�B�u���ǂ���
	/// </summary>
	bool GetWindowActive();

private:
	//�C���X�^���X
	static WindowsApp* instance;

	//�E�B���h�E�����ɕK�v�ȕϐ�
	WNDCLASSEX wnd = {};
	HWND hwnd = nullptr;

public:
	static const int window_width = 1280; //��ʕ�
	static const int window_height = 720; //��ʍ�
};


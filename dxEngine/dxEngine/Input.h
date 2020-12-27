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
	/// ������
	/// </summary>
	/// <param name="hwnd">�E�B���h�E�n���h��</param>
	void Initialize(HWND hwnd);

public:
	~Input();

	//���t���[������
	void Update();
	//�L�[��������Ă��邩
	bool isKey(int keyNum);
	//�g���K�[
	bool isTrigger(int keyNum);

	/// <summary>
	/// �폜
	/// </summary>
	void Terminate();

public:
	/// <summary>
	/// �C���X�^���X����
	/// </summary>
	static void CreateInstance(
		HWND hwnd
	);

	/// <summary>
	/// �C���X�^���X�擾
	/// </summary>
	/// <returns>�C���X�^���X</returns>
	static Input* GetInstance();

private:
	//�C���X�^���X
	static Input* instance;
	//�L�[�o�[�h�f�o�C�X
	IDirectInputDevice8* devkeyboard = nullptr;
	//�L�[���͏��
	BYTE key[256] = {};
	//1�t���[���O�̃L�[���͏��
	BYTE preKey[256] = {};
};


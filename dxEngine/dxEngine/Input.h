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
	//�C���X�^���X
	static Input* instance;
	//DirectInput�I�u�W�F�N�g
	ComPtr<IDirectInput8> dInput;
	//�L�[�o�[�h�f�o�C�X
	ComPtr<IDirectInputDevice8> devkeyboard;
	//�L�[���͏��
	BYTE key[256] = {};
	//1�t���[���O�̃L�[���͏��
	BYTE preKey[256] = {};

	//�}�E�X����
	ComPtr<IDirectInputDevice8> devMouse;
	DIMOUSESTATE2 mouseStat = {};
	DIMOUSESTATE2 preMouseStat = {};

private:
	Input() = default;
	~Input() = default;

	//�R�s�[�E����֎~
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

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
	/// <summary>
	/// ������
	/// </summary>
	/// <param name="hwnd">�E�B���h�E�n���h��</param>
	void Initialize(HWND hwnd);

public:
	//���t���[������
	void Update();
	//�L�[��������Ă��邩
	bool IsKey(int keyNum);
	//�g���K�[
	bool IsTrigger(int keyNum);
	//�}�E�X��
	bool IsMouseLeft();
	//�z�C�[���N���b�N
	bool IsMouseWheel();
	//�}�E�X�E
	bool IsMouseRight();
	//�}�E�X���E�g���K�[
	bool TriggerMouseLeft();
	//�z�C�[���N���b�N�E�g���K�[
	bool TriggerMouseWheel();
	//�}�E�X�E�E�g���K�[
	bool TriggerMouseRight();
	//�}�E�X�ړ���
	MouseMove GetMouseMove();

	/// <summary>
	/// �폜
	/// </summary>
	void Terminate();

};


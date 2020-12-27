#pragma once

#include<d3d12.h>
#include<dxgi1_6.h>
#include<d3dx12.h>
#include<wrl.h>

#include"WindowsApp.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


class DirectXSystem
{
private:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

private:
	//�C���X�^���X
	static DirectXSystem* instance;

private:
	//�R���X�g���N�^
	DirectXSystem();

	//�f�X�g���N�^
	~DirectXSystem();

	//�R�s�[�E����֎~
	DirectXSystem(const DirectXSystem&) = delete;
	void operator=(const DirectXSystem&) = delete;

public:
	/// <summary>
	/// �f�o�b�O���C���[�̗L����
	/// </summary>
	static void EnableDebugLayer();

	/// <summary>
	/// �C���X�^���X����
	/// </summary>
	/// <returns></returns>
	static void Create();

	/// <summary>
	/// �C���X�^���X�擾
	/// </summary>
	/// <returns></returns>
	static DirectXSystem* GetInstance();

	/// <summary>
	/// �C���X�^���X�폜
	/// </summary>
	static void Destroy();

public:

	//������
	void Initialize(WindowsApp* winApp);

	//�`��O����
	void DrawBefore();

	//�`��㏈��
	void DrawAfter();

	//�f�o�C�X�̎擾
	ID3D12Device* GetDevice() { return device.Get(); }

	//�R�}���h���X�g�̎擾
	ID3D12GraphicsCommandList* GetCmdList() { return cmdList.Get(); }

private:
	//�E�B���h�E�Y�A�v���P�[�V����
	WindowsApp* winApp = nullptr;

	HRESULT result;
	//�f�o�C�X
	ComPtr<ID3D12Device> device = nullptr;
	//DXGI�t�@�N�g���[
	ComPtr<IDXGIFactory6> dxgiFactory = nullptr;
	//�X���b�v�`�F�[��
	ComPtr<IDXGISwapChain4> swapchain = nullptr;
	//�����_�[�^�[�Q�b�g�r���[�q�[�v
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	//�o�b�N�o�b�t�@�z��
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	//�[�x�o�b�t�@
	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	//�[�x�o�b�t�@�r���[�q�[�v
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	//�R�}���h�A���P�[�^�[
	ComPtr<ID3D12CommandAllocator> cmdAllocator = nullptr;
	//�R�}���h���X�g
	ComPtr<ID3D12GraphicsCommandList> cmdList = nullptr;
	//�R�}���h�L���[
	ComPtr<ID3D12CommandQueue> cmdQueue = nullptr;
	//
	UINT64 fenceVal = 0;
	//�t�F���X
	ComPtr<ID3D12Fence> fence = nullptr;

private:
	/// <summary>
	/// DirectX12�̏�����
	/// </summary>
	/// <returns></returns>
	HRESULT InitDirectX();

	/// <summary>
	/// �R�}���h�n�̏�����
	/// </summary>
	/// <returns></returns>
	HRESULT InitCommand();

	/// <summary>
	/// �X���b�v�`�F�[���̏�����
	/// </summary>
	/// <returns></returns>
	HRESULT InitSwapchain();
	
	/// <summary>
	/// �[�x�o�b�t�@�̐���
	/// </summary>
	/// <returns></returns>
	HRESULT CreateDepthBuffer();

	/// <summary>
	/// �t�F���X�̐���
	/// </summary>
	/// <returns></returns>
	HRESULT CreateFence();

};


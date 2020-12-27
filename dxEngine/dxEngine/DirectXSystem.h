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
	//インスタンス
	static DirectXSystem* instance;

private:
	//コンストラクタ
	DirectXSystem();

	//デストラクタ
	~DirectXSystem();

	//コピー・代入禁止
	DirectXSystem(const DirectXSystem&) = delete;
	void operator=(const DirectXSystem&) = delete;

public:
	/// <summary>
	/// デバッグレイヤーの有効化
	/// </summary>
	static void EnableDebugLayer();

	/// <summary>
	/// インスタンス生成
	/// </summary>
	/// <returns></returns>
	static void Create();

	/// <summary>
	/// インスタンス取得
	/// </summary>
	/// <returns></returns>
	static DirectXSystem* GetInstance();

	/// <summary>
	/// インスタンス削除
	/// </summary>
	static void Destroy();

public:

	//初期化
	void Initialize(WindowsApp* winApp);

	//描画前処理
	void DrawBefore();

	//描画後処理
	void DrawAfter();

	//デバイスの取得
	ID3D12Device* GetDevice() { return device.Get(); }

	//コマンドリストの取得
	ID3D12GraphicsCommandList* GetCmdList() { return cmdList.Get(); }

private:
	//ウィンドウズアプリケーション
	WindowsApp* winApp = nullptr;

	HRESULT result;
	//デバイス
	ComPtr<ID3D12Device> device = nullptr;
	//DXGIファクトリー
	ComPtr<IDXGIFactory6> dxgiFactory = nullptr;
	//スワップチェーン
	ComPtr<IDXGISwapChain4> swapchain = nullptr;
	//レンダーターゲットビューヒープ
	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;
	//バックバッファ配列
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	//深度バッファ
	ComPtr<ID3D12Resource> depthBuffer = nullptr;
	//深度バッファビューヒープ
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	//コマンドアロケーター
	ComPtr<ID3D12CommandAllocator> cmdAllocator = nullptr;
	//コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> cmdList = nullptr;
	//コマンドキュー
	ComPtr<ID3D12CommandQueue> cmdQueue = nullptr;
	//
	UINT64 fenceVal = 0;
	//フェンス
	ComPtr<ID3D12Fence> fence = nullptr;

private:
	/// <summary>
	/// DirectX12の初期化
	/// </summary>
	/// <returns></returns>
	HRESULT InitDirectX();

	/// <summary>
	/// コマンド系の初期化
	/// </summary>
	/// <returns></returns>
	HRESULT InitCommand();

	/// <summary>
	/// スワップチェーンの初期化
	/// </summary>
	/// <returns></returns>
	HRESULT InitSwapchain();
	
	/// <summary>
	/// 深度バッファの生成
	/// </summary>
	/// <returns></returns>
	HRESULT CreateDepthBuffer();

	/// <summary>
	/// フェンスの生成
	/// </summary>
	/// <returns></returns>
	HRESULT CreateFence();

};


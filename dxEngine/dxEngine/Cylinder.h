#pragma once

#include"GameObject.h"

class Cylinder : public GameObject
{
private:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT2;

public:
	//頂点データ構造体
	struct VertexData
	{
		XMFLOAT3 pos;    //xyz座標
		XMFLOAT3 normal; //法線ベクトル
		XMFLOAT2 uv;     //uv座標
	};

	//定数バッファ用データ構造体
	struct ConstBuffData
	{
		XMFLOAT4 color;  //色
		XMMATRIX matrix; //3D変換行列
	};

private:
	static const int division = 36;					        // 分割数
	const float radius = 0.5f;				            // 底面の半径
	const float prizmHeight = 1.0f;			            // 柱の高さ
	static const int planeCount = division * 2 + division * 2; // 面の数
	static const int vertexCount = planeCount * 3;		        // 頂点数

private:
	//DirectXデバイス
	ID3D12Device* device = nullptr;

	//球体頂点データ
	VertexData vertices[vertexCount];
	//球体インデックスデータ
	unsigned short indices[planeCount * 3];

	//頂点バッファ
	ComPtr<ID3D12Resource> vertBuff = nullptr;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//インデックスバッファ
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	//インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView{};

	//デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeap = nullptr;
	//デスクリプタのサイズ
	UINT descHandleIncrementSize = 0;
	//テクスチャバッファ
	ComPtr<ID3D12Resource> texbuff = nullptr;
	// シェーダリソースビューのハンドル(CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	// シェーダリソースビューのハンドル(GPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;

	//頂点シェーダーオブジェクト
	ComPtr<ID3DBlob> vsBlob = nullptr;
	//ピクセルシェーダーオブジェクト
	ComPtr<ID3DBlob> psBlob = nullptr;
	//ルートシグネチャバイナリコード
	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	//ルートシグネチャオブジェクト
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//パイプラインステートオブジェクト
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

private:
	Cylinder() {};

	//コピー・代入禁止
	Cylinder(const Cylinder&) = delete;
	void operator=(const Cylinder&) = delete;

public:
	~Cylinder();

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw(ID3D12GraphicsCommandList* cmdList) override;
	virtual void Terminate() override;

public:
	static Cylinder* Create(
		ID3D12Device* dev
	);

private:
	virtual HRESULT CreateVertBuff() override;
	virtual HRESULT CreateIndexBuff() override;
	virtual HRESULT ShaderCompile() override;
	virtual HRESULT CreateRootSignature() override;
	virtual HRESULT CreateGraphicsPipeline() override;
	virtual HRESULT CreateDescriptorHeap() override;
	virtual HRESULT LoadTexture(const std::string& textureName) override;
	virtual void CreateModel() override;
	virtual void InitMatrix() override;
	virtual HRESULT CreateConstBuff() override;
	virtual HRESULT CreateConstView() override;
};


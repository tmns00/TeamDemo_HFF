#pragma once

#include"GameObject.h"

class OBJObject :public GameObject
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
	struct ConstBuffDataB0
	{
		XMMATRIX matrix; //3D変換行列
	};

	//定数バッファ用データ構造体
	struct ConstBuffDataB1
	{
		XMFLOAT3 ambient;  //アンビエント係数
		float pad1;        //パディング
		XMFLOAT3 diffuse;  //ディフューズ係数
		float pad2;        //パディング
		XMFLOAT3 specular; //スペキュラー係数
		float alpha;       //アルファ
	};

	//マテリアル構造体
	struct Material
	{
		std::string name; //
		XMFLOAT3 ambient; //
		XMFLOAT3 diffuse; //
		XMFLOAT3 specular; //
		float alpha; //アルファ
		std::string textureFileName; //テクスチャファイル名
		//コンストラクタ
		Material(){
			ambient = { 0.3f,0.3f,0.3f };
			diffuse = { 0.0f,0.0f,0.0f };
			specular = { 0.0f,0.0f,0.0f };
			alpha = 1.0f;
		}
	};

private:
	std::string useFileName = "";

private:
	//DirectXデバイス
	ID3D12Device* device = nullptr;

	//球体頂点データ
	std::vector<VertexData>vertices;
	//球体インデックスデータ
	std::vector<unsigned short>indices;

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

	//マテリアル
	Material material;
	//定数バッファその2
	ComPtr<ID3D12Resource> constBuffB1; 

private:
	OBJObject() {};

	//コピー・代入禁止
	OBJObject(const OBJObject&) = delete;
	void operator=(const OBJObject&) = delete;

public:
	~OBJObject();

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw(ID3D12GraphicsCommandList* cmdList) override;
	virtual void Terminate() override;

public:
	static OBJObject* Create(
		ID3D12Device* dev,
		const std::string& fileName
	);

private:
	virtual HRESULT CreateVertBuff() override;
	virtual HRESULT CreateIndexBuff() override;
	virtual HRESULT ShaderCompile() override;
	virtual HRESULT CreateRootSignature() override;
	virtual HRESULT CreateGraphicsPipeline() override;
	virtual HRESULT CreateDescriptorHeap() override;
	virtual HRESULT LoadTexture(
		const std::string& textureName
	) override;
	HRESULT LoadTexture(
		const std::string& directoryPath,
		const std::string& fileName
	);
	virtual void CreateModel() override;
	virtual void InitMatrix() override;
	virtual HRESULT CreateConstBuff() override;
	virtual HRESULT CreateConstView() override;

	void LoadMaterial(
		const std::string& directoryPath,
		const std::string& fileName
	);
};


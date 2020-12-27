#pragma once

#include"GameObject.h"

class Sphere:public GameObject
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
	static const int u_max = 32; //縦方向の分割数
	static const int v_max = 16; //横方向の分割数
	static const int vertexCount = u_max * (v_max + 1);    //頂点数
	static const int indexCount = 2 * v_max * (u_max + 1); //インデックス数

private:
	//DirectXデバイス
	ID3D12Device* device = nullptr;

	//球体頂点データ
	VertexData vertices[vertexCount];
	//球体インデックスデータ
	unsigned short indices[indexCount];

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
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Sphere() {};

	//コピー・代入禁止
	Sphere(const Sphere&) = delete;
	void operator=(const Sphere&) = delete;

public:
	///デストラクタ
	~Sphere();

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize() override;

	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update() override;
	
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="cmdList"></param>
	virtual void Draw(
		ID3D12GraphicsCommandList* cmdList
	) override;

	/// <summary>
	/// 削除
	/// </summary>
	virtual void Terminate() override;

public:
	/// <summary>
	/// 生成
	/// </summary>
	/// <param name="dev">デバイス</param>
	/// <returns>インスタンス</returns>
	static Sphere* Create(
		ID3D12Device* dev
	);

private:
	/// <summary>
	/// 頂点バッファ生成
	/// </summary>
	/// <returns>生成の確認</returns>
	virtual HRESULT CreateVertBuff() override;

	/// <summary>
	/// インデックスバッファ生成
	/// </summary>
	/// <returns>生成の確認</returns>
	virtual HRESULT CreateIndexBuff() override;

	/// <summary>
	/// シェーダーファイルのコンパイル
	/// </summary>
	/// <returns>成功の確認</returns>
	virtual HRESULT ShaderCompile() override;

	/// <summary>
	/// ルートシグネチャ生成
	/// </summary>
	/// <returns>生成の確認</returns>
	virtual HRESULT CreateRootSignature() override;

	/// <summary>
	/// グラフィックパイプライン生成
	/// </summary>
	/// <returns>生成の確認</returns>
	virtual HRESULT CreateGraphicsPipeline() override;

	/// <summary>
	/// デスクリプタヒープ生成
	/// </summary>
	/// <returns>生成の確認</returns>
	virtual HRESULT CreateDescriptorHeap() override;

	/// <summary>
	/// テクスチャの読み込み
	/// </summary>
	/// <param name="textureName">テクスチャのファイル名</param>
	/// <returns>生成の確認</returns>
	virtual HRESULT LoadTexture(
		const std::string& textureName
	) override;

	/// <summary>
	/// 立体生成
	/// </summary>
	virtual void CreateModel() override;

	/// <summary>
	/// 変換行列の初期化
	/// </summary>
	virtual void InitMatrix() override;

	/// <summary>
	/// 定数バッファ生成
	/// </summary>
	/// <returns>生成の確認</returns>
	virtual HRESULT CreateConstBuff() override;

	/// <summary>
	/// 当たり判定
	/// </summary>
	/// <param name="obj">当たっているオブジェクト</param>
	virtual void Collision(
		const GameObject& obj
	)override;
};


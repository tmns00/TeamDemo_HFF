#pragma once

#include"GameObject.h"
#include"PMDMotionStructs.h"
#include"VMDLoader.h"

#include <unordered_map>
#include <map>

class PMDObject :public GameObject
{
private:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMMATRIX = DirectX::XMMATRIX;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT2;

public:
	//シェーダー側に渡すための基本的な行列データ
	struct SceneMatrix
	{
		XMMATRIX world;      //モデル本体の回転・移動行列
		XMMATRIX view;       //ビュー行列
		XMMATRIX proj;       //プロジェクション行列
		XMMATRIX view_proj;  //ビュー・プロジェクション行列
		XMFLOAT3 eye;        //視点座標
		XMMATRIX bones[256]; //ボーン行列
	};

	//PMDヘッダー構造体
	struct PMDHeader
	{
		float version;       //例：00 00 80 3f == 1.00
		char model_name[20]; //モデル名
		char comment[256];   //モデルコメント
	};

	//PMD頂点構造体
	struct PMDVertex
	{
		XMFLOAT3 pos;             //頂点座標：12バイト
		XMFLOAT3 normal;          //法線ベクトル：12バイト
		XMFLOAT2 uv;              //uv座標：8バイト
		unsigned short boneNo[2]; //ボーン番号：4バイト
		unsigned char boneWeight; //ボーン影響度：1バイト
		unsigned char edgeFlg;    //輪郭線フラグ：1バイト
	};

#pragma pack(1)
	//PMDマテリアル構造体
	struct PMDMaterial
	{
		XMFLOAT3 diffuse;        //ディフューズ色
		float alpha;             //ディフューズα
		float specularity;       //スペキュラーの強さ
		XMFLOAT3 specular;       //スペキュラー色
		XMFLOAT3 ambient;        //アンビエント色
		unsigned char toonIdx;   //トゥーン番号
		unsigned char edgeFlg;   //マテリアルごとの輪郭線フラグ
		unsigned int indicesNum; //このマテリアルが割り当てられる　インデックス数
		char texFilePath[20];    //テクスチャファイルパス＋α
	};
#pragma pack()

	//シェーダーに送るマテリアルデータ
	struct MaterialForHlsl
	{
		XMFLOAT3 diffuse;  //ディフューズ色
		float alpha;       //ディフューズα
		XMFLOAT3 specular; //スペキュラー色
		float specularity; //スペキュラーα
		XMFLOAT3 ambient;  //アンビエント色
	};

	//上記以外のマテリアルデータ
	struct AdditionalMaterial
	{
		std::string texPath; //テクスチャファイルパス
		int toonIdx = 0;         //トゥーン番号
		bool edgeFlg = false;        //マテリアルごとの輪郭線フラグ
	};

	//全体をまとめるデータ
	struct Material
	{
		unsigned int indicesNum = 0;        //インデックス数
		MaterialForHlsl material;       //
		AdditionalMaterial addMaterial; //
	};

#pragma pack(1)
	//読み込み用ボーン構造体
	struct PMDBone
	{
		char boneName[20];       //ボーン名
		unsigned short parentNo; //親ボーン番号
		unsigned short nextNo;   //先端のボーン番号
		unsigned char type;      //ボーン種別
		unsigned short ikBoneNo; //IKボーン番号
		XMFLOAT3 pos;            //ボーンの基準点座標
	};
#pragma pack()

	//ボーンノード構造体
	struct BoneNode
	{
		uint32_t boneIndex;              //ボーンインデックス
		uint32_t boneType;               //ボーン種別
		uint32_t parentBone;             //親ボーン
		uint32_t ikParentBone;           //IK親ボーン
		XMFLOAT3 startPos;               //ボーン基準点(回転中心)
		std::vector<BoneNode*> children; //子ノード
	};

	//IKデータ構造体
	struct PMDIK
	{
		uint16_t boneIndex;                //IK対象のボーンを示す
		uint16_t targetIndex;              //ターゲットに近づけるためのボーンのインデックス
		uint16_t iterations;               //試行回数
		float limit;                       //1回あたりの回転制限
		std::vector<uint16_t> nodeIndices; //間のノード番号
	};

private:
	//pmdファイルパス
	std::string strModelPath;

private:
	//デバイス
	ComPtr<ID3D12Device> device = nullptr;

	//頂点配列データ
	std::vector<unsigned char> vertices{};
	//インデックス配列データ
	std::vector<unsigned short> indices{};

	//頂点数
	unsigned int vertNum = 0;
	//頂点バッファ
	ComPtr<ID3D12Resource> vertBuff = nullptr;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	//インデックス数
	unsigned int indicesNum = 0;
	//インデックスバッファ
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	//インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	//デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> basicDescHeap = nullptr;

	//定数用デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> constDescHeap = nullptr;

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

	//PMDヘッダーオブジェクト
	PMDHeader pmdheader{};
	//マテリアルデータ配列
	std::vector<Material> materials{};
	//マテリアル数
	unsigned int materialNum = 0;
	//マテリアルデスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> materialDescHeap = nullptr;
	//テクスチャバッファ配列
	std::vector<ComPtr<ID3D12Resource>> textureResources{};
	//スフィアマップバッファ配列
	std::vector<ComPtr<ID3D12Resource>> sphereResources{};
	//加算スフィアマップバッファ配列
	std::vector<ComPtr<ID3D12Resource>> addSphResources{};

	//ボーン情報
	std::vector<PMDBone> pmdBones;
	//ボーン情報配列
	std::vector<XMMATRIX> boneMatrices{};
	//ボーンノードテーブル
	std::map<std::string, BoneNode> boneNodeTable;
	//
	//std::unordered_map<std::string, std::vector<Motion>> motionData;
	//マップ先のポインター
	SceneMatrix* mapMatrix;
	//アニメーション開始時のミリ秒
	DWORD startTime = 0;
	//最大フレーム番号
	//unsigned int duration = 0;
	//コマンドリスト
	ID3D12GraphicsCommandList* cmdList = nullptr;

	//IKデータの格納配列
	std::vector<PMDIK> pmdIkData;
	//インデックスから名前を検索しやすいように
	std::vector<std::string> boneNameArray;
	//インデックスからノードを検索しやすいように
	std::vector<BoneNode*> boneNodeAddressArray;
	//
	std::vector<uint32_t> kneeIndices;
	//IKオンオフデータの保持コンテナ
	//std::vector<VMDIKEnable>ikEnableData;

	//VMDロードクラス
	VMDLoader* vmdLoader;
	//
	MotionDatas loadMotion;

private:
	PMDObject() {};

	//コピー・代入禁止
	PMDObject(const PMDObject&) = delete;
	void operator=(const PMDObject&) = delete;

public:
	~PMDObject();

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw(ID3D12GraphicsCommandList* cmdList) override;
	virtual void Terminate() override;

	/// <summary>
	/// アニメーション開始
	/// </summary>
	void PlayAnimation(std::string key);

	/// <summary>
	/// アニメーション停止
	/// </summary>
	void StopAnimation();

public:
	static PMDObject* Create(
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
	virtual HRESULT LoadTexture(const std::string& textureName) override;
	virtual void CreateModel() override;
	virtual void InitMatrix() override;
	virtual HRESULT CreateConstBuff() override;
	virtual HRESULT CreateConstView() override;

	/// <summary>
	/// VMDデータ読み込み
	/// </summary>
	void LoadVMDFile(std::string key);

	/// <summary>
	/// モーション更新
	/// </summary>
	void MotionUpdate();

	/// <summary>
	/// マテリアルバッファ・ビューの作成
	/// </summary>
	/// <returns></returns>
	HRESULT CreateMaterialBuff();

	/// <summary>
	/// pmdモデルからテクスチャのロード
	/// </summary>
	/// <param name="texPath">pmdファイルパス</param>
	/// <returns>テクスチャバッファ</returns>
	ID3D12Resource* LoadTextureFromFile(
		std::string& texPath
	);

	/// <summary>
	/// 白テクスチャ
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* CreateWhiteTexture();

	/// <summary>
	/// 黒テクスチャ
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* CreateBlackTexture();

	/// <summary>
	/// モデルのパスとテクスチャのパスから合成パスを得る
	/// フォルダセパレータ2種類に対応
	/// </summary>
	/// <param name="modelPath">アプリケーションからのpmdモデルのパス</param>
	/// <param name="texPath">pmdモデルからのテクスチャのパス</param>
	/// <returns>アプリからのテクスチャのパス</returns>
	std::string GetTexturePathFromModelAndTexPath(
		const std::string& modelPath,
		const char* texPath
	);

	/// <summary>
	/// マルチバイト文字列からワイド文字列を得る
	/// </summary>
	/// <param name="str">マルチバイト文字列</param>
	/// <returns>変換されたワイド文字列</returns>
	std::wstring GetWideStringFromString(
		const std::string& str
	);

	/// <summary>
	/// ファイル名から拡張子を取得する
	/// </summary>
	/// <param name="path">対象のパス</param>
	/// <returns>拡張子</returns>
	std::string GetExtension(
		const std::string& path
	);

	/// <summary>
	/// テクスチャのパスをセパレータ文字で分割する
	/// </summary>
	/// <param name="path">対象のパス</param>
	/// <param name="splitter">セパレータ文字</param>
	/// <returns>分離前後の文字列のペア</returns>
	std::pair<std::string, std::string> SplitFilePath(
		const std::string& path,
		const char splitter = '*'
	);

	/// <summary>
	/// ボーンの変換行列の再帰処理
	/// </summary>
	/// <param name="node">ノード</param>
	/// <param name="mat">変換行列</param>
	void RecursiveMatrixMultiply(
		BoneNode* node,
		const XMMATRIX& mat
	);

	/// <summary>
	/// ベジェ曲線補間メソッド
	/// </summary>
	float GetYFromXOnBezier(
		float x,
		const XMFLOAT2& a,
		const XMFLOAT2& b,
		uint8_t n
	);

	/// <summary>
	/// IKの種類を選択する
	/// </summary>
	void PickUpIKSolve(
		int frameNo
	);

	/// <summary>
	/// CCD-IKによってボーン方向を解決
	/// </summary>
	/// <param name="ik">対象のIKオブジェクト</param>
	void SolveCCDIK(
		const PMDIK& ik
	);

	/// <summary>
	/// 余弦定理IKによってボーン方向を解決
	/// </summary>
	/// <param name="ik">対象のIKオブジェクト</param>
	void SolveCosineIK(
		const PMDIK& ik
	);

	/// <summary>
	/// LookAt行列によってボーン方向を解決
	/// </summary>
	/// <param name="ik">対象のIKオブジェクト</param>
	void SolveLookAt(
		const PMDIK& ik
	);

	/// <summary>
	/// z軸を特定の方向に向ける行列を返す関数
	/// </summary>
	/// <param name="lookat">向かせたい方向のベクトル</param>
	/// <param name="up">上ベクトル</param>
	/// <param name="right">右ベクトル</param>
	/// <returns></returns>
	XMMATRIX LookAtMatrix(
		const XMVECTOR& lookat,
		XMFLOAT3& up,
		XMFLOAT3& right
	);

	/// <summary>
	/// 特定のベクトルを特定の方向に向けるための行列を返す
	/// </summary>
	/// <param name="origin">特定のベクトル</param>
	/// <param name="lookat">向かせたい方向</param>
	/// <param name="up">上ベクトル</param>
	/// <param name="right">右ベクトル</param>
	/// <returns>特定のベクトルを特定の方向に向けるための行列</returns>
	XMMATRIX LookAtMatrix(
		const XMVECTOR& origin,
		const XMVECTOR& lookat,
		XMFLOAT3& up,
		XMFLOAT3& right
	);
};


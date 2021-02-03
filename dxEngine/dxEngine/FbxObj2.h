#pragma once

#include"GameObject.h"
#include<fbxsdk.h>
#include<map>

class FbxObj2 :public GameObject
{
private:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT3;
	using XMMATRIX = DirectX::XMMATRIX;

private:
	//頂点データ
	struct VertexData
	{
		XMFLOAT3 pos{};
		XMFLOAT3 normal{};
		//XMFLOAT2 uv{};
		UINT BoneIndex[4];
		float BoneWeight[4];
		VertexData()
		{
			ZeroMemory(this, sizeof(VertexData));
		}
	};

	//定数バッファ用データ構造体
	struct ConstBuffData
	{
		XMFLOAT4 color;  //色
		XMMATRIX matrix; //3D変換行列
	};

	//ボーン構造体
	struct BoneData
	{
		XMMATRIX bindPose{};
		XMMATRIX newPose{};
		BoneData()
		{
			ZeroMemory(this, sizeof(BoneData));
		}
	};

	struct Bone
	{
		std::string name;
		FbxCluster* bone;
	};

public:
	//DirectX関連
	ID3D12Device* device = nullptr;


private:
	

	//fbx関連
	FbxManager* fbxManager = nullptr;
	FbxImporter* fbxImporter = nullptr;
	FbxScene* fbxScene = nullptr;

	//メッシュを構成する要素
	FbxNode* rootNode = nullptr;
	int vertNum = 0;
	int uvNum = 0;
	int faceNum = 0;
	std::map<std::string, FbxNode*> meshNodeList;
	std::map<std::string, std::vector<UINT>> indices;
	std::map<std::string, std::vector<VertexData>> vertices;
	std::map<std::string, ID3D12Resource*>vertBuff;
	std::map<std::string, D3D12_VERTEX_BUFFER_VIEW> vbView;
	std::map<std::string, ID3D12Resource*>indexBuff;
	std::map<std::string, D3D12_INDEX_BUFFER_VIEW> ibView;

	//アニメーション
	int AnimStackNumber = 0;
	FbxTime frameTime, timeCount, start, stop;
	FbxArray<FbxString*> AnimStackNameArray;
	FbxAnimStack* AnimationStack = nullptr;

	//ボーン
	int useBoneNum = 0;
	BoneData* boneArray;
	FbxCluster** ppCluster;
	FbxMatrix* clusterDeformation; //各頂点にかける最終的な行列の配列

	int boneNum = 0; //ボーン数
	std::map<std::string,std::vector<Bone>> bones;

	//描画関連
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

public:
	// GameObject を介して継承されました
	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw(ID3D12GraphicsCommandList* cmdList) override;
	virtual void Terminate() override;
private:
	HRESULT InitFBX(
		std::string fileName
	);

	void DestroyFBX();

	void LoadContents();

	void LoadNode(
		FbxNode* node,
		int hierarchy
	);

	void CollectMeshNode(
		FbxNode* node,
		std::map<std::string, FbxNode*>& list
	);

	void LoadIndex(
		const char* nodeName,
		FbxMesh* mesh
	);

	void LoadVertex(
		const char* nodeName,
		FbxMesh* mesh
	);

	void LoadNormal(
		const char* nodeName,
		FbxMesh* mesh
	);

	void LoadUV(
		const char* nodeName,
		FbxMesh* mesh
	);

	void LoadSkinInfo(
		const char* nodeName,
		FbxMesh* mesh
	);

	void CreateMesh(
		const char* nodeName,
		FbxMesh* mesh
	);

	HRESULT InitAnim();

	void UpdateAnim();

	void SetPosBone(
		XMFLOAT3 setPos,
		int boneIndex
	);

	void SetRotBone(
		XMFLOAT3 setRot,
		int boneIndex
	);

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

	const char* GetNodeAttributeName(
		FbxNodeAttribute::EType attribute
	);

	XMMATRIX FbxAMatrixConvertToXMMatrix(
		FbxAMatrix fbxMat
	);
};


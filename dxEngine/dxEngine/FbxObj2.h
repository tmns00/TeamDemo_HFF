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
	//���_�f�[�^
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

	//�萔�o�b�t�@�p�f�[�^�\����
	struct ConstBuffData
	{
		XMFLOAT4 color;  //�F
		XMMATRIX matrix; //3D�ϊ��s��
	};

	//�{�[���\����
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
	//DirectX�֘A
	ID3D12Device* device = nullptr;


private:
	

	//fbx�֘A
	FbxManager* fbxManager = nullptr;
	FbxImporter* fbxImporter = nullptr;
	FbxScene* fbxScene = nullptr;

	//���b�V�����\������v�f
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

	//�A�j���[�V����
	int AnimStackNumber = 0;
	FbxTime frameTime, timeCount, start, stop;
	FbxArray<FbxString*> AnimStackNameArray;
	FbxAnimStack* AnimationStack = nullptr;

	//�{�[��
	int useBoneNum = 0;
	BoneData* boneArray;
	FbxCluster** ppCluster;
	FbxMatrix* clusterDeformation; //�e���_�ɂ�����ŏI�I�ȍs��̔z��

	int boneNum = 0; //�{�[����
	std::map<std::string,std::vector<Bone>> bones;

	//�`��֘A
	//���_�V�F�[�_�[�I�u�W�F�N�g
	ComPtr<ID3DBlob> vsBlob = nullptr;
	//�s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	ComPtr<ID3DBlob> psBlob = nullptr;
	//���[�g�V�O�l�`���o�C�i���R�[�h
	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	//���[�g�V�O�l�`���I�u�W�F�N�g
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//�p�C�v���C���X�e�[�g�I�u�W�F�N�g
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

public:
	// GameObject ����Čp������܂���
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


#pragma once

#include"GameObject.h"

#include<fbxsdk.h>
#include<map>

class FBXObject :public GameObject
{
private:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT2;

public:
	//���_�f�[�^�\����
	struct VertexData
	{
		XMFLOAT3 pos;    //xyz���W
		XMFLOAT3 normal; //�@���x�N�g��
		XMFLOAT2 uv;     //uv���W
	};

	//�萔�o�b�t�@�p�f�[�^�\����
	struct ConstBuffData
	{
		XMFLOAT4 color;  //�F
		XMMATRIX matrix; //3D�ϊ��s��
	};


private:
	//DirectX�f�o�C�X
	ID3D12Device* device = nullptr;

	//���_�f�[�^
	std::vector<VertexData> vertices;
	//�C���f�b�N�X�f�[�^
	std::vector<unsigned short> indices;

	//���_�o�b�t�@
	ComPtr<ID3D12Resource> vertBuff;
	//���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView;
	//�C���f�b�N�X�o�b�t�@
	ComPtr<ID3D12Resource> idxBuff;
	//�C���f�b�N�X�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW ibView{};

	//�f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> descHeap = nullptr;
	//�f�X�N���v�^�̃T�C�Y
	UINT descHandleIncrementSize = 0;
	//�e�N�X�`���o�b�t�@
	ComPtr<ID3D12Resource> texbuff = nullptr;
	// �V�F�[�_���\�[�X�r���[�̃n���h��(CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	// �V�F�[�_���\�[�X�r���[�̃n���h��(GPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;

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

	//FBXSDK�֘A
	//SDK�}�l�[�W���[
	FbxManager* fbxManager;
	//FBX�C���|�[�^�[
	FbxImporter* fbxImporter;
	//FBX�V�[��
	FbxScene* fbxScene;
	//Mesh�ۑ��R���e�i
	std::map<std::string, FbxNode*> meshNodeList;

private:
	FBXObject() {};

	//�R�s�[�E����֎~
	FBXObject(const FBXObject&) = delete;
	void operator=(const FBXObject&) = delete;

public:
	~FBXObject();
	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw(
		ID3D12GraphicsCommandList* cmdList
	) override;
	virtual void Terminate() override;

	HRESULT InitFBX(
		const std::string& szFileName
	);

public:
	static FBXObject* Create(
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
	virtual void CreateModel() override;
	virtual void InitMatrix() override;
	virtual HRESULT CreateConstBuff() override;

	void DisplayContent(
		FbxScene* scene
	);


	void DisplayContent(
		FbxNode* node
	);


	void DisplayMesh(
		FbxNode* node
	);


	void DisplayIndex(
		FbxMesh* mesh
	);


	void DisplayPosition(
		FbxMesh* mesh
	);
};
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
	//���_�f�[�^�\����
	struct VertexData
	{
		XMFLOAT3 pos;    //xyz���W
		XMFLOAT3 normal; //�@���x�N�g��
		XMFLOAT2 uv;     //uv���W
	};

	//�萔�o�b�t�@�p�f�[�^�\����
	struct ConstBuffDataB0
	{
		XMMATRIX matrix; //3D�ϊ��s��
	};

	//�萔�o�b�t�@�p�f�[�^�\����
	struct ConstBuffDataB1
	{
		XMFLOAT3 ambient;  //�A���r�G���g�W��
		float pad1;        //�p�f�B���O
		XMFLOAT3 diffuse;  //�f�B�t���[�Y�W��
		float pad2;        //�p�f�B���O
		XMFLOAT3 specular; //�X�y�L�����[�W��
		float alpha;       //�A���t�@
	};

	//�}�e���A���\����
	struct Material
	{
		std::string name; //
		XMFLOAT3 ambient; //
		XMFLOAT3 diffuse; //
		XMFLOAT3 specular; //
		float alpha; //�A���t�@
		std::string textureFileName; //�e�N�X�`���t�@�C����
		//�R���X�g���N�^
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
	//DirectX�f�o�C�X
	ID3D12Device* device = nullptr;

	//���̒��_�f�[�^
	std::vector<VertexData>vertices;
	//���̃C���f�b�N�X�f�[�^
	std::vector<unsigned short>indices;

	//���_�o�b�t�@
	ComPtr<ID3D12Resource> vertBuff = nullptr;
	//���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//�C���f�b�N�X�o�b�t�@
	ComPtr<ID3D12Resource> idxBuff = nullptr;
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

	//�}�e���A��
	Material material;
	//�萔�o�b�t�@����2
	ComPtr<ID3D12Resource> constBuffB1; 

private:
	OBJObject() {};

	//�R�s�[�E����֎~
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


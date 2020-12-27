#pragma once

#include"GameObject.h"

class Capsule:public GameObject
{
private:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMMATRIX = DirectX::XMMATRIX;
	using XMVECTOR = DirectX::XMVECTOR;
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
	//���������̃x�N�g��
	const XMVECTOR cutVec = { 0.1f,1.0f,0.0f,0.0f };
	//�������̔��a
	const float radius = 0.5f;
	//���𒆐S�Ƃ���������
	const UINT slice = 16;
	//���������̕�����
	const UINT stack_1_2 = 8;

private:
	//DirectX�f�o�C�X
	ID3D12Device* device = nullptr;

	//���̒��_�f�[�^
	std::vector<VertexData> vertices{};
	//���̃C���f�b�N�X�f�[�^
	std::vector<unsigned short> indices{};

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

private:
	Capsule() {};

	//�R�s�[�E����֎~
	Capsule(const Capsule&) = delete;
	void operator=(const Capsule&) = delete;

public:
	~Capsule();
	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw(
		ID3D12GraphicsCommandList* cmdList
	) override;
	virtual void Terminate() override;

public:
	static Capsule* Create(
		ID3D12Device* dev
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
};


#pragma once

#include"GameObject.h"

class DeltaCone :public GameObject
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
	//�V��̍���
	const float topHeight = 0.75f;
	//��ʂ̔��a
	const float radius = 0.5f;
	//�ʂ̐�
	static const int planeCount = 6;
	//���_��
	static const int vertexCount = planeCount * 3;
	//���_�f�[�^�z��
	VertexData vertices[vertexCount]{};
	//�C���f�b�N�X�z��
	unsigned short indices[planeCount * 3]{};

private:
	//DirectX�f�o�C�X
	ID3D12Device* device = nullptr;
	//�R�}���h���X�g
	//ID3D12GraphicsCommandList* cmdList = nullptr;

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
	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	DeltaCone() {};

	//�R�s�[�E����֎~
	DeltaCone(const DeltaCone&) = delete;
	void operator=(const DeltaCone&) = delete;

public:
	///�f�X�g���N�^
	~DeltaCone();

	/// <summary>
	/// ������
	/// </summary>
	virtual void Initialize() override;

	/// <summary>
	/// �X�V
	/// </summary>
	virtual void Update() override;

	/// <summary>
	/// �`��
	/// </summary>
	/// <param name="cmdList">�R�}���h���X�g</param>
	virtual void Draw(
		ID3D12GraphicsCommandList* cmdList
	) override;

	/// <summary>
	/// �폜
	/// </summary>
	virtual void Terminate() override;

public:
	/// <summary>
	/// ����
	/// </summary>
	/// <param name="dev">dx�f�o�C�X</param>
	/// <returns>�C���X�^���X</returns>
	static DeltaCone* Create(
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

	virtual HRESULT CreateConstView() override;
};


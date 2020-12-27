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
	static const int u_max = 32; //�c�����̕�����
	static const int v_max = 16; //�������̕�����
	static const int vertexCount = u_max * (v_max + 1);    //���_��
	static const int indexCount = 2 * v_max * (u_max + 1); //�C���f�b�N�X��

private:
	//DirectX�f�o�C�X
	ID3D12Device* device = nullptr;

	//���̒��_�f�[�^
	VertexData vertices[vertexCount];
	//���̃C���f�b�N�X�f�[�^
	unsigned short indices[indexCount];

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
	Sphere() {};

	//�R�s�[�E����֎~
	Sphere(const Sphere&) = delete;
	void operator=(const Sphere&) = delete;

public:
	///�f�X�g���N�^
	~Sphere();

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
	/// <param name="cmdList"></param>
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
	/// <param name="dev">�f�o�C�X</param>
	/// <returns>�C���X�^���X</returns>
	static Sphere* Create(
		ID3D12Device* dev
	);

private:
	/// <summary>
	/// ���_�o�b�t�@����
	/// </summary>
	/// <returns>�����̊m�F</returns>
	virtual HRESULT CreateVertBuff() override;

	/// <summary>
	/// �C���f�b�N�X�o�b�t�@����
	/// </summary>
	/// <returns>�����̊m�F</returns>
	virtual HRESULT CreateIndexBuff() override;

	/// <summary>
	/// �V�F�[�_�[�t�@�C���̃R���p�C��
	/// </summary>
	/// <returns>�����̊m�F</returns>
	virtual HRESULT ShaderCompile() override;

	/// <summary>
	/// ���[�g�V�O�l�`������
	/// </summary>
	/// <returns>�����̊m�F</returns>
	virtual HRESULT CreateRootSignature() override;

	/// <summary>
	/// �O���t�B�b�N�p�C�v���C������
	/// </summary>
	/// <returns>�����̊m�F</returns>
	virtual HRESULT CreateGraphicsPipeline() override;

	/// <summary>
	/// �f�X�N���v�^�q�[�v����
	/// </summary>
	/// <returns>�����̊m�F</returns>
	virtual HRESULT CreateDescriptorHeap() override;

	/// <summary>
	/// �e�N�X�`���̓ǂݍ���
	/// </summary>
	/// <param name="textureName">�e�N�X�`���̃t�@�C����</param>
	/// <returns>�����̊m�F</returns>
	virtual HRESULT LoadTexture(
		const std::string& textureName
	) override;

	/// <summary>
	/// ���̐���
	/// </summary>
	virtual void CreateModel() override;

	/// <summary>
	/// �ϊ��s��̏�����
	/// </summary>
	virtual void InitMatrix() override;

	/// <summary>
	/// �萔�o�b�t�@����
	/// </summary>
	/// <returns>�����̊m�F</returns>
	virtual HRESULT CreateConstBuff() override;

	/// <summary>
	/// �����蔻��
	/// </summary>
	/// <param name="obj">�������Ă���I�u�W�F�N�g</param>
	virtual void Collision(
		const GameObject& obj
	)override;
};


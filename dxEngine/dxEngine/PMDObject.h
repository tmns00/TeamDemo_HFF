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
	//�V�F�[�_�[���ɓn�����߂̊�{�I�ȍs��f�[�^
	struct SceneMatrix
	{
		XMMATRIX world;      //���f���{�̂̉�]�E�ړ��s��
		XMMATRIX view;       //�r���[�s��
		XMMATRIX proj;       //�v���W�F�N�V�����s��
		XMMATRIX view_proj;  //�r���[�E�v���W�F�N�V�����s��
		XMFLOAT3 eye;        //���_���W
		XMMATRIX bones[256]; //�{�[���s��
	};

	//PMD�w�b�_�[�\����
	struct PMDHeader
	{
		float version;       //��F00 00 80 3f == 1.00
		char model_name[20]; //���f����
		char comment[256];   //���f���R�����g
	};

	//PMD���_�\����
	struct PMDVertex
	{
		XMFLOAT3 pos;             //���_���W�F12�o�C�g
		XMFLOAT3 normal;          //�@���x�N�g���F12�o�C�g
		XMFLOAT2 uv;              //uv���W�F8�o�C�g
		unsigned short boneNo[2]; //�{�[���ԍ��F4�o�C�g
		unsigned char boneWeight; //�{�[���e���x�F1�o�C�g
		unsigned char edgeFlg;    //�֊s���t���O�F1�o�C�g
	};

#pragma pack(1)
	//PMD�}�e���A���\����
	struct PMDMaterial
	{
		XMFLOAT3 diffuse;        //�f�B�t���[�Y�F
		float alpha;             //�f�B�t���[�Y��
		float specularity;       //�X�y�L�����[�̋���
		XMFLOAT3 specular;       //�X�y�L�����[�F
		XMFLOAT3 ambient;        //�A���r�G���g�F
		unsigned char toonIdx;   //�g�D�[���ԍ�
		unsigned char edgeFlg;   //�}�e���A�����Ƃ̗֊s���t���O
		unsigned int indicesNum; //���̃}�e���A�������蓖�Ă���@�C���f�b�N�X��
		char texFilePath[20];    //�e�N�X�`���t�@�C���p�X�{��
	};
#pragma pack()

	//�V�F�[�_�[�ɑ���}�e���A���f�[�^
	struct MaterialForHlsl
	{
		XMFLOAT3 diffuse;  //�f�B�t���[�Y�F
		float alpha;       //�f�B�t���[�Y��
		XMFLOAT3 specular; //�X�y�L�����[�F
		float specularity; //�X�y�L�����[��
		XMFLOAT3 ambient;  //�A���r�G���g�F
	};

	//��L�ȊO�̃}�e���A���f�[�^
	struct AdditionalMaterial
	{
		std::string texPath; //�e�N�X�`���t�@�C���p�X
		int toonIdx = 0;         //�g�D�[���ԍ�
		bool edgeFlg = false;        //�}�e���A�����Ƃ̗֊s���t���O
	};

	//�S�̂��܂Ƃ߂�f�[�^
	struct Material
	{
		unsigned int indicesNum = 0;        //�C���f�b�N�X��
		MaterialForHlsl material;       //
		AdditionalMaterial addMaterial; //
	};

#pragma pack(1)
	//�ǂݍ��ݗp�{�[���\����
	struct PMDBone
	{
		char boneName[20];       //�{�[����
		unsigned short parentNo; //�e�{�[���ԍ�
		unsigned short nextNo;   //��[�̃{�[���ԍ�
		unsigned char type;      //�{�[�����
		unsigned short ikBoneNo; //IK�{�[���ԍ�
		XMFLOAT3 pos;            //�{�[���̊�_���W
	};
#pragma pack()

	//�{�[���m�[�h�\����
	struct BoneNode
	{
		uint32_t boneIndex;              //�{�[���C���f�b�N�X
		uint32_t boneType;               //�{�[�����
		uint32_t parentBone;             //�e�{�[��
		uint32_t ikParentBone;           //IK�e�{�[��
		XMFLOAT3 startPos;               //�{�[����_(��]���S)
		std::vector<BoneNode*> children; //�q�m�[�h
	};

	//IK�f�[�^�\����
	struct PMDIK
	{
		uint16_t boneIndex;                //IK�Ώۂ̃{�[��������
		uint16_t targetIndex;              //�^�[�Q�b�g�ɋ߂Â��邽�߂̃{�[���̃C���f�b�N�X
		uint16_t iterations;               //���s��
		float limit;                       //1�񂠂���̉�]����
		std::vector<uint16_t> nodeIndices; //�Ԃ̃m�[�h�ԍ�
	};

private:
	//pmd�t�@�C���p�X
	std::string strModelPath;

private:
	//�f�o�C�X
	ComPtr<ID3D12Device> device = nullptr;

	//���_�z��f�[�^
	std::vector<unsigned char> vertices{};
	//�C���f�b�N�X�z��f�[�^
	std::vector<unsigned short> indices{};

	//���_��
	unsigned int vertNum = 0;
	//���_�o�b�t�@
	ComPtr<ID3D12Resource> vertBuff = nullptr;
	//���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	//�C���f�b�N�X��
	unsigned int indicesNum = 0;
	//�C���f�b�N�X�o�b�t�@
	ComPtr<ID3D12Resource> idxBuff = nullptr;
	//�C���f�b�N�X�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	//�f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> basicDescHeap = nullptr;

	//�萔�p�f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> constDescHeap = nullptr;

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

	//PMD�w�b�_�[�I�u�W�F�N�g
	PMDHeader pmdheader{};
	//�}�e���A���f�[�^�z��
	std::vector<Material> materials{};
	//�}�e���A����
	unsigned int materialNum = 0;
	//�}�e���A���f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> materialDescHeap = nullptr;
	//�e�N�X�`���o�b�t�@�z��
	std::vector<ComPtr<ID3D12Resource>> textureResources{};
	//�X�t�B�A�}�b�v�o�b�t�@�z��
	std::vector<ComPtr<ID3D12Resource>> sphereResources{};
	//���Z�X�t�B�A�}�b�v�o�b�t�@�z��
	std::vector<ComPtr<ID3D12Resource>> addSphResources{};

	//�{�[�����
	std::vector<PMDBone> pmdBones;
	//�{�[�����z��
	std::vector<XMMATRIX> boneMatrices{};
	//�{�[���m�[�h�e�[�u��
	std::map<std::string, BoneNode> boneNodeTable;
	//
	//std::unordered_map<std::string, std::vector<Motion>> motionData;
	//�}�b�v��̃|�C���^�[
	SceneMatrix* mapMatrix;
	//�A�j���[�V�����J�n���̃~���b
	DWORD startTime = 0;
	//�ő�t���[���ԍ�
	//unsigned int duration = 0;
	//�R�}���h���X�g
	ID3D12GraphicsCommandList* cmdList = nullptr;

	//IK�f�[�^�̊i�[�z��
	std::vector<PMDIK> pmdIkData;
	//�C���f�b�N�X���疼�O���������₷���悤��
	std::vector<std::string> boneNameArray;
	//�C���f�b�N�X����m�[�h���������₷���悤��
	std::vector<BoneNode*> boneNodeAddressArray;
	//
	std::vector<uint32_t> kneeIndices;
	//IK�I���I�t�f�[�^�̕ێ��R���e�i
	//std::vector<VMDIKEnable>ikEnableData;

	//VMD���[�h�N���X
	VMDLoader* vmdLoader;
	//
	MotionDatas loadMotion;

private:
	PMDObject() {};

	//�R�s�[�E����֎~
	PMDObject(const PMDObject&) = delete;
	void operator=(const PMDObject&) = delete;

public:
	~PMDObject();

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw(ID3D12GraphicsCommandList* cmdList) override;
	virtual void Terminate() override;

	/// <summary>
	/// �A�j���[�V�����J�n
	/// </summary>
	void PlayAnimation(std::string key);

	/// <summary>
	/// �A�j���[�V������~
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
	/// VMD�f�[�^�ǂݍ���
	/// </summary>
	void LoadVMDFile(std::string key);

	/// <summary>
	/// ���[�V�����X�V
	/// </summary>
	void MotionUpdate();

	/// <summary>
	/// �}�e���A���o�b�t�@�E�r���[�̍쐬
	/// </summary>
	/// <returns></returns>
	HRESULT CreateMaterialBuff();

	/// <summary>
	/// pmd���f������e�N�X�`���̃��[�h
	/// </summary>
	/// <param name="texPath">pmd�t�@�C���p�X</param>
	/// <returns>�e�N�X�`���o�b�t�@</returns>
	ID3D12Resource* LoadTextureFromFile(
		std::string& texPath
	);

	/// <summary>
	/// ���e�N�X�`��
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* CreateWhiteTexture();

	/// <summary>
	/// ���e�N�X�`��
	/// </summary>
	/// <returns></returns>
	ID3D12Resource* CreateBlackTexture();

	/// <summary>
	/// ���f���̃p�X�ƃe�N�X�`���̃p�X���獇���p�X�𓾂�
	/// �t�H���_�Z�p���[�^2��ނɑΉ�
	/// </summary>
	/// <param name="modelPath">�A�v���P�[�V���������pmd���f���̃p�X</param>
	/// <param name="texPath">pmd���f������̃e�N�X�`���̃p�X</param>
	/// <returns>�A�v������̃e�N�X�`���̃p�X</returns>
	std::string GetTexturePathFromModelAndTexPath(
		const std::string& modelPath,
		const char* texPath
	);

	/// <summary>
	/// �}���`�o�C�g�����񂩂烏�C�h������𓾂�
	/// </summary>
	/// <param name="str">�}���`�o�C�g������</param>
	/// <returns>�ϊ����ꂽ���C�h������</returns>
	std::wstring GetWideStringFromString(
		const std::string& str
	);

	/// <summary>
	/// �t�@�C��������g���q���擾����
	/// </summary>
	/// <param name="path">�Ώۂ̃p�X</param>
	/// <returns>�g���q</returns>
	std::string GetExtension(
		const std::string& path
	);

	/// <summary>
	/// �e�N�X�`���̃p�X���Z�p���[�^�����ŕ�������
	/// </summary>
	/// <param name="path">�Ώۂ̃p�X</param>
	/// <param name="splitter">�Z�p���[�^����</param>
	/// <returns>�����O��̕�����̃y�A</returns>
	std::pair<std::string, std::string> SplitFilePath(
		const std::string& path,
		const char splitter = '*'
	);

	/// <summary>
	/// �{�[���̕ϊ��s��̍ċA����
	/// </summary>
	/// <param name="node">�m�[�h</param>
	/// <param name="mat">�ϊ��s��</param>
	void RecursiveMatrixMultiply(
		BoneNode* node,
		const XMMATRIX& mat
	);

	/// <summary>
	/// �x�W�F�Ȑ���ԃ��\�b�h
	/// </summary>
	float GetYFromXOnBezier(
		float x,
		const XMFLOAT2& a,
		const XMFLOAT2& b,
		uint8_t n
	);

	/// <summary>
	/// IK�̎�ނ�I������
	/// </summary>
	void PickUpIKSolve(
		int frameNo
	);

	/// <summary>
	/// CCD-IK�ɂ���ă{�[������������
	/// </summary>
	/// <param name="ik">�Ώۂ�IK�I�u�W�F�N�g</param>
	void SolveCCDIK(
		const PMDIK& ik
	);

	/// <summary>
	/// �]���藝IK�ɂ���ă{�[������������
	/// </summary>
	/// <param name="ik">�Ώۂ�IK�I�u�W�F�N�g</param>
	void SolveCosineIK(
		const PMDIK& ik
	);

	/// <summary>
	/// LookAt�s��ɂ���ă{�[������������
	/// </summary>
	/// <param name="ik">�Ώۂ�IK�I�u�W�F�N�g</param>
	void SolveLookAt(
		const PMDIK& ik
	);

	/// <summary>
	/// z�������̕����Ɍ�����s���Ԃ��֐�
	/// </summary>
	/// <param name="lookat">���������������̃x�N�g��</param>
	/// <param name="up">��x�N�g��</param>
	/// <param name="right">�E�x�N�g��</param>
	/// <returns></returns>
	XMMATRIX LookAtMatrix(
		const XMVECTOR& lookat,
		XMFLOAT3& up,
		XMFLOAT3& right
	);

	/// <summary>
	/// ����̃x�N�g�������̕����Ɍ����邽�߂̍s���Ԃ�
	/// </summary>
	/// <param name="origin">����̃x�N�g��</param>
	/// <param name="lookat">��������������</param>
	/// <param name="up">��x�N�g��</param>
	/// <param name="right">�E�x�N�g��</param>
	/// <returns>����̃x�N�g�������̕����Ɍ����邽�߂̍s��</returns>
	XMMATRIX LookAtMatrix(
		const XMVECTOR& origin,
		const XMVECTOR& lookat,
		XMFLOAT3& up,
		XMFLOAT3& right
	);
};


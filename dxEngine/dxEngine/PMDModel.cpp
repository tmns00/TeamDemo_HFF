#include<d3dcompiler.h>
#include<string>
#include"PMDModel.h"

#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

//���_1������̃T�C�Y
const size_t pmdvertex_size = 38;
//pmd�t�@�C���p�X
const std::string strModelPath = "Resources/Model/�����~�Nmetal.pmd";

void PMDModel::Initialize(
	ID3D12Device* device
) {
	//nullptr�`�F�b�N
	assert(device);

	PMDModel::device = device;

	CreatePMDModel();

	if (FAILED(CreateVertBuff())) {
		assert(0);
	}

	if (FAILED(CreateIdxBuff())) {
		assert(0);
	}

	if (FAILED(CreateMaterialBuff())) {
		assert(0);
	}

	if (FAILED(ShaderCompile())) {
		assert(0);
	}

	if (FAILED(CreateRootSignature())) {
		assert(0);
	}

	if (FAILED(CreateGraphicsPipeline())) {
		assert(0);
	}

	InitMatrix();

	if (FAILED(CreateConstBuff())) {
		assert(0);
	}

	if (FAILED(CreateConstView())) {
		assert(0);
	}
}

void PMDModel::Update()
{
	HRESULT res = S_FALSE;
	XMMATRIX matScale, matRot, matTrans;

	// �X�P�[���A��]�A���s�ړ��s��̌v�Z
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	// ���[���h�s��̍���
	worldMat = XMMatrixIdentity(); // �ό`�����Z�b�g

	worldMat *= matScale; // ���[���h�s��ɃX�P�[�����O�𔽉f
	worldMat *= matRot; // ���[���h�s��ɉ�]�𔽉f
	worldMat *= matTrans; // ���[���h�s��ɕ��s�ړ��𔽉f

	//�s���萔�o�b�t�@�ɃR�s�[
	SceneMatrix* mapMatrix; //�}�b�v��̃|�C���^�[
	res = constBuff->Map(
		0,
		nullptr,
		(void**)&mapMatrix
	);
	if (FAILED(res)) {
		assert(0);
	}
	//�s��̃R�s�[
	mapMatrix->world = worldMat;
	mapMatrix->view = viewMat;
	mapMatrix->proj = projectionMat;
	mapMatrix->view_proj = viewMat * projectionMat;
	mapMatrix->eye = eye;
	constBuff->Unmap(0, nullptr);
}

void PMDModel::Draw()
{
	// nullptr�`�F�b�N
	assert(device);
	assert(PMDModel::cmdList);

	//���_���̃Z�b�g
	cmdList->IASetVertexBuffers(0, 1, &vbView);
	//�C���f�b�N�X���̃Z�b�g
	cmdList->IASetIndexBuffer(&ibView);


	//�ϊ��s��̃Z�b�g
	ID3D12DescriptorHeap* constDH = constDescHeap.Get();
	cmdList->SetDescriptorHeaps(1, &constDH);
	cmdList->SetGraphicsRootDescriptorTable(
		0,
		constDescHeap->GetGPUDescriptorHandleForHeapStart()
	);

	//�}�e���A��
	ID3D12DescriptorHeap* matDH = materialDescHeap.Get();
	cmdList->SetDescriptorHeaps(1, &matDH);
	//�q�[�v�擪
	CD3DX12_GPU_DESCRIPTOR_HANDLE materialH = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		materialDescHeap->GetGPUDescriptorHandleForHeapStart()
	);
	unsigned int idxOffset = 0; //�ŏ��̓I�t�Z�b�g�Ȃ�
	auto cbvSrvIncSize = device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	) * 4; //4�{

	for (auto& m : materials)
	{
		cmdList->SetGraphicsRootDescriptorTable(
			1,
			materialH
		);
		cmdList->DrawIndexedInstanced(
			m.indicesNum,
			1,
			idxOffset,
			0,
			0
		);
		//�q�[�v�|�C���^�[���C���f�b�N�X�����ɐi�߂�
		materialH.ptr += cbvSrvIncSize;
		idxOffset += m.indicesNum;
	}
}

void PMDModel::DrawBefore(ID3D12GraphicsCommandList* cmdList)
{
	// PreDraw��PostDraw���y�A�ŌĂ΂�Ă��Ȃ���΃G���[
	assert(PMDModel::cmdList == nullptr);

	//�R�}���h���X�g���Z�b�g
	PMDModel::cmdList = cmdList;

	// �p�C�v���C���X�e�[�g�̐ݒ�
	cmdList->SetPipelineState(pipelineState.Get());
	// ���[�g�V�O�l�`���̐ݒ�
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	// �v���~�e�B�u�`���ݒ�
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PMDModel::DrawAfter()
{
	// �R�}���h���X�g������
	PMDModel::cmdList = nullptr;
}

void PMDModel::DebugShader(const HRESULT& result)
{
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
		::OutputDebugStringA("�t�@�C������������܂���");
		exit(0);
	}
	else {
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
	}
}

std::string PMDModel::GetTexturePathFromModelAndTexPath(
	const std::string& modelPath,
	const char* texPath
) {
	int pathIdx1 = modelPath.rfind('/');
	int pathIdx2 = modelPath.rfind('\\');

	auto pathIndex = max(pathIdx1, pathIdx2);
	auto folderPath = modelPath.substr(
		0,
		pathIndex + 1
	);
	return folderPath + texPath;
}

std::wstring PMDModel::GetWideStringFromString(
	const std::string& str
) {
	//�Ăяo��1��� ������̌����擾
	auto num1 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		nullptr,
		0
	);
	std::wstring wstr; //string��wchar_t��
	wstr.resize(num1);

	//�Ăяo��2��� �������m�ۂ���wstr�ɕ�������R�s�[
	auto num2 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		&wstr[0],
		num1
	);

	assert(num1 == num2); //��������v���Ă��邩�m�F
	return wstr;
}

std::string PMDModel::GetExtension(
	const std::string& path
) {
	int index = path.rfind('.');
	return path.substr(
		index + 1,
		path.length() - index - 1
	);
}

std::pair<std::string, std::string> PMDModel::SplitFilePath(
	const std::string& path,
	const char splitter
) {
	int index = path.find(splitter);
	std::pair<std::string, std::string> ret;
	ret.first = path.substr(
		0,
		index
	);
	ret.second = path.substr(
		index + 1,
		path.length() - index - 1
	);

	return ret;
}

HRESULT PMDModel::CreateVertBuff()
{
	HRESULT res = S_FALSE;

	//���_�o�b�t�@
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOAD�q�[�v�Ƃ���
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertices.size()), //�T�C�Y�ɉ����ēK�؂Ȑݒ�����Ă����
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}
	res = S_FALSE;
	//�o�b�t�@�ɒ��_�f�[�^�R�s�[
	unsigned char* vertMap = nullptr;
	res = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}
	copy(vertices.begin(), vertices.end(), vertMap);
	vertBuff->Unmap(0, nullptr);
	//���_�o�b�t�@�r���[
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); //�o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = static_cast<UINT>(vertices.size());
	vbView.StrideInBytes = pmdvertex_size; //1���_������̃o�C�g��

	return res;
}

HRESULT PMDModel::CreateIdxBuff()
{
	HRESULT res = S_FALSE;

	//�C���f�b�N�X�o�b�t�@
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOAD�q�[�v�Ƃ���
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(
			indices.size() * sizeof(indices[0])
		), //�T�C�Y�ɉ����ēK�؂Ȑݒ�����Ă����,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff)
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}
	res = S_FALSE;
	//������o�b�t�@�ɃC���f�b�N�X�f�[�^�R�s�[
	unsigned short* mappedIdx = nullptr;
	res = idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}
	copy(indices.begin(), indices.end(), mappedIdx);
	idxBuff->Unmap(0, nullptr);
	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(indices[0]));

	return res;
}

HRESULT PMDModel::ShaderCompile()
{
	HRESULT res = S_FALSE;

	res = D3DCompileFromFile(
		L"PMDVertexShader.hlsl",                       //�V�F�[�_�[��
		nullptr,                                         //define�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,               //�C���N���[�h�̓f�t�H���g
		"VSmain", "vs_5_0",                              //�֐���VSmain,�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //�f�o�b�O�p & �œK���Ȃ�
		0,
		&vsBlob, &errorBlob);                          //�G���[����errorBlob�Ƀ��b�Z�[�W������
	if (FAILED(res))
		DebugShader(res);

	res = S_FALSE;
	res = D3DCompileFromFile(
		L"PMDPixelShader.hlsl",                        //�V�F�[�_�[��
		nullptr,                                         //define�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,               //�C���N���[�h�̓f�t�H���g
		"PSmain", "ps_5_0",                              //�֐���PSmain,�ΏۃV�F�[�_�[��ps_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //�f�o�b�O�p & �œK���Ȃ�
		0,
		&psBlob, &errorBlob);                          //�G���[����errorBlob�Ƀ��b�Z�[�W������
	if (FAILED(res))
		DebugShader(res);

	return res;
}

HRESULT PMDModel::CreateRootSignature()
{
	HRESULT res = S_FALSE;

	//�f�X�N���v�^�����W
	CD3DX12_DESCRIPTOR_RANGE descTableRanges[3] = {}; //�e�N�X�`���ƒ萔2��
	//�萔�p���W�X�^�[0�� �s��
	descTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//�萔�p���W�X�^�[1�� �}�e���A��
	descTableRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	//�e�N�X�`���p���W�X�^�[0�� �e�N�X�`��3�� ��{ sph spa
	descTableRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

	//���[�g�p�����[�^�[
	CD3DX12_ROOT_PARAMETER rootParams[2] = {};
	//�萔
	rootParams[0].InitAsDescriptorTable(1, &descTableRanges[0]);
	//�}�e���A��
	rootParams[1].InitAsDescriptorTable(2, &descTableRanges[1]);

	//�T���v���[
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Init(0);

	//���[�g�V�O�l�`���̍쐬
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(
		_countof(rootParams), //���[�g�p�����[�^�[��
		rootParams,           //���[�g�p�����[�^�[�̐擪�A�h���X
		1,                    //�T���v���[��
		&samplerDesc,         //�T���v���[�̐擪�A�h���X
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);
	//�o�C�i���R�[�h�̍쐬
	res = D3D12SerializeRootSignature(
		&rootSignatureDesc,             //���[�g�V�O�l�`���ݒ�
		D3D_ROOT_SIGNATURE_VERSION_1_0, //���[�g�V�O�l�`���o�[�W����
		&rootSigBlob,                   //�V�F�[�_�[����������Ɠ���
		&errorBlob                     //�G���[����
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	res = S_FALSE;
	//���[�g�V�O�l�`���I�u�W�F�N�g�̐���
	res = device->CreateRootSignature(
		0,                               //nodemask
		rootSigBlob->GetBufferPointer(), //�V�F�[�_�[�̂Ƃ��Ɠ��l
		rootSigBlob->GetBufferSize(),    //�V�F�[�_�[�̂Ƃ��Ɠ��l
		IID_PPV_ARGS(&rootSignature)
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}
	rootSigBlob->Release(); //�s�v�ɂȂ����̂ŉ��

	return res;
}

HRESULT PMDModel::CreateGraphicsPipeline()
{
	HRESULT res = S_FALSE;

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ //���W���
			"POSITION",0,
			DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{ //�@��
			"NORMAL",0,
			DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{ //uv
			"TEXCOORD",0,
			DXGI_FORMAT_R32G32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{ //�{�[���ԍ�
			"BONE_NO",0,
			DXGI_FORMAT_R16G16_UINT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{ //�{�[���E�F�C�g
			"WEIGHT",0,
			DXGI_FORMAT_R8_UINT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{ //�֊s���t���O
			"EDGE_FLG",0,
			DXGI_FORMAT_R8_UINT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipeline = {};
	//�V�O�l�`��
	gPipeline.pRootSignature = rootSignature.Get();
	//���_�V�F�[�_�[
	gPipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	//�s�N�Z���V�F�[�_�[
	gPipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	//�T���v���}�X�N
	gPipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; //�f�t�H���g
	//���X�^���C�U�[�X�e�[�g
	gPipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); //�f�t�H���g
	gPipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //�J�����O���Ȃ�
	//�[�x�ݒ�
	gPipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gPipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//�u�����h�X�e�[�g
	gPipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//���̓��C�A�E�g
	gPipeline.InputLayout.pInputElementDescs = inputLayout; //���C�A�E�g�擪�A�h���X
	gPipeline.InputLayout.NumElements = _countof(inputLayout); //���C�A�E�g�z��̗v�f��
	//���_�̐؂藣���ݒ�
	gPipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; //�J�b�g�Ȃ�
	//�v���~�e�B�u�g�|���W�[
	gPipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; //�O�p�`�ō\��
	//�����_�[�^�[�Q�b�g
	gPipeline.NumRenderTargets = 1; //1��
	gPipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0~1�ɐ��K�����ꂽRGBA
	//�A���`�G�C���A�V���O
	gPipeline.SampleDesc.Count = 1; //�T���v�����O��1�s�N�Z���ɂ�1
	gPipeline.SampleDesc.Quality = 0; //�N�I���e�B�͍Œ�

	//�O���t�B�b�N�p�C�v���C���X�e�[�g�I�u�W�F�N�g����
	res = device->CreateGraphicsPipelineState(
		&gPipeline,
		IID_PPV_ARGS(&pipelineState)
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	return res;
}

void PMDModel::CreatePMDModel()
{
	//PMD���f���f�[�^
	char signature[3] = {}; //�V�O�l�`��
	auto fp = fopen(strModelPath.c_str(), "rb");

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	//���_�̓ǂݍ���
	fread(&vertNum, sizeof(vertNum), 1, fp);
	vertices.resize(vertNum * pmdvertex_size); //�o�b�t�@�̊m��
	fread(vertices.data(), vertices.size(), 1, fp); //�ǂݍ���
	//�C���f�b�N�X�̓ǂݍ���
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	indices.resize(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);
	//�}�e���A���ǂݍ���
	fread(&materialNum, sizeof(materialNum), 1, fp);
	std::vector<PMDMaterial> pmdMaterials(materialNum);
	fread(
		pmdMaterials.data(),
		pmdMaterials.size() * sizeof(PMDMaterial),
		1,
		fp
	);

	materials.resize(pmdMaterials.size());
	//�R�s�[
	for (int i = 0; i < pmdMaterials.size(); ++i) {
		materials[i].indicesNum = pmdMaterials[i].indicesNum;
		materials[i].material.diffuse = pmdMaterials[i].diffuse;
		materials[i].material.alpha = pmdMaterials[i].alpha;
		materials[i].material.specular = pmdMaterials[i].specular;
		materials[i].material.specularity = pmdMaterials[i].specularity;
		materials[i].material.ambient = pmdMaterials[i].ambient;
	}

	textureResources.resize(materialNum);
	sphereResources.resize(materialNum);
	addSphResources.resize(materialNum);
	//���\�[�X�̃��[�h
	for (int i = 0; i < pmdMaterials.size(); ++i) {
		if (strlen(pmdMaterials[i].texFilePath) == 0) {
			textureResources[i] = nullptr;
		}

		//�e�N�X�`���p�X�̕������m�F
		std::string texFileName = pmdMaterials[i].texFilePath;
		std::string sphereFileName = "";
		std::string addSphFileName = "";
		if (count(texFileName.begin(), texFileName.end(), '*') > 0) {
			//�X�v���b�^������
			auto namepair = SplitFilePath(texFileName);
			if (GetExtension(namepair.first) == "sph") {
				texFileName = namepair.second;
				sphereFileName = namepair.first;
			}
			else if (GetExtension(namepair.first) == "spa") {
				texFileName = namepair.second;
				addSphFileName = namepair.first;
			}
			else {
				texFileName = namepair.first;
				if (GetExtension(namepair.second) == "sph") {
					sphereFileName = namepair.second;
				}
				if (GetExtension(namepair.second) == "spa") {
					addSphFileName = namepair.second;
				}
			}
		}
		else {
			if (GetExtension(pmdMaterials[i].texFilePath) == "sph") {
				sphereFileName = pmdMaterials[i].texFilePath;
				texFileName = "";
			}
			else if (GetExtension(pmdMaterials[i].texFilePath) == "spa") {
				addSphFileName = pmdMaterials[i].texFilePath;
				texFileName = "";
			}
			else {
				texFileName = pmdMaterials[i].texFilePath;
			}
		}

		if (texFileName != "") {
			//���f���ƃe�N�X�`���p�X����A�v���P�[�V��������̃e�N�X�`���p�X�𓾂�
			auto texFilePath = GetTexturePathFromModelAndTexPath(
				strModelPath,
				texFileName.c_str()
			);
			textureResources[i] = LoadTextureFromFile(texFilePath);
		}


		if (sphereFileName != "") {
			//�A�v���P�[�V��������݂��X�t�B�A�}�b�v�̃p�X
			auto sphFilePath = GetTexturePathFromModelAndTexPath(
				strModelPath,
				sphereFileName.c_str()
			);
			sphereResources[i] = LoadTextureFromFile(sphFilePath);
		}

		if (addSphFileName != "") {
			//�A�v���P�[�V��������݂����Z�X�t�B�A�}�b�v�̃p�X
			auto spaFilePath = GetTexturePathFromModelAndTexPath(
				strModelPath,
				addSphFileName.c_str()
			);
			addSphResources[i] = LoadTextureFromFile(spaFilePath);
		}
	}

	fclose(fp);
}

void PMDModel::InitMatrix()
{
	//���[���h�s��
	worldMat = XMMatrixIdentity();
	//�r���[�s��
	viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));
	//�v���W�F�N�V�����s��
	projectionMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2, //��p90��
		1280 / 720, //�A�X�y�N�g��
		1.0f, //near
		1000.0f //far
	);
}

HRESULT PMDModel::CreateConstBuff()
{
	HRESULT res = S_FALSE;

	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneMatrix) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff)
	);

	return res;
}

HRESULT PMDModel::CreateConstView()
{
	HRESULT res = S_FALSE;

	//�f�X�N���v�^�q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC constDescHeapDesc = {};
	constDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //�V�F�[�_�[���猩����悤��
	constDescHeapDesc.NodeMask = 0; //�}�X�N��0
	constDescHeapDesc.NumDescriptors = 1; //CBV1��
	constDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; //�V�F�[�_�[���\�[�X�r���[�p
	res = device->CreateDescriptorHeap(
		&constDescHeapDesc,
		IID_PPV_ARGS(&constDescHeap)
	);

	//�擪�n���h�����擾
	auto constHeapHandle = constDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(constBuff->GetDesc().Width);

	//�萔�o�b�t�@�r���[�쐬
	device->CreateConstantBufferView(&cbvDesc, constHeapHandle);

	return res;
}

HRESULT PMDModel::CreateMaterialBuff()
{
	HRESULT res = S_FALSE;

	//�}�e���A���o�b�t�@���쐬
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	ID3D12Resource* materialBuff = nullptr;
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(
			materialBuffSize * materialNum
		),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&materialBuff)
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	res = S_FALSE;
	//�}�b�v�}�e���A���ɃR�s�[
	char* mapMaterial = nullptr;
	res = materialBuff->Map(
		0,
		nullptr,
		(void**)&mapMaterial
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}
	for (auto& m : materials) {
		*((MaterialForHlsl*)mapMaterial) = m.material; //�f�[�^�R�s�[
		mapMaterial += materialBuffSize; //���̃A���C�����g�ʒu�܂Ői�߂�
	}
	materialBuff->Unmap(0, nullptr);

	res = S_FALSE;
	//�f�X�N���v�^�q�[�v�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC matDescHeapDesc = {};
	matDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matDescHeapDesc.NodeMask = 0;
	matDescHeapDesc.NumDescriptors = materialNum * 4; //�}�e���A�������w��
	matDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	res = device->CreateDescriptorHeap(
		&matDescHeapDesc,
		IID_PPV_ARGS(&materialDescHeap)
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	//�r���[�̍쐬
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress(); //�o�b�t�@�A�h���X
	matCBVDesc.SizeInBytes = static_cast<UINT>(materialBuffSize); //�}�e���A����256�A���C�����g�T�C�Y

	//�ʏ�e�N�X�`���r���[�̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //�f�t�H���g
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1; //�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

	//�擪�A�h���X���L�^
	CD3DX12_CPU_DESCRIPTOR_HANDLE matDescHeapH = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		materialDescHeap->GetCPUDescriptorHandleForHeapStart()
	);

	auto increment = device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	ComPtr<ID3D12Resource> whiteTex = CreateWhiteTexture();
	ComPtr<ID3D12Resource> blackTex = CreateBlackTexture();
	for (int i = 0; i < materialNum; ++i) {
		//�}�e���A���p�萔�o�b�t�@�r���[
		device->CreateConstantBufferView(
			&matCBVDesc,
			matDescHeapH
		);
		matDescHeapH.ptr += increment;
		matCBVDesc.BufferLocation += materialBuffSize;

		//�V�F�[�_�[���\�[�X�r���[
		if (textureResources[i] == nullptr) {
			srvDesc.Format = whiteTex->GetDesc().Format;
			device->CreateShaderResourceView(
				whiteTex.Get(),
				&srvDesc,
				matDescHeapH
			);
		}
		else {
			srvDesc.Format = textureResources[i]->GetDesc().Format;
			device->CreateShaderResourceView(
				textureResources[i].Get(),
				&srvDesc,
				matDescHeapH
			);
		}
		matDescHeapH.ptr += increment;

		//�X�t�B�A�}�b�v���\�[�X�r���[
		if (sphereResources[i] == nullptr) {
			srvDesc.Format = whiteTex->GetDesc().Format;
			device->CreateShaderResourceView(
				whiteTex.Get(),
				&srvDesc,
				matDescHeapH
			);
		}
		else {
			srvDesc.Format = sphereResources[i]->GetDesc().Format;
			device->CreateShaderResourceView(
				sphereResources[i].Get(),
				&srvDesc,
				matDescHeapH
			);
		}
		matDescHeapH.ptr += increment;

		//���Z�X�t�B�A�}�b�v���\�[�X�r���[
		if (addSphResources[i] == nullptr) {
			srvDesc.Format = blackTex->GetDesc().Format;
			device->CreateShaderResourceView(
				blackTex.Get(),
				&srvDesc,
				matDescHeapH
			);
		}
		else {
			srvDesc.Format = addSphResources[i]->GetDesc().Format;
			device->CreateShaderResourceView(
				addSphResources[i].Get(),
				&srvDesc,
				matDescHeapH
			);
		}
		matDescHeapH.ptr += increment;
	}

	return res;
}

ID3D12Resource* PMDModel::LoadTextureFromFile(
	std::string& texPath
) {
	//WIC�e�N�X�`���̃��[�h
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};

	//�e�N�X�`���̃t�@�C���p�X
	auto wTexPath = GetWideStringFromString(texPath);
	//�g���q���擾
	auto ext = GetExtension(texPath);

	HRESULT res = S_FALSE;

	//�g���q�ɂ���Ċ֐��g������
	if (ext == "sph" ||
		ext == "spa" ||
		ext == "bmp" ||
		ext == "png" ||
		ext == "jpg"
		) {
		res = LoadFromWICFile(
			wTexPath.c_str(),
			WIC_FLAGS_NONE,
			&metadata,
			scratchImg
		);
		if (FAILED(res)) {
			assert(0);
			return nullptr;
		}
	}
	else if (ext == "tga") {
		res = LoadFromTGAFile(
			wTexPath.c_str(),
			&metadata,
			scratchImg
		);
		if (FAILED(res)) {
			assert(0);
			return nullptr;
		}
	}
	else if (ext == "dds") {
		res = LoadFromDDSFile(
			wTexPath.c_str(),
			0,
			&metadata,
			scratchImg
		);
		if (FAILED(res)) {
			assert(0);
			return nullptr;
		}
	}

	if (ext == "") {
		assert(0);
		return nullptr;
	}


	auto img = scratchImg.GetImage(0, 0, 0); //���f�[�^���o

	CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,             //��
		(UINT)metadata.height,      //����
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	//�o�b�t�@�쐬
	ID3D12Resource* texBuff = nullptr;
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(
			D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
			D3D12_MEMORY_POOL_L0
		), //WriteToSubresource�œ]������p�̃q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE, //���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texBuff)
	);
	if (FAILED(res)) {
		return nullptr;
	}

	res = texBuff->WriteToSubresource(
		0,
		nullptr,        //�S�̈�փR�s�[
		img->pixels,    //���f�[�^�T�C�Y
		img->rowPitch,  //���C���T�C�Y
		img->slicePitch //�S�T�C�Y
	);
	if (FAILED(res)) {
		return nullptr;
	}

	return texBuff;
}

ID3D12Resource* PMDModel::CreateWhiteTexture()
{
	//���e�N�X�`���̃o�b�t�@����
	ID3D12Resource* whiteBuff = nullptr;

	auto res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(
			D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
			D3D12_MEMORY_POOL_L0
		), //�q�[�v�v���p�e�B
		D3D12_HEAP_FLAG_NONE, //���Ɏw��Ȃ�
		&CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R8G8B8A8_UNORM,
			4, //��
			4  //����
		), //���\�[�X�f�X�N���v�^
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&whiteBuff)
	);
	if (FAILED(res)) {
		return nullptr;
	}

	//���e�N�X�`���f�[�^
	std::vector<unsigned char> data(4 * 4 * 4);
	fill(data.begin(), data.end(), 0xff); //�S��255�Ŗ��߂�

	//�f�[�^�]��
	res = whiteBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size()
	);
	if (FAILED(res)) {
		assert(0);
	}

	return whiteBuff;
}

ID3D12Resource* PMDModel::CreateBlackTexture()
{
	//���e�N�X�`���̃o�b�t�@����
	ID3D12Resource* blackBuff = nullptr;

	auto res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(
			D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
			D3D12_MEMORY_POOL_L0
		), //�q�[�v�v���p�e�B
		D3D12_HEAP_FLAG_NONE, //���Ɏw��Ȃ�
		&CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R8G8B8A8_UNORM,
			4, //��
			4  //����
		), //���\�[�X�f�X�N���v�^
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&blackBuff)
	);
	if (FAILED(res)) {
		return nullptr;
	}

	//���e�N�X�`���f�[�^
	std::vector<unsigned char> data(4 * 4 * 4);
	fill(data.begin(), data.end(), 0x00); //�S��0�Ŗ��߂�

	//�f�[�^�]��
	res = blackBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size()
	);
	if (FAILED(res)) {
		assert(0);
	}

	return blackBuff;
}
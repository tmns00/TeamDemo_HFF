#include "OBJObject.h"
#include"Camera.h"

#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<fstream>
#include<sstream>

using namespace DirectX;

OBJObject::~OBJObject(){
}

void OBJObject::Initialize(){
	assert(device);

	if (FAILED(CreateDescriptorHeap())) {
		assert(0);
	}

	CreateModel();

	if (FAILED(CreateVertBuff())) {
		assert(0);
	}

	if (FAILED(CreateIndexBuff())) {
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

	/*if (FAILED(LoadTexture("Resources/Texture/white1x1.png"))) {
		assert(0);
	}*/

	InitMatrix();

	if (FAILED(CreateConstBuff())) {
		assert(0);
	}
}

void OBJObject::Update(){
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
	matWorld = XMMatrixIdentity(); // �ό`�����Z�b�g
	matWorld *= matScale; // ���[���h�s��ɃX�P�[�����O�𔽉f
	matWorld *= matRot; // ���[���h�s��ɉ�]�𔽉f
	matWorld *= matTrans; // ���[���h�s��ɕ��s�ړ��𔽉f

	// �萔�o�b�t�@�փf�[�^�]��
	ConstBuffDataB0* constMap = nullptr;
	res = constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->matrix = matWorld *
		camera->GetViewMatrix() *
		camera->GetProjectionMatrix();	// �s��̍���
	constBuff->Unmap(0, nullptr);

	// �萔�o�b�t�@�փf�[�^�]��
	ConstBuffDataB1* constMap1 = nullptr;
	res = constBuffB1->Map(
		0,
		nullptr,
		(void**)&constMap1
	);
	constMap1->ambient = material.ambient;
	constMap1->diffuse = material.diffuse;
	constMap1->specular = material.specular;
	constMap1->alpha = material.alpha;
	constBuffB1->Unmap(
		0,
		nullptr
	);
}

void OBJObject::Draw(
	ID3D12GraphicsCommandList* cmdList
){
	// �p�C�v���C���X�e�[�g�̐ݒ�
	cmdList->SetPipelineState(pipelineState.Get());
	// ���[�g�V�O�l�`���̐ݒ�
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	// �v���~�e�B�u�`���ݒ�
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ���_�o�b�t�@�̐ݒ�
	cmdList->IASetVertexBuffers(0, 1, &vbView);
	// �C���f�b�N�X�o�b�t�@�̐ݒ�
	cmdList->IASetIndexBuffer(&ibView);

	// �f�X�N���v�^�q�[�v�̔z��
	ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// �萔�o�b�t�@�r���[���Z�b�g
	cmdList->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(1, constBuffB1->GetGPUVirtualAddress());
	// �V�F�[�_���\�[�X�r���[���Z�b�g
	cmdList->SetGraphicsRootDescriptorTable(2, gpuDescHandleSRV);
	// �`��R�}���h
	cmdList->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);
}

void OBJObject::Terminate(){
	delete this;
}

OBJObject* OBJObject::Create(
	ID3D12Device* dev,
	const std::string& fileName
){
	OBJObject* instance = new OBJObject;
	if (!instance) {
		assert(0);
		return nullptr;
	}
	instance->device = dev;
	instance->useFileName = fileName;

	instance->Initialize();

	return instance;
}

HRESULT OBJObject::CreateVertBuff(){
	HRESULT res = S_FALSE;

	//���_�o�b�t�@�T�C�Y
	UINT sizeVB = (UINT)(sizeof(VertexData) * vertices.size());

	//���_�o�b�t�@
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOAD�q�[�v�Ƃ���
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB), //�T�C�Y�ɉ����ēK�؂Ȑݒ�����Ă����
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);
	if (FAILED(res)) {
		assert(0);
	}
	res = S_FALSE;
	//�o�b�t�@�ɒ��_�f�[�^�R�s�[
	VertexData* vertMap = nullptr;
	res = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(res)) {
		assert(0);
	}
	std::copy(vertices.begin(), vertices.end(), vertMap);
	vertBuff->Unmap(0, nullptr);
	//���_�o�b�t�@�r���[
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); //�o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]); //1���_������̃o�C�g��

	return res;
}

HRESULT OBJObject::CreateIndexBuff(){
	HRESULT res = S_FALSE;

	//�C���f�b�N�X�o�b�t�@�T�C�Y
	UINT sizeIB = (UINT)(sizeof(unsigned short) * indices.size());

	//�C���f�b�N�X�o�b�t�@
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOAD�q�[�v�Ƃ���
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB), //�T�C�Y�ɉ����ēK�؂Ȑݒ�����Ă����,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff)
	);
	if (FAILED(res)) {
		assert(0);
	}

	res = S_FALSE;
	//������o�b�t�@�ɃC���f�b�N�X�f�[�^�R�s�[
	unsigned short* indexMap = nullptr;
	res = idxBuff->Map(0, nullptr, (void**)&indexMap);
	if (FAILED(res)) {
		assert(0);
	}

	std::copy(indices.begin(), indices.end(), indexMap);
	idxBuff->Unmap(0, nullptr);
	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;

	return res;
}

HRESULT OBJObject::ShaderCompile(){
	HRESULT res = S_FALSE;

	//���_�V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	res = D3DCompileFromFile(
		L"OBJVertexShader.hlsl",	// �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"VSmain", "vs_5_0",	// �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&vsBlob, &errorBlob);
	if (FAILED(res)) {
		DebugShader(res);
	}

	//�s�N�Z���V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	res = D3DCompileFromFile(
		L"OBJPixelShader.hlsl",	// �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"PSmain", "ps_5_0",	// �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&psBlob, &errorBlob
	);
	if (FAILED(res)) {
		DebugShader(res);
	}

	return res;
}

HRESULT OBJObject::CreateRootSignature(){
	HRESULT res = S_FALSE;

	//�f�X�N���v�^�����W
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,
		0
	);

	//���[�g�p�����[�^�[
	CD3DX12_ROOT_PARAMETER rootParams[3];
	rootParams[0].InitAsConstantBufferView(
		0,
		0,
		D3D12_SHADER_VISIBILITY_ALL
	);
	rootParams[1].InitAsConstantBufferView(
		1,
		0,
		D3D12_SHADER_VISIBILITY_ALL
	);
	rootParams[2].InitAsDescriptorTable(
		1,
		&descRangeSRV,
		D3D12_SHADER_VISIBILITY_ALL
	);

	//�X�^�e�B�b�N�T���v���[
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	//���[�g�V�O�l�`���̐ݒ�
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(
		_countof(rootParams), //���[�g�p�����[�^�[��
		rootParams,           //���[�g�p�����[�^�[�̐擪�A�h���X
		1,                    //�T���v���[��
		&samplerDesc,         //�T���v���[�̐擪�A�h���X
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	//�o�C�i���R�[�h�̍쐬
	res = D3DX12SerializeVersionedRootSignature(
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

HRESULT OBJObject::CreateGraphicsPipeline(){
	HRESULT res = S_FALSE;

	// ���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xy���W(1�s�ŏ������ق������₷��)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // �@���x�N�g��(1�s�ŏ������ق������₷��)
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // uv���W(1�s�ŏ������ق������₷��)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
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

HRESULT OBJObject::CreateDescriptorHeap(){
	HRESULT res = S_FALSE;

	// �f�X�N���v�^�q�[�v�𐶐�	
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	descHeapDesc.NumDescriptors = 1; // �V�F�[�_�[���\�[�X�r���[1��
	res = device->CreateDescriptorHeap(
		&descHeapDesc,
		IID_PPV_ARGS(&descHeap)
	);//����
	if (FAILED(res)) {
		assert(0);
		return res;
	}

	// �f�X�N���v�^�T�C�Y���擾
	descHandleIncrementSize = device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	return res;
}

HRESULT OBJObject::LoadTexture(
	const std::string& textureName
){
	HRESULT res = S_FALSE;

	// WIC�e�N�X�`���̃��[�h
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	res = LoadFromWICFile(
		L"Resources/Texture/testtex.png",
		WIC_FLAGS_NONE,
		&metadata,
		scratchImg
	);
	if (FAILED(res)) {
		return res;
	}

	const Image* img = scratchImg.GetImage(0, 0, 0); // ���f�[�^���o

	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// �e�N�X�`���p�o�b�t�@�̐���
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // �e�N�X�`���p�w��
		nullptr,
		IID_PPV_ARGS(&texbuff));
	if (FAILED(res)) {
		return res;
	}

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	res = texbuff->WriteToSubresource(
		0,
		nullptr, // �S�̈�փR�s�[
		img->pixels,    // ���f�[�^�A�h���X
		(UINT)img->rowPitch,  // 1���C���T�C�Y
		(UINT)img->slicePitch // 1���T�C�Y
	);
	if (FAILED(res)) {
		return res;
	}

	// �V�F�[�_���\�[�X�r���[�쐬
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		descHeap->GetCPUDescriptorHandleForHeapStart(),
		0,
		descHandleIncrementSize
	);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		descHeap->GetGPUDescriptorHandleForHeapStart(),
		0,
		descHandleIncrementSize
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // �ݒ�\����
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(texbuff.Get(), //�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc, //�e�N�X�`���ݒ���
		cpuDescHandleSRV
	);

	return res;
}

HRESULT OBJObject::LoadTexture(
	const std::string& directoryPath,
	const std::string& fileName
){
	HRESULT res = S_FALSE;

	//�t�@�C���p�X������
	std::string filePath = directoryPath + fileName;

	//���j�R�[�h������ɕϊ�����
	wchar_t wFileName[128];
	int iBuffSize = MultiByteToWideChar(
		CP_ACP,
		0,
		filePath.c_str(),
		-1,
		wFileName,
		_countof(wFileName)
	);

	// WIC�e�N�X�`���̃��[�h
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	res = LoadFromWICFile(
		wFileName,
		WIC_FLAGS_NONE,
		&metadata,
		scratchImg
	);
	if (FAILED(res)) {
		return res;
	}

	const Image* img = scratchImg.GetImage(0, 0, 0); // ���f�[�^���o

	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// �e�N�X�`���p�o�b�t�@�̐���
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // �e�N�X�`���p�w��
		nullptr,
		IID_PPV_ARGS(&texbuff));
	if (FAILED(res)) {
		return res;
	}

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	res = texbuff->WriteToSubresource(
		0,
		nullptr, // �S�̈�փR�s�[
		img->pixels,    // ���f�[�^�A�h���X
		(UINT)img->rowPitch,  // 1���C���T�C�Y
		(UINT)img->slicePitch // 1���T�C�Y
	);
	if (FAILED(res)) {
		return res;
	}

	// �V�F�[�_���\�[�X�r���[�쐬
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		descHeap->GetCPUDescriptorHandleForHeapStart(),
		0,
		descHandleIncrementSize
	);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		descHeap->GetGPUDescriptorHandleForHeapStart(),
		0,
		descHandleIncrementSize
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // �ݒ�\����
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(texbuff.Get(), //�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc, //�e�N�X�`���ݒ���
		cpuDescHandleSRV
	);

	return res;
}

void OBJObject::CreateModel(){
	//�t�@�C���X�g���[��
	std::ifstream file;
	//.obj�t�@�C�����J��
	std::string modelName = useFileName;
	const std::string fileName = modelName + ".obj";
	const std::string directoryPath = "Resources/" + modelName + "/";
	file.open(directoryPath + fileName);
	//�t�@�C���I�[�v�����`�F�b�N
	if (file.fail()) {
		assert(0);
	}

	int indexCountTex = 0;

	std::vector<XMFLOAT3>positions; //���_���W
	std::vector<XMFLOAT3>normals;   //�@���x�N�g��
	std::vector<XMFLOAT2>texcoords; //�e�N�X�`��UV
	//1�s���ǂݍ���
	std::string line;
	while (getline(file, line)) {
		//1�s���̕�������X�g���[���ɕϊ�
		std::istringstream line_stream(line);

		//���p�X�y�[�X��؂�ōs�̐擪��������擾
		std::string key;
		getline(line_stream, key, ' ');

		//�擪������mtllib�Ȃ�}�e���A��
		if (key == "mtllib") {
			//�}�e���A���̃t�@�C�����ǂݍ���
			std::string fileName;
			line_stream >> fileName;
			//�}�e���A���ǂݍ���
			LoadMaterial(directoryPath, fileName);
		}

		//�擪������v�Ȃ璸�_���W
		if (key == "v") {
			//X,Y,Z���W�ǂݍ���
			XMFLOAT3 position{};
			line_stream >> position.x;
			line_stream >> position.y;
			line_stream >> position.z;
			//���W�f�[�^�ɒǉ�
			positions.emplace_back(position);
		}

		//�擪������vt�Ȃ�e�N�X�`��
		if (key == "vt") {
			//U,V�����ǂݍ���
			XMFLOAT2 texcoord{};
			line_stream >> texcoord.x;
			line_stream >> texcoord.y;
			//V�������]
			texcoord.y = 1.0f - texcoord.y;
			//�e�N�X�`�����W�f�[�^�ɒǉ�
			texcoords.emplace_back(texcoord);
		}

		//�擪������vn�Ȃ�@���x�N�g��
		if (key == "vn") {
			//X,Y,Z�����ǂݍ���
			XMFLOAT3 normal{};
			line_stream >> normal.x;
			line_stream >> normal.y;
			line_stream >> normal.z;
			//�@���x�N�g���f�[�^�ɒǉ�
			normals.emplace_back(normal);
		}

		//�擪������f�Ȃ�|���S��
		if (key == "f") {
			//�J��Ԃ��̃J�E���g
			int faceCount = 0;

			//���p�X�y�[�X��؂�ōs�̑�����ǂݍ���
			std::string index_string;
			while (getline(line_stream, index_string, ' ')) {
				//���_�C���f�b�N�X1���̕�������X�g���[���ɕϊ�
				std::istringstream index_stream(index_string);
				unsigned short indexPosition, indexNormal, indexTexcoord;
				index_stream >> indexPosition;
				index_stream.seekg(1, std::ios_base::cur); //�X���b�V�����΂�
				index_stream >> indexTexcoord;
				index_stream.seekg(1, std::ios_base::cur); //�X���b�V�����΂�
				index_stream >> indexNormal;
				//���_�f�[�^�̒ǉ�
				VertexData vertex{};
				vertex.pos = positions[indexPosition - 1];
				vertex.normal = normals[indexNormal - 1];
				vertex.uv = texcoords[indexTexcoord - 1];
				vertices.emplace_back(vertex);

				//�C���f�b�N�X�f�[�^�ɒǉ�
				if (faceCount >= 3) {
					//�l�p�`�|���S����4�_�ڂ̗��p
					indices.emplace_back(indexCountTex - 1);
					indices.emplace_back(indexCountTex);
					indices.emplace_back(indexCountTex - 3);
				}
				else {
					indices.emplace_back(indexCountTex);
				}
				++indexCountTex;
				++faceCount;
			}
		}
	}
	//�t�@�C�������
	file.close();
}

void OBJObject::InitMatrix(){
	//���[���h�s��
	matWorld = XMMatrixIdentity();
}

HRESULT OBJObject::CreateConstBuff(){
	HRESULT res = S_FALSE;
	// �萔�o�b�t�@�̐���
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// �A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBuffDataB0) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));

	// �萔�o�b�t�@�̐���
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// �A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBuffDataB1) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB1));

	return res;
}

HRESULT OBJObject::CreateConstView(){
    return S_OK;
}

void OBJObject::LoadMaterial(
	const std::string& directoryPath,
	const std::string& fileName
){
	//�t�@�C���X�g���[��
	std::ifstream file;
	//�}�e���A���t�@�C�����J��
	file.open(directoryPath + fileName);
	//�t�@�C���I�[�v�����s���`�F�b�N
	if (file.fail()) {
		assert(0);
	}

	//1�s���ǂݍ���
	std::string line;
	while (getline(file, line)) {
		//1�s���̕�������X�g���[���ɕϊ�
		std::istringstream line_stream(line);

		//���p�X�y�[�X��؂�ōs�̐擪��������擾
		std::string key;
		getline(line_stream, key, ' ');

		//�擪�̃^�u�����͖�������
		if (key[0] == '\t') {
			key.erase(key.begin()); //�擪�����폜
		}

		//�擪������newmtl�Ȃ�}�e���A����
		if (key == "newmtl") {
			//�}�e���A�����ǂݍ���
			line_stream >> material.name;
		}

		//�擪������Ka�Ȃ�A���r�G���g�F
		if (key == "Ka") {
			line_stream >> material.ambient.x;
			line_stream >> material.ambient.y;
			line_stream >> material.ambient.z;
		}

		//�擪������Kd�Ȃ�f�B�t���[�Y�F
		if (key == "Kd") {
			line_stream >> material.diffuse.x;
			line_stream >> material.diffuse.y;
			line_stream >> material.diffuse.z;
		}

		//�擪������Ks�Ȃ�X�y�L�����[�F
		if (key == "Ks") {
			line_stream >> material.specular.x;
			line_stream >> material.specular.y;
			line_stream >> material.specular.z;
		}

		//�擪������map_Kd�Ȃ�e�N�X�`���t�@�C����
		if (key == "map_Kd") {
			//�e�N�X�`���̃t�@�C�����ǂݍ���
			line_stream >> material.textureFileName;
			//�e�N�X�`���ǂݍ���
			LoadTexture(
				directoryPath,
				material.textureFileName
			);
		}
	}

	//�t�@�C�������
	file.close();
}

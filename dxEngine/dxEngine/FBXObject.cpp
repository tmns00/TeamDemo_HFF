#include "FBXObject.h"
#include"Camera.h"

#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<iostream>

using namespace DirectX;

FBXObject::~FBXObject() {
}

void FBXObject::Initialize() {
	assert(device);

	//CreateModel();

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

	if (FAILED(CreateDescriptorHeap())) {
		assert(0);
	}

	if (FAILED(LoadTexture(
		"Resources/Texture/white1x1.png"
	))) {
		assert(0);
	}

	InitMatrix();

	if (FAILED(CreateConstBuff())) {
		assert(0);
	}
}

void FBXObject::Update() {
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
	ConstBuffData* constMap = nullptr;
	res = constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = color;
	constMap->matrix = matWorld *
		Camera::GetViewMatrix() *
		Camera::GetProjectionMatrix();	// �s��̍���
	constBuff->Unmap(0, nullptr);

	UpdateColPos();
}

void FBXObject::Draw(
	ID3D12GraphicsCommandList* cmdList
) {
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
	// �V�F�[�_���\�[�X�r���[���Z�b�g
	cmdList->SetGraphicsRootDescriptorTable(1, gpuDescHandleSRV);
	// �`��R�}���h
	cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
}

void FBXObject::Terminate() {
	if (fbxManager)fbxManager->Destroy();
	delete this;
}

HRESULT FBXObject::InitFBX(
	const std::string& szFileName
) {
	//FBX�}�l�[�W���[����
	fbxManager = FbxManager::Create();
	assert(fbxManager);

	//FBX�C���|�[�^�[����
	fbxImporter = FbxImporter::Create(
		fbxManager,
		"myInporter"
	);
	assert(fbxImporter);

	//FBX�V�[������
	fbxScene = FbxScene::Create(
		fbxManager,
		"myScene"
	);
	assert(fbxScene);

	//�C���|�[�^�[������
	if (!fbxImporter->Initialize(
		"Resources/Model/fbxcube.fbx"
	)) {
		fbxImporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();

		return S_FALSE;
	}

	//�C���|�[�g
	if (!fbxImporter->Import(fbxScene)) {
		fbxImporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();

		return S_FALSE;
	}

	CreateModel();

	if (fbxImporter)fbxImporter->Destroy();

	if (fbxScene)fbxScene->Destroy();

	if (fbxManager)fbxManager->Destroy();

	return S_OK;
}

FBXObject* FBXObject::Create(
	ID3D12Device* dev,
	const std::string& fileName
) {
	FBXObject* instance = new FBXObject;
	if (!instance) {
		assert(0);
		return nullptr;
	}
	assert(SUCCEEDED(
		instance->InitFBX(fileName)
	));

	instance->device = dev;

	instance->Initialize();

	return instance;
}

HRESULT FBXObject::CreateVertBuff() {
	HRESULT res = S_FALSE;

	//���_�o�b�t�@
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOAD�q�[�v�Ƃ���
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(
			vertices.size() * sizeof(vertices[0])
		), //�T�C�Y�ɉ����ēK�؂Ȑݒ�����Ă����
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
	VertexData* vertMap = nullptr;
	res = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}
	std::copy(vertices.begin(), vertices.end(), vertMap);
	vertBuff->Unmap(0, nullptr);
	//���_�o�b�t�@�r���[
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); //�o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(vertices[0]));
	vbView.StrideInBytes = sizeof(vertices[0]); //1���_������̃o�C�g��

	return res;
}

HRESULT FBXObject::CreateIndexBuff() {
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

HRESULT FBXObject::ShaderCompile() {
	HRESULT res = S_FALSE;

	//���_�V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	res = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",	// �V�F�[�_�t�@�C����
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
		L"BasicPixelShader.hlsl",	// �V�F�[�_�t�@�C����
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

HRESULT FBXObject::CreateRootSignature() {
	HRESULT res = S_FALSE;

	//�f�X�N���v�^�����W
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,
		0
	);

	//���[�g�p�����[�^�[
	CD3DX12_ROOT_PARAMETER rootParams[2];
	rootParams[0].InitAsConstantBufferView(
		0,
		0,
		D3D12_SHADER_VISIBILITY_ALL
	);
	rootParams[1].InitAsDescriptorTable(
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

HRESULT FBXObject::CreateGraphicsPipeline() {
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

HRESULT FBXObject::CreateDescriptorHeap() {
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

HRESULT FBXObject::LoadTexture(
	const std::string& textureName
) {
	HRESULT res = S_FALSE;

	// WIC�e�N�X�`���̃��[�h
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	res = LoadFromWICFile(
		L"Resources/Texture/white1x1.png",
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

void FBXObject::CreateModel() {
	//�l�p�`�|���S�����O�p�`�ɕϊ�
	FbxGeometryConverter fbxConverter(fbxManager);
	fbxConverter.Triangulate(fbxScene, true);

	DisplayContent(fbxScene);
}

void FBXObject::InitMatrix() {
	//���[���h�s��
	matWorld = XMMatrixIdentity();
}

HRESULT FBXObject::CreateConstBuff() {
	HRESULT res = S_FALSE;
	// �萔�o�b�t�@�̐���
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// �A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBuffData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));

	return res;
}


void FBXObject::DisplayContent(
	FbxScene* scene
) {
	FbxNode* node = scene->GetRootNode();

	if (node) {
		for (int i = 0; i < node->GetChildCount(); ++i) {
			DisplayContent(node->GetChild(i));
		}
	}
}

void FBXObject::DisplayContent(
	FbxNode* node
) {
	FbxNodeAttribute::EType attrType;

	if (node->GetNodeAttribute() == NULL)
		std::cout << "NULL Node Attributr\n\n" << std::endl;
	else {
		attrType = (node->GetNodeAttribute()->GetAttributeType());

		switch (attrType) {
		default:
			break;

		case FbxNodeAttribute::eMesh:
			DisplayMesh(node);
			break;
		}
	}

	for (int i = 0; i < node->GetChildCount(); ++i) {
		DisplayContent(node->GetChild(i));
	}
}

void FBXObject::DisplayMesh(
	FbxNode* node
){
	FbxMesh* mesh = (FbxMesh*)node->GetNodeAttribute();

	std::cout << "\n\nMesh Name: " << (char*)node->GetName() << std::endl;

	DisplayIndex(mesh);
	DisplayPosition(mesh);
}

void FBXObject::DisplayIndex(
	FbxMesh* mesh
){
	//���|���S����
	int polygonNum = mesh->GetPolygonCount();

	//p�߂̃|���S���ւ̏���
	for (int p = 0; p < polygonNum; ++p) {
		for (int n = 0; n < 3; ++n) {
			int index = mesh->GetPolygonVertex(p, n);
			int setIndex = p + n;
			std::cout << "index[" << setIndex << "] : " << index << std::endl;
			indices.push_back(index);
		}
	}
}

void FBXObject::DisplayPosition(
	FbxMesh* mesh
){
	int positionNum = mesh->GetControlPointsCount();
	FbxVector4* position = mesh->GetControlPoints();

	for (int i = 0; i < positionNum; ++i) {
		VertexData setVert;
		std::cout << "position[" << i << "] : ("
			<< position[i][0] << ","
			<< position[i][1] << ","
			<< position[i][2] << ","
			<< position[i][3] << ")" << std::endl;
		setVert.pos.x = position[i][0];
		setVert.pos.y = position[i][1];
		setVert.pos.z = position[i][2];
		vertices.emplace_back(setVert);
	}
}

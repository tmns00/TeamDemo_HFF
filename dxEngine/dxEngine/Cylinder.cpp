#include"Cylinder.h"
#include"Camera.h"

#include<d3dcompiler.h>
#include<DirectXTex.h>

using namespace DirectX;

Cylinder::~Cylinder(){
}

void Cylinder::Initialize(){
	assert(device);

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

	if (FAILED(CreateDescriptorHeap())) {
		assert(0);
	}

	if (FAILED(LoadTexture("Resources/Texture/testtex.png"))) {
		assert(0);
	}

	InitMatrix();

	if (FAILED(CreateConstBuff())) {
		assert(0);
	}
}

void Cylinder::Update(){
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
		camera->GetViewMatrix() *
		camera->GetProjectionMatrix();	// �s��̍���
	constBuff->Unmap(0, nullptr);
}

void Cylinder::Draw(
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
	// �V�F�[�_���\�[�X�r���[���Z�b�g
	cmdList->SetGraphicsRootDescriptorTable(1, gpuDescHandleSRV);
	// �`��R�}���h
	cmdList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
}

void Cylinder::Terminate(){
	delete this;
}

Cylinder* Cylinder::Create(
	ID3D12Device* dev
){
	Cylinder* instance = new Cylinder;
	if (!instance) {
		assert(0);
		return nullptr;
	}
	instance->device = dev;

	instance->Initialize();

	return instance;
}

HRESULT Cylinder::CreateVertBuff(){
	HRESULT res = S_FALSE;

	//���_�o�b�t�@
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOAD�q�[�v�Ƃ���
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)), //�T�C�Y�ɉ����ēK�؂Ȑݒ�����Ă����
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
	memcpy(vertMap, vertices, sizeof(vertices));
	vertBuff->Unmap(0, nullptr);
	//���_�o�b�t�@�r���[
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); //�o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices);
	vbView.StrideInBytes = sizeof(vertices[0]); //1���_������̃o�C�g��

	return res;
}

HRESULT Cylinder::CreateIndexBuff(){
	HRESULT res = S_FALSE;

	//�C���f�b�N�X�o�b�t�@
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOAD�q�[�v�Ƃ���
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)), //�T�C�Y�ɉ����ēK�؂Ȑݒ�����Ă����,
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

	memcpy(indexMap, indices, sizeof(indices));
	idxBuff->Unmap(0, nullptr);
	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	return res;
}

HRESULT Cylinder::ShaderCompile(){
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

HRESULT Cylinder::CreateRootSignature(){
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

HRESULT Cylinder::CreateGraphicsPipeline(){
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

HRESULT Cylinder::CreateDescriptorHeap(){
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

HRESULT Cylinder::LoadTexture(
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

void Cylinder::CreateModel(){
	std::vector<VertexData> realVertices;
	// ���_���W�̌v�Z�i�d������j
	{
		realVertices.resize((division + 1) * 2);
		int index = 0;
		float zValue;

		// ���
		zValue = prizmHeight / 2.0f;
		for (int i = 0; i < division; i++)
		{
			XMFLOAT3 vertex;
			vertex.x = radius * sinf(XM_2PI / division * i);
			vertex.y = radius * cosf(XM_2PI / division * i);
			vertex.z = zValue;
			realVertices[index++].pos = vertex;
		}
		realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// ��ʂ̒��S�_
		// �V��
		zValue = -prizmHeight / 2.0f;
		for (int i = 0; i < division; i++)
		{
			XMFLOAT3 vertex;
			vertex.x = radius * sinf(XM_2PI / division * i);
			vertex.y = radius * cosf(XM_2PI / division * i);
			vertex.z = zValue;
			realVertices[index++].pos = vertex;
		}
		realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// �V�ʂ̒��S�_
	}

	// ���_���W�̌v�Z�i�d���Ȃ��j
	{
		int index = 0;
		// ���
		for (int i = 0; i < division; i++)
		{
			unsigned short index0 = i + 1;
			unsigned short index1 = i;
			unsigned short index2 = division;

			vertices[index++] = realVertices[index0];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index2]; // ��ʂ̒��S�_
		}
		// ��ʂ̍Ō�̎O�p�`��1�Ԗڂ̃C���f�b�N�X��0�ɏ�������
		vertices[index - 3] = realVertices[0];

		int topStart = division + 1;
		// �V��
		for (int i = 0; i < division; i++)
		{
			unsigned short index0 = topStart + i;
			unsigned short index1 = topStart + i + 1;
			unsigned short index2 = topStart + division;

			vertices[index++] = realVertices[index0];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index2]; // �V�ʂ̒��S�_
		}
		// �V�ʂ̍Ō�̎O�p�`��1�Ԗڂ̃C���f�b�N�X��0�ɏ�������
		vertices[index - 2] = realVertices[topStart];

		// ����
		for (int i = 0; i < division; i++)
		{
			unsigned short index0 = i + 1;
			unsigned short index1 = topStart + i + 1;
			unsigned short index2 = i;
			unsigned short index3 = topStart + i;

			if (i == division - 1)
			{
				index0 = 0;
				index1 = topStart;
			}

			vertices[index++] = realVertices[index0];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index2];

			vertices[index++] = realVertices[index2];
			vertices[index++] = realVertices[index1];
			vertices[index++] = realVertices[index3];
		}
	}

	// ���_�C���f�b�N�X�̐ݒ�
	{
		for (int i = 0; i < _countof(indices); i++)
		{
			indices[i] = i;
		}
	}

	// �@�������̌v�Z
	for (int i = 0; i < _countof(indices) / 3; i++)
	{// �O�p�`�P���ƂɌv�Z���Ă���
		// �O�p�`�̃C���f�b�N�X���擾
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		// �O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		// p0��p1�x�N�g���Ap0��p2�x�N�g�����v�Z
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		// �O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1, v2);
		// ���K���i������1�ɂ���)
		normal = XMVector3Normalize(normal);
		// ���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);
	}
}

void Cylinder::InitMatrix(){
	//���[���h�s��
	matWorld = XMMatrixIdentity();
}

HRESULT Cylinder::CreateConstBuff(){
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

HRESULT Cylinder::CreateConstView(){
    return S_OK;
}

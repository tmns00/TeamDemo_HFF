#include "FbxObj2.h"

#include<iostream>
#include<sstream>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Camera.h"

using namespace DirectX;

void FbxObj2::Initialize()
{
	if (FAILED(InitFBX("Resources/Model/MyHuman.fbx")))
		assert(0);

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

	if (FAILED(CreateConstBuff())) {
		assert(0);
	}
}

void FbxObj2::Update() {
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

	UpdateAnim();
}

void FbxObj2::Draw(
	ID3D12GraphicsCommandList* cmdList
) {
	// �p�C�v���C���X�e�[�g�̐ݒ�
	cmdList->SetPipelineState(pipelineState.Get());
	// ���[�g�V�O�l�`���̐ݒ�
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	// �v���~�e�B�u�`���ݒ�
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto index : indices) {
		//�C���f�b�N�X�o�b�t�@���}�e���A���@�̐������`�����

		// ���_�o�b�t�@�̐ݒ�
		cmdList->IASetVertexBuffers(0, 1, &vbView[index.first]);
		// �C���f�b�N�X�o�b�t�@�̐ݒ�
		cmdList->IASetIndexBuffer(&ibView[index.first]);

		// �萔�o�b�t�@�r���[���Z�b�g
		cmdList->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());
		// �`��R�}���h
		cmdList->DrawIndexedInstanced(indices[index.first].size(), 1, 0, 0, 0);
	}
}

void FbxObj2::Terminate() {
	DestroyFBX();
}

HRESULT FbxObj2::InitFBX(
	std::string fileName
) {
	//fbx�}�l�[�W���[
	fbxManager = FbxManager::Create();
	if (fbxManager == nullptr) {
		assert(0);
		return S_FALSE;
	}
	//fbx�C���|�[�^�[
	fbxImporter = FbxImporter::Create(fbxManager, "MyImporter");
	if (fbxImporter == nullptr) {
		fbxManager->Destroy();
		assert(0);
		return S_FALSE;
	}
	//Fbx�V�[��
	fbxScene = FbxScene::Create(fbxManager, "MyScene");
	if (fbxScene == nullptr) {
		fbxImporter->Destroy();
		fbxManager->Destroy();
		assert(0);
		return S_FALSE;
	}

	//�t�@�C��������
	if (fbxImporter->Initialize(fileName.c_str()) == false) {
		fbxImporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();
		assert(0);
		return S_FALSE;
	}

	//fbxScene�ɃC���|�[�g
	if (fbxImporter->Import(fbxScene) == false) {
		fbxImporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();
		assert(0);
		return S_FALSE;
	}

	FbxGeometryConverter converter(fbxManager);
	//�|���S�����O�p�`�ɂ���
	//converter.Triangulate(fbxScene, true);

	LoadContents();

	if (FAILED(InitAnim())) {
		DestroyFBX();
		assert(0);
		return S_FALSE;
	}

	//DestroyFBX();

	return S_OK;
}

void FbxObj2::DestroyFBX() {
	if (fbxImporter != nullptr)
		fbxImporter->Destroy();

	if (fbxScene != nullptr)
		fbxScene->Destroy();

	if (fbxManager != nullptr)
		fbxManager->Destroy();
}

void FbxObj2::LoadContents() {
	rootNode = fbxScene->GetRootNode();
	if (rootNode == nullptr) {
		DestroyFBX();
		assert(0);
	}

	//LoadNode(rootNode, 0);
	CollectMeshNode(rootNode, meshNodeList);

	for (auto data : meshNodeList) {
		//mesh�쐬
		CreateMesh(data.first.c_str(), data.second->GetMesh());
	}
}

void FbxObj2::LoadNode(
	FbxNode* node,
	int hierarchy
) {
	if (node == nullptr)
		return;

	const char* name = node->GetName();

	std::ostringstream oss;

	for (int i = 0; i < hierarchy; ++i)
		oss << "\t";
	oss << "name:" << name << std::endl;

	for (int i = 0; i < node->GetNodeAttributeCount(); ++i) {
		for (int j = 0; j < hierarchy; ++j)
			oss << "\t";
		oss << "\tAttribute " << GetNodeAttributeName(
			node->GetNodeAttributeByIndex(i)->GetAttributeType()
		) << std::endl;

	}
	OutputDebugStringA(oss.str().c_str());

	for (int i = 0; i < node->GetChildCount(); ++i)
		LoadNode(node->GetChild(i), hierarchy + 1);
}

void FbxObj2::CollectMeshNode(
	FbxNode* node,
	std::map<std::string, FbxNode*>& list
) {
	for (int i = 0; i < node->GetNodeAttributeCount(); ++i) {
		FbxNodeAttribute* attribute = node->GetNodeAttributeByIndex(i);

		//Attribute�����b�V���Ȃ�ǉ�
		if (attribute->GetAttributeType() == FbxNodeAttribute::EType::eMesh) {
			list[node->GetName()] = node;
			break;
		}
	}

	for (int i = 0; i < node->GetChildCount(); ++i) {
		CollectMeshNode(node->GetChild(i), list);
	}
}

void FbxObj2::LoadIndex(
	const char* nodeName,
	FbxMesh* mesh
) {
	std::ostringstream oss;

	indices[nodeName].reserve(faceNum * 3);
	for (int p = 0; p < faceNum; ++p) {
		for (int i = 0; i < 3; ++i) {
			indices[nodeName].emplace_back(mesh->GetPolygonVertex(p, i));
			oss << mesh->GetPolygonVertex(p, i) << ",";
		}
		oss << std::endl;
	}
	oss << std::endl;
	OutputDebugStringA(oss.str().c_str());
}

void FbxObj2::LoadVertex(
	const char* nodeName,
	FbxMesh* mesh
) {
	//���_�o�b�t�@�̎擾
	FbxVector4* fbxVert = mesh->GetControlPoints();

	std::ostringstream oss;

	vertices[nodeName].resize(vertNum);
	for (int i = 0; i < vertNum; ++i) {
		VertexData addVert;

		addVert.pos.x = static_cast<float>(fbxVert[i][0]);
		addVert.pos.y = static_cast<float>(fbxVert[i][1]);
		addVert.pos.z = static_cast<float>(fbxVert[i][2]);

		//�ǉ�
		vertices[nodeName][i] = addVert;
		oss << "{"
			<< addVert.pos.x << ","
			<< addVert.pos.y << ","
			<< addVert.pos.z << "}"
			<< std::endl;
	}
	oss << std::endl;
	OutputDebugStringA(oss.str().c_str());
}

void FbxObj2::LoadNormal(
	const char* nodeName,
	FbxMesh* mesh
) {
	FbxVector4 normal;

	//�@���ݒ�
	for (int i = 0; i < faceNum; ++i) {
		int startIndex = mesh->GetPolygonVertexIndex(i);
		int* pIndex = mesh->GetPolygonVertices();

		int index0 = pIndex[startIndex];
		int index1 = pIndex[startIndex + 1];
		int index2 = pIndex[startIndex + 2];

		if (index0 <= 0) continue;

		mesh->GetPolygonVertexNormal(i, 0, normal);
		vertices[nodeName][index0].normal.x = -normal[0];
		vertices[nodeName][index0].normal.y = normal[1];
		vertices[nodeName][index0].normal.z = normal[2];

		mesh->GetPolygonVertexNormal(i, 1, normal);
		vertices[nodeName][index1].normal.x = -normal[0];
		vertices[nodeName][index1].normal.y = normal[1];
		vertices[nodeName][index1].normal.z = normal[2];

		mesh->GetPolygonVertexNormal(i, 2, normal);
		vertices[nodeName][index2].normal.x = -normal[0];
		vertices[nodeName][index2].normal.y = normal[1];
		vertices[nodeName][index2].normal.z = normal[2];
	}
}

void FbxObj2::LoadUV(
	const char* nodeName,
	FbxMesh* mesh
) {
	FbxLayerElementUV* useUV;
	useUV = mesh->GetLayer(0)->GetUVs();

	for (int i = 0; i < vertices.size(); ++i) {
		FbxVector2 vec2;
		vec2 = useUV->GetDirectArray().GetAt(i);
		//vertices[nodeName][i].uv.x = vec2[0];
		//vertices[nodeName][i].uv.y = 1.0f - vec2[1];
	}
}

void FbxObj2::LoadSkinInfo(
	const char* nodeName,
	FbxMesh* mesh
) {
	//�f�t�H�[�}�[���擾
	FbxDeformer* deformer = mesh->GetDeformer(0);
	FbxSkin* skinInfo = static_cast<FbxSkin*>(deformer);

	//�{�[�����擾
	boneNum = skinInfo->GetClusterCount();
	for (int i = 0; i < boneNum; ++i) {
		Bone addBone = {};
		addBone.name = skinInfo->GetCluster(i)->GetLink()->GetName();
		addBone.bone = skinInfo->GetCluster(i);
		bones[nodeName].emplace_back(
			addBone
		);
	}
}

void FbxObj2::CreateMesh(
	const char* nodeName,
	FbxMesh* mesh
) {
	//���R�ɐ����擾
	vertNum = mesh->GetControlPointsCount();
	uvNum = mesh->GetTextureUVCount();
	faceNum = mesh->GetPolygonCount();

	LoadVertex(nodeName, mesh);
	LoadNormal(nodeName, mesh);
	LoadIndex(nodeName, mesh);
	//LoadUV(nodeName, mesh);
	LoadSkinInfo(nodeName, mesh);

	std::ostringstream oss;
	for (auto b : bones) {
		for (int i = 0; i < b.second.size(); ++i) {
			oss << bones[b.first][i].name << std::endl;
		}
	}
	OutputDebugStringA(oss.str().c_str());
}

HRESULT FbxObj2::InitAnim() {
	fbxScene->FillAnimStackNameArray(AnimStackNameArray);
	if (AnimStackNameArray != NULL) {
		AnimationStack = fbxScene->FindMember<FbxAnimStack>(AnimStackNameArray[AnimStackNumber]->Buffer());
		fbxScene->SetCurrentAnimationStack(AnimationStack);

		FbxTakeInfo* takeInfo = fbxScene->GetTakeInfo(*(AnimStackNameArray[AnimStackNumber]));
		start = takeInfo->mLocalTimeSpan.GetStart();
		stop = takeInfo->mLocalTimeSpan.GetStop();
		frameTime.SetTime(0, 0, 0, 1, 0, fbxScene->GetGlobalSettings().GetTimeMode());
		timeCount = start;
	}
	return S_OK;
}

void FbxObj2::UpdateAnim() {
	if (AnimStackNameArray != NULL) {
		timeCount += frameTime;
		if (timeCount > stop)timeCount = start;
	}

	for (auto meshNode : meshNodeList) {
		//�ړ��E��]�E�g��̍s����쐬
		FbxMatrix globalPosition = meshNode.second->EvaluateGlobalTransform(timeCount);
		FbxVector4 t0 = meshNode.second->GetGeometricTranslation(FbxNode::eSourcePivot);
		FbxVector4 r0 = meshNode.second->GetGeometricRotation(FbxNode::eSourcePivot);
		FbxVector4 s0 = meshNode.second->GetGeometricScaling(FbxNode::eSourcePivot);
		FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);

		//�e���_�ɂ�����ŏI�I�ȍs��̔z��
		clusterDeformation = new FbxMatrix[meshNode.second->GetMesh()->GetPolygonVertexCount()];
		memset(clusterDeformation, 0, sizeof(FbxMatrix) * meshNode.second->GetMesh()->GetPolygonVertexCount());

		//�e���_�ɉe����^���邽�߂̍s��̍쐬
		for (int clusterIndex = 0; clusterIndex < boneNum; ++clusterIndex) {
			//�{�[���̎��o��
			FbxMatrix vertexTransformMatrix;
			FbxAMatrix referenceGloabalInitPosition;
			FbxAMatrix clusterGlobalInitPosition;
			FbxMatrix clusterGlobalCurrentPosition;
			FbxMatrix clusterRelativeInitposition;
			FbxMatrix clusterRelativeCurrentPositionInverse;

			bones[meshNode.first][clusterIndex].bone->GetTransformMatrix(referenceGloabalInitPosition);
			referenceGloabalInitPosition *= geometryOffset;

			bones[meshNode.first][clusterIndex].bone->GetTransformLinkMatrix(clusterGlobalInitPosition);
			clusterGlobalCurrentPosition = bones[meshNode.first][clusterIndex].bone->GetLink()->EvaluateGlobalTransform(timeCount);

			clusterRelativeInitposition = clusterGlobalInitPosition.Inverse() * referenceGloabalInitPosition;
			clusterRelativeCurrentPositionInverse = globalPosition.Inverse() * clusterGlobalCurrentPosition;

			vertexTransformMatrix = clusterRelativeCurrentPositionInverse * clusterRelativeInitposition;

			//��ō�����s��ɏd�݂�������
			for (int i = 0; i < bones[meshNode.first][clusterIndex].bone->GetControlPointIndicesCount(); ++i) {
				int index = bones[meshNode.first][clusterIndex].bone->GetControlPointIndices()[i];
				double weight = bones[meshNode.first][clusterIndex].bone->GetControlPointWeights()[i];
				FbxMatrix influence = vertexTransformMatrix * weight;
				clusterDeformation[index] += influence;
			}
		}

		//�ŏI�I�Ȓ��_���W���v�Z����
		for (int i = 0; i < meshNodeList[meshNode.first]->GetMesh()->GetControlPointsCount(); ++i) {
			FbxVector4 outVertex = clusterDeformation[i].MultNormalize(meshNode.second->GetMesh()->GetControlPointAt(i));
			vertices[meshNode.first][i].pos.x = static_cast<float>(-outVertex[0]);
			vertices[meshNode.first][i].pos.y = static_cast<float>(outVertex[1]);
			vertices[meshNode.first][i].pos.z = static_cast<float>(outVertex[2]);
		}

		HRESULT res = S_FALSE;
		VertexData* vertMap = nullptr;
		res = vertBuff[meshNode.first]->Map(0, nullptr, (void**)&vertMap);
		if (FAILED(res)) {
			assert(0);
		}
		std::copy(
			vertices[meshNode.first].begin(),
			vertices[meshNode.first].end(),
			vertMap
		);

		delete[] clusterDeformation;
	}

}

void FbxObj2::CreateFrameMatrix(
	const std::string meshName,
	FbxNode* node
) {

}

void FbxObj2::SetPosBone(
	XMFLOAT3 setPos,
	int boneIndex
) {
	XMMATRIX posMat = {};
	posMat = XMMatrixTranslation(setPos.x, setPos.y, setPos.z);
}

void FbxObj2::SetRotBone(
	XMFLOAT3 setRot,
	int boneIndex
) {
	XMMATRIX rotMat = {};
	rotMat = XMMatrixIdentity();
	rotMat *= XMMatrixRotationZ(XMConvertToRadians(setRot.z));
	rotMat *= XMMatrixRotationX(XMConvertToRadians(setRot.x));
	rotMat *= XMMatrixRotationY(XMConvertToRadians(setRot.y));
}

HRESULT FbxObj2::CreateVertBuff() {
	HRESULT res = S_FALSE;

	for (auto vertexBuffer : vertices) {
		//���_�o�b�t�@
		ID3D12Resource* addVertBuff = nullptr;
		res = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOAD�q�[�v�Ƃ���
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(
				vertices[vertexBuffer.first].size() * sizeof(VertexData)
			), //�T�C�Y�ɉ����ēK�؂Ȑݒ�����Ă����
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&addVertBuff)
		);
		if (FAILED(res)) {
			assert(0);
			return S_FALSE;
		}
		vertBuff[vertexBuffer.first] = addVertBuff;

		res = S_FALSE;
		//�o�b�t�@�ɒ��_�f�[�^�R�s�[
		VertexData* vertMap = nullptr;
		res = vertBuff[vertexBuffer.first]->Map(0, nullptr, (void**)&vertMap);
		if (FAILED(res)) {
			assert(0);
			return S_FALSE;
		}
		std::copy(
			vertices[vertexBuffer.first].begin(),
			vertices[vertexBuffer.first].end(),
			vertMap
		);
		vertBuff[vertexBuffer.first]->Unmap(0, nullptr);
		//���_�o�b�t�@�r���[
		vbView[vertexBuffer.first].BufferLocation
			= vertBuff[vertexBuffer.first]->GetGPUVirtualAddress(); //�o�b�t�@�[�̉��z�A�h���X
		vbView[vertexBuffer.first].SizeInBytes
			= static_cast<UINT>(vertices[vertexBuffer.first].size() * sizeof(VertexData));
		vbView[vertexBuffer.first].StrideInBytes
			= static_cast<UINT>(sizeof(VertexData)); //1���_������̃o�C�g��
	}

	return res;
}

HRESULT FbxObj2::CreateIndexBuff() {
	HRESULT res = S_FALSE;

	for (auto index : indices) {
		ID3D12Resource* addIndexBuff;
		//�C���f�b�N�X�o�b�t�@
		res = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOAD�q�[�v�Ƃ���
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(
				indices[index.first].size() * sizeof(UINT)
			), //�T�C�Y�ɉ����ēK�؂Ȑݒ�����Ă����,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&addIndexBuff)
		);
		if (FAILED(res)) {
			assert(0);
		}
		indexBuff[index.first] = addIndexBuff;

		res = S_FALSE;
		//������o�b�t�@�ɃC���f�b�N�X�f�[�^�R�s�[
		unsigned short* mappedIdx = nullptr;
		res = indexBuff[index.first]->Map(0, nullptr, (void**)&mappedIdx);
		if (FAILED(res)) {
			assert(0);
		}
		copy(
			indices[index.first].begin(),
			indices[index.first].end(),
			mappedIdx
		);
		//memcpy(index, mappedIdx, sizeof(index));
		indexBuff[index.first]->Unmap(0, nullptr);
		//�C���f�b�N�X�o�b�t�@�r���[���쐬
		ibView[index.first].BufferLocation
			= indexBuff[index.first]->GetGPUVirtualAddress();
		ibView[index.first].Format
			= DXGI_FORMAT_R16_UINT;
		ibView[index.first].SizeInBytes
			= static_cast<UINT>(indices[index.first].size() * sizeof(UINT));
	}

	return res;
}

HRESULT FbxObj2::ShaderCompile() {
	HRESULT res = S_FALSE;

	//���_�V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	res = D3DCompileFromFile(
		L"FBXVertexShader.hlsl",	// �V�F�[�_�t�@�C����
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
		L"FBXPixelShader.hlsl",	// �V�F�[�_�t�@�C����
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

HRESULT FbxObj2::CreateRootSignature() {
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

HRESULT FbxObj2::CreateGraphicsPipeline() {
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

HRESULT FbxObj2::CreateDescriptorHeap() {
	return E_NOTIMPL;
}

HRESULT FbxObj2::LoadTexture(
	const std::string& textureName
) {
	return E_NOTIMPL;
}

void FbxObj2::CreateModel() {
}

void FbxObj2::InitMatrix() {
}

HRESULT FbxObj2::CreateConstBuff() {
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

const char* FbxObj2::GetNodeAttributeName(
	FbxNodeAttribute::EType attribute
) {
	switch (attribute)
	{
	case fbxsdk::FbxNodeAttribute::eUnknown:
		return "eUnknown";
		break;
	case fbxsdk::FbxNodeAttribute::eNull:
		return "eNull";
		break;
	case fbxsdk::FbxNodeAttribute::eMarker:
		return "eMarker";
		break;
	case fbxsdk::FbxNodeAttribute::eSkeleton:
		return "eSkeleton";
		break;
	case fbxsdk::FbxNodeAttribute::eMesh:
		return "eMesh";
		break;
	case fbxsdk::FbxNodeAttribute::eNurbs:
		return "eNurbs";
		break;
	case fbxsdk::FbxNodeAttribute::ePatch:
		return "ePatch";
		break;
	case fbxsdk::FbxNodeAttribute::eCamera:
		return "eCamera";
		break;
	case fbxsdk::FbxNodeAttribute::eCameraStereo:
		return "eCameraStereo";
		break;
	case fbxsdk::FbxNodeAttribute::eCameraSwitcher:
		return "eCameraSwitcher";
		break;
	case fbxsdk::FbxNodeAttribute::eLight:
		return "eLight";
		break;
	case fbxsdk::FbxNodeAttribute::eOpticalReference:
		return "eOpticalReference";
		break;
	case fbxsdk::FbxNodeAttribute::eOpticalMarker:
		return "eOpticalMarker";
		break;
	case fbxsdk::FbxNodeAttribute::eNurbsCurve:
		return "eNurbsCurve";
		break;
	case fbxsdk::FbxNodeAttribute::eTrimNurbsSurface:
		return "eTrimNurbsSurface";
		break;
	case fbxsdk::FbxNodeAttribute::eBoundary:
		return "eBoundary";
		break;
	case fbxsdk::FbxNodeAttribute::eNurbsSurface:
		return "eNurbsSurface";
		break;
	case fbxsdk::FbxNodeAttribute::eShape:
		return "eShape";
		break;
	case fbxsdk::FbxNodeAttribute::eLODGroup:
		return "eLODGroup";
		break;
	case fbxsdk::FbxNodeAttribute::eSubDiv:
		return "eSubDiv";
		break;
	case fbxsdk::FbxNodeAttribute::eCachedEffect:
		return "eCachedEffect";
		break;
	case fbxsdk::FbxNodeAttribute::eLine:
		return "eLine";
		break;
	}

	return "";
}

XMMATRIX FbxObj2::FbxAMatrixConvertToXMMatrix(
	FbxAMatrix fbxMat
) {
	XMMATRIX retMat = {};

	retMat = XMMatrixSet(
		static_cast<float>(fbxMat.Get(0, 0)), static_cast<float>(fbxMat.Get(0, 1)), static_cast<float>(fbxMat.Get(0, 2)), static_cast<float>(fbxMat.Get(0, 3)),
		static_cast<float>(fbxMat.Get(1, 0)), static_cast<float>(fbxMat.Get(1, 1)), static_cast<float>(fbxMat.Get(1, 2)), static_cast<float>(fbxMat.Get(1, 3)),
		static_cast<float>(fbxMat.Get(2, 0)), static_cast<float>(fbxMat.Get(2, 1)), static_cast<float>(fbxMat.Get(2, 2)), static_cast<float>(fbxMat.Get(2, 3)),
		static_cast<float>(fbxMat.Get(3, 0)), static_cast<float>(fbxMat.Get(3, 1)), static_cast<float>(fbxMat.Get(3, 2)), static_cast<float>(fbxMat.Get(3, 3))
	);

	return retMat;
}

#include "PMDObject.h"
#include"Camera.h"

#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<iostream>
#include<sstream>
#include<array>

#pragma comment(lib,"winmm.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

//���_1������̃T�C�Y
const size_t pmdvertex_size = 38;

const float epsilon = 1.0e-5f;

namespace {
    //�{�[�����
    enum class BoneType {
        Rotation,      //��]
        RotAndMove,    //��]���ړ�
        IK,            //IK
        Undefind,      //����`
        IKChild,       //IK�e���{�[��
        RotationChild, //��]�e���{�[��
        IKDestination, //IK�ڑ���
        Invisible      //�����Ȃ��{�[��
    };
}

PMDObject::~PMDObject(){
}

void PMDObject::Initialize(){
    //nullptr�`�F�b�N
    assert(device);

    vmdLoader = VMDLoader::GetInstance();
    assert(vmdLoader);

    CreateModel();

    if (FAILED(CreateVertBuff())) {
        assert(0);
    }

    if (FAILED(CreateIndexBuff())) {
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

    //LoadVMDFile();
}

void PMDObject::Update(){
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

    //�s���萔�o�b�t�@�ɃR�s�[
    //SceneMatrix* mapMatrix; //�}�b�v��̃|�C���^�[
    res = constBuff->Map(
        0,
        nullptr,
        (void**)&mapMatrix
    );
    if (FAILED(res)) {
        assert(0);
    }
    //�s��̃R�s�[
    mapMatrix->world = matWorld;
    mapMatrix->view = camera->GetViewMatrix();
    mapMatrix->proj = camera->GetProjectionMatrix();
    mapMatrix->view_proj = camera->GetViewMatrix() * camera->GetProjectionMatrix();
    mapMatrix->eye = camera->GetEye();
    std::copy(
        boneMatrices.begin(),
        boneMatrices.end(),
        mapMatrix->bones
    );
    constBuff->Unmap(0, nullptr);

    MotionUpdate();
}

void PMDObject::Draw(
    ID3D12GraphicsCommandList* cmdList
){
    // �p�C�v���C���X�e�[�g�̐ݒ�
    cmdList->SetPipelineState(pipelineState.Get());
    // ���[�g�V�O�l�`���̐ݒ�
    cmdList->SetGraphicsRootSignature(rootSignature.Get());
    // �v���~�e�B�u�`���ݒ�
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

void PMDObject::Terminate(){
    delete this;
}

void PMDObject::PlayAnimation(std::string key){
    startTime = timeGetTime();

    LoadVMDFile(key);
}

void PMDObject::StopAnimation(){
    startTime = timeGetTime();

    loadMotion = {};
}

PMDObject* PMDObject::Create(
    ID3D12Device* dev,
    const std::string& fileName
) {
    PMDObject* instance = new PMDObject;
    if (!instance) {
        assert(0);
        return nullptr;
    }
    instance->device = dev;
    instance->strModelPath = fileName;

    instance->Initialize();

    return instance;
}

HRESULT PMDObject::CreateVertBuff(){
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

HRESULT PMDObject::CreateIndexBuff(){
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

HRESULT PMDObject::ShaderCompile(){
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

HRESULT PMDObject::CreateRootSignature(){
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

HRESULT PMDObject::CreateGraphicsPipeline(){
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

HRESULT PMDObject::CreateDescriptorHeap(){
    return S_OK;
}

HRESULT PMDObject::LoadTexture(
    const std::string& textureName
){
    return S_OK;
}

void PMDObject::CreateModel(){
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

    //�{�[����
    unsigned short boneNum = 0;
    fread(&boneNum, sizeof(boneNum), 1, fp);

    //�{�[�����
    pmdBones.resize(boneNum);
    fread(pmdBones.data(), sizeof(PMDBone), boneNum, fp);

    uint16_t ikNum = 0;
    fread(&ikNum, sizeof(ikNum), 1, fp);

    pmdIkData.resize(ikNum);
    for (auto& ik : pmdIkData) {
        fread(&ik.boneIndex, sizeof(ik.boneIndex), 1, fp);
        fread(&ik.targetIndex, sizeof(ik.targetIndex), 1, fp);

        uint8_t chainLength = 0; //�Ԃɂ����m�[�h�����邩
        fread(&chainLength, sizeof(chainLength), 1, fp);
        ik.nodeIndices.resize(chainLength);
        fread(&ik.iterations, sizeof(ik.iterations), 1, fp);
        fread(&ik.limit, sizeof(ik.limit), 1, fp);

        if (chainLength == 0) continue; //�Ԃ̃m�[�h��0�Ȃ�΂����ŏI���

        fread(ik.nodeIndices.data(), sizeof(ik.nodeIndices[0]), chainLength, fp);
    }

    fclose(fp);

    boneNameArray.resize(pmdBones.size());
    boneNodeAddressArray.resize(pmdBones.size());

    kneeIndices.clear();

    for (int i = 0; i < pmdBones.size(); ++i) {
        auto& pb = pmdBones[i];
        auto& node = boneNodeTable[pb.boneName];

        node.boneIndex = i;
        node.startPos = pb.pos;
        node.boneType = pb.type;
        node.parentBone = pb.parentNo;
        node.ikParentBone = pb.ikBoneNo;

        //�C���f�b�N�X�v�Z�����₷���悤��
        boneNameArray[i] = pb.boneName;
        boneNodeAddressArray[i] = &node;

        std::string boneName = pb.boneName;
        if (boneName.find("�Ђ�") != std::string::npos) {
            kneeIndices.emplace_back(i);
        }
    }
    //�e�q�֌W�̍\�z
    for (auto& pb : pmdBones) {
        //�e�C���f�b�N�X���`�F�b�N
        if (pb.parentNo >= pmdBones.size()) {
            continue;
        }

        auto parentName = boneNameArray[pb.parentNo];
        boneNodeTable[parentName].children.emplace_back(
            &boneNodeTable[pb.boneName]
        );
    }

    boneMatrices.resize(pmdBones.size());
    //�{�[�������ׂď���������
    std::fill(
        boneMatrices.begin(),
        boneMatrices.end(),
        XMMatrixIdentity()
    );

#ifndef DEBUG
    //IK�f�o�b�O�p
    auto getNameFromIndex = [&](uint16_t idx)->std::string {
        auto it = find_if(
            boneNodeTable.begin(),
            boneNodeTable.end(),
            [idx](const std::pair<std::string, BoneNode>& obj) {
                return obj.second.boneIndex == idx;
            });

        if (it != boneNodeTable.end()) {
            return it->first;
        }
        else {
            return "";
        }
    };

    for (auto& ik : pmdIkData) {
        std::ostringstream oss;
        oss << "IK�{�[���ԍ���" << ik.boneIndex << "�F" << getNameFromIndex(ik.boneIndex) << std::endl;

        for (auto& node : ik.nodeIndices) {
            oss << "\t�m�[�h�{�[����" << node << "�F" << getNameFromIndex(node) << std::endl;
        }

        OutputDebugStringA(
            oss.str().c_str()
        );
    }
#endif // DEBUG
}

void PMDObject::InitMatrix(){
    //���[���h�s��
    matWorld = XMMatrixIdentity();
}

HRESULT PMDObject::CreateConstBuff(){
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

HRESULT PMDObject::CreateConstView(){
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

void PMDObject::LoadVMDFile(std::string key){
    loadMotion = vmdLoader->GetMotion(key);

    for (auto& motion : loadMotion.motion) {
        std::sort(
            motion.second.begin(),
            motion.second.end(),
            [](const Motion& lval, const Motion& rval) {
                return lval.frameNo <= rval.frameNo;
            }
        );
    }

    for (auto& boneMotion : loadMotion.motion) {
        auto itBoneNode = boneNodeTable.find(boneMotion.first);
        if (itBoneNode == boneNodeTable.end()) {
            continue;
        }
        auto& node = itBoneNode->second;

        auto& pos = node.startPos;
        auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
            * XMMatrixRotationQuaternion(boneMotion.second[0].quaternion)
            * XMMatrixTranslation(pos.x, pos.y, pos.z);
        boneMatrices[node.boneIndex] = mat;
    }

    RecursiveMatrixMultiply(
        &boneNodeTable["�Z���^�["],
        XMMatrixIdentity()
    );

    HRESULT res = constBuff->Map(
        0,
        nullptr,
        (void**)&mapMatrix
    );
    std::copy(
        boneMatrices.begin(),
        boneMatrices.end(),
        mapMatrix->bones
    );
    constBuff->Unmap(0, nullptr);
}

void PMDObject::MotionUpdate(){
    //�v�����Ԃ𑪂�
    auto elapsedTime = timeGetTime() - startTime;
    //�o�߃t���[�����̌v�Z
    unsigned int frameTime = 30 * (elapsedTime / 1000.0f);
    //���[�v�p����
    if (frameTime > loadMotion.duration) {
        startTime = timeGetTime();
        frameTime = 0;
    }

    //�s����̃N���A
    //(�N���A���ĂȂ��ƑO�t���[���̃|�[�Y���d�ˊ|�������)
    std::fill(
        boneMatrices.begin(),
        boneMatrices.end(),
        XMMatrixIdentity()
    );

    //���[�V�����f�[�^�X�V
    for (auto& boneMotion : loadMotion.motion) {
        //auto node = boneNodeTable[boneMotion.first];
        auto& boneName = boneMotion.first;
        auto itBoneNode = boneNodeTable.find(boneName);
        if (itBoneNode == boneNodeTable.end()) {
            continue;
        }

        auto node = itBoneNode->second;

        //���v������̂�T��
        auto motions = boneMotion.second;
        auto it = std::find_if(
            motions.rbegin(),
            motions.rend(),
            [frameTime](const Motion& motion) {
                return motion.frameNo <= frameTime;
            }
        );
        //���v������̂��Ȃ���Ώ������΂�
        if (it == motions.rend()) {
            continue;
        }

        auto& pos = node.startPos;

        XMMATRIX rotation = XMMatrixIdentity();
        XMVECTOR offset = XMLoadFloat3(&it->offset);
        auto rotIt = it.base();

        if (rotIt != motions.end()) {
            auto t = static_cast<float>(frameTime - it->frameNo)
                / static_cast<float>(rotIt->frameNo - it->frameNo);
            t = GetYFromXOnBezier(t, rotIt->p1, rotIt->p2, 12);

            /*rotation = XMMatrixRotationQuaternion(it->quaternion)
                * (1 - t)
                + XMMatrixRotationQuaternion(rotIt->quaternion)
                * t;*/
            rotation = XMMatrixRotationQuaternion(
                XMQuaternionSlerp(
                    it->quaternion,
                    rotIt->quaternion,
                    t
                )
            );
            offset = XMVectorLerp(
                offset,
                XMLoadFloat3(&rotIt->offset),
                t
            );
        }
        else {
            rotation = XMMatrixRotationQuaternion(it->quaternion);
        }

        auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
            * rotation
            * XMMatrixTranslation(pos.x, pos.y, pos.z);
        boneMatrices[node.boneIndex] = mat * XMMatrixTranslationFromVector(offset);
    }

    RecursiveMatrixMultiply(
        &boneNodeTable["�Z���^�["],
        XMMatrixIdentity()
    );

    PickUpIKSolve(frameTime);

    HRESULT res = constBuff->Map(
        0,
        nullptr,
        (void**)&mapMatrix
    );
    std::copy(
        boneMatrices.begin(),
        boneMatrices.end(),
        mapMatrix->bones
    );
    constBuff->Unmap(0, nullptr);
}

HRESULT PMDObject::CreateMaterialBuff(){
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

ID3D12Resource* PMDObject::LoadTextureFromFile(
    std::string& texPath
){
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

ID3D12Resource* PMDObject::CreateWhiteTexture(){
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

ID3D12Resource* PMDObject::CreateBlackTexture(){
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

std::string PMDObject::GetTexturePathFromModelAndTexPath(
    const std::string& modelPath,
    const char* texPath
){
    int pathIdx1 = modelPath.rfind('/');
    int pathIdx2 = modelPath.rfind('\\');

    int pathIndex = max(pathIdx1, pathIdx2);
    auto folderPath = modelPath.substr(
        0,
        pathIndex + 1
    );
    return folderPath + texPath;
}

std::wstring PMDObject::GetWideStringFromString(
    const std::string& str
){
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

std::string PMDObject::GetExtension(
    const std::string& path
){
    int index = path.rfind('.');
    return path.substr(
        index + 1,
        path.length() - index - 1
    );
}

std::pair<std::string, std::string> PMDObject::SplitFilePath(
    const std::string& path,
    const char splitter
){
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

void PMDObject::RecursiveMatrixMultiply(
    BoneNode* node,
    const XMMATRIX& mat
){
    boneMatrices[node->boneIndex] *= mat;
    for (auto& cnode : node->children) {
        RecursiveMatrixMultiply(
            cnode,
            boneMatrices[node->boneIndex]
        );
    }
}

float PMDObject::GetYFromXOnBezier(
    float x,
    const XMFLOAT2& a,
    const XMFLOAT2& b,
    uint8_t n
){
    if (a.x == a.y
        && b.x == b.y) {
        return x; //�v�Z���Ȃ�
    }

    float t = x;
    const float k0 = 1 + 3 * a.x - 3 * b.x; //t^3�̌W��
    const float k1 = 3 * b.x - 6 * a.x;     //t^2�̌W��
    const float k2 = 3 * a.x;               //t�̌W��

    //�덷�͈̔͂��m�F����萔
    constexpr float epsilon = 0.0005f;

    //t���ߎ��ŋ��߂�
    for (int i = 0; i < n; ++i) {
        //f(t)�����߂�
        auto ft = k0 * t * t * t
            + k1 * t * t +
            k2 * t
            - x;
        //���ʂ�0�ɋ߂Â����^�C�~���O�őł��؂�
        if (ft <= epsilon
            && ft >= -epsilon) {
            break;
        }

        t -= ft / 2; //2����
    }

    //y���v�Z
    auto r = 1 - t;
    return t * t * t
        + 3 * t * t * r * b.y
        + 3 * t * r * r * a.y;
}

void PMDObject::PickUpIKSolve(
    int frameNo
){
    //�t���猟��
    auto it = find_if(
        loadMotion.ikEnableData.rbegin(),
        loadMotion.ikEnableData.rend(),
        [frameNo](const VMDIKEnable& ikEnable) {
            return ikEnable.frameNo <= frameNo;
        }
    );

    //IK�����̂��߂̃��[�v
    for (auto& ik : pmdIkData) {
        if (it != loadMotion.ikEnableData.rend()) {
            auto ikEnableIt = it->ikEnableTable.find(
                boneNameArray[ik.boneIndex]
            );

            if (ikEnableIt != it->ikEnableTable.end()) {
                //�����I�t�Ȃ�ł��؂�
                if (!ikEnableIt->second) {
                    continue;
                }
            }
        }
        auto childrenNodesCount = ik.nodeIndices.size();

        switch (childrenNodesCount) {
        case 0: //�Ԃ̃{�[����0�̂Ƃ�(��O����)
            assert(0);
            continue;
        case 1: //�Ԃ̃{�[����1�̂Ƃ�
            SolveLookAt(ik);
            break;
        case 2: //�Ԃ̃{�[����2�̂Ƃ�
            SolveCosineIK(ik);
            break;
        default: //�Ԃ̃{�[����3�ȏ�̂Ƃ�
            SolveCCDIK(ik);
        }
    }
}

void PMDObject::SolveCCDIK(
    const PMDIK& ik
){
    //�^�[�Q�b�g
    auto targetBoneNode = boneNodeAddressArray[ik.boneIndex];
    auto targetOriginPos = XMLoadFloat3(
        &targetBoneNode->startPos
    );

    auto parentMat = boneMatrices[boneNodeAddressArray[ik.boneIndex]->ikParentBone];

    XMVECTOR det;
    auto invParentMat = XMMatrixInverse(
        &det,
        parentMat
    );
    auto targetNextPos = XMVector3Transform(
        targetOriginPos,
        boneMatrices[ik.boneIndex] * invParentMat
    );

    //IK�̊Ԃɂ���{�[���̍��W�̓��ꕨ
    std::vector<XMVECTOR> bonePositions;

    //���[�m�[�h
    auto endPos = XMLoadFloat3(
        &boneNodeAddressArray[ik.targetIndex]->startPos
    );

    //���ԃm�[�h(���[�g���܂�)
    for (auto& ci : ik.nodeIndices) {
        bonePositions.push_back(
            XMLoadFloat3(
                &boneNodeAddressArray[ci]->startPos
            )
        );
    }

    //IK�̊Ԃɂ���{�[���̉�]�s��̓��ꕨ
    std::vector<XMMATRIX> mats(bonePositions.size());
    fill(mats.begin(), mats.end(), XMMatrixIdentity());

    auto ikLimit = ik.limit * XM_PI;

    //ik�ɐݒ肳��Ă��鎎�s�񐔂����J��Ԃ�
    for (int c = 0; c < ik.iterations; ++c) {
        //�^�[�Q�b�g�Ɩ��[���قڈ�v�����甲����
        if (XMVector3Length(
            XMVectorSubtract(
                endPos,
                targetNextPos
            )
        ).m128_f32[0] <= epsilon)break;

        //���ꂼ��̃{�[���������̂ڂ�Ȃ���
        //�p�x�����Ɉ���������Ȃ��悤�ɋȂ��Ă���
        for (int bi = 0; bi < bonePositions.size(); ++bi) {
            const auto& pos = bonePositions[bi];

            //�Ώۃm�[�h���疖�[�m�[�h�܂ł�
            //�Ώۃm�[�h����^�[�Q�b�g�܂ł̃x�N�g���쐬
            auto vec2End = XMVectorSubtract(
                endPos,
                pos
            ); //���[��
            auto vec2Target = XMVectorSubtract(
                targetNextPos,
                pos
            ); //�^�[�Q�b�g��

            //�������K��
            vec2End = XMVector3Normalize(
                vec2End
            );
            vec2Target = XMVector3Normalize(
                vec2Target
            );

            //�����قړ����x�N�g�����ƊO�ς��ł��Ȃ��̂�
            //���̃{�[���ֈ����n��
            if (XMVector3Length(
                XMVectorSubtract(
                    vec2End,
                    vec2Target
                )
            ).m128_f32[0] <= epsilon)continue;

            //�O�όv�Z�y�ъp�x�v�Z
            auto cross = XMVector3Normalize(
                XMVector3Cross(
                    vec2End,
                    vec2Target
                )
            ); //���ɂȂ�

            float angle = XMVector3AngleBetweenVectors(
                vec2End,
                vec2Target
            ).m128_f32[0];
            //��]���E�𒴂��Ă��܂����Ƃ��͌��E�l�ɕ␳
            angle = min(angle, ikLimit);
            //��]�s��쐬
            XMMATRIX rot = XMMatrixRotationAxis(
                cross,
                angle
            );
            //���_���S�łȂ�pos���S�ɉ�]
            auto mat = XMMatrixTranslationFromVector(-pos)
                * rot
                * XMMatrixTranslationFromVector(pos);
            //��]�s���ێ����Ă���
            mats[bi] *= mat;

            //�ΏۂƂȂ�_�����ׂĉ�]������
            for (auto i = bi - 1; i >= 0; --i) {
                bonePositions[i] = XMVector3Transform(
                    bonePositions[i],
                    mat
                );
            }

            endPos = XMVector3Transform(
                endPos,
                mat
            );

            //���������ɋ߂��Ȃ��Ă����烋�[�v�𔲂���
            if (XMVector3Length(
                XMVectorSubtract(
                    endPos,
                    targetNextPos
                )
            ).m128_f32[0] <= epsilon)break;
        }
    }

    int index = 0;
    for (auto& ci : ik.nodeIndices) {
        boneMatrices[ci] = mats[index];
        ++index;
    }

    auto rootNode = boneNodeAddressArray[ik.nodeIndices.back()];
    RecursiveMatrixMultiply(
        rootNode,
        parentMat
    );
}

void PMDObject::SolveCosineIK(
    const PMDIK& ik
){
    //IK�\���_��ۑ�
    std::vector<XMVECTOR> positions;

    //IK�̂��ꂼ��̃{�[���Ԃ̋�����ۑ�
    std::array<float, 2> edgeLengths;

    //�^�[�Q�b�g(���[�{�[���ł͂Ȃ��A���[�{�[�����߂Â��ڕW�{�[���̍��W���擾)
    auto& targetNode = boneNodeAddressArray[ik.boneIndex];
    auto targetPos = XMVector3Transform(
        XMLoadFloat3(&targetNode->startPos),
        boneMatrices[ik.boneIndex]
    );

    //IK�`�F�[�����t���Ȃ��߁A�t�ɕ��Ԃ悤�ɂ���
    //���[�{�[��
    auto endNode = boneNodeAddressArray[ik.targetIndex];
    positions.emplace_back(
        XMLoadFloat3(&endNode->startPos)
    );

    //���ԋy�у��[�g�{�[��
    for (auto& chainBoneIndex : ik.nodeIndices) {
        auto boneNode = boneNodeAddressArray[chainBoneIndex];

        positions.emplace_back(
            XMLoadFloat3(&boneNode->startPos)
        );
    }

    //������Â炢�̂ŋt�ɂ���
    reverse(positions.begin(), positions.end());

    //���̒����𑪂��Ă���
    edgeLengths[0] = XMVector3Length(
        XMVectorSubtract(
            positions[1],
            positions[0]
        )
    ).m128_f32[0];
    edgeLengths[1] = XMVector3Length(
        XMVectorSubtract(
            positions[2],
            positions[1]
        )
    ).m128_f32[0];

    //���[�g�{�[�����W����
    positions[0] = XMVector3Transform(
        positions[0],
        boneMatrices[ik.nodeIndices[1]]
    );

    //���Ԃ͎����v�Z�����̂Ōv�Z���Ȃ�

    //��[�{�[��
    positions[2] = XMVector3Transform(
        positions[2],
        boneMatrices[ik.boneIndex]
    );

    //���[�g�����[�ւ̃x�N�g��������Ă���
    auto linearVec = XMVectorSubtract(
        positions[2],
        positions[0]
    );

    float A = XMVector3Length(linearVec).m128_f32[0];
    float B = edgeLengths[0];
    float C = edgeLengths[1];

    linearVec = XMVector3Normalize(linearVec);

    //���[�g����^�񒆂ւ̊p�x�v�Z
    float theta1 = acosf((A * A + B * B - C * C) / (2 * A * B));
    //�^�񒆂���^�[�Q�b�g�ւ̊p�x�v�Z
    float theta2 = acosf((B * B + C * C - A * A) / (2 * B * C));

    //�������߂�
    //�����^�񒆂��Ђ��ł������ꍇ�ɂ͋����I��x���Ƃ���
    XMVECTOR axis;
    if (find(
        kneeIndices.begin(),
        kneeIndices.end(),
        ik.nodeIndices[0]) == kneeIndices.end()
        ) {
        auto vm = XMVector3Normalize(
            XMVectorSubtract(
                positions[2],
                positions[0]
            )
        );
        auto vt = XMVector3Normalize(
            XMVectorSubtract(
                targetPos,
                positions[0]
            )
        );
        axis = XMVector3Cross(vt, vm);
    }
    else {
        auto right = XMFLOAT3(1, 0, 0);
        axis = XMLoadFloat3(&right);
    }

    //IK�`�F�[���̓��[�g�Ɍ������Ă��琔�����邽��1�����[�g�ɋ߂�
    auto mat1 = XMMatrixTranslationFromVector(
        -positions[0]
    );
    mat1 *= XMMatrixRotationAxis(
        axis,
        theta1
    );
    mat1 *= XMMatrixTranslationFromVector(
        positions[0]
    );

    auto mat2 = XMMatrixTranslationFromVector(
        -positions[1]
    );
    mat2 *= XMMatrixRotationAxis(
        axis,
        theta2 - XM_PI
    );
    mat2 *= XMMatrixTranslationFromVector(
        positions[1]
    );

    boneMatrices[ik.nodeIndices[1]] *= mat1;
    boneMatrices[ik.nodeIndices[0]] = mat2 * boneMatrices[ik.nodeIndices[1]];
    boneMatrices[ik.targetIndex] = boneMatrices[ik.nodeIndices[0]];
}

void PMDObject::SolveLookAt(
    const PMDIK& ik
){
    auto rootNode = boneNodeAddressArray[ik.nodeIndices[0]];
    auto targetNode = boneNodeAddressArray[ik.targetIndex];

    auto opos1 = XMLoadFloat3(
        &rootNode->startPos
    );
    auto tpos1 = XMLoadFloat3(
        &targetNode->startPos
    );

    auto opos2 = XMVector3TransformCoord(
        opos1,
        boneMatrices[ik.nodeIndices[0]]
    );
    auto tpos2 = XMVector3TransformCoord(
        tpos1,
        boneMatrices[ik.boneIndex]
    );

    auto originVec = XMVectorSubtract(tpos1, opos1);
    auto targetVec = XMVectorSubtract(tpos2, opos2);

    originVec = XMVector3Normalize(originVec);
    targetVec = XMVector3Normalize(targetVec);

    XMFLOAT3 upvec = { 0, 1, 0 };
    XMFLOAT3 rightvec = { 1,0,0 };
    boneMatrices[ik.nodeIndices[0]] = XMMatrixTranslationFromVector(-opos2)
        * LookAtMatrix(
            originVec,
            targetVec,
            upvec,
            rightvec
        )
        * XMMatrixTranslationFromVector(opos2);
}

XMMATRIX PMDObject::LookAtMatrix(
    const XMVECTOR& lookat,
    XMFLOAT3& up,
    XMFLOAT3& right
){
    //��������������(z��)
    XMVECTOR vz = lookat;

    //(�������������������������Ƃ���)����y���x�N�g��
    XMVECTOR vy = XMVector3Normalize(
        XMLoadFloat3(&up)
    );

    //(�������������������������Ƃ���)y��
    /*XMVECTOR vx = XMVector3Normalize(
        XMVector3Cross(vz, vx)
    );*/
    XMVECTOR vx = XMVector3Normalize(
        XMVector3Cross(vy, vz)
    );
    vy = XMVector3Normalize(
        XMVector3Cross(vz, vx)
    );

    //LookAt��up�����������������Ă�����right����ɍ�蒼��
    if (std::abs(XMVector3Dot(vy, vz).m128_f32[0]) == 1.0f) {
        //����x�������`
        vx = XMVector3Normalize(
            XMLoadFloat3(&right)
        );

        //�������������������������Ƃ���Y�����v�Z
        vy = XMVector3Normalize(
            XMVector3Cross(vz, vx)
        );

        //�^��x�����v�Z
        vx = XMVector3Normalize(
            XMVector3Cross(vy, vz)
        );
    }

    XMMATRIX ret = XMMatrixIdentity();
    ret.r[0] = vx;
    ret.r[1] = vy;
    ret.r[2] = vz;

    return ret;
}

XMMATRIX PMDObject::LookAtMatrix(
    const XMVECTOR& origin,
    const XMVECTOR& lookat,
    XMFLOAT3& up,
    XMFLOAT3& right
){
    return XMMatrixTranspose(LookAtMatrix(origin, up, right))
        * LookAtMatrix(lookat, up, right);
}

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

//頂点1つあたりのサイズ
const size_t pmdvertex_size = 38;

const float epsilon = 1.0e-5f;

namespace {
    //ボーン種別
    enum class BoneType {
        Rotation,      //回転
        RotAndMove,    //回転＆移動
        IK,            //IK
        Undefind,      //未定義
        IKChild,       //IK影響ボーン
        RotationChild, //回転影響ボーン
        IKDestination, //IK接続先
        Invisible      //見えないボーン
    };
}

PMDObject::~PMDObject(){
}

void PMDObject::Initialize(){
    //nullptrチェック
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

    // スケール、回転、平行移動行列の計算
    matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
    matRot = XMMatrixIdentity();
    matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
    matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
    matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
    matTrans = XMMatrixTranslation(position.x, position.y, position.z);

    // ワールド行列の合成
    matWorld = XMMatrixIdentity(); // 変形をリセット

    matWorld *= matScale; // ワールド行列にスケーリングを反映
    matWorld *= matRot; // ワールド行列に回転を反映
    matWorld *= matTrans; // ワールド行列に平行移動を反映

    //行列を定数バッファにコピー
    //SceneMatrix* mapMatrix; //マップ先のポインター
    res = constBuff->Map(
        0,
        nullptr,
        (void**)&mapMatrix
    );
    if (FAILED(res)) {
        assert(0);
    }
    //行列のコピー
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
    // パイプラインステートの設定
    cmdList->SetPipelineState(pipelineState.Get());
    // ルートシグネチャの設定
    cmdList->SetGraphicsRootSignature(rootSignature.Get());
    // プリミティブ形状を設定
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //頂点情報のセット
    cmdList->IASetVertexBuffers(0, 1, &vbView);
    //インデックス情報のセット
    cmdList->IASetIndexBuffer(&ibView);


    //変換行列のセット
    ID3D12DescriptorHeap* constDH = constDescHeap.Get();
    cmdList->SetDescriptorHeaps(1, &constDH);
    cmdList->SetGraphicsRootDescriptorTable(
        0,
        constDescHeap->GetGPUDescriptorHandleForHeapStart()
    );

    //マテリアル
    ID3D12DescriptorHeap* matDH = materialDescHeap.Get();
    cmdList->SetDescriptorHeaps(1, &matDH);
    //ヒープ先頭
    CD3DX12_GPU_DESCRIPTOR_HANDLE materialH = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        materialDescHeap->GetGPUDescriptorHandleForHeapStart()
    );
    unsigned int idxOffset = 0; //最初はオフセットなし
    auto cbvSrvIncSize = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    ) * 4; //4倍

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
        //ヒープポインターをインデックスを次に進める
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

    //頂点バッファ
    res = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOADヒープとして
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertices.size()), //サイズに応じて適切な設定をしてくれる
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertBuff)
    );
    if (FAILED(res)) {
        assert(0);
        return S_FALSE;
    }

    res = S_FALSE;
    //バッファに頂点データコピー
    unsigned char* vertMap = nullptr;
    res = vertBuff->Map(0, nullptr, (void**)&vertMap);
    if (FAILED(res)) {
        assert(0);
        return S_FALSE;
    }
    copy(vertices.begin(), vertices.end(), vertMap);
    vertBuff->Unmap(0, nullptr);
    //頂点バッファビュー
    vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); //バッファーの仮想アドレス
    vbView.SizeInBytes = static_cast<UINT>(vertices.size());
    vbView.StrideInBytes = pmdvertex_size; //1頂点あたりのバイト数

    return res;
}

HRESULT PMDObject::CreateIndexBuff(){
    HRESULT res = S_FALSE;

    //インデックスバッファ
    res = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOADヒープとして
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(
            indices.size() * sizeof(indices[0])
        ), //サイズに応じて適切な設定をしてくれる,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&idxBuff)
    );
    if (FAILED(res)) {
        assert(0);
        return S_FALSE;
    }

    res = S_FALSE;
    //作ったバッファにインデックスデータコピー
    unsigned short* mappedIdx = nullptr;
    res = idxBuff->Map(0, nullptr, (void**)&mappedIdx);
    if (FAILED(res)) {
        assert(0);
        return S_FALSE;
    }
    copy(indices.begin(), indices.end(), mappedIdx);
    idxBuff->Unmap(0, nullptr);
    //インデックスバッファビューを作成
    ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
    ibView.Format = DXGI_FORMAT_R16_UINT;
    ibView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(indices[0]));

    return res;
}

HRESULT PMDObject::ShaderCompile(){
    HRESULT res = S_FALSE;

    res = D3DCompileFromFile(
        L"PMDVertexShader.hlsl",                       //シェーダー名
        nullptr,                                         //defineはなし
        D3D_COMPILE_STANDARD_FILE_INCLUDE,               //インクルードはデフォルト
        "VSmain", "vs_5_0",                              //関数はVSmain,対象シェーダーはvs_5_0
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //デバッグ用 & 最適化なし
        0,
        &vsBlob, &errorBlob);                          //エラー時にerrorBlobにメッセージが入る
    if (FAILED(res))
        DebugShader(res);

    res = S_FALSE;
    res = D3DCompileFromFile(
        L"PMDPixelShader.hlsl",                        //シェーダー名
        nullptr,                                         //defineはなし
        D3D_COMPILE_STANDARD_FILE_INCLUDE,               //インクルードはデフォルト
        "PSmain", "ps_5_0",                              //関数はPSmain,対象シェーダーはps_5_0
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //デバッグ用 & 最適化なし
        0,
        &psBlob, &errorBlob);                          //エラー時にerrorBlobにメッセージが入る
    if (FAILED(res))
        DebugShader(res);

    return res;
}

HRESULT PMDObject::CreateRootSignature(){
    HRESULT res = S_FALSE;

    //デスクリプタレンジ
    CD3DX12_DESCRIPTOR_RANGE descTableRanges[3] = {}; //テクスチャと定数2つ
    //定数用レジスター0番 行列
    descTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    //定数用レジスター1番 マテリアル
    descTableRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    //テクスチャ用レジスター0番 テクスチャ3つ 基本 sph spa
    descTableRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

    //ルートパラメーター
    CD3DX12_ROOT_PARAMETER rootParams[2] = {};
    //定数
    rootParams[0].InitAsDescriptorTable(1, &descTableRanges[0]);
    //マテリアル
    rootParams[1].InitAsDescriptorTable(2, &descTableRanges[1]);

    //サンプラー
    CD3DX12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Init(0);

    //ルートシグネチャの作成
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Init(
        _countof(rootParams), //ルートパラメーター数
        rootParams,           //ルートパラメーターの先頭アドレス
        1,                    //サンプラー数
        &samplerDesc,         //サンプラーの先頭アドレス
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );
    //バイナリコードの作成
    res = D3D12SerializeRootSignature(
        &rootSignatureDesc,             //ルートシグネチャ設定
        D3D_ROOT_SIGNATURE_VERSION_1_0, //ルートシグネチャバージョン
        &rootSigBlob,                   //シェーダーを作った時と同じ
        &errorBlob                     //エラー処理
    );
    if (FAILED(res)) {
        assert(0);
        return S_FALSE;
    }

    res = S_FALSE;
    //ルートシグネチャオブジェクトの生成
    res = device->CreateRootSignature(
        0,                               //nodemask
        rootSigBlob->GetBufferPointer(), //シェーダーのときと同様
        rootSigBlob->GetBufferSize(),    //シェーダーのときと同様
        IID_PPV_ARGS(&rootSignature)
    );
    if (FAILED(res)) {
        assert(0);
        return S_FALSE;
    }
    rootSigBlob->Release(); //不要になったので解放

    return res;
}

HRESULT PMDObject::CreateGraphicsPipeline(){
    HRESULT res = S_FALSE;

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { //座標情報
            "POSITION",0,
            DXGI_FORMAT_R32G32B32_FLOAT,0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
        },
        { //法線
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
        { //ボーン番号
            "BONE_NO",0,
            DXGI_FORMAT_R16G16_UINT,0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
        },
        { //ボーンウェイト
            "WEIGHT",0,
            DXGI_FORMAT_R8_UINT,0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
        },
        { //輪郭線フラグ
            "EDGE_FLG",0,
            DXGI_FORMAT_R8_UINT,0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
        },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipeline = {};
    //シグネチャ
    gPipeline.pRootSignature = rootSignature.Get();
    //頂点シェーダー
    gPipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
    //ピクセルシェーダー
    gPipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
    //サンプルマスク
    gPipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; //デフォルト
    //ラスタライザーステート
    gPipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); //デフォルト
    gPipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //カリングしない
    //深度設定
    gPipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    gPipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    //ブレンドステート
    gPipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    //入力レイアウト
    gPipeline.InputLayout.pInputElementDescs = inputLayout; //レイアウト先頭アドレス
    gPipeline.InputLayout.NumElements = _countof(inputLayout); //レイアウト配列の要素数
    //頂点の切り離し設定
    gPipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; //カットなし
    //プリミティブトポロジー
    gPipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; //三角形で構成
    //レンダーターゲット
    gPipeline.NumRenderTargets = 1; //1つ
    gPipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0~1に正規化されたRGBA
    //アンチエイリアシング
    gPipeline.SampleDesc.Count = 1; //サンプリングは1ピクセルにつき1
    gPipeline.SampleDesc.Quality = 0; //クオリティは最低

    //グラフィックパイプラインステートオブジェクト生成
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
    //PMDモデルデータ
    char signature[3] = {}; //シグネチャ
    auto fp = fopen(strModelPath.c_str(), "rb");

    fread(signature, sizeof(signature), 1, fp);
    fread(&pmdheader, sizeof(pmdheader), 1, fp);

    //頂点の読み込み
    fread(&vertNum, sizeof(vertNum), 1, fp);
    vertices.resize(vertNum * pmdvertex_size); //バッファの確保
    fread(vertices.data(), vertices.size(), 1, fp); //読み込み
    //インデックスの読み込み
    fread(&indicesNum, sizeof(indicesNum), 1, fp);
    indices.resize(indicesNum);
    fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);
    //マテリアル読み込み
    fread(&materialNum, sizeof(materialNum), 1, fp);
    std::vector<PMDMaterial> pmdMaterials(materialNum);
    fread(
        pmdMaterials.data(),
        pmdMaterials.size() * sizeof(PMDMaterial),
        1,
        fp
    );

    materials.resize(pmdMaterials.size());
    //コピー
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
    //リソースのロード
    for (int i = 0; i < pmdMaterials.size(); ++i) {
        if (strlen(pmdMaterials[i].texFilePath) == 0) {
            textureResources[i] = nullptr;
        }

        //テクスチャパスの分離を確認
        std::string texFileName = pmdMaterials[i].texFilePath;
        std::string sphereFileName = "";
        std::string addSphFileName = "";
        if (count(texFileName.begin(), texFileName.end(), '*') > 0) {
            //スプリッタがある
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
            //モデルとテクスチャパスからアプリケーションからのテクスチャパスを得る
            auto texFilePath = GetTexturePathFromModelAndTexPath(
                strModelPath,
                texFileName.c_str()
            );
            textureResources[i] = LoadTextureFromFile(texFilePath);
        }


        if (sphereFileName != "") {
            //アプリケーションからみたスフィアマップのパス
            auto sphFilePath = GetTexturePathFromModelAndTexPath(
                strModelPath,
                sphereFileName.c_str()
            );
            sphereResources[i] = LoadTextureFromFile(sphFilePath);
        }

        if (addSphFileName != "") {
            //アプリケーションからみた加算スフィアマップのパス
            auto spaFilePath = GetTexturePathFromModelAndTexPath(
                strModelPath,
                addSphFileName.c_str()
            );
            addSphResources[i] = LoadTextureFromFile(spaFilePath);
        }
    }

    //ボーン数
    unsigned short boneNum = 0;
    fread(&boneNum, sizeof(boneNum), 1, fp);

    //ボーン情報
    pmdBones.resize(boneNum);
    fread(pmdBones.data(), sizeof(PMDBone), boneNum, fp);

    uint16_t ikNum = 0;
    fread(&ikNum, sizeof(ikNum), 1, fp);

    pmdIkData.resize(ikNum);
    for (auto& ik : pmdIkData) {
        fread(&ik.boneIndex, sizeof(ik.boneIndex), 1, fp);
        fread(&ik.targetIndex, sizeof(ik.targetIndex), 1, fp);

        uint8_t chainLength = 0; //間にいくつノードがあるか
        fread(&chainLength, sizeof(chainLength), 1, fp);
        ik.nodeIndices.resize(chainLength);
        fread(&ik.iterations, sizeof(ik.iterations), 1, fp);
        fread(&ik.limit, sizeof(ik.limit), 1, fp);

        if (chainLength == 0) continue; //間のノードが0ならばここで終わり

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

        //インデックス計算がしやすいように
        boneNameArray[i] = pb.boneName;
        boneNodeAddressArray[i] = &node;

        std::string boneName = pb.boneName;
        if (boneName.find("ひざ") != std::string::npos) {
            kneeIndices.emplace_back(i);
        }
    }
    //親子関係の構築
    for (auto& pb : pmdBones) {
        //親インデックスをチェック
        if (pb.parentNo >= pmdBones.size()) {
            continue;
        }

        auto parentName = boneNameArray[pb.parentNo];
        boneNodeTable[parentName].children.emplace_back(
            &boneNodeTable[pb.boneName]
        );
    }

    boneMatrices.resize(pmdBones.size());
    //ボーンをすべて初期化する
    std::fill(
        boneMatrices.begin(),
        boneMatrices.end(),
        XMMatrixIdentity()
    );

#ifndef DEBUG
    //IKデバッグ用
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
        oss << "IKボーン番号＝" << ik.boneIndex << "：" << getNameFromIndex(ik.boneIndex) << std::endl;

        for (auto& node : ik.nodeIndices) {
            oss << "\tノードボーン＝" << node << "：" << getNameFromIndex(node) << std::endl;
        }

        OutputDebugStringA(
            oss.str().c_str()
        );
    }
#endif // DEBUG
}

void PMDObject::InitMatrix(){
    //ワールド行列
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

    //デスクリプタヒープ
    D3D12_DESCRIPTOR_HEAP_DESC constDescHeapDesc = {};
    constDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //シェーダーから見えるように
    constDescHeapDesc.NodeMask = 0; //マスクは0
    constDescHeapDesc.NumDescriptors = 1; //CBV1つ
    constDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; //シェーダーリソースビュー用
    res = device->CreateDescriptorHeap(
        &constDescHeapDesc,
        IID_PPV_ARGS(&constDescHeap)
    );

    //先頭ハンドルを取得
    auto constHeapHandle = constDescHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(constBuff->GetDesc().Width);

    //定数バッファビュー作成
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
        &boneNodeTable["センター"],
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
    //計測時間を測る
    auto elapsedTime = timeGetTime() - startTime;
    //経過フレーム数の計算
    unsigned int frameTime = 30 * (elapsedTime / 1000.0f);
    //ループ用処理
    if (frameTime > loadMotion.duration) {
        startTime = timeGetTime();
        frameTime = 0;
    }

    //行列情報のクリア
    //(クリアしてないと前フレームのポーズが重ね掛けされる)
    std::fill(
        boneMatrices.begin(),
        boneMatrices.end(),
        XMMatrixIdentity()
    );

    //モーションデータ更新
    for (auto& boneMotion : loadMotion.motion) {
        //auto node = boneNodeTable[boneMotion.first];
        auto& boneName = boneMotion.first;
        auto itBoneNode = boneNodeTable.find(boneName);
        if (itBoneNode == boneNodeTable.end()) {
            continue;
        }

        auto node = itBoneNode->second;

        //合致するものを探す
        auto motions = boneMotion.second;
        auto it = std::find_if(
            motions.rbegin(),
            motions.rend(),
            [frameTime](const Motion& motion) {
                return motion.frameNo <= frameTime;
            }
        );
        //合致するものがなければ処理を飛ばす
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
        &boneNodeTable["センター"],
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

    //マテリアルバッファを作成
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
    //マップマテリアルにコピー
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
        *((MaterialForHlsl*)mapMaterial) = m.material; //データコピー
        mapMaterial += materialBuffSize; //次のアライメント位置まで進める
    }
    materialBuff->Unmap(0, nullptr);

    res = S_FALSE;
    //デスクリプタヒープの作成
    D3D12_DESCRIPTOR_HEAP_DESC matDescHeapDesc = {};
    matDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    matDescHeapDesc.NodeMask = 0;
    matDescHeapDesc.NumDescriptors = materialNum * 4; //マテリアル数を指定
    matDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    res = device->CreateDescriptorHeap(
        &matDescHeapDesc,
        IID_PPV_ARGS(&materialDescHeap)
    );
    if (FAILED(res)) {
        assert(0);
        return S_FALSE;
    }

    //ビューの作成
    D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
    matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress(); //バッファアドレス
    matCBVDesc.SizeInBytes = static_cast<UINT>(materialBuffSize); //マテリアルの256アライメントサイズ

    //通常テクスチャビューの作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //デフォルト
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2Dテクスチャ
    srvDesc.Texture2D.MipLevels = 1; //ミップマップは使用しないので1

    //先頭アドレスを記録
    CD3DX12_CPU_DESCRIPTOR_HANDLE matDescHeapH = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        materialDescHeap->GetCPUDescriptorHandleForHeapStart()
    );

    auto increment = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );

    ComPtr<ID3D12Resource> whiteTex = CreateWhiteTexture();
    ComPtr<ID3D12Resource> blackTex = CreateBlackTexture();
    for (int i = 0; i < materialNum; ++i) {
        //マテリアル用定数バッファビュー
        device->CreateConstantBufferView(
            &matCBVDesc,
            matDescHeapH
        );
        matDescHeapH.ptr += increment;
        matCBVDesc.BufferLocation += materialBuffSize;

        //シェーダーリソースビュー
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

        //スフィアマップリソースビュー
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

        //加算スフィアマップリソースビュー
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
    //WICテクスチャのロード
    TexMetadata metadata = {};
    ScratchImage scratchImg = {};

    //テクスチャのファイルパス
    auto wTexPath = GetWideStringFromString(texPath);
    //拡張子を取得
    auto ext = GetExtension(texPath);

    HRESULT res = S_FALSE;

    //拡張子によって関数使い分け
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


    auto img = scratchImg.GetImage(0, 0, 0); //生データ抽出

    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        metadata.format,
        metadata.width,             //幅
        (UINT)metadata.height,      //高さ
        (UINT16)metadata.arraySize,
        (UINT16)metadata.mipLevels
    );

    //バッファ作成
    ID3D12Resource* texBuff = nullptr;
    res = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(
            D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
            D3D12_MEMORY_POOL_L0
        ), //WriteToSubresourceで転送する用のヒープ設定
        D3D12_HEAP_FLAG_NONE, //特に指定なし
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
        nullptr,        //全領域へコピー
        img->pixels,    //元データサイズ
        img->rowPitch,  //ラインサイズ
        img->slicePitch //全サイズ
    );
    if (FAILED(res)) {
        return nullptr;
    }

    return texBuff;
}

ID3D12Resource* PMDObject::CreateWhiteTexture(){
    //白テクスチャのバッファ生成
    ID3D12Resource* whiteBuff = nullptr;

    auto res = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(
            D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
            D3D12_MEMORY_POOL_L0
        ), //ヒーププロパティ
        D3D12_HEAP_FLAG_NONE, //特に指定なし
        &CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            4, //幅
            4  //高さ
        ), //リソースデスクリプタ
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&whiteBuff)
    );
    if (FAILED(res)) {
        return nullptr;
    }

    //白テクスチャデータ
    std::vector<unsigned char> data(4 * 4 * 4);
    fill(data.begin(), data.end(), 0xff); //全部255で埋める

    //データ転送
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
    //黒テクスチャのバッファ生成
    ID3D12Resource* blackBuff = nullptr;

    auto res = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(
            D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
            D3D12_MEMORY_POOL_L0
        ), //ヒーププロパティ
        D3D12_HEAP_FLAG_NONE, //特に指定なし
        &CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            4, //幅
            4  //高さ
        ), //リソースデスクリプタ
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&blackBuff)
    );
    if (FAILED(res)) {
        return nullptr;
    }

    //黒テクスチャデータ
    std::vector<unsigned char> data(4 * 4 * 4);
    fill(data.begin(), data.end(), 0x00); //全部0で埋める

    //データ転送
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
    //呼び出し1回目 文字列の個数を取得
    auto num1 = MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
        str.c_str(),
        -1,
        nullptr,
        0
    );
    std::wstring wstr; //stringのwchar_t版
    wstr.resize(num1);

    //呼び出し2回目 長さを確保したwstrに文字列をコピー
    auto num2 = MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
        str.c_str(),
        -1,
        &wstr[0],
        num1
    );

    assert(num1 == num2); //長さが一致しているか確認
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
        return x; //計算しない
    }

    float t = x;
    const float k0 = 1 + 3 * a.x - 3 * b.x; //t^3の係数
    const float k1 = 3 * b.x - 6 * a.x;     //t^2の係数
    const float k2 = 3 * a.x;               //tの係数

    //誤差の範囲か確認する定数
    constexpr float epsilon = 0.0005f;

    //tを近似で求める
    for (int i = 0; i < n; ++i) {
        //f(t)を求める
        auto ft = k0 * t * t * t
            + k1 * t * t +
            k2 * t
            - x;
        //結果が0に近づいたタイミングで打ち切り
        if (ft <= epsilon
            && ft >= -epsilon) {
            break;
        }

        t -= ft / 2; //2分割
    }

    //yを計算
    auto r = 1 - t;
    return t * t * t
        + 3 * t * t * r * b.y
        + 3 * t * r * r * a.y;
}

void PMDObject::PickUpIKSolve(
    int frameNo
){
    //逆から検索
    auto it = find_if(
        loadMotion.ikEnableData.rbegin(),
        loadMotion.ikEnableData.rend(),
        [frameNo](const VMDIKEnable& ikEnable) {
            return ikEnable.frameNo <= frameNo;
        }
    );

    //IK解決のためのループ
    for (auto& ik : pmdIkData) {
        if (it != loadMotion.ikEnableData.rend()) {
            auto ikEnableIt = it->ikEnableTable.find(
                boneNameArray[ik.boneIndex]
            );

            if (ikEnableIt != it->ikEnableTable.end()) {
                //もしオフなら打ち切る
                if (!ikEnableIt->second) {
                    continue;
                }
            }
        }
        auto childrenNodesCount = ik.nodeIndices.size();

        switch (childrenNodesCount) {
        case 0: //間のボーンが0のとき(例外処理)
            assert(0);
            continue;
        case 1: //間のボーンが1のとき
            SolveLookAt(ik);
            break;
        case 2: //間のボーンが2のとき
            SolveCosineIK(ik);
            break;
        default: //間のボーンが3以上のとき
            SolveCCDIK(ik);
        }
    }
}

void PMDObject::SolveCCDIK(
    const PMDIK& ik
){
    //ターゲット
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

    //IKの間にあるボーンの座標の入れ物
    std::vector<XMVECTOR> bonePositions;

    //末端ノード
    auto endPos = XMLoadFloat3(
        &boneNodeAddressArray[ik.targetIndex]->startPos
    );

    //中間ノード(ルートを含む)
    for (auto& ci : ik.nodeIndices) {
        bonePositions.push_back(
            XMLoadFloat3(
                &boneNodeAddressArray[ci]->startPos
            )
        );
    }

    //IKの間にあるボーンの回転行列の入れ物
    std::vector<XMMATRIX> mats(bonePositions.size());
    fill(mats.begin(), mats.end(), XMMatrixIdentity());

    auto ikLimit = ik.limit * XM_PI;

    //ikに設定されている試行回数だけ繰り返す
    for (int c = 0; c < ik.iterations; ++c) {
        //ターゲットと末端がほぼ一致したら抜ける
        if (XMVector3Length(
            XMVectorSubtract(
                endPos,
                targetNextPos
            )
        ).m128_f32[0] <= epsilon)break;

        //それぞれのボーンをさかのぼりながら
        //角度制限に引っかからないように曲げていく
        for (int bi = 0; bi < bonePositions.size(); ++bi) {
            const auto& pos = bonePositions[bi];

            //対象ノードから末端ノードまでと
            //対象ノードからターゲットまでのベクトル作成
            auto vec2End = XMVectorSubtract(
                endPos,
                pos
            ); //末端へ
            auto vec2Target = XMVectorSubtract(
                targetNextPos,
                pos
            ); //ターゲットへ

            //両方正規化
            vec2End = XMVector3Normalize(
                vec2End
            );
            vec2Target = XMVector3Normalize(
                vec2Target
            );

            //上二つがほぼ同じベクトルだと外積ができないので
            //次のボーンへ引き渡す
            if (XMVector3Length(
                XMVectorSubtract(
                    vec2End,
                    vec2Target
                )
            ).m128_f32[0] <= epsilon)continue;

            //外積計算及び角度計算
            auto cross = XMVector3Normalize(
                XMVector3Cross(
                    vec2End,
                    vec2Target
                )
            ); //軸になる

            float angle = XMVector3AngleBetweenVectors(
                vec2End,
                vec2Target
            ).m128_f32[0];
            //回転限界を超えてしまったときは限界値に補正
            angle = min(angle, ikLimit);
            //回転行列作成
            XMMATRIX rot = XMMatrixRotationAxis(
                cross,
                angle
            );
            //原点中心でなくpos中心に回転
            auto mat = XMMatrixTranslationFromVector(-pos)
                * rot
                * XMMatrixTranslationFromVector(pos);
            //回転行列を保持しておく
            mats[bi] *= mat;

            //対象となる点をすべて回転させる
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

            //もし正解に近くなっていたらループを抜ける
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
    //IK構成点を保存
    std::vector<XMVECTOR> positions;

    //IKのそれぞれのボーン間の距離を保存
    std::array<float, 2> edgeLengths;

    //ターゲット(末端ボーンではなく、末端ボーンが近づく目標ボーンの座標を取得)
    auto& targetNode = boneNodeAddressArray[ik.boneIndex];
    auto targetPos = XMVector3Transform(
        XMLoadFloat3(&targetNode->startPos),
        boneMatrices[ik.boneIndex]
    );

    //IKチェーンが逆順なため、逆に並ぶようにする
    //末端ボーン
    auto endNode = boneNodeAddressArray[ik.targetIndex];
    positions.emplace_back(
        XMLoadFloat3(&endNode->startPos)
    );

    //中間及びルートボーン
    for (auto& chainBoneIndex : ik.nodeIndices) {
        auto boneNode = boneNodeAddressArray[chainBoneIndex];

        positions.emplace_back(
            XMLoadFloat3(&boneNode->startPos)
        );
    }

    //分かりづらいので逆にする
    reverse(positions.begin(), positions.end());

    //元の長さを測っておく
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

    //ルートボーン座標交換
    positions[0] = XMVector3Transform(
        positions[0],
        boneMatrices[ik.nodeIndices[1]]
    );

    //中間は自動計算されるので計算しない

    //先端ボーン
    positions[2] = XMVector3Transform(
        positions[2],
        boneMatrices[ik.boneIndex]
    );

    //ルートから先端へのベクトルを作っておく
    auto linearVec = XMVectorSubtract(
        positions[2],
        positions[0]
    );

    float A = XMVector3Length(linearVec).m128_f32[0];
    float B = edgeLengths[0];
    float C = edgeLengths[1];

    linearVec = XMVector3Normalize(linearVec);

    //ルートから真ん中への角度計算
    float theta1 = acosf((A * A + B * B - C * C) / (2 * A * B));
    //真ん中からターゲットへの角度計算
    float theta2 = acosf((B * B + C * C - A * A) / (2 * B * C));

    //軸を求める
    //もし真ん中がひざであった場合には強制的にx軸とする
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

    //IKチェーンはルートに向かってから数えられるため1がルートに近い
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
    //向かせたい方向(z軸)
    XMVECTOR vz = lookat;

    //(向かせたい方向を向かせたときの)仮のy軸ベクトル
    XMVECTOR vy = XMVector3Normalize(
        XMLoadFloat3(&up)
    );

    //(向かせたい方向を向かせたときの)y軸
    /*XMVECTOR vx = XMVector3Normalize(
        XMVector3Cross(vz, vx)
    );*/
    XMVECTOR vx = XMVector3Normalize(
        XMVector3Cross(vy, vz)
    );
    vy = XMVector3Normalize(
        XMVector3Cross(vz, vx)
    );

    //LookAtとupが同じ方向を向いていたらrightを基準に作り直す
    if (std::abs(XMVector3Dot(vy, vz).m128_f32[0]) == 1.0f) {
        //仮のx方向を定義
        vx = XMVector3Normalize(
            XMLoadFloat3(&right)
        );

        //向かせたい方向を向かせたときのY軸を計算
        vy = XMVector3Normalize(
            XMVector3Cross(vz, vx)
        );

        //真のx軸を計算
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

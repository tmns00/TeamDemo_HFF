#include "Cube.h"

#include"Camera.h"

#include<d3dcompiler.h>
#include<DirectXTex.h>

using namespace DirectX;

Cube::~Cube(){
}

void Cube::Initialize(){
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

void Cube::Update(){
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

    // 定数バッファへデータ転送
    ConstBuffData* constMap = nullptr;
    res = constBuff->Map(0, nullptr, (void**)&constMap);
    constMap->color = color;
    constMap->matrix = matWorld *
        camera->GetViewMatrix() *
        camera->GetProjectionMatrix();	// 行列の合成
    constBuff->Unmap(0, nullptr);
}

void Cube::Draw(
    ID3D12GraphicsCommandList* cmdList
){
    // パイプラインステートの設定
    cmdList->SetPipelineState(pipelineState.Get());
    // ルートシグネチャの設定
    cmdList->SetGraphicsRootSignature(rootSignature.Get());
    // プリミティブ形状を設定
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // 頂点バッファの設定
    cmdList->IASetVertexBuffers(0, 1, &vbView);
    // インデックスバッファの設定
    cmdList->IASetIndexBuffer(&ibView);

    // デスクリプタヒープの配列
    ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
    cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // 定数バッファビューをセット
    cmdList->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());
    // シェーダリソースビューをセット
    cmdList->SetGraphicsRootDescriptorTable(1, gpuDescHandleSRV);
    // 描画コマンド
    cmdList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
}

void Cube::Terminate(){
    delete this;
}

Cube* Cube::Create(
    ID3D12Device* dev
){
    Cube* instance = new Cube;
    if (!instance) {
        assert(0);
        return nullptr;
    }
    instance->device = dev;

    instance->Initialize();

    return instance;
}

HRESULT Cube::CreateVertBuff(){
    HRESULT res = S_FALSE;

    //頂点バッファ
    res = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOADヒープとして
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)), //サイズに応じて適切な設定をしてくれる
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertBuff)
    );
    if (FAILED(res)) {
        assert(0);
    }
    res = S_FALSE;
    //バッファに頂点データコピー
    VertexData* vertMap = nullptr;
    res = vertBuff->Map(0, nullptr, (void**)&vertMap);
    if (FAILED(res)) {
        assert(0);
    }
    memcpy(vertMap, vertices, sizeof(vertices));
    vertBuff->Unmap(0, nullptr);
    //頂点バッファビュー
    vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); //バッファーの仮想アドレス
    vbView.SizeInBytes = sizeof(vertices);
    vbView.StrideInBytes = sizeof(vertices[0]); //1頂点あたりのバイト数

    return res;
}

HRESULT Cube::CreateIndexBuff(){
    HRESULT res = S_FALSE;

    //インデックスバッファ
    res = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //UPLOADヒープとして
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)), //サイズに応じて適切な設定をしてくれる,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&idxBuff)
    );
    if (FAILED(res)) {
        assert(0);
    }

    res = S_FALSE;
    //作ったバッファにインデックスデータコピー
    unsigned short* indexMap = nullptr;
    res = idxBuff->Map(0, nullptr, (void**)&indexMap);
    if (FAILED(res)) {
        assert(0);
    }

    memcpy(indexMap, indices, sizeof(indices));
    idxBuff->Unmap(0, nullptr);
    //インデックスバッファビューを作成
    ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
    ibView.Format = DXGI_FORMAT_R16_UINT;
    ibView.SizeInBytes = sizeof(indices);

    return res;
}

HRESULT Cube::ShaderCompile(){
    HRESULT res = S_FALSE;

    //頂点シェーダーの読み込みとコンパイル
    res = D3DCompileFromFile(
        L"BasicVertexShader.hlsl",	// シェーダファイル名
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
        "VSmain", "vs_5_0",	// エントリーポイント名、シェーダーモデル指定
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
        0,
        &vsBlob, &errorBlob);
    if (FAILED(res)) {
        DebugShader(res);
    }

    //ピクセルシェーダーの読み込みとコンパイル
    res = D3DCompileFromFile(
        L"BasicPixelShader.hlsl",	// シェーダファイル名
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
        "PSmain", "ps_5_0",	// エントリーポイント名、シェーダーモデル指定
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
        0,
        &psBlob, &errorBlob
    );
    if (FAILED(res)) {
        DebugShader(res);
    }

    return res;
}

HRESULT Cube::CreateRootSignature(){
    HRESULT res = S_FALSE;

    //デスクリプタレンジ
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
    descRangeSRV.Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        1,
        0
    );

    //ルートパラメーター
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

    //スタティックサンプラー
    CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

    //ルートシグネチャの設定
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_0(
        _countof(rootParams), //ルートパラメーター数
        rootParams,           //ルートパラメーターの先頭アドレス
        1,                    //サンプラー数
        &samplerDesc,         //サンプラーの先頭アドレス
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    //バイナリコードの作成
    res = D3DX12SerializeVersionedRootSignature(
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

HRESULT Cube::CreateGraphicsPipeline(){
    HRESULT res = S_FALSE;

    // 頂点レイアウト
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { // xy座標(1行で書いたほうが見やすい)
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        { // 法線ベクトル(1行で書いたほうが見やすい)
            "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        { // uv座標(1行で書いたほうが見やすい)
            "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
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

HRESULT Cube::CreateDescriptorHeap(){
    HRESULT res = S_FALSE;

    // デスクリプタヒープを生成	
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
    descHeapDesc.NumDescriptors = 1; // シェーダーリソースビュー1つ
    res = device->CreateDescriptorHeap(
        &descHeapDesc,
        IID_PPV_ARGS(&descHeap)
    );//生成
    if (FAILED(res)) {
        assert(0);
        return res;
    }

    // デスクリプタサイズを取得
    descHandleIncrementSize = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );

    return res;
}

HRESULT Cube::LoadTexture(
    const std::string& textureName
){
    HRESULT res = S_FALSE;

    // WICテクスチャのロード
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

    const Image* img = scratchImg.GetImage(0, 0, 0); // 生データ抽出

    // リソース設定
    CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        metadata.format,
        metadata.width,
        (UINT)metadata.height,
        (UINT16)metadata.arraySize,
        (UINT16)metadata.mipLevels
    );

    // テクスチャ用バッファの生成
    res = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
        D3D12_HEAP_FLAG_NONE,
        &texresDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
        nullptr,
        IID_PPV_ARGS(&texbuff));
    if (FAILED(res)) {
        return res;
    }

    // テクスチャバッファにデータ転送
    res = texbuff->WriteToSubresource(
        0,
        nullptr, // 全領域へコピー
        img->pixels,    // 元データアドレス
        (UINT)img->rowPitch,  // 1ラインサイズ
        (UINT)img->slicePitch // 1枚サイズ
    );
    if (FAILED(res)) {
        return res;
    }

    // シェーダリソースビュー作成
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

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
    D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

    srvDesc.Format = resDesc.Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    srvDesc.Texture2D.MipLevels = 1;

    device->CreateShaderResourceView(texbuff.Get(), //ビューと関連付けるバッファ
        &srvDesc, //テクスチャ設定情報
        cpuDescHandleSRV
    );

    return res;
}

void Cube::CreateModel(){
    //上面
    //頂点
    {
        vertices[0] = { {-0.5f,+0.5f,+0.5f},{0.0f,0.5f,0.0f},{0,0} };
        vertices[1] = { {-0.5f,+0.5f,-0.5f},{0.0f,0.5f,0.0f},{0,1} };
        vertices[2] = { {+0.5f,+0.5f,-0.5f},{0.0f,0.5f,0.0f},{1,1} };
        vertices[3] = { {+0.5f,+0.5f,+0.5f},{0.0f,0.5f,0.0f},{1,0} };
    }
    //インデックス
    {
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;

        indices[3] = 3;
        indices[4] = 0;
        indices[5] = 2;
    }

    //前面
    //頂点
    {
        vertices[4] = { {-0.5f,+0.5f,-0.5f},{0.0f,0.0f,-0.5f},{0,1} };
        vertices[5] = { {-0.5f,-0.5f,-0.5f},{0.0f,0.0f,-0.5f},{0,0} };
        vertices[6] = { {+0.5f,-0.5f,-0.5f},{0.0f,0.0f,-0.5f},{1,0} };
        vertices[7] = { {+0.5f,+0.5f,-0.5f},{0.0f,0.0f,-0.5f},{1,1} };
    }
    //インデックス
    {
        indices[6] = 4;
        indices[7] = 5;
        indices[8] = 6;

        indices[9] = 7;
        indices[10] = 4;
        indices[11] = 6;
    }

    //右面
    //頂点
    {
        vertices[8] = { {+0.5f,+0.5f,-0.5f},{0.5f,0.0f,0.0f},{0,0} };
        vertices[9] = { {+0.5f,-0.5f,-0.5f},{0.5f,0.0f,0.0f},{0,1} };
        vertices[10] = { {+0.5f,-0.5f,+0.5f},{0.5f,0.0f,0.0f},{1,1} };
        vertices[11] = { {+0.5f,+0.5f,+0.5f},{0.5f,0.0f,0.0f},{1,0} };
    }
    //インデックス
    {
        indices[12] = 8;
        indices[13] = 9;
        indices[14] = 10;

        indices[15] = 10;
        indices[16] = 8;
        indices[17] = 11;
    }

    //奥面
    //頂点
    {
        vertices[12] = { {+0.5f,+0.5f,+0.5f},{0.0f,0.0f,0.5f},{0,0} };
        vertices[13] = { {+0.5f,-0.5f,+0.5f},{0.0f,0.0f,0.5f},{0,1} };
        vertices[14] = { {-0.5f,-0.5f,+0.5f},{0.0f,0.0f,0.5f},{1,1} };
        vertices[15] = { {-0.5f,+0.5f,+0.5f},{0.0f,0.0f,0.5f},{1,0} };
    }
    //インデックス
    {
        indices[18] = 12;
        indices[19] = 13;
        indices[20] = 14;

        indices[21] = 15;
        indices[22] = 12;
        indices[23] = 14;
    }

    //左面
    //頂点
    {
        vertices[16] = { {-0.5f,+0.5f,+0.5f},{-0.5f,0.0f,0.0f},{0,0} };
        vertices[17] = { {-0.5f,-0.5f,+0.5f},{-0.5f,0.0f,0.0f},{0,1} };
        vertices[18] = { {-0.5f,-0.5f,-0.5f},{-0.5f,0.0f,0.0f},{1,1} };
        vertices[19] = { {-0.5f,+0.5f,-0.5f},{-0.5f,0.0f,0.0f},{1,0} };
    }
    //インデックス
    {
        indices[24] = 16;
        indices[25] = 17;
        indices[26] = 18;

        indices[27] = 19;
        indices[28] = 16;
        indices[29] = 18;
    }

    //下面
    //頂点
    {
        vertices[20] = { {-0.5f,-0.5f,-0.5f},{0.0f,-0.5f,0.0f},{0,0} };
        vertices[21] = { {-0.5f,-0.5f,+0.5f},{0.0f,-0.5f,0.0f},{0,1} };
        vertices[22] = { {+0.5f,-0.5f,+0.5f},{0.0f,-0.5f,0.0f},{1,1} };
        vertices[23] = { {+0.5f,-0.5f,-0.5f},{0.0f,-0.5f,0.0f},{1,0} };
    }
    //インデックス
    {
        indices[30] = 20;
        indices[31] = 21;
        indices[32] = 22;

        indices[33] = 23;
        indices[34] = 20;
        indices[35] = 22;
    }
}

void Cube::InitMatrix(){
    matWorld = XMMatrixIdentity();
}

HRESULT Cube::CreateConstBuff(){
    HRESULT res = S_FALSE;
    // 定数バッファの生成
    res = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBuffData) + 0xff) & ~0xff),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constBuff));

    return res;;
}

HRESULT Cube::CreateConstView(){
    return S_OK;
}

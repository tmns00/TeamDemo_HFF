#include"DeltaCone.h"
#include"WindowsApp.h"
#include"Camera.h"

#include<d3dcompiler.h>
#include<vector>
#include<DirectXTex.h>

#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

DeltaCone::~DeltaCone(){
}

void DeltaCone::Initialize(){
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

void DeltaCone::Update(){
	HRESULT result;
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
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = color;
	constMap->matrix = matWorld *
		camera->GetViewMatrix() *
		camera->GetProjectionMatrix();	// 行列の合成
	constBuff->Unmap(0, nullptr);
}

void DeltaCone::Draw(
	ID3D12GraphicsCommandList* cmdList
){
	// パイプラインステートの設定
	cmdList->SetPipelineState(pipelineState.Get());
	// ルートシグネチャの設定
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	// プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

void DeltaCone::Terminate(){
	delete this;
}

DeltaCone* DeltaCone::Create(
	ID3D12Device* dev
){
	DeltaCone* instance = new DeltaCone;
	if (!instance) {
		assert(0);
		return nullptr;
	}
	instance->device = dev;

	instance->Initialize();

	return instance;
}

HRESULT DeltaCone::CreateVertBuff()
{
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

HRESULT DeltaCone::CreateIndexBuff()
{
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
	//copy(indices.begin(), indices.end(), mappedIdx);
	memcpy(indexMap, indices, sizeof(indices));
	idxBuff->Unmap(0, nullptr);
	//インデックスバッファビューを作成
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	return res;
}

HRESULT DeltaCone::ShaderCompile()
{
	HRESULT res = S_FALSE;

	//頂点シェーダーの読み込みとコンパイル
	res= D3DCompileFromFile(
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

HRESULT DeltaCone::CreateRootSignature(){
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

HRESULT DeltaCone::CreateGraphicsPipeline(){
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

HRESULT DeltaCone::CreateDescriptorHeap(){
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

HRESULT DeltaCone::LoadTexture(
	const std::string& textureName
){
	HRESULT res = S_FALSE;

	// WICテクスチャのロード
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

void DeltaCone::CreateModel(){
	//三角錐の実装	
	std::vector<VertexData> realVert;
	realVert.resize(5);
	for (int i = 0; i < 3; i++)
	{
		realVert[i].pos.x = radius * sinf(XM_2PI / 3 * i);
		realVert[i].pos.y = radius * cosf(XM_2PI / 3 * i);
		realVert[i].pos.z = 0;
	}
	realVert[3].pos = { 0,0,0 };
	realVert[4].pos = { 0,0,-topHeight };

	//重複ありの座標を用意
	vertices[0] = realVert[1];
	vertices[1] = realVert[0];
	vertices[2] = realVert[3];

	vertices[3] = realVert[2];
	vertices[4] = realVert[1];
	vertices[5] = realVert[3];

	vertices[6] = realVert[0];
	vertices[7] = realVert[2];
	vertices[8] = realVert[3];

	vertices[9] = realVert[0];
	vertices[10] = realVert[1];
	vertices[11] = realVert[4];

	vertices[12] = realVert[1];
	vertices[13] = realVert[2];
	vertices[14] = realVert[4];

	vertices[15] = realVert[2];
	vertices[16] = realVert[0];
	vertices[17] = realVert[4];

	for (int i = 0; i < _countof(indices); ++i) {
		indices[i] = i;
	}


	for (int i = 0; i < _countof(indices) / 3; ++i)
	{
		//
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		//
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//
		normal = XMVector3Normalize(normal);
		//
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);
	}

	for (int i = 0; i < _countof(vertices); i++)
	{
		vertices[i].uv = { 0,0 };
	}
}

void DeltaCone::InitMatrix(){
	//ワールド行列
	matWorld = XMMatrixIdentity();
}

HRESULT DeltaCone::CreateConstBuff()
{
	HRESULT res = S_FALSE;
	// 定数バッファの生成
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBuffData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));

	return res;
}

HRESULT DeltaCone::CreateConstView(){
	return S_OK;
}

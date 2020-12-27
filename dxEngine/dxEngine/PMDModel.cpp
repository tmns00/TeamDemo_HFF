#include<d3dcompiler.h>
#include<string>
#include"PMDModel.h"

#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

//頂点1つあたりのサイズ
const size_t pmdvertex_size = 38;
//pmdファイルパス
const std::string strModelPath = "Resources/Model/初音ミクmetal.pmd";

void PMDModel::Initialize(
	ID3D12Device* device
) {
	//nullptrチェック
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

	// スケール、回転、平行移動行列の計算
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	// ワールド行列の合成
	worldMat = XMMatrixIdentity(); // 変形をリセット

	worldMat *= matScale; // ワールド行列にスケーリングを反映
	worldMat *= matRot; // ワールド行列に回転を反映
	worldMat *= matTrans; // ワールド行列に平行移動を反映

	//行列を定数バッファにコピー
	SceneMatrix* mapMatrix; //マップ先のポインター
	res = constBuff->Map(
		0,
		nullptr,
		(void**)&mapMatrix
	);
	if (FAILED(res)) {
		assert(0);
	}
	//行列のコピー
	mapMatrix->world = worldMat;
	mapMatrix->view = viewMat;
	mapMatrix->proj = projectionMat;
	mapMatrix->view_proj = viewMat * projectionMat;
	mapMatrix->eye = eye;
	constBuff->Unmap(0, nullptr);
}

void PMDModel::Draw()
{
	// nullptrチェック
	assert(device);
	assert(PMDModel::cmdList);

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

void PMDModel::DrawBefore(ID3D12GraphicsCommandList* cmdList)
{
	// PreDrawとPostDrawがペアで呼ばれていなければエラー
	assert(PMDModel::cmdList == nullptr);

	//コマンドリストをセット
	PMDModel::cmdList = cmdList;

	// パイプラインステートの設定
	cmdList->SetPipelineState(pipelineState.Get());
	// ルートシグネチャの設定
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	// プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PMDModel::DrawAfter()
{
	// コマンドリストを解除
	PMDModel::cmdList = nullptr;
}

void PMDModel::DebugShader(const HRESULT& result)
{
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
		::OutputDebugStringA("ファイルが見当たりません");
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

HRESULT PMDModel::CreateIdxBuff()
{
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

HRESULT PMDModel::ShaderCompile()
{
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

HRESULT PMDModel::CreateRootSignature()
{
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

HRESULT PMDModel::CreateGraphicsPipeline()
{
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

void PMDModel::CreatePMDModel()
{
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

	fclose(fp);
}

void PMDModel::InitMatrix()
{
	//ワールド行列
	worldMat = XMMatrixIdentity();
	//ビュー行列
	viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));
	//プロジェクション行列
	projectionMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2, //画角90°
		1280 / 720, //アスペクト比
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

HRESULT PMDModel::CreateMaterialBuff()
{
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

ID3D12Resource* PMDModel::LoadTextureFromFile(
	std::string& texPath
) {
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

ID3D12Resource* PMDModel::CreateWhiteTexture()
{
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

ID3D12Resource* PMDModel::CreateBlackTexture()
{
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
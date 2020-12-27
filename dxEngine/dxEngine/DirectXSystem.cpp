#include<cassert>
#include<vector>
#include<string>

#include"DirectXSystem.h"

using namespace Microsoft::WRL;

DirectXSystem* DirectXSystem::instance = nullptr;

DirectXSystem::DirectXSystem() {
}

DirectXSystem::~DirectXSystem(){
}

void DirectXSystem::EnableDebugLayer(){
	ID3D12Debug* debugLayer = nullptr;
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	if (FAILED(result)) {
		assert(0);
	}
	debugLayer->EnableDebugLayer(); //�f�o�b�O���C���[��L��������
	debugLayer->Release(); //�L����������C���^�[�t�F�[�X���J������
}

void DirectXSystem::Create(){
	if (!instance) {
		instance = new DirectXSystem;
	}
}

DirectXSystem* DirectXSystem::GetInstance()
{
	return instance;
}

void DirectXSystem::Destroy()
{
	delete instance;
	instance = nullptr;
}

void DirectXSystem::Initialize(WindowsApp* winApp)
{
	assert(winApp);

	this->winApp = winApp;

	if (FAILED(InitDirectX())) {
		assert(0);
	}

	if (FAILED(InitCommand())) {
		assert(0);
	}

	if (FAILED(InitSwapchain())) {
		assert(0);
	}

	if (FAILED(CreateDepthBuffer())) {
		assert(0);
	}

	if (FAILED(CreateFence())) {
		assert(0);
	}
}

void DirectXSystem::DrawBefore()
{
	//�o�b�N�o�b�t�@�̔ԍ����擾
	auto bbIdx = swapchain->GetCurrentBackBufferIndex();

	cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffers[bbIdx].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET)
	);

	auto rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
		bbIdx,
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	auto dsvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);
	//�����_�[�^�[�Q�b�g���Z�b�g
	cmdList->OMSetRenderTargets(1, &rtvH, true, &dsvH);

	//��ʃN���A
	float clearColor[] = { 0.75f,0.75f,0.75f,1.0f };
	cmdList->ClearRenderTargetView(
		rtvH,
		clearColor,
		0,
		nullptr
	);
	//�[�x�o�b�t�@�N���A
	cmdList->ClearDepthStencilView(
		dsvH,
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0,
		nullptr
	);

	//�r���[�|�[�g�ƃV�U�[��`�̃Z�b�g
	cmdList->RSSetViewports(
		1,
		&CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			WindowsApp::window_width,
			WindowsApp::window_height
		)
	);
	cmdList->RSSetScissorRects(
		1,
		&CD3DX12_RECT(
			0,
			0,
			WindowsApp::window_width,
			WindowsApp::window_height
		)
	);
}

void DirectXSystem::DrawAfter()
{
	auto bbIdx = swapchain->GetCurrentBackBufferIndex();
	//�O�ゾ������ւ���
	cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffers[bbIdx].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT)
	);

	//���߂̃N���[�Y
	cmdList->Close();

	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { cmdList.Get() };
	cmdQueue->ExecuteCommandLists(1, cmdlists);

	cmdQueue->Signal(fence.Get(), ++fenceVal);
	while (fence->GetCompletedValue() != fenceVal)
	{
		//�C�x���g�n���h���̎擾
		auto event = CreateEvent(nullptr, false, false, nullptr);

		fence->SetEventOnCompletion(fenceVal, event);

		//�C�x���g����������܂ő҂�������
		WaitForSingleObject(event, INFINITE);

		//�C�x���g�n���h�������
		CloseHandle(event);
	}

	cmdAllocator->Reset(); //�L���[���N���A
	cmdList->Reset(
		cmdAllocator.Get(),
		nullptr
	); //�ĂуR�}���h�����߂鏀��

	//�t���b�v
	swapchain->Present(1, 0);
}

HRESULT DirectXSystem::InitDirectX()
{
	HRESULT res = S_FALSE;

#ifdef _DEBUG
	//�f�o�b�O���C���[�I��
	EnableDebugLayer();
#endif

#ifdef _DEBUG
	res = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}
#else
	res = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}
#endif

	//�A�_�v�^�[�̗񋓗p
	std::vector<IDXGIAdapter*> adapters;

	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0;
		dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(tmpAdapter);
	}

	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc); //�A�_�v�^�[�̐����I�u�W�F�N�g�擾

		std::wstring strDesc = adesc.Description;

		//Microsoft Basic Render Driver�����
		if (strDesc.find(L"Intel") == std::wstring::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	res = S_FALSE;
	//Direct3D�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featureLevel;

	for (int i = 0; i < _countof(levels); ++i)
	{
		res = D3D12CreateDevice(tmpAdapter, levels[i], IID_PPV_ARGS(&device));

		if (SUCCEEDED(res)) {
			featureLevel = levels[i];
			break; //�����\�ȃo�[�W���������������烋�[�v�ł��؂�
		}
	}
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	return res;
}

HRESULT DirectXSystem::InitCommand()
{
	HRESULT res = S_FALSE;

	res = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	res = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&cmdList));
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	//�^�C���A�E�g�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//�A�_�v�^�[��1�����g��Ȃ��Ƃ���0�ł悢
	cmdQueueDesc.NodeMask = 0;
	//�v���C�I���e�B�͓��ɂȂ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//�R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//�L���[����
	res = device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	return res;
}

HRESULT DirectXSystem::InitSwapchain()
{
	HRESULT res = S_FALSE;

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = WindowsApp::window_width;
	swapchainDesc.Height = WindowsApp::window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	//�o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	//�t���b�v��͑��₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//���Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//�E�B���h�E�̃t���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	ComPtr<IDXGISwapChain1> _swapchain1 = nullptr;
	res = dxgiFactory->CreateSwapChainForHwnd(
		cmdQueue.Get(),
		winApp->GetHWND(),
		&swapchainDesc,
		nullptr,
		nullptr,
		&_swapchain1
	);
	_swapchain1.As(&swapchain);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	res = S_FALSE;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //�����_�[�^�[�Q�b�g�r���[�Ȃ̂�RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; //�\����2��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; //���Ɏw��Ȃ�

	res = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	res = S_FALSE;

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	res = swapchain->GetDesc(&swcDesc);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	res = S_FALSE;

	//SRGB�����_�[�^�[�Q�b�g�r���[�ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //�K���}�␳����
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	backBuffers.resize(swcDesc.BufferCount);
	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		res = swapchain->GetBuffer(idx, IID_PPV_ARGS(&backBuffers[idx]));
		if (FAILED(res)) {
			assert(0);
			return S_FALSE;
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
			idx,
			device->GetDescriptorHandleIncrementSize(heapDesc.Type));
		device->CreateRenderTargetView(
			backBuffers[idx].Get(),
			&rtvDesc,
			handle
		);
	}

	return res;
}

HRESULT DirectXSystem::CreateDepthBuffer()
{
	HRESULT res = S_FALSE;

	//�[�x�o�b�t�@�̍쐬
	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		WindowsApp::window_width,
		WindowsApp::window_height,
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	//�N���A�o�����[
	CD3DX12_CLEAR_VALUE depthClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	res = S_FALSE;
	//�[�x�o�b�t�@����
	res = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) ,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //�[�x�l�������݂Ɏg�p
		&depthClearValue,
		IID_PPV_ARGS(&depthBuffer)
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	//�[�x�p�f�X�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	res = S_FALSE;
	//�[�x�f�X�N���v�^�q�[�v����
	res = device->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(&dsvHeap)
	);
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;                //�[�x�l��32�r�b�g���悤
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2D�e�N�X�`��
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;                   //�t���O�͓��ɂȂ�
	device->CreateDepthStencilView(
		depthBuffer.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);

	return res;
}

HRESULT DirectXSystem::CreateFence()
{
	HRESULT res = S_FALSE;

	res = device->CreateFence(
		fenceVal,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&fence));
	if (FAILED(res)) {
		assert(0);
		return S_FALSE;
	}

	return res;
}


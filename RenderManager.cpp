#include "Main.h"
#include "RenderManager.h"

#include "D3DX12.h"
#include "DDSTextureLoader12.h"





RenderManager* RenderManager::m_Instance = nullptr;




RenderManager::RenderManager()
{
	m_Instance = this;

	Init();
}




RenderManager::~RenderManager()
{
/*
#if defined(_DEBUG)
	//ReportLiveDeviceObjects
	{
		ComPtr<ID3D12DebugDevice> debugInterface;

		if (SUCCEEDED(m_Device->QueryInterface(IID_PPV_ARGS(&debugInterface))))
		{
			debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		}
	}
#endif
*/
}





void RenderManager::Init()
{

	m_WindowMode = true;

	m_BackBufferWidth = 1280;
	m_BackBufferHeight = 720;



	m_WindowHandle = GetWindow();
	m_Frame[0] = 1;
	m_Frame[1] = 0;
	m_RTIndex = 0;


	m_Viewport.TopLeftX = 0.f;
	m_Viewport.TopLeftY = 0.f;
	m_Viewport.Width = (FLOAT)m_BackBufferWidth;
	m_Viewport.Height = (FLOAT)m_BackBufferHeight;
	m_Viewport.MinDepth = 0.f;
	m_Viewport.MaxDepth = 1.f;

	m_ScissorRect.top = 0;
	m_ScissorRect.left = 0;
	m_ScissorRect.right = m_BackBufferWidth;
	m_ScissorRect.bottom = m_BackBufferHeight;

	HRESULT hr;



#if defined(_DEBUG)

	//デバッグレイヤー有効
	{
		ComPtr<ID3D12Debug1>	debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			//debugController->SetEnableGPUBasedValidation(true);
		}
	}
/*	
	//DRED
	{
		ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> d3dDredSettings1;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDredSettings1))))
		{
			d3dDredSettings1->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			d3dDredSettings1->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			d3dDredSettings1->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		}
	}
*/
#endif


	//デバイス生成
	{
		UINT flag{};
		hr = CreateDXGIFactory2(flag, IID_PPV_ARGS(&m_Factory));
		assert(SUCCEEDED(hr));

		hr = m_Factory->EnumAdapters(0, (IDXGIAdapter**)m_Adapter.GetAddressOf());
		assert(SUCCEEDED(hr));

		hr = D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&m_Device));
		assert(SUCCEEDED(hr));
	}




	//コマンドキュー生成
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = 0;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;

		hr = m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_CommandQueue));
		assert(SUCCEEDED(hr));

		m_FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		assert(m_FenceEvent);

		hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
		assert(SUCCEEDED(hr));
	}



	//コマンドアロケータ・リスト生成
	{
		hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_GraphicsCommandAllocator[0]));
		assert(SUCCEEDED(hr));
		m_GraphicsCommandAllocator[0]->SetName(L"GraphicsCommandAllocator[0]");

		hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_GraphicsCommandAllocator[1]));
		assert(SUCCEEDED(hr));
		m_GraphicsCommandAllocator[1]->SetName(L"GraphicsCommandAllocator[1]");

		hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_GraphicsCommandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_GraphicsCommandList));
		assert(SUCCEEDED(hr));
		m_GraphicsCommandList->SetName(L"GraphicsCommandList");
	}






	//スワップチェーン生成
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		ComPtr<IDXGISwapChain> swapChain{};

		swapChainDesc.BufferDesc.Width = m_BackBufferWidth;
		swapChainDesc.BufferDesc.Height = m_BackBufferHeight;
		swapChainDesc.OutputWindow = m_WindowHandle;
		swapChainDesc.Windowed = m_WindowMode;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = 0;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;


		hr = m_Factory->CreateSwapChain(m_CommandQueue.Get(), &swapChainDesc, &swapChain);
		assert(SUCCEEDED(hr));

		hr = swapChain.As(&m_SwapChain);
		assert(SUCCEEDED(hr));

		m_RTIndex = m_SwapChain->GetCurrentBackBufferIndex();
	}





	//レンダーターゲット生成
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.NumDescriptors = 2;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;

		hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RenderTargetDescriptorHeap));
		assert(SUCCEEDED(hr));

		UINT size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		for (UINT i = 0; i < 2; ++i)
		{
			hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]));
			assert(SUCCEEDED(hr));

			m_RenderTarget[i]->SetName(L"RenderTarget");

			m_RenderTargetHandle[i] = m_RenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			m_RenderTargetHandle[i].ptr += size * i;
			m_Device->CreateRenderTargetView(m_RenderTarget[i].Get(), nullptr, m_RenderTargetHandle[i]);
		}
	}








	//デプスバッファ用デスクリプタヒープ
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.NumDescriptors = 1;///////////////////////////////////
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;

		hr = m_Device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_DepthBufferDescriptorHeap));
		assert(SUCCEEDED(hr));
	}




	//デプスバッファ生成
	{
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Width = m_BackBufferWidth;
		resourceDesc.Height = m_BackBufferHeight;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		hr = m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
												D3D12_HEAP_FLAG_NONE,
												&resourceDesc,
												D3D12_RESOURCE_STATE_DEPTH_WRITE,
												&clearValue,
												IID_PPV_ARGS(&m_DepthBuffer));
		assert(SUCCEEDED(hr));

		m_DepthBuffer->SetName(L"DepthBuffer");



		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.Texture2D.MipSlice = 0;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_DepthBufferHandle = m_DepthBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsvDesc, m_DepthBufferHandle);

	}







	//汎用デスクリプタヒープ
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NumDescriptors = SRV_DESCRIPTOR_MAX;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NodeMask = 0;

		m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_SRVDescriptorHeap));

		for (int i = 0; i < SRV_DESCRIPTOR_MAX; i++)
			m_SRVDescriptorPool.push_back(i);
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = RTV_DESCRIPTOR_MAX;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NodeMask = 0;

		m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVDescriptorHeap));

		for (int i = 0; i < RTV_DESCRIPTOR_MAX; i++)
			m_RTVDescriptorPool.push_back(i);
	}

    //ImGUI
	{
		//Setup Dear ImGUI context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		//Setup Dear ImGUI style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		//Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(m_WindowHandle);
		ImGui_ImplDX12_Init(m_Device.Get(), 2,
			DXGI_FORMAT_R8G8B8A8_UNORM, m_SRVDescriptorHeap.Get(),
			m_SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			m_SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		m_SRVDescriptorPool.pop_front();
	}



	//汎用定数バッファ
	for (int i = 0; i < 2; i++)
	{
		{
			D3D12_HEAP_PROPERTIES properties{};
			properties.Type = D3D12_HEAP_TYPE_UPLOAD;
			properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			properties.CreationNodeMask = 0;
			properties.VisibleNodeMask = 0;

			D3D12_RESOURCE_DESC desc{};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			desc.Height = 1;
			desc.DepthOrArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Width = CONSTANT_BUFFER_SIZE * CONSTANT_BUFFER_MAX;


			HRESULT hr = m_Device->CreateCommittedResource(&properties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_ConstantBuffer[i]));
			assert(SUCCEEDED(hr));
		}


		hr = m_ConstantBuffer[i]->Map(0, nullptr, (void**)&m_ConstantBufferPointer[i]);
		assert(SUCCEEDED(hr));


		for (int j = 0; j < CONSTANT_BUFFER_MAX; j++)
		{
			unsigned int index = m_SRVDescriptorPool.front();
			m_SRVDescriptorPool.pop_front();




			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = m_ConstantBuffer[i]->GetGPUVirtualAddress() + j * CONSTANT_BUFFER_SIZE;
			desc.SizeInBytes = CONSTANT_BUFFER_SIZE;



			D3D12_CPU_DESCRIPTOR_HANDLE handle = m_SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			unsigned int size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			handle.ptr += size * index;

			m_Device->CreateConstantBufferView(&desc, handle);

			m_ConstantBufferView[i][j] = index;
		}

		m_ConstantBufferIndex[i] = 0;
	}






	//ルートシグネチャ生成
	{

		D3D12_ROOT_PARAMETER		rootParameters[12]{};
		D3D12_DESCRIPTOR_RANGE		range[12]{};


		//定数バッファ
		for (unsigned int i = 0; i < 4; i++)
		{
			range[i].NumDescriptors = 1;
			range[i].BaseShaderRegister = i;
			range[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			range[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			rootParameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameters[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParameters[i].DescriptorTable.NumDescriptorRanges = 1;
			rootParameters[i].DescriptorTable.pDescriptorRanges = &range[i];
		}


		//テクスチャ
		for (unsigned int i = 4; i < 12; i++)
		{
			range[i].NumDescriptors = 1;
			range[i].BaseShaderRegister = i - 4;
			range[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			rootParameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameters[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			rootParameters[i].DescriptorTable.NumDescriptorRanges = 1;
			rootParameters[i].DescriptorTable.pDescriptorRanges = &range[i];
		}


		//サンプラー
		D3D12_STATIC_SAMPLER_DESC	samplerDesc[2]{};
		//samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc[0].Filter = D3D12_FILTER_ANISOTROPIC;
		samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[0].MipLODBias = 0.0f;
		samplerDesc[0].MaxAnisotropy = 4;
		samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc[0].MinLOD = 0.0f;
		samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc[0].ShaderRegister = 0;
		samplerDesc[0].RegisterSpace = 0;
		samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		samplerDesc[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[1].MipLODBias = 0.0f;
		samplerDesc[1].MaxAnisotropy = 16;
		samplerDesc[1].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc[1].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc[1].MinLOD = 0.0f;
		samplerDesc[1].MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc[1].ShaderRegister = 1;
		samplerDesc[1].RegisterSpace = 0;
		samplerDesc[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;



		D3D12_ROOT_SIGNATURE_DESC	rootSignatureDesc{};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSignatureDesc.NumParameters = _countof(rootParameters);
		rootSignatureDesc.pParameters = rootParameters;
		rootSignatureDesc.NumStaticSamplers = 2;
		rootSignatureDesc.pStaticSamplers = samplerDesc;


		HRESULT hr;
		ComPtr<ID3DBlob> blob{};
		hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);
		assert(SUCCEEDED(hr));

		hr = m_Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
		assert(SUCCEEDED(hr));
	}





	//スプライトポリゴン
	{
		m_VertexBuffer = CreateVertexBuffer(sizeof(VERTEX_3D), 4);

		//頂点データの書き込み
		VERTEX_3D* buffer{};
		hr = m_VertexBuffer->Resource->Map(0, nullptr, (void**)&buffer);
		assert(SUCCEEDED(hr));

		buffer[0].Position = { -1.0f,  1.0f,  0.0f };
		buffer[1].Position = { 1.0f,  1.0f, 0.0f };
		buffer[2].Position = { -1.0f,  -1.0f, 0.0f };
		buffer[3].Position = { 1.0f,  -1.0f, 0.0f };

		buffer[0].Color = { 1.0f,  1.0f,  1.0f, 1.0f };
		buffer[1].Color = { 1.0f,  1.0f,  1.0f, 1.0f };
		buffer[2].Color = { 1.0f,  1.0f,  1.0f, 1.0f };
		buffer[3].Color = { 1.0f,  1.0f,  1.0f, 1.0f };

		buffer[0].Normal = { 0.0f, 1.0f, 0.0f };
		buffer[1].Normal = { 0.0f, 1.0f, 0.0f };
		buffer[2].Normal = { 0.0f, 1.0f, 0.0f };
		buffer[3].Normal = { 0.0f, 1.0f, 0.0f };

		buffer[0].TexCoord = { 0.0f, 0.0f };
		buffer[1].TexCoord = { 1.0f, 0.0f };
		buffer[2].TexCoord = { 0.0f, 1.0f };
		buffer[3].TexCoord = { 1.0f, 1.0f };

		m_VertexBuffer->Resource->Unmap(0, nullptr);
	}

	//パイプライン生成
	{
		DXGI_FORMAT RTVFormats[] =
		{
			DXGI_FORMAT_R8G8B8A8_UNORM
		};

		m_PipelineState["Unlit"] = CreatePipeline(
			"Shader\\UnlitVS.cso",
			"Shader\\UnlitPS.cso",
			RTVFormats, _countof(RTVFormats));
	}

	//ジオメトリパイプライン生成
	{
		DXGI_FORMAT RTVFormats[] =
		{
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
		};

		m_PipelineState["Geometry"] = CreatePipeline(
			"Shader\\GeometryVS.cso",
			"Shader\\GeometryPS.cso",
			RTVFormats, _countof(RTVFormats));
	}

	//デファードパイプライン生成
	{
		DXGI_FORMAT RTVFormats[] =
		{
			DXGI_FORMAT_R16G16B16A16_FLOAT,
		};

		m_PipelineState["Deferred"] = CreatePipeline(
			"Shader\\DeferredVS.cso",
			"Shader\\DeferredPS.cso",
			RTVFormats, _countof(RTVFormats));
	}

	//ポストエフェクトパイプライン生成
	{
		DXGI_FORMAT RTVFormats[] =
		{
			DXGI_FORMAT_R8G8B8A8_UNORM,
		};

		m_PipelineState["PostEffect"] = CreatePipeline(
			"Shader\\PostEffectVS.cso",
			"Shader\\PostEffectPS.cso",
			RTVFormats, _countof(RTVFormats));
	}

	//ブルームパイプライン生成
	{
		DXGI_FORMAT RTVFormats[] =
		{
			DXGI_FORMAT_R16G16B16A16_FLOAT,
		};

		m_PipelineState["Bloom"] = CreatePipeline(
			"Shader\\BloomVS.cso",
			"Shader\\BloomPS.cso",
			RTVFormats, _countof(RTVFormats));
	}

	//バッファ作成
	{
		m_ColorBuffer = CreateRenderTarget(
			m_BackBufferWidth,
			m_BackBufferHeight,
			DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_ColorBuffer->Resource->SetName(L"ColorBuffer");

		m_NormalBuffer = CreateRenderTarget(
			m_BackBufferWidth,
			m_BackBufferHeight,
			DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_NormalBuffer->Resource->SetName(L"NormalBuffer");

		m_PositionBuffer = CreateRenderTarget(
			m_BackBufferWidth,
			m_BackBufferHeight,
			DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_PositionBuffer->Resource->SetName(L"PositionBuffer");

		m_ArmBuffer = CreateRenderTarget(
			m_BackBufferWidth,
			m_BackBufferHeight,
			DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_ArmBuffer->Resource->SetName(L"ARMBuffer");

		m_EmissionBuffer = CreateRenderTarget(
			m_BackBufferWidth,
			m_BackBufferHeight,
			DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_EmissionBuffer->Resource->SetName(L"EmissionBuffer");

		m_PostEffectBuffer = CreateRenderTarget(
			m_BackBufferWidth,
			m_BackBufferHeight,
			DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_PostEffectBuffer->Resource->SetName(L"PostEffectBuffer");

		int width = m_BackBufferWidth;
		int height = m_BackBufferHeight;

		for (int i = 0; i < 5; i++)
		{
			width /= 2;
			height /= 2;

			m_BloomBuffer[i] = CreateRenderTarget(
				width, height,
				DXGI_FORMAT_R16G16B16A16_FLOAT);

			m_BloomBuffer[i]->Resource->SetName(L"BloomBuffer");
		}
	}


	//テクスチャ読み込み
	m_EnvironmentTexture = LoadTexture("Asset\\alps_field_2k_EXR_BC6H_1.DDS");

}


void RenderManager::WaitGPU()
{

	//実行したコマンドの終了待ち
	m_CommandQueue->Signal(m_Fence.Get(), m_Frame[m_RTIndex]);

	m_Fence->SetEventOnCompletion(m_Frame[m_RTIndex], m_FenceEvent);
	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	m_Frame[m_RTIndex]++;

}









void RenderManager::DrawBegin()
{

	//汎用デスクリプタヒープ
	ID3D12DescriptorHeap* dh[] = { m_SRVDescriptorHeap.Get() };
	m_GraphicsCommandList->SetDescriptorHeaps(_countof(dh), dh);


	//ルートシグネチャの設定
	m_GraphicsCommandList->SetGraphicsRootSignature(m_RootSignature.Get());



	//コンスタントバッファ初期化
	m_ConstantBufferIndex[m_RTIndex] = 0;



	//ビューポートとシザー矩形の設定
	m_GraphicsCommandList->RSSetViewports(1, &m_Viewport);
	m_GraphicsCommandList->RSSetScissorRects(1, &m_ScissorRect);



   //リソースバリア
    m_GraphicsCommandList->ResourceBarrier(1, 
		&CD3DX12_RESOURCE_BARRIER::Transition(   											
			m_ColorBuffer->Resource.Get(),
    		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET));
    
	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_NormalBuffer->Resource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_PositionBuffer->Resource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_ArmBuffer->Resource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_EmissionBuffer->Resource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

    //レンダーターゲットの設定
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[] =
	{
		m_ColorBuffer->RTVHandle,
		m_NormalBuffer->RTVHandle,
		m_PositionBuffer->RTVHandle,
		m_ArmBuffer->RTVHandle,
		m_EmissionBuffer->RTVHandle,
	};
	m_GraphicsCommandList->OMSetRenderTargets(_countof(renderTargets),
		renderTargets,false,&m_DepthBufferHandle);
    
    //デプスバッファ・レンダーターゲットのクリア
    FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_GraphicsCommandList->ClearRenderTargetView(m_ColorBuffer->RTVHandle,    clearColor, 0, nullptr);
	m_GraphicsCommandList->ClearRenderTargetView(m_NormalBuffer->RTVHandle,   clearColor, 0, nullptr);
	m_GraphicsCommandList->ClearRenderTargetView(m_PositionBuffer->RTVHandle, clearColor, 0, nullptr);
	m_GraphicsCommandList->ClearRenderTargetView(m_ArmBuffer->RTVHandle, clearColor, 0, nullptr);
	m_GraphicsCommandList->ClearRenderTargetView(m_EmissionBuffer->RTVHandle, clearColor, 0, nullptr);
    m_GraphicsCommandList->ClearDepthStencilView(m_DepthBufferHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//パイプライン設定
	SetPipelineState("Geometry");

	//IMGUI
	{
		RECT rect;
		GetClientRect(GetWindow(), &rect);

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame((float)(rect.right - rect.left) / m_BackBufferWidth, (float)(rect.bottom - rect.top) / m_BackBufferHeight);

		ImGui::NewFrame();
	}


}





void RenderManager::DrawEnd()
{
	HRESULT hr;


	//リソースバリア
	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_ColorBuffer->Resource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_NormalBuffer->Resource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_PositionBuffer->Resource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_ArmBuffer->Resource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_EmissionBuffer->Resource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));


	//ライティングパス
	{
		SetPipelineState("Deferred");

		//リソースバリア
		m_GraphicsCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_PostEffectBuffer->Resource.Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

		//レンダーターゲットの設定
		m_GraphicsCommandList->OMSetRenderTargets(1,
			&m_PostEffectBuffer->RTVHandle, TRUE, &m_DepthBufferHandle);

		//デプスバッファ・レンダーターゲットのクリア
		FLOAT clearColor[4] = { 0.8f, 0.5f, 0.9f, 1.0f };
		m_GraphicsCommandList->ClearRenderTargetView(m_PostEffectBuffer->RTVHandle, clearColor, 0, nullptr);
		m_GraphicsCommandList->ClearDepthStencilView(m_DepthBufferHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		SetTexture(RenderManager::TEXTURE_TYPE::BASE_COLOR,
			m_ColorBuffer.get());

		SetTexture(RenderManager::TEXTURE_TYPE::NORMAL,
			m_NormalBuffer.get());

		SetTexture(RenderManager::TEXTURE_TYPE::POSITION,
			m_PositionBuffer.get());

		SetTexture(RenderManager::TEXTURE_TYPE::ARM,
			m_ArmBuffer.get());

		SetTexture(RenderManager::TEXTURE_TYPE::EMISSION,
			m_EmissionBuffer.get());

		SetTexture(RenderManager::TEXTURE_TYPE::ENVIRONMENT,
			m_EnvironmentTexture.get());

		DrawScreen();

		m_GraphicsCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_PostEffectBuffer->Resource.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		m_GraphicsCommandList->OMSetRenderTargets(1,
			&m_PostEffectBuffer->RTVHandle, TRUE, &m_DepthBufferHandle);
	}

	//ブルームパス
	{
		SetPipelineState("Bloom");

		D3D12_VIEWPORT viewport = m_Viewport;

		SetTexture(RenderManager::TEXTURE_TYPE::BASE_COLOR,
			m_PostEffectBuffer.get());

		for (int i = 0; i < 5; i++)
		{
			//ビューポートの設定
			viewport.Width /= 2.0f;
			viewport.Height /= 2.0f;

			m_GraphicsCommandList->RSSetViewports(1, &viewport);

			//リソースバリア
			m_GraphicsCommandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					m_BloomBuffer[i]->Resource.Get(),
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_RENDER_TARGET));

			//レンダーターゲットの設定
			m_GraphicsCommandList->OMSetRenderTargets(1,
				&m_BloomBuffer[i]->RTVHandle, TRUE, &m_DepthBufferHandle);

			//デプスバッファ・レンダーターゲットのクリア
			FLOAT clearColor[4] = { 0.8f, 0.5f, 0.9f, 1.0f };
			m_GraphicsCommandList->ClearRenderTargetView(m_BloomBuffer[i]->RTVHandle, clearColor, 0, nullptr);
			m_GraphicsCommandList->ClearDepthStencilView(m_DepthBufferHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			DrawScreen();

			//リソースバリア
			m_GraphicsCommandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					m_BloomBuffer[i]->Resource.Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

			SetTexture(RenderManager::TEXTURE_TYPE::BASE_COLOR,
				m_BloomBuffer[i].get());
		}
	}

	//ビューポートの設定
	m_GraphicsCommandList->RSSetViewports(1, &m_Viewport);

	//ポストエフェクトパス
	{
		SetPipelineState("PostEffect");

		//リソースバリア
		m_GraphicsCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_RenderTarget[m_RTIndex].Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

		//レンダーターゲットの設定
		m_GraphicsCommandList->OMSetRenderTargets(1,
			&m_RenderTargetHandle[m_RTIndex], TRUE, &m_DepthBufferHandle);

		//デプスバッファ・レンダーターゲットのクリア
		FLOAT clearColor[4] = { 0.8f, 0.5f, 0.9f, 1.0f };
		m_GraphicsCommandList->ClearRenderTargetView(m_RenderTargetHandle[m_RTIndex], clearColor, 0, nullptr);
		m_GraphicsCommandList->ClearDepthStencilView(m_DepthBufferHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		SetTexture(RenderManager::TEXTURE_TYPE::BASE_COLOR,
			m_PostEffectBuffer.get());

		SetTexture(RenderManager::TEXTURE_TYPE::BLOOM0,
			m_BloomBuffer[0].get());
		SetTexture(RenderManager::TEXTURE_TYPE::BLOOM1,
			m_BloomBuffer[1].get());
		SetTexture(RenderManager::TEXTURE_TYPE::BLOOM2,
			m_BloomBuffer[2].get());
		SetTexture(RenderManager::TEXTURE_TYPE::BLOOM3,
			m_BloomBuffer[3].get());
		SetTexture(RenderManager::TEXTURE_TYPE::BLOOM4,
			m_BloomBuffer[4].get());

		DrawScreen();
	}

	//ImGUI
	{
		ImGui::Begin("G-Buffer");

		ImGui::Text("ColorBuffer");
		ImGui::Image((void*)m_ColorBuffer->SRVHandle.ptr,
			ImVec2(256.0f, 128.0f));

		ImGui::Text("NormalBuffer");
		ImGui::Image((void*)m_NormalBuffer->SRVHandle.ptr,
			ImVec2(256.0f, 128.0f));

		ImGui::Text("PositionBuffer");
		ImGui::Image((void*)m_PositionBuffer->SRVHandle.ptr,
			ImVec2(256.0f, 128.0f));

		ImGui::Text("ArmBuffer");
		ImGui::Image((void*)m_ArmBuffer->SRVHandle.ptr,
			ImVec2(320.0f, 180.0f));

		ImGui::Text("EmissionBuffer");
		ImGui::Image((void*)m_EmissionBuffer->SRVHandle.ptr,
			ImVec2(320.0f, 180.0f));

		ImGui::Text("PostEffectBuffer");
		ImGui::Image((void*)m_PostEffectBuffer->SRVHandle.ptr,
			ImVec2(320.0f, 180.0f));

		for (int i = 0; i < 5; i++)
		{
			ImGui::Text("BloomEffect");
			ImGui::Image((void*)m_BloomBuffer[i]->SRVHandle.ptr,
				ImVec2(256.0f, 128.0f));
		}

		ImGui::End();

		ImGui::Render();

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_GraphicsCommandList.Get());
	}


	//リソースバリア
	m_GraphicsCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_RenderTarget[m_RTIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	//コマンド発行
	{
		hr = m_GraphicsCommandList->Close();
		assert(SUCCEEDED(hr));

		ID3D12CommandList* const command_lists[1] = { m_GraphicsCommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(1, command_lists);

		m_CommandQueue->Signal(m_Fence.Get(), m_Frame[m_RTIndex]);
	}



	hr = m_SwapChain->Present(1, 0);
	assert(SUCCEEDED(hr));



	//前フレーム待ち
	{
		UINT64 frame = m_Frame[m_RTIndex];

		m_RTIndex = m_SwapChain->GetCurrentBackBufferIndex();


		if (m_Fence->GetCompletedValue() < m_Frame[m_RTIndex])
		{
			m_Fence->SetEventOnCompletion(m_Frame[m_RTIndex], m_FenceEvent);
			WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
		}

		m_Frame[m_RTIndex] = frame + 1;
	}




	hr = m_GraphicsCommandAllocator[m_RTIndex]->Reset();
	assert(SUCCEEDED(hr));

	hr = m_GraphicsCommandList->Reset(m_GraphicsCommandAllocator[m_RTIndex].Get(), nullptr);
	assert(SUCCEEDED(hr));

}

void RenderManager::DrawScreen()
{



	//頂点バッファ設定
	SetVertexBuffer(m_VertexBuffer.get());


	//トポロジ設定
	m_GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	//描画
	m_GraphicsCommandList->DrawInstanced(4, 1, 0, 0);


}





std::unique_ptr<TEXTURE> RenderManager::LoadTexture(const char* FileName)
{
	std::unique_ptr<TEXTURE> texture = std::make_unique<TEXTURE>();


	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> subresouceData;

	wchar_t wFileName[MAX_PATH];
	size_t size;
	mbstowcs_s(&size, wFileName, FileName, MAX_PATH);

	HRESULT hr = LoadDDSTextureFromFile(m_Device.Get(), wFileName, &texture->Resource, ddsData, subresouceData);
	assert(SUCCEEDED(hr));

	texture->Resource->SetName(wFileName);



	D3D12_RESOURCE_DESC desc = texture->Resource->GetDesc();

	unsigned int bpp, block;

	if (desc.Format == DXGI_FORMAT_BC1_UNORM)
	{
		bpp = 4;
		block = 4;
	}
	else if (desc.Format == DXGI_FORMAT_BC6H_UF16)
	{
		bpp = 8;
		block = 4;
	}
	else
	{
		bpp = 32;
		block = 1;
	}
	




	for (unsigned int a = 0; a < desc.DepthOrArraySize; a++)
	{
		for (unsigned int m = 0; m < desc.MipLevels; m++)
		{
			unsigned int s = a * desc.MipLevels + m;

			unsigned int width = (unsigned int)subresouceData[s].RowPitch * 8 / bpp / block;
			unsigned int height = (unsigned int)subresouceData[s].SlicePitch / (unsigned int)subresouceData[s].RowPitch * block;

			D3D12_BOX box = { 0, 0, 0, width, height, 1 };

			hr = texture->Resource->WriteToSubresource(s, &box, subresouceData[s].pData, (UINT)subresouceData[s].RowPitch, (UINT)subresouceData[s].SlicePitch);
			assert(SUCCEEDED(hr));
		}
	}


	m_GraphicsCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
												texture->Resource.Get(),
												D3D12_RESOURCE_STATE_COPY_DEST,
												D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));





	texture->SRVIndex = CreateShaderResourceView(texture->Resource.Get());



	return std::move(texture);
}






ComPtr<ID3D12PipelineState> RenderManager::CreatePipeline(const char* VertexShaderFile, const char* PixelShaderFile, const DXGI_FORMAT* RTVFormats, unsigned int NumRenderTargets)
{

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};




	//頂点シェーダー読み込み
	std::vector<char> vertexShader;
	{
		std::ifstream file(VertexShaderFile, std::ios_base::in | std::ios_base::binary);
		assert(file);

		file.seekg(0, std::ios_base::end);
		int filesize = (int)file.tellg();
		file.seekg(0, std::ios_base::beg);

		vertexShader.resize(filesize);
		file.read(&vertexShader[0], filesize);

		file.close();


		pipelineStateDesc.VS.pShaderBytecode = &vertexShader[0];
		pipelineStateDesc.VS.BytecodeLength = filesize;
	}


	//ピクセルシェーダー読み込み
	std::vector<char> pixelShader;
	{
		std::ifstream file(PixelShaderFile, std::ios_base::in | std::ios_base::binary);
		assert(file);

		file.seekg(0, std::ios_base::end);
		int filesize = (int)file.tellg();
		file.seekg(0, std::ios_base::beg);

		pixelShader.resize(filesize);
		file.read(&pixelShader[0], filesize);

		file.close();


		pipelineStateDesc.PS.pShaderBytecode = &pixelShader[0];
		pipelineStateDesc.PS.BytecodeLength = filesize;
	}


	//インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC InputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	pipelineStateDesc.InputLayout.pInputElementDescs = InputElementDesc;
	pipelineStateDesc.InputLayout.NumElements = _countof(InputElementDesc);



	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.SampleMask = UINT_MAX;



	pipelineStateDesc.NumRenderTargets = NumRenderTargets;
	for (int i = 0; i < NumRenderTargets; i++)
		pipelineStateDesc.RTVFormats[i] = RTVFormats[i];



	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	pipelineStateDesc.pRootSignature = m_RootSignature.Get();


	//ラスタライザステート
	pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	pipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipelineStateDesc.RasterizerState.FrontCounterClockwise = FALSE;
	pipelineStateDesc.RasterizerState.DepthBias = 0;
	pipelineStateDesc.RasterizerState.DepthBiasClamp = 0;
	pipelineStateDesc.RasterizerState.SlopeScaledDepthBias = 0;
	pipelineStateDesc.RasterizerState.DepthClipEnable = FALSE;
	pipelineStateDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	pipelineStateDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	pipelineStateDesc.RasterizerState.MultisampleEnable = FALSE;


	//ブレンドステート
	for (int i = 0; i < _countof(pipelineStateDesc.BlendState.RenderTarget); ++i)
	{	
		pipelineStateDesc.BlendState.RenderTarget[i].BlendEnable = TRUE;
		pipelineStateDesc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
		pipelineStateDesc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
		pipelineStateDesc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
		pipelineStateDesc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		pipelineStateDesc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
		pipelineStateDesc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		pipelineStateDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		pipelineStateDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
		pipelineStateDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_CLEAR;
		
	}
	pipelineStateDesc.BlendState.AlphaToCoverageEnable = FALSE;
	pipelineStateDesc.BlendState.IndependentBlendEnable = FALSE;


	//デプス・ステンシルステート
	pipelineStateDesc.DepthStencilState.DepthEnable = TRUE;
	pipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	pipelineStateDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	pipelineStateDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	pipelineStateDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	pipelineStateDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	pipelineStateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	pipelineStateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	pipelineStateDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	pipelineStateDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	pipelineStateDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	pipelineStateDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;




	ComPtr<ID3D12PipelineState> pipelineState;
	HRESULT hr = m_Device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(hr));


	return pipelineState;
}








unsigned int RenderManager::CreateShaderResourceView(ID3D12Resource* Resource)
{

	unsigned int index = m_SRVDescriptorPool.front();
	m_SRVDescriptorPool.pop_front();



	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	unsigned int size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.ptr += size * index;


	D3D12_RESOURCE_DESC resDesc = Resource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = resDesc.MipLevels;


	m_Device->CreateShaderResourceView(Resource, &srvDesc, handle);


	return index;
}


D3D12_GPU_DESCRIPTOR_HANDLE RenderManager::GetShaderResourceViewHandle(unsigned int SRVIndex)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	unsigned int size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.ptr += size * SRVIndex;

	return handle;
}


void RenderManager::ReleaseShaderResourceView(unsigned int SRVIndex)
{
	m_SRVDescriptorPool.push_front(SRVIndex);
}





unsigned int RenderManager::CreateRenderTargetView(ID3D12Resource* Resource, unsigned int MipLevel)
{
	unsigned int index = m_RTVDescriptorPool.front();
	m_RTVDescriptorPool.pop_front();



	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	unsigned int size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += size * index;


	m_Device->CreateRenderTargetView(Resource, nullptr, handle);


	return index;
}


D3D12_CPU_DESCRIPTOR_HANDLE RenderManager::GetRenderTargetViewHandle(unsigned int RTVIndex)
{

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	unsigned int size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += size * RTVIndex;

	return handle;
}



void RenderManager::ReleaseRenderTargetView(unsigned int SRVIndex)
{
	m_RTVDescriptorPool.push_front(SRVIndex);
}





std::unique_ptr<RENDER_TARGET> RenderManager::CreateRenderTarget(unsigned int Width, unsigned int Height, DXGI_FORMAT Format, unsigned int MipLeve)
{

	D3D12_HEAP_PROPERTIES properties{};
	properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	properties.CreationNodeMask = 0;
	properties.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = Width;
	desc.Height = Height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = MipLeve;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	desc.Format = Format;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;
	clearValue.Format = Format;


	std::unique_ptr<RENDER_TARGET> renderTarget = std::make_unique<RENDER_TARGET>();

	HRESULT hr = m_Device->CreateCommittedResource(&properties,
													D3D12_HEAP_FLAG_NONE,
													&desc,
													D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
													&clearValue,
													IID_PPV_ARGS(&renderTarget->Resource));
	assert(SUCCEEDED(hr));


	renderTarget->SRVIndex = CreateShaderResourceView(renderTarget->Resource.Get());
	renderTarget->SRVHandle = GetShaderResourceViewHandle(renderTarget->SRVIndex);

	renderTarget->RTVIndex = CreateRenderTargetView(renderTarget->Resource.Get());
	renderTarget->RTVHandle = GetRenderTargetViewHandle(renderTarget->RTVIndex);


	return std::move(renderTarget);
}







void RenderManager::SetConstant(CONSTANT_TYPE Type, const void* Constant, unsigned int Size)
{
	assert(m_ConstantBufferIndex[m_RTIndex] < CONSTANT_BUFFER_MAX);


	memcpy(m_ConstantBufferPointer[m_RTIndex] + CONSTANT_BUFFER_SIZE * m_ConstantBufferIndex[m_RTIndex], Constant, Size);


	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	unsigned int size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.ptr += size * m_ConstantBufferView[m_RTIndex][m_ConstantBufferIndex[m_RTIndex]];

	m_GraphicsCommandList->SetGraphicsRootDescriptorTable((unsigned int)Type, handle);


	m_ConstantBufferIndex[m_RTIndex]++;
}





void RenderManager::SetTexture(TEXTURE_TYPE Type, const TEXTURE* Texture)
{

	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	unsigned int size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.ptr += size * Texture->SRVIndex;

	m_GraphicsCommandList->SetGraphicsRootDescriptorTable((unsigned int)Type, handle);

}



void RenderManager::SetTexture(TEXTURE_TYPE Type, const RENDER_TARGET* Texture)
{

	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	unsigned int size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.ptr += size * Texture->SRVIndex;

	m_GraphicsCommandList->SetGraphicsRootDescriptorTable((unsigned int)Type, handle);

}




std::unique_ptr<VERTEX_BUFFER> RenderManager::CreateVertexBuffer(unsigned int Stride, unsigned int Size)
{
	std::unique_ptr<VERTEX_BUFFER> vertexBuffer = std::make_unique<VERTEX_BUFFER>();


	HRESULT hr = m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
													D3D12_HEAP_FLAG_NONE,
													&CD3DX12_RESOURCE_DESC::Buffer(Stride * Size),
													D3D12_RESOURCE_STATE_GENERIC_READ,
													nullptr,
													IID_PPV_ARGS(&vertexBuffer->Resource));
	assert(SUCCEEDED(hr));


	vertexBuffer->Stride = Stride;
	vertexBuffer->Size = Size;


	return std::move(vertexBuffer);
}

void RenderManager::SetVertexBuffer(const VERTEX_BUFFER* VertexBuffer)
{

	D3D12_VERTEX_BUFFER_VIEW vertexView{};
	vertexView.BufferLocation = VertexBuffer->Resource->GetGPUVirtualAddress();
	vertexView.StrideInBytes = VertexBuffer->Stride;
	vertexView.SizeInBytes = VertexBuffer->Stride * VertexBuffer->Size;

	m_GraphicsCommandList->IASetVertexBuffers(0, 1, &vertexView);
}






std::unique_ptr<INDEX_BUFFER> RenderManager::CreateIndexBuffer(unsigned int Size)
{
	std::unique_ptr<INDEX_BUFFER> indexBuffer = std::make_unique<INDEX_BUFFER>();


	HRESULT hr = m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(unsigned int) * Size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer->Resource));
	assert(SUCCEEDED(hr));


	indexBuffer->Size = Size;


	return std::move(indexBuffer);
}

void RenderManager::SetIndexBuffer(const INDEX_BUFFER* IndexBuffer)
{

	D3D12_INDEX_BUFFER_VIEW indexView{};
	indexView.BufferLocation = IndexBuffer->Resource->GetGPUVirtualAddress();
	indexView.SizeInBytes = sizeof(unsigned int) * IndexBuffer->Size;
	indexView.Format = DXGI_FORMAT_R32_UINT;
	m_GraphicsCommandList->IASetIndexBuffer(&indexView);

}







void RenderManager::SetPipelineState(const char* PiplineName)
{
	ID3D12PipelineState* pipeline = m_PipelineState[PiplineName].Get();
	assert(pipeline);

	m_GraphicsCommandList->SetPipelineState(pipeline);
}







TEXTURE::~TEXTURE()
{
	RenderManager::GetInstance()->ReleaseShaderResourceView(SRVIndex);
}


CONSTANT_BUFFER::~CONSTANT_BUFFER()
{
	RenderManager::GetInstance()->ReleaseShaderResourceView(SRVIndex);
}


RENDER_TARGET::~RENDER_TARGET()
{
	RenderManager::GetInstance()->ReleaseShaderResourceView(SRVIndex);
	RenderManager::GetInstance()->ReleaseRenderTargetView(RTVIndex);
}



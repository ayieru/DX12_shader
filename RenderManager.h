#pragma once



struct VERTEX_3D
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT4 Color;
};


struct MATERIAL
{
	XMFLOAT4		BaseColor;
	XMFLOAT4		EmissionColor;

	float		Metallic;
	float		Specular;
	float		Roughness;
};



// 16*4バイト境界///////////////////////

struct ENV_CONSTANT
{
	XMFLOAT4		LightDirection;
	XMFLOAT4		LightColor;

};


struct CAMERA_CONSTANT
{
	XMFLOAT4X4		View;
	XMFLOAT4X4		Projection;
	XMFLOAT4		Position;
	float       Exposure; //露出補正
	float       Vignette; //ビネット
	float       Gamma;    //ガンマ補正
	float       Bloom;    //ブルーム
	float       Tone;     //トーンマッピング
	float       Chromatic;//色収差
	float       Monokuro; //モノクロ
};


struct OBJECT_CONSTANT
{
	XMFLOAT4X4 World;
};


struct SUBSET_CONSTANT
{
	MATERIAL Material;
};







struct TEXTURE
{
	ComPtr<ID3D12Resource>	Resource;
	unsigned int			SRVIndex;
	~TEXTURE();
};


struct CONSTANT_BUFFER
{
	ComPtr<ID3D12Resource>	Resource;
	unsigned int			SRVIndex;
	~CONSTANT_BUFFER();
};


struct RENDER_TARGET
{
	ComPtr<ID3D12Resource>	Resource;
	unsigned int			SRVIndex;
	unsigned int			RTVIndex;
	D3D12_GPU_DESCRIPTOR_HANDLE SRVHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle;
	~RENDER_TARGET();
};



struct VERTEX_BUFFER
{
	ComPtr<ID3D12Resource>		Resource;
	unsigned int				Stride;
	unsigned int				Size;
};

struct INDEX_BUFFER
{
	ComPtr<ID3D12Resource>		Resource;
	unsigned int				Size;
};








class RenderManager
{

private:
	static RenderManager*				m_Instance;




	HWND								m_WindowHandle;


	bool								m_WindowMode;
	int									m_BackBufferWidth;
	int									m_BackBufferHeight;




	UINT64								m_Frame[2];
	UINT								m_RTIndex;

	ComPtr<IDXGIFactory4>				m_Factory;
	ComPtr<IDXGIAdapter3>				m_Adapter;
	ComPtr<ID3D12Device>				m_Device;
	ComPtr<ID3D12CommandQueue>			m_CommandQueue;
	ComPtr<ID3D12Fence>					m_Fence;
	ComPtr<IDXGISwapChain3>				m_SwapChain;
	ComPtr<ID3D12GraphicsCommandList>	m_GraphicsCommandList;
	ComPtr<ID3D12CommandAllocator>		m_GraphicsCommandAllocator[2];

	HANDLE								m_FenceEvent;




	ComPtr<ID3D12Resource>				m_RenderTarget[2];
	ComPtr<ID3D12DescriptorHeap>		m_RenderTargetDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_RenderTargetHandle[2];

	ComPtr<ID3D12Resource>				m_DepthBuffer;
	ComPtr<ID3D12DescriptorHeap>		m_DepthBufferDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_DepthBufferHandle;





	D3D12_RECT							m_ScissorRect;
	D3D12_VIEWPORT						m_Viewport;



	ComPtr<ID3D12DescriptorHeap>		m_SRVDescriptorHeap;
	std::list<unsigned int>				m_SRVDescriptorPool;
	static const unsigned int			SRV_DESCRIPTOR_MAX = 10000;

	ComPtr<ID3D12DescriptorHeap>		m_RTVDescriptorHeap;
	std::list<unsigned int>				m_RTVDescriptorPool;
	static const unsigned int			RTV_DESCRIPTOR_MAX = 1000;




	static const unsigned int			CONSTANT_BUFFER_SIZE = 512;
	static const unsigned int			CONSTANT_BUFFER_MAX = 1000;
	ComPtr<ID3D12Resource>				m_ConstantBuffer[2];
	byte*								m_ConstantBufferPointer[2];
	unsigned int						m_ConstantBufferView[2][CONSTANT_BUFFER_MAX];
	unsigned int						m_ConstantBufferIndex[2];



	ComPtr<ID3D12RootSignature>			m_RootSignature;


	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>	m_PipelineState;
	ComPtr<ID3D12PipelineState> CreatePipeline(const char* VertexShaderFile, const char* PixelShaderFile, const DXGI_FORMAT* RTVFormats, unsigned int NumRenderTargets);


	std::unique_ptr<VERTEX_BUFFER>		m_VertexBuffer;

	std::unique_ptr<RENDER_TARGET>      m_ColorBuffer;
	std::unique_ptr<RENDER_TARGET>      m_NormalBuffer;
	std::unique_ptr<RENDER_TARGET>      m_PositionBuffer;
	std::unique_ptr<RENDER_TARGET>      m_ArmBuffer;
	std::unique_ptr<RENDER_TARGET>      m_EmissionBuffer;
	std::unique_ptr<RENDER_TARGET>      m_PostEffectBuffer;
	std::unique_ptr<RENDER_TARGET>      m_BloomBuffer[5];

	std::unique_ptr<TEXTURE>      m_EnvironmentTexture;

	SUBSET_CONSTANT m_SubsetConstant{};


	void Init();

	unsigned int CreateShaderResourceView(ID3D12Resource* Resource);
	D3D12_GPU_DESCRIPTOR_HANDLE GetShaderResourceViewHandle(unsigned int SRVIndex);

	unsigned int CreateRenderTargetView(ID3D12Resource* Resource, unsigned int MipLevel = 0);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetViewHandle(unsigned int RTVIndex);


public:



	static RenderManager* GetInstance() { return m_Instance; }

	ID3D12Device* GetDevice() { return m_Device.Get(); }
	ID3D12GraphicsCommandList* GetGraphicsCommandList() { return m_GraphicsCommandList.Get(); }

	int GetBackBufferWidth() { return m_BackBufferWidth; }
	int GetBackBufferHeight() { return m_BackBufferHeight; }



	RenderManager();
	~RenderManager();


	void WaitGPU();

	void DrawBegin();
	void DrawEnd();


	void DrawScreen();



	void ReleaseShaderResourceView(unsigned int SRVIndex);
	void ReleaseRenderTargetView(unsigned int SRVIndex);





	//レンダーターゲット
	std::unique_ptr<RENDER_TARGET> CreateRenderTarget(unsigned int Width, unsigned int Height, DXGI_FORMAT Format, unsigned int MipLevel = 1);



	//定数バッファ
	enum class CONSTANT_TYPE
	{
		ENV,
		CAMERA,
		OBJECT,
		SUBSET
	};
	void SetConstant(CONSTANT_TYPE Type, const void* Constant, unsigned int Size);




	//テクスチャ
	enum class TEXTURE_TYPE
	{
		BASE_COLOR = (int)CONSTANT_TYPE::SUBSET+1,
		NORMAL,
		POSITION,
		ARM,
		EMISSION,

		ENVIRONMENT,

		BLOOM0 = NORMAL,
		BLOOM1,
		BLOOM2,
		BLOOM3,
		BLOOM4,
	};
	std::unique_ptr<TEXTURE> LoadTexture(const char* FileName);
	void SetTexture(TEXTURE_TYPE Type, const TEXTURE* Texture);
	void SetTexture(TEXTURE_TYPE Type, const RENDER_TARGET* Texture);




	//頂点バッファ
	std::unique_ptr<VERTEX_BUFFER> CreateVertexBuffer(unsigned int Stride, unsigned int Size);
	void SetVertexBuffer(const VERTEX_BUFFER* VertexBuffer);


	//インデックスバッファ
	std::unique_ptr<INDEX_BUFFER> CreateIndexBuffer(unsigned int Size);
	void SetIndexBuffer(const INDEX_BUFFER* IndexBuffer);


	void SetPipelineState(const char* PiplineName);




};


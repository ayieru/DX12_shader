#include "Main.h"
#include "RenderManager.h"
#include "Polygon2D.h"

Polygon2D::Polygon2D()
{
	RenderManager* renderManager = RenderManager::GetInstance();

	//テクスチャ読み込み
	m_Texture = renderManager->LoadTexture("Asset\\field004.dds");

	m_VertexBuffer = renderManager->CreateVertexBuffer(sizeof(VERTEX_3D), 4);

	//頂点データの読み込み
	VERTEX_3D* buffer{};
	HRESULT hr = m_VertexBuffer->Resource->Map(0, nullptr, (void**)&buffer);
	assert(SUCCEEDED(hr));

	buffer[0].Position = {   0.0f,  0.0f,0.0f };
	buffer[1].Position = { 200.0f,  0.0f,0.0f };
	buffer[2].Position = {   0.0f,200.0f,0.0f };
	buffer[3].Position = { 200.0f,200.0f,0.0f };

	buffer[0].Color = { 1.0f,1.0f,1.0f,1.0f };
	buffer[1].Color = { 1.0f,1.0f,1.0f,1.0f };
	buffer[2].Color = { 1.0f,1.0f,1.0f,1.0f };
	buffer[3].Color = { 1.0f,1.0f,1.0f,1.0f };

	buffer[0].Normal = { 0.0f,1.0f,0.0f };
	buffer[1].Normal = { 0.0f,1.0f,0.0f };
	buffer[2].Normal = { 0.0f,1.0f,0.0f };
	buffer[3].Normal = { 0.0f,1.0f,0.0f };

	buffer[0].TexCoord = { 0.0f,0.0f };
	buffer[1].TexCoord = { 1.0f,0.0f };
	buffer[2].TexCoord = { 0.0f,1.0f };
	buffer[3].TexCoord = { 1.0f,1.0f };

	m_VertexBuffer->Resource->Unmap(0, nullptr);

}

void Polygon2D::Update()
{

}

void Polygon2D::Draw()
{
	RenderManager* renderManager = RenderManager::GetInstance();

	//マトリクス設定
	{
		XMMATRIX world = XMMatrixIdentity();
		OBJECT_CONSTANT constant{};
		XMStoreFloat4x4(&constant.World, XMMatrixTranspose(world));

		renderManager->SetConstant(RenderManager::CONSTANT_TYPE::OBJECT, &constant, sizeof(constant));
	}

	{
		XMMATRIX view = XMMatrixIdentity();

		XMMATRIX projection;
		projection = XMMatrixOrthographicOffCenterLH(0.0f,
			renderManager->GetBackBufferWidth(),
			renderManager->GetBackBufferHeight(),
			0.0f, 0.0f, 1.0f);

		CAMERA_CONSTANT constant{};
		XMStoreFloat4x4(&constant.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&constant.Projection, XMMatrixTranspose(projection));

		renderManager->SetConstant(RenderManager::CONSTANT_TYPE::CAMERA,
			&constant, sizeof(constant));
	}

	//テクスチャ設定
	renderManager->SetTexture(
		RenderManager::TEXTURE_TYPE::BASE_COLOR, m_Texture.get());

	//頂点バッファ設定
	renderManager->SetVertexBuffer(m_VertexBuffer.get());

	//トポロジ設定
	renderManager->GetGraphicsCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//描画
	renderManager->GetGraphicsCommandList()->DrawInstanced(4, 1, 0, 0);

}
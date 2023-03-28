#include "Main.h"
#include "RenderManager.h"
#include "Camera.h"

Camera::Camera()
{
	m_Position = { 0.0f,1.0f,-2.5f };
	m_Target = { 0.0f,0.0f,0.0f };

	Exposure = 0.0f; //露出補正
	Vignette = 0.0f; //ビネット
	Gamma = 0.0f;    //ガンマ補正
	Bloom = 0.0f;    //ブルーム
	Tone = 0.0f;     //トーンマッピング
	Chromatic = 0.0f;//色収差
	Monokuro = 0.0f; //モノクロ
}

void Camera::Update()
{

}

void Camera::Draw()
{
	RenderManager* renderManager = RenderManager::GetInstance();

	//ImGUI
	{
		ImGui::Begin("Camera");
		ImGui::DragFloat3("Position", (float*)&m_Position, 0.1f);
		ImGui::End();

		ImGui::Begin("PostEffect");
		ImGui::Text("Exposure");
		ImGui::SliderFloat("Exposure", &Exposure, 0.0f, 1.0f, "%.2f");

		ImGui::Text("Vignette");
		ImGui::SliderFloat("Vignette", &Vignette, 0.0f, 1.0f, "%.2f");

		ImGui::Text("Gamma");
		ImGui::SliderFloat("Gamma", &Gamma, 0.0f, 1.0f, "%.2f");

		ImGui::Text("Bloom");
		ImGui::SliderFloat("Bloom", &Bloom, 0.0f, 1.0f, "%.2f");

		ImGui::Text("Tone");
		ImGui::SliderFloat("Tone", &Tone, 0.0f, 1.0f, "%.2f");

		ImGui::Text("Chromatic");
		ImGui::SliderFloat("Chromatic", &Chromatic, 0.0f, 1.0f, "%.2f");

		ImGui::Text("Monokuro");
		ImGui::SliderFloat("Monokuro", &Monokuro, 0.0f, 1.0f, "%.2f");
		ImGui::End();
	}

	//マトリクス設定
	{
		XMMATRIX view;
		XMFLOAT3 up{ 0.0f,1.0f,0.0f };
		view = XMMatrixLookAtLH(XMLoadFloat3(&m_Position),
			XMLoadFloat3(&m_Target),
			XMLoadFloat3(&up));

		XMMATRIX projection;
		float aspect = (float)renderManager->GetBackBufferWidth() / renderManager->GetBackBufferHeight();
		projection = XMMatrixPerspectiveFovLH(1.0f, aspect, 0.1f, 100.0f);

		CAMERA_CONSTANT constant{};
		XMStoreFloat4x4(&constant.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&constant.Projection, XMMatrixTranspose(projection));
		constant.Position = { m_Position.x,m_Position.y,m_Position.z,0.0f };
		constant.Exposure = Exposure; 
		constant.Vignette = Vignette; 
		constant.Gamma = Gamma;  
		constant.Bloom = Bloom;
		constant.Tone = Tone;     
		constant.Chromatic = Chromatic;
		constant.Monokuro = Monokuro; 

		renderManager->SetConstant(RenderManager::CONSTANT_TYPE::CAMERA, &constant, sizeof(constant));
	}


}


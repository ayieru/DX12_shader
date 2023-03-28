#include "Main.h"
#include "RenderManager.h"
#include "Camera.h"

Camera::Camera()
{
	m_Position = { 0.0f,1.0f,-2.5f };
	m_Target = { 0.0f,0.0f,0.0f };

	Exposure = 0.0f; //�I�o�␳
	Vignette = 0.0f; //�r�l�b�g
	Gamma = 0.0f;    //�K���}�␳
	Bloom = 0.0f;    //�u���[��
	Tone = 0.0f;     //�g�[���}�b�s���O
	Chromatic = 0.0f;//�F����
	Monokuro = 0.0f; //���m�N��
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

	//�}�g���N�X�ݒ�
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


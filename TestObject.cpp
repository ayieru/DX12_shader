#include "Main.h"
#include "RenderManager.h"
#include "TestObject.h"

TestObject::TestObject()
{
	m_Model.Load("Asset\\Chandelier_01_4k.obj");
	m_Model2.Load("Asset\\Drill_01_4k.obj");
	m_Model3.Load("Asset\\d.obj");

	m_Position = { 0.0f,1.5f,0.0f };
	m_Rotation = { 0.0f,0.0f,0.0f };
	m_Scale    = { 2.0f,2.0f,2.0f };

	m_Position2 = { -1.0f,0.0f,0.0f };
	m_Rotation2 = { 0.0f,0.0f,0.0f };
	m_Scale2 = { 5.0f,5.0f,5.0f };

	m_Position3 = { 1.0f,0.0f,0.0f };
	m_Rotation3 = { 0.0f,0.0f,0.0f };
	m_Scale3 = { 2.0f,2.0f,2.0f };

}

void TestObject::Update()
{
	m_Rotation.y += 0.001f;
	m_Rotation2.y += 0.02f;
}

void TestObject::Draw()
{
	RenderManager* renderManager = RenderManager::GetInstance();

	//ImGUI
	{
		ImGui::Begin("Object");
		ImGui::DragFloat3("Position", (float*)&m_Position, 0.1f);
		ImGui::DragFloat3("Rotation", (float*)&m_Rotation, 0.1f);
		ImGui::DragFloat3("Scale", (float*)&m_Scale, 0.1f);
		ImGui::End();

		ImGui::Begin("Object2");
		ImGui::DragFloat3("Position", (float*)&m_Position2, 0.1f);
		ImGui::DragFloat3("Rotation", (float*)&m_Rotation2, 0.1f);
		ImGui::DragFloat3("Scale", (float*)&m_Scale2, 0.1f);
		ImGui::End();


		ImGui::Begin("Object3");
		ImGui::DragFloat3("Position", (float*)&m_Position3, 0.1f);
		ImGui::DragFloat3("Rotation", (float*)&m_Rotation3, 0.1f);
		ImGui::DragFloat3("Scale", (float*)&m_Scale3, 0.1f);
		ImGui::End();
	}


	//マトリクス設定
	{
		XMMATRIX world = XMMatrixIdentity();
		world *= XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
		world *= XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		world *= XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

		OBJECT_CONSTANT constant{};
		XMStoreFloat4x4(&constant.World, XMMatrixTranspose(world));

		renderManager->SetConstant(RenderManager::CONSTANT_TYPE::OBJECT, &constant, sizeof(constant));
	}

	//パイプライン設定
	//renderManager->SetPipelineState("Unlit");

	m_Model.Draw(true);

	//マトリクス設定
	{
		XMMATRIX world = XMMatrixIdentity();
		world *= XMMatrixScaling(m_Scale2.x, m_Scale2.y, m_Scale2.z);
		world *= XMMatrixRotationRollPitchYaw(m_Rotation2.x, m_Rotation2.y, m_Rotation2.z);
		world *= XMMatrixTranslation(m_Position2.x, m_Position2.y, m_Position2.z);

		OBJECT_CONSTANT constant{};
		XMStoreFloat4x4(&constant.World, XMMatrixTranspose(world));

		renderManager->SetConstant(RenderManager::CONSTANT_TYPE::OBJECT, &constant, sizeof(constant));
	}

	m_Model2.Draw(true);

	//マトリクス設定
	{
		XMMATRIX world = XMMatrixIdentity();
		world *= XMMatrixScaling(m_Scale3.x, m_Scale3.y, m_Scale3.z);
		world *= XMMatrixRotationRollPitchYaw(m_Rotation3.x, m_Rotation3.y, m_Rotation3.z);
		world *= XMMatrixTranslation(m_Position3.x, m_Position3.y, m_Position3.z);

		OBJECT_CONSTANT constant{};
		XMStoreFloat4x4(&constant.World, XMMatrixTranspose(world));

		renderManager->SetConstant(RenderManager::CONSTANT_TYPE::OBJECT, &constant, sizeof(constant));
	}

	m_Model3.Draw(true);
}



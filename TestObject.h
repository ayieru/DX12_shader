#pragma once
#include "Model.h"

class TestObject
{
private:
	XMFLOAT3 m_Position{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Rotation{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Scale{ 1.0f,1.0f,1.0f };
	Model m_Model;

	Model m_Model2;
	XMFLOAT3 m_Position2{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Rotation2{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Scale2{ 1.0f,1.0f,1.0f };

	Model m_Model3;
	XMFLOAT3 m_Position3{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Rotation3{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Scale3{ 1.0f,1.0f,1.0f };

public:
	TestObject();

	void Update();
	void Draw();


};


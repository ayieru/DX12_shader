#pragma once
#include "Model.h"

class Sky
{
private:
	XMFLOAT3 m_Position{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Rotation{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Scale{ 1.0f,1.0f,1.0f };

	Model m_Model;
	std::unique_ptr<TEXTURE>       m_Texture;

public:
	Sky();

	void Update();
	void Draw();
};


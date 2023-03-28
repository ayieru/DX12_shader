#pragma once

class Field
{
private:

	XMFLOAT3 m_Position{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Rotation{ 0.0f,0.0f,0.0f };
	XMFLOAT3 m_Scale{ 1.0f,1.0f,1.0f };

	XMFLOAT2 m_LightRotation{ 0.0f,0.0f };
	XMFLOAT3 m_LightColor{ 1.0f,1.0f,1.0f };
	float    m_LightIntensity{ 0.4f };

	std::unique_ptr<VERTEX_BUFFER> m_VertexBuffer;
	std::unique_ptr<TEXTURE>       m_Texture;

public:

	Field();

	void Update();
	void Draw();

};


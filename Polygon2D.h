#pragma once

class Polygon2D
{
private: 

	std::unique_ptr<VERTEX_BUFFER> m_VertexBuffer;
	std::unique_ptr<TEXTURE>       m_Texture;

public:

	Polygon2D();

	void Update();
	void Draw();

};
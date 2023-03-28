#pragma once


#include "RenderManager.h"
#include "Polygon2D.h"
#include "Camera.h"
#include "Field.h"
#include "TestObject.h"
#include "sky.h"

class GameManager
{
private:

	static GameManager* m_Instance;

	RenderManager	m_RenderManger;
	Camera          m_Camera;
	Sky             m_Sky;
	Polygon2D       m_Polygon2D;
	Field           m_Field;
	TestObject      m_TestObject;


public:
	static GameManager* GetInstance() { return m_Instance; }

	GameManager();
	~GameManager();



	void Update();
	void Draw();


};


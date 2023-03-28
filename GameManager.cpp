

#include "Main.h"
#include "GameManager.h"




GameManager* GameManager::m_Instance = nullptr;





GameManager::GameManager()
{
	m_Instance = this;


}




GameManager::~GameManager()
{
	//•`‰æ‚ÌI—¹‚ğ‘Ò‚Â
	m_RenderManger.WaitGPU();
}





void GameManager::Update()
{
	m_TestObject.Update();
	//m_Polygon2D.Update();
}




void GameManager::Draw()
{

	m_RenderManger.DrawBegin();

	m_Camera.Draw();

	m_Sky.Draw();

	m_Field.Draw();

	m_TestObject.Draw();

	//m_Polygon2D.Draw();

	m_RenderManger.DrawEnd();

}



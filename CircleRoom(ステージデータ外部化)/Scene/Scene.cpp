#include "Scene.h"
#include "GameManager.h"

Scene::Scene(GameManager& mgr) :
	m_mgr(mgr),
	m_sound(mgr.GetSound())
{
}

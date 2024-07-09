#ifdef _DEBUG
#include <DxLib.h>
#include "DebugScene.h"
#include "Input.h"
#include "TitleScene.h"
#include "GamePlayingScene.h"
#include "GameClearScene.h"
#include  "GameManager.h"
#include "SceneManager.h"

DebugScene::DebugScene(GameManager& mgr) :
	Scene(mgr),
	m_line(0)
{
}

DebugScene::~DebugScene()
{
}

void DebugScene::Update(Input& input)
{
	if (input.IsTriggered("up"))
	{
		m_line = (m_line - 1 + 3) % 3;
	}
	if (input.IsTriggered("down"))
	{
		m_line = (m_line + 1) % 3;
	}

	if (input.IsTriggered("OK"))
	{
		if (m_line == 0)
		{
			m_mgr.GetScene()->ChangeScene(std::make_shared<TitleScene>(m_mgr, input), 1.0f);
		}
		else if (m_line == 1)
		{
			m_mgr.GetScene()->ChangeScene(std::make_shared<GamePlayingScene>(m_mgr, input), 0.5f);
		}
		else if (m_line == 2)
		{
			m_mgr.GetScene()->ChangeScene(std::make_shared<GameClearScene>(m_mgr));
		}
	}
}

void DebugScene::Draw() const
{
	DrawLine(100, 26 + m_line * 30, 300, 26 + m_line * 30, 0xff0000);

	DrawString(100, 10, L"Title", 0xffffff);
	DrawString(100, 40, L"Game", 0xffffff);
	DrawString(100, 70, L"Clear", 0xffffff);
}
#endif
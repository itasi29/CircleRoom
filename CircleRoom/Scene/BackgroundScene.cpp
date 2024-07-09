#include <DxLib.h>
#include "Application.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "BackgroundScene.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include <sstream>

namespace
{
	// スクロールスピード
	constexpr float kSpeed = 2.0f;

	// 背景画像数
	constexpr int kBgNum = 4;
}

BackgroundScene::BackgroundScene(std::shared_ptr<FileManager> mgr,  bool isMove, bool isDraw) :
	m_scroll(0),
	m_scrollSpeed(kSpeed),
	m_isMove(isMove),
	m_isDraw(isDraw)
{
	m_bg.resize(kBgNum);
	for (int i = 0; i < kBgNum; i++)
	{
		std::wostringstream woss;
		woss << L"Bg/bg" << i << L".png";
		m_bg[i] = mgr->LoadGraphic(woss.str());
	}
}

BackgroundScene::~BackgroundScene()
{
}

void BackgroundScene::Update()
{
	if (!m_isDraw) return;

	m_scroll -= m_scrollSpeed;
}

void BackgroundScene::Draw() const
{
	if (!m_isDraw) return;

	const auto& size = Application::GetInstance().GetWindowSize();

	for (int i = 0; i < kBgNum; i++)
	{
		int posX = static_cast<int>(m_scroll * ((i + 1) / static_cast<float>(kBgNum))) % size.w;
		DrawGraph(posX, 0, m_bg[i]->GetHandle(), true);
		posX += size.w;
		DrawGraph(posX, 0, m_bg[i]->GetHandle(), true);
	}
}

void BackgroundScene::Init(float speed)
{
	if (speed < 0)
	{
		m_scrollSpeed = kSpeed;
	}
	else
	{
		m_scrollSpeed = speed;
	}

	m_scroll = 0;
}

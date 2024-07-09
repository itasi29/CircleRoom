#include <DxLib.h>
#include <cassert>
#include <string>
#include "Common/Input.h"
#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/SoundSystem.h"
#include "GameClearScene.h"
#include "TitleScene.h"
#include "Application.h"

namespace
{
	// 文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	constexpr unsigned int kYellowColor = 0xffde00;
	constexpr unsigned int kRedColor = 0xd2001a;

	// フェード時間
	constexpr int kFadeFrame = 60;

	// リザルト数
	constexpr int kResultNum = 3;

	// 文字のX座標描画位置
	constexpr int kDrawResultX = 350;
	// リザルト結果のX座標描画位置
	constexpr int kDrawResultDataX = 750;

	// 文字同士の間隔
	constexpr int kTextInterval = 180;

	const std::wstring kResultStr[kResultNum] = {
		L"クリア時間",
		L"死亡数",
		L"ダッシュ使用回数"
	};
}

GameClearScene::GameClearScene(GameManager& mgr) :
	Scene(mgr),
	m_frame(kFadeFrame),
	m_textFrame(0),
	m_index(0)
{
	m_updateFunc = &GameClearScene::FadeInUpdate;
	m_drawFunc = &GameClearScene::FadeDraw;

	auto& file = mgr.GetFile();
	m_result = file->LoadGraphic(L"UI/result.png");
	m_bgm = file->LoadSound(L"Bgm/end.mp3");
}

GameClearScene::~GameClearScene()
{
}

void GameClearScene::Update(Input& input)
{
	(this->*m_updateFunc)(input);
}

void GameClearScene::Draw() const
{
	(this->*m_drawFunc)();
}

void GameClearScene::FadeInUpdate(Input&)
{
	m_sound->PlayFadeBgm(m_bgm->GetHandle(), 1.0f - (m_frame / static_cast<float>(kFadeFrame)));
	m_frame--;
	if (m_frame <= 0)
	{
		m_updateFunc = &GameClearScene::NormalUpdate;
		m_drawFunc = &GameClearScene::NormalDraw;

		m_frame = 0;
	}
}

void GameClearScene::NormalUpdate(Input& input)
{
	m_sound->PlayBgm(m_bgm->GetHandle());
	m_textFrame++;

	if (m_index < kResultNum)
	{
		if (m_textFrame > kTextInterval || input.IsTriggered("OK"))
		{
			m_textFrame = 0;
			m_index++;
		}

		return;
	}

	if (input.IsAnyTriggerd())
	{
		m_updateFunc = &GameClearScene::FadeOutUpdate;
		m_drawFunc = &GameClearScene::FadeDraw;

		m_frame = 0;
	}
}

void GameClearScene::FadeOutUpdate(Input& input)
{
	m_sound->PlayFadeBgm(m_bgm->GetHandle(), 1.0f - (m_frame / static_cast<float>(kFadeFrame)));
	m_frame++;
	if (kFadeFrame <= m_frame)
	{
		m_sound->Stop(m_bgm->GetHandle());
		m_mgr.GetScene()->ChangeScene(std::make_shared<TitleScene>(m_mgr, input), 1.0f);
	}
}

void GameClearScene::FadeDraw() const
{
	NormalDraw();

	int alpha = static_cast<int>(255 * (static_cast<float>(m_frame) / 60.0f));
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, 1280, 720, 0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameClearScene::NormalDraw() const
{
//	DrawStringToHandle(550, 50, L"リザルト", kYellowColor, m_mgr.GetFont()->GetHandle(48));
	const auto& size = Application::GetInstance().GetWindowSize();
	DrawRotaGraph(static_cast<int>(size.w * 0.5f), static_cast<int>(size.h * 0.5f), 1.0, 0.0, m_result->GetHandle(), true);
	int fontHandle = m_mgr.GetFont()->GetHandle(32);

	int drawY = 256;
	for (int i = 0; i < m_index + 1; i++)
	{
		if (!(i < kResultNum))
		{
			int val = (m_textFrame + kTextInterval) % (kTextInterval * 2) - kTextInterval;
			float rate = (fabs(static_cast<float>(val)) / kTextInterval);
			int alpha = static_cast <int>(255 * rate);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawStringToHandle(500, drawY - 16, L"～　PressAnyKey　～", kYellowColor, fontHandle);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			break;
		}

		if (i == m_index)
		{
			float rate = (static_cast<float>(m_textFrame) / kTextInterval);
			int alpha = static_cast <int>(255 * rate);
			int y = drawY - static_cast<int>(100 * (1 - rate));

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawFormatStringToHandle(kDrawResultX, y, kYellowColor, fontHandle, L"%s", kResultStr[i].c_str());
			DrawInf(i, y, fontHandle);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else
		{
			DrawFormatStringToHandle(kDrawResultX, drawY, kYellowColor, fontHandle, L"%s", kResultStr[i].c_str());
			DrawInf(i, drawY, fontHandle);
		}

		drawY += 100;
	}
}

void GameClearScene::DrawInf(int index, int drawY, int handle) const
{
	const auto& data = m_mgr.GetData();

	if (index == 0)
	{
		int sec = (data.playTime / 60) % 60;
		int min = (data.playTime / 3600) % 60;
		int hour = (data.playTime / 216000);
		DrawFormatStringToHandle(kDrawResultDataX, drawY, kRedColor, handle, L"%02d時間%02d分%02d秒", hour, min, sec);
	}
	else if (index == 1)
	{
		DrawFormatStringToHandle(kDrawResultDataX, drawY, kRedColor, handle, L"%-3d回", data.deathCount);
	}
	else
	{
		DrawFormatStringToHandle(kDrawResultDataX, drawY, kRedColor, handle, L"%-3d回", data.dashCount);
	}
}
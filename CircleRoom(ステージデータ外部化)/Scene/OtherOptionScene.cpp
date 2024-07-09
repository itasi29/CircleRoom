#include <DxLib.h>
#include <cassert>
#include <vector>
#include <string>
#include <memory>

#include "Application.h"
#include "Input.h"
#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/BottansFile.h"
#include "FileSystem/KeyFile.h"

#include "TitleScene.h"
#include "ExplanationScene.h"
#include "OneShotScene.h"

#include "OtherOptionScene.h"

namespace
{
	// フレームの色
	constexpr unsigned int kFrameColor = 0xd2001a;
	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	// 選択時文字列の色
	constexpr unsigned int kYellowColor = 0xffde00;;
	// 点滅間隔
	constexpr int kFlashInterval = 40;

	// X座標描画基準
	constexpr int kDrawX = 132;
	// フレーム幅
	constexpr int kFrameWidht = 788;
	// フレームの左余白
	constexpr int kFrameMargin = 20;

	// フェードフレーム
	constexpr int kFadeFrame = 60;

	constexpr int kAppeaInterval = 5;
	constexpr int kMenuMargin = 120;
	// 文字間の幅
	constexpr int kMenuLineInterval = 96;

	// MEMO:権利表記を作らなかったので未表示に
	enum
	{
		kTitle,
		kWindowsMode,
		kExplanation,
//		kRightsNotation,
		kEnd
	};

	const std::vector<std::wstring> kGameMenu = { L"タイトルへ",
		L"ウィンドウモード",
		L"説明書類",
//		L"権利表記",
		L"終了"
	};

	// スタート文字のウェーブスピード
	constexpr float kWaveSpeed = DX_PI_F / 180 * 5;
	// ウェーブの間隔
	constexpr float kWaveInterval = DX_PI_F / 15.0f;

	// ウェーブ文字列
	int kSelectWaveNum = 4;
	int kSelectWavePosX = 1064;
	int kSelectWavePosY = 592;
	const wchar_t* const kSelectWave[] = { L"け", L"っ", L"て", L"い" };

	int kBackWaveNum = 3;
	int kBackWavePosX = 1128;
	int kBackWavePosY = 544;
	const wchar_t* const kBackWave[] = { L"も", L"ど", L"る" };
}

OtherOptionScene::OtherOptionScene(GameManager& mgr, Input& input, std::shared_ptr<SceneManager> scn) :
	Scene(mgr),
	m_input(input),
	m_optionScn(scn),
	m_currentLineIndex(0),
	m_fadeFrame(0),
	m_waveAngle(DX_PI_F),
	m_isWaveDraw(true)
{
	auto& file = m_mgr.GetFile();
	m_rightNotationImg = file->LoadGraphic(L"UI/16-9.png");
	m_frame = file->LoadGraphic(L"UI/normalFrame.png", true);
	m_startFrame = file->LoadGraphic(L"UI/startFrame.png");

	m_cursorSe = file->LoadSound(L"Se/cursor.mp3", true);
	m_selectSe = file->LoadSound(L"Se/select.mp3", true);

	m_bt = std::make_shared<BottansFile>(file);
	m_key = std::make_shared<KeyFile>(file);

	const auto& size = Application::GetInstance().GetWindowSize();

	m_updateFunc = &OtherOptionScene::NormalUpdate;
	m_drawFunc = &OtherOptionScene::NormalDraw;
}

OtherOptionScene::~OtherOptionScene()
{
}

void OtherOptionScene::Update(Input& input)
{
	m_isWaveDraw = true;
	m_waveAngle -= kWaveSpeed;
	(this->*m_updateFunc)(input);
}

void OtherOptionScene::Draw() const
{
	(this->*m_drawFunc)();
}

void OtherOptionScene::FadeUpdate(Input& input)
{
	m_fadeFrame++;
	float rate = (1.0f - m_fadeFrame / static_cast<float>(kFadeFrame));
	m_sound->PlayFadeBgm(-1, rate);

	if (m_fadeFrame > kFadeFrame)
	{
		m_mgr.GetScene()->ChangeSceneWithClear(std::make_shared<TitleScene>(m_mgr, input), 1.0f);
		return;
	}
}

void OtherOptionScene::NormalUpdate(Input& input)
{
	m_fadeFrame++;

	if (input.IsTriggered("up"))
	{
		m_currentLineIndex = (m_currentLineIndex - 1 + static_cast<int>(kGameMenu.size())) % static_cast<int>(kGameMenu.size());
		m_fadeFrame = 0;
		m_sound->PlaySe(m_cursorSe->GetHandle());
	}
	if (input.IsTriggered("down"))
	{
		m_currentLineIndex = (m_currentLineIndex + 1) % kGameMenu.size();
		m_fadeFrame = 0;
		m_sound->PlaySe(m_cursorSe->GetHandle());
	}

	if (input.IsTriggered("OK"))
	{
		auto& sound = GameManager::GetInstance().GetSound();
		sound->PlaySe(m_selectSe->GetHandle());
		if (m_currentLineIndex == kTitle)
		{
			m_fadeFrame = 0;
			m_updateFunc = &OtherOptionScene::FadeUpdate;
			m_drawFunc = &OtherOptionScene::FadeDraw;
		}
		else if (m_currentLineIndex == kWindowsMode)
		{
			Application::GetInstance().ChangeWindows();
		}
		else if (m_currentLineIndex == kExplanation)
		{
			m_isWaveDraw = false;
			m_optionScn->ChangeScene(std::make_shared<ExplanationScene>(m_mgr, input, m_optionScn));
		}
		//else if (m_currentLineIndex == kRightsNotation)
		//{
		//	m_mgr.GetScene()->PushScene(std::make_shared<OneShotScene>(m_mgr, m_rightNotationImg->GetHandle()));
		//}
		else if (m_currentLineIndex == kEnd)
		{
			Application& app = Application::GetInstance();
			app.End();
		}
	}
}

void OtherOptionScene::FadeDraw() const
{
	NormalDraw();

	const auto& size = Application::GetInstance().GetWindowSize();

	float rate = m_fadeFrame / static_cast<float>(kFadeFrame);
	int alpha = static_cast<int>(255 * rate);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, size.w, size.h, 0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void OtherOptionScene::NormalDraw() const
{
	// 選択している場所を描画
	int y = kMenuMargin + 36 + m_currentLineIndex * kMenuLineInterval;

	DrawGraph(kDrawX + kFrameWidht, y, m_frame->GetHandle(), true);
	DrawBox(kDrawX - kFrameMargin, y,
		kDrawX + kFrameWidht, y + 44,
		kFrameColor, true);

	int fontHandle = m_mgr.GetFont()->GetHandle(32);

	y = kMenuMargin + 42;

	// メニューの文字列群
	for (int i = 0; i < kGameMenu.size(); i++)
	{
		if (m_currentLineIndex == i)
		{
			int frame = (m_fadeFrame % (kFlashInterval * 2)) - kFlashInterval;
			float rate = fabsf(static_cast<float>(frame)) / kFlashInterval;
			int alpha = static_cast <int>(255 * rate);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawStringToHandle(kDrawX, y, kGameMenu[i].c_str(), kYellowColor, fontHandle);
			DrawWindowMode(i, fontHandle, y, kYellowColor);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else
		{
			DrawWindowMode(i, fontHandle, y, kWhiteColor);
			DrawStringToHandle(kDrawX, y, kGameMenu[i].c_str(), kWhiteColor, fontHandle);
		}

		y += kMenuLineInterval;
	}

	DrawWave(kBackWavePosX, kBackWavePosY, "cancel", kBackWave, kBackWaveNum);
	DrawWave(kSelectWavePosX, kSelectWavePosY, "OK", kSelectWave, kSelectWaveNum);
}

void OtherOptionScene::DrawWindowMode(int index, int handle, int y, unsigned int color) const
{
	if (index != kWindowsMode) return;

	if (Application::GetInstance().IsWindows())
	{
		DrawStringToHandle(500, y, L"ウィンドウ", color, handle);
	}
	else
	{
		DrawStringToHandle(500, y, L"フルスクリーン", color, handle);
	}
}

void OtherOptionScene::DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const
{
	if (!m_isWaveDraw) return;

	DrawGraph(x - 84, y - 5, m_startFrame->GetHandle(), true);

	const auto& type = m_input.GetType();
	if (type == InputType::keybd)
	{
		m_key->DrawKey(m_input.GetHardDataName(cmd, InputType::keybd), x - 48, y, 2.0);
	}
	else if (type == InputType::pad)
	{
		m_bt->DrawBottan(m_input.GetHardDataName(cmd, InputType::pad), x - 48, y, 2.0);
	}

	int handle = m_mgr.GetFont()->GetHandle(32);

	int strX = x;

	for (int i = 0; i < num; i++)
	{
		int add = static_cast<int>(sinf(m_waveAngle + kWaveInterval * i) * -10);

		if (add > 0)
		{
			add = 0;
		}

		int strY = y + add;


		DrawStringToHandle(strX, strY, str[i], kWhiteColor, handle);
		strX += 24;
	}
}

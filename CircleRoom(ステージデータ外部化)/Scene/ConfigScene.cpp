#include <DxLib.h>
#include <cassert>
#include "Application.h"
#include "Input.h"

#include "GameManager.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/BottansFile.h"
#include "FileSystem/KeyFile.h"

#include "SceneManager.h"
#include "KeyConfigScene.h"
#include "PadConfigScene.h"

#include "ConfigScene.h"

namespace
{
	// フレームの色
	constexpr unsigned int kFrameColor = 0xd2001a;
	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	// 選択時文字列の色
	constexpr unsigned int kYellowColor = 0xffde00;
	// 点滅間隔
	constexpr int kFlashInterval = 40;

	constexpr int kMenuMargin = 120;

	// フレームの左余白
	constexpr int kFrameMargin = 16;

	constexpr int kMenuLineInterval = 128;

	enum
	{
		kKey,
		kPad,
		kMax
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

ConfigScene::ConfigScene(GameManager& mgr, Input& input, std::shared_ptr<SceneManager> scene) :
	Scene(mgr),
	m_input(input),
	m_optionScn(scene),
	m_currentLineIndex(0),
	m_fadeFrame(0),
	m_waveAngle(DX_PI_F),
	m_isWaveDraw(true)
{
	m_soundSys = mgr.GetSound();

	auto& file = m_mgr.GetFile();
	m_frame = file->LoadGraphic(L"UI/normalFrame.png", true);
	m_startFrame = file->LoadGraphic(L"UI/startFrame.png");

	m_selectSe = file->LoadSound(L"Se/select.mp3", true);
	m_cursorSe = file->LoadSound(L"Se/cursor.mp3", true);

	m_bt = std::make_shared<BottansFile>(file);
	m_key = std::make_shared<KeyFile>(file);

	const auto& size = Application::GetInstance().GetWindowSize();
}

ConfigScene::~ConfigScene()
{
}

void ConfigScene::Update(Input& input)
{
	m_isWaveDraw = true;
	m_waveAngle -= kWaveSpeed;

	if (input.IsTriggered("OK"))
	{
		m_isWaveDraw = false;
		m_soundSys->PlaySe(m_selectSe->GetHandle());

		if (m_currentLineIndex == kKey)
		{
			m_optionScn->ChangeScene(std::make_shared<KeyConfigScene>(m_mgr, input, m_optionScn));
		}
		else if (m_currentLineIndex == kPad)
		{
			m_optionScn->ChangeScene(std::make_shared<PadConfigScene>(m_mgr, input, m_optionScn));
		}

		return;
	}

	m_fadeFrame++;

	if (input.IsTriggered("up"))
	{
		m_currentLineIndex = (m_currentLineIndex - 1 + kMax) % kMax;
		m_fadeFrame = 0;
		m_sound->PlaySe(m_cursorSe->GetHandle());
	}
	if (input.IsTriggered("down"))
	{
		m_currentLineIndex = (m_currentLineIndex + 1) % kMax;
		m_fadeFrame = 0;
		m_sound->PlaySe(m_cursorSe->GetHandle());
	}
}

void ConfigScene::Draw() const
{
	int y = kMenuMargin + 36 + m_currentLineIndex * kMenuLineInterval;

	DrawGraph(kMenuMargin + 800, y, m_frame->GetHandle(), true);
	DrawBox(128 - kFrameMargin, y,
		kMenuMargin + 800, y + 44,
		kFrameColor, true);

	y = kMenuMargin + 42;

	DrawName(y, kKey, L"キー設定");

	y += kMenuLineInterval;

	DrawName(y, kPad, L"PAD設定");

	DrawWave(kSelectWavePosX, kSelectWavePosY, "OK", kSelectWave, kSelectWaveNum);
	DrawWave(kBackWavePosX, kBackWavePosY, "cancel", kBackWave, kBackWaveNum);
}

void ConfigScene::DrawName(int drawY, int index, std::wstring str) const
{
	int fontHandle = m_mgr.GetFont()->GetHandle(32);

	if (m_currentLineIndex == index)
	{
		int frame = (m_fadeFrame % (kFlashInterval * 2)) - kFlashInterval;
		float rate = fabsf(static_cast<float>(frame)) / kFlashInterval;
		int alpha = static_cast <int>(255 * rate);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawStringToHandle(132, drawY, str.c_str(), kYellowColor, fontHandle);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	else
	{
		DrawStringToHandle(132, drawY, str.c_str(), kWhiteColor, fontHandle);
	}
}

void ConfigScene::DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const
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

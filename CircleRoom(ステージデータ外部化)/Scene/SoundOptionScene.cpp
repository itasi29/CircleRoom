#include <DxLib.h>
#include <cassert>
#include "Application.h"
#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/BottansFile.h"
#include "FileSystem/KeyFile.h"
#include "Input.h"

#include "OptionScene.h"

#include "SoundOptionScene.h"

// ここはフレームの描画の仕方が違うから後で確認

namespace
{
	// フレームの色
	constexpr unsigned int kFrameColor = 0xd2001a;
	// フレーム点滅時の色の差
	constexpr unsigned int kFrameColorDeff = 0x225036;

	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	// 選択時文字列の色
	constexpr unsigned int kYellowColor = 0xffde00;
	// 点滅間隔
	constexpr int kFlashInterval = 40;

	// ゲージの色
	constexpr int kGaugeColor = 0x2dffe5;
	// ゲージの下地色
	constexpr int kGaugeBaseColor = 0xa9a9a9;

	// フレームの左余白
	constexpr int kFrameMargin = 16;

	constexpr int kAppeaInterval = 5;
	constexpr int kMenuMargin = 120;

	constexpr int kMenuLineInterval = 128;

	// 音声リピート間隔
	constexpr int kRepeatInterval = 6;

	// ゲージの長さ
	constexpr int kGaugeLength = 100;

	enum
	{
		kBgm,
		kSe,
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

SoundOptionScene::SoundOptionScene(GameManager& mgr, Input& input) :
	Scene(mgr),
	m_input(input),
	m_currentLineIndex(0),
	m_isEdit(false),
	m_fadeFrame(0),
	m_repeatUpFrame(0),
	m_repeatDownFrame(0),
	m_waveAngle(DX_PI_F)
{
	m_updateFunc = &SoundOptionScene::NormalUpdate;

	m_soundSys = mgr.GetSound();
	auto& file = m_mgr.GetFile();
	m_frame = file->LoadGraphic(L"UI/normalFrame.png", true);
	m_addFrame = file->LoadGraphic(L"UI/addFrame.png");
	m_startFrame = file->LoadGraphic(L"UI/startFrame.png");

	m_selectSe = file->LoadSound(L"Se/select.mp3", true);
	m_cursorSe = file->LoadSound(L"Se/cursor.mp3", true);

	m_bt = std::make_shared<BottansFile>(file);
	m_key = std::make_shared<KeyFile>(file);

	const auto& size = Application::GetInstance().GetWindowSize();
}

SoundOptionScene::~SoundOptionScene()
{
}

void SoundOptionScene::Update(Input& input)
{
	m_waveAngle -= kWaveSpeed;
	(this->*m_updateFunc)(input);
}

void SoundOptionScene::Draw() const
{	
	int y = kMenuMargin + 36 + m_currentLineIndex * kMenuLineInterval;

	// 選択している場所を描画
	DrawGraph(kMenuMargin + 800, y, m_frame->GetHandle(), true);
	DrawBox(128 - kFrameMargin, y,
		kMenuMargin + 800, y + 44,
		kFrameColor, true);

	// 選択中の場合は色を追加して点滅させる
	if (m_isEdit)
	{
		int frame = (m_fadeFrame % (kFlashInterval * 2)) - kFlashInterval;
		float rate = fabsf(static_cast<float>(frame)) / kFlashInterval;
		int add = static_cast<int>(255 * rate);
		SetDrawBlendMode(DX_BLENDMODE_ADD, add);
		DrawGraph(kMenuMargin + 800, y, m_addFrame->GetHandle(), true);
		DrawBox(128 - kFrameMargin, y,
			kMenuMargin + 800, y + 44,
			kFrameColorDeff, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	else
	{
		DrawWave(kBackWavePosX, kBackWavePosY, "cancel", kBackWave, kBackWaveNum);
	}

	int fontHandle = m_mgr.GetFont()->GetHandle(32);

	y = kMenuMargin + 42;
	auto rate = m_mgr.GetSound()->GetBgmVolRate();
	DrawName(y, kBgm, L"BGM");
	DrawFormatStringToHandle(200, y, kWhiteColor, fontHandle, L"%3d％", static_cast<int>(rate * 100));
	DrawGauge(500, y, rate);

	y += kMenuLineInterval;
	rate = m_mgr.GetSound()->GetSeVolRate();
	DrawName(y, kSe, L"SE");
	DrawFormatStringToHandle(200, y, kWhiteColor, fontHandle, L"%3d％", static_cast<int>(rate * 100));
	DrawGauge(500, y, rate);

	DrawWave(kSelectWavePosX, kSelectWavePosY, "OK", kSelectWave, kSelectWaveNum);
}

void SoundOptionScene::NormalUpdate(Input& input)
{
	if (input.IsTriggered("OK"))
	{
		m_soundSys->PlaySe(m_selectSe->GetHandle());
		m_isEdit = true;
		m_fadeFrame = kFlashInterval;
		std::shared_ptr<OptionScene > optionScene = std::dynamic_pointer_cast<OptionScene>(m_mgr.GetScene()->GetTopScene());
		optionScene->InverseIsEdit();

		m_updateFunc = &SoundOptionScene::EditUpdate;
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

void SoundOptionScene::EditUpdate(Input& input)
{
	if (input.IsTriggered("OK"))
	{
		m_soundSys->PlaySe(m_selectSe->GetHandle());
		m_isEdit = false;
		std::shared_ptr<OptionScene > optionScene = std::dynamic_pointer_cast<OptionScene>(m_mgr.GetScene()->GetTopScene());
		optionScene->InverseIsEdit();

		m_updateFunc = &SoundOptionScene::NormalUpdate;
	}


	if (input.IsReepat("right", m_repeatUpFrame, kRepeatInterval))
	{
		if (m_currentLineIndex == kBgm)
		{
			m_mgr.GetSound()->ChangeBgmVol(10);
		}
		else if (m_currentLineIndex == kSe)
		{
			m_mgr.GetSound()->ChangeSeVol(10);
		}
	}
	if (input.IsReepat("left", m_repeatDownFrame, kRepeatInterval))
	{
		if (m_currentLineIndex == kBgm)
		{
			m_mgr.GetSound()->ChangeBgmVol(-10);
		}
		else if (m_currentLineIndex == kSe)
		{
			m_mgr.GetSound()->ChangeSeVol(-10);
		}
	}

	m_fadeFrame++;
}

void SoundOptionScene::DrawName(int drawY, int index, std::wstring str) const
{
	int fontHandle = m_mgr.GetFont()->GetHandle(32);

	if (m_currentLineIndex == index)
	{
		if (!m_isEdit)
		{
			int frame = (m_fadeFrame % (kFlashInterval * 2)) - kFlashInterval;
			float rate = fabsf(static_cast<float>(frame)) / kFlashInterval;
			int alpha = static_cast <int>(255 * rate);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		}
		DrawStringToHandle(132, drawY, str.c_str(), kYellowColor, fontHandle);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	else
	{
		DrawStringToHandle(132, drawY, str.c_str(), kWhiteColor, fontHandle);
	}
}

void SoundOptionScene::DrawGauge(int drawX, int drawY, float rate) const
{
	// 下地描画
	DrawBox(drawX, drawY, drawX + kGaugeLength, drawY + 32, kGaugeBaseColor, true);
	
	// ゲージ割合描画
	DrawBox(drawX, drawY, drawX + static_cast<int>(kGaugeLength * rate), drawY + 32, kGaugeColor, true);
}

void SoundOptionScene::DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const
{
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

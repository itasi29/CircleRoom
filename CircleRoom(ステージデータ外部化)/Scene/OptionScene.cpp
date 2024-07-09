#include <DxLib.h>
#include <cassert>
#include "Application.h"
#include "Common/Input.h"

#include "OptionScene.h"

#include "GameManager.h"
#include "SceneManager.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/BottansFile.h"
#include "FileSystem/KeyFile.h"

#include "StageSelectScene.h"
#include "ConfigScene.h"
#include "SoundOptionScene.h"
#include "OtherOptionScene.h"
#include "TitleScene.h"

namespace
{
	// フレームの色
	constexpr unsigned int kFrameColor = 0xffde00;
	// 通常文字列の色
	constexpr unsigned int kStrColor = 0xf0ece5;
	// 選択時文字列の色 
	constexpr unsigned int kYellowColor = 0xffde00;
	// 強調文字列の色
	constexpr unsigned int kRedColor = 0xd2001a;

	// フェード時間
	constexpr int kAppeaInterval = 5;

	// 余白
	constexpr int kMenuMargin = 50;
	// 選択枠の縦幅
	constexpr int kMenuMarginHeight = 48;
	// 文字Y座標
	constexpr int kStrPosY = 58;

	// 左右移動のボタン画像を揺らす高さ
	constexpr int kShakeHeight = 10;
	// 揺らす速度
	constexpr float kRad = DX_PI_F / 180;

	// ゲーム中選択幅
	constexpr int kGameMargin = 270;
	// タイトル中選択幅
	constexpr int kTitleMargin = 360;

	// タイトルでの文字列
	const std::vector<std::wstring> kTitleMenu = { L"操作設定",
		L"音量設定",
		L"その他" };

	// ゲーム中の文字列
	const std::vector<std::wstring> kGameMenu = { L"ステージ選択",
		L"操作設定",
		L"音量設定",
		L"その他" };

	enum
	{
		kStageSelect,
		kOperat,
		kValume,
		kOther
	};
}

OptionScene::OptionScene(GameManager& mgr, Input& input, bool isGame) :
	Scene(mgr),
	m_isGame(isGame),
	m_isEdit{ false },
	m_currentMenuLine(0),
	m_isFadeOut(false),
	m_type(InputType::pad),
	m_radian(0.0f)
{
	m_updateFunc = &OptionScene::AppearUpdate;

	m_bt = std::make_shared<BottansFile>(m_mgr.GetFile());
	m_key = std::make_shared<KeyFile>(m_mgr.GetFile());

	auto& file = m_mgr.GetFile();
	m_cancelSe = file->LoadSound(L"Se/cancel.mp3", true);
	m_cursorSe = file->LoadSound(L"Se/cursor.mp3", true);

	m_optionScn = std::make_shared<SceneManager>(false);
	m_optionScn->Init();

	m_mgr.GetScene()->OnGaussianBlur();

	ChangeScene(input);
}

OptionScene::~OptionScene()
{
	m_mgr.GetScene()->OnNormal();
}

void OptionScene::Update(Input& input)
{
	m_sound->PlayBgm();
	m_isEdit[1] = m_isEdit[0];

	m_optionScn->Update(input);

	// 左右移動の揺らす位置更新
	m_radian += kRad;

	// 編集中は処理の変更をしない
	if (m_isEdit[1]) return;
	(this->*m_updateFunc)(input);
}

void OptionScene::Draw() const
{
	// フェードアウト時は描画しない
	if (m_isFadeOut) return;

	// MEMO:現状後で変更させる可能性があるので元のに似た形で書いている
	// MEMO:変更なければ関数挟むのやめる
	NormalDraw();
	m_optionScn->Draw();
}

void OptionScene::ChangeScene(std::shared_ptr<Scene> scene)
{
	m_optionScn->ChangeScene(scene);
}

void OptionScene::AppearUpdate(Input&)
{
	m_frame++;
	if (kAppeaInterval <= m_frame)
	{
		m_updateFunc = &OptionScene::NormalUpdate;
	}
}

void OptionScene::NormalUpdate(Input& input)
{
	m_type = input.GetType();

	if (input.IsTriggered("pause"))
	{
		m_updateFunc = &OptionScene::DisappearUpdate;
		m_isFadeOut = true;

		return;
	}

	if (input.IsTriggered("cancel"))
	{
		m_sound->PlaySe(m_cancelSe->GetHandle());
		m_updateFunc = &OptionScene::DisappearUpdate;
		m_isFadeOut = true;

		return;
	}

	if (input.IsTriggered("optionLeft"))
	{
		if (m_isGame)
		{
			m_currentMenuLine = (m_currentMenuLine - 1 + static_cast<int>(kGameMenu.size())) % static_cast<int>(kGameMenu.size());
		}
		else
		{
			m_currentMenuLine = (m_currentMenuLine - 1 + static_cast<int>(kTitleMenu.size())) % static_cast<int>(kTitleMenu.size());
		}
		m_sound->PlaySe(m_cursorSe->GetHandle());
		ChangeScene(input);
	}
	if (input.IsTriggered("optionRight"))
	{
		if (m_isGame)
		{
			m_currentMenuLine = (m_currentMenuLine + 1) % static_cast<int>(kGameMenu.size());
		}
		else
		{
			m_currentMenuLine = (m_currentMenuLine + 1) % static_cast<int>(kTitleMenu.size());
		}
		m_sound->PlaySe(m_cursorSe->GetHandle());
		ChangeScene(input);
	}
}

void OptionScene::DisappearUpdate(Input&)
{
	m_frame--;
	if (m_frame > 0) return;

	m_mgr.GetScene()->PopScene();
}

void OptionScene::NormalDraw() const
{
	Application& app = Application::GetInstance();
	const auto& size = app.GetWindowSize();
	// ちょっと暗い矩形を描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 189);
	DrawBox(kMenuMargin, kMenuMargin, size.w - kMenuMargin, size.h - kMenuMargin,
		0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	// 枠線
	DrawBox(kMenuMargin, kMenuMargin, size.w - kMenuMargin, size.h - kMenuMargin,
		0xffffff, false);
	// 左右移動のボタン後ろ塗りつぶし
	DrawBox(kMenuMargin, kMenuMargin, kMenuMargin * 2, kMenuMargin * 2 - 1, 0xffffff, true);
	DrawBox(size.w - kMenuMargin * 2, kMenuMargin, size.w - kMenuMargin, kMenuMargin * 2 - 1, 0xffffff, true);

	DrawWave(size);
	if (m_isGame)
	{
		DrawFrame(static_cast<int>(kGameMenu.size()), kGameMargin, size);
		DrawContent(kGameMenu, kGameMargin);
	}
	else
	{
		DrawFrame(static_cast<int>(kTitleMenu.size()), kTitleMargin, size);
		DrawContent(kTitleMenu, kTitleMargin);
	}
}

void OptionScene::DrawWave(const size& size) const
{
	int addY = static_cast<int>(sinf(m_radian) * kShakeHeight);

	if (m_type == InputType::keybd)
	{
		m_key->DrawKey(L"Ｑキー", kMenuMargin + 7, kMenuMargin + addY , 2.5);
		m_key->DrawKey(L"Ｅキー", size.w - kMenuMargin - 43, kMenuMargin + addY, 2.5);
	}
	else
	{
		m_bt->DrawBottan(L"ＬBottan", kMenuMargin + 7, kMenuMargin + addY, 2.5);
		m_bt->DrawBottan(L"ＲBottan", size.w - kMenuMargin - 48, kMenuMargin + addY, 2.5);
	}

}

void OptionScene::DrawFrame(int divisionNum, int width, const size& size) const
{
	int y = kMenuMargin + kMenuMarginHeight;
	// 下の線の描画
	DrawLine(kMenuMargin, y, size.w - kMenuMargin, y, 0xffffff);

	int x = kMenuMargin * 2;

	for (int i = 0; i < divisionNum + 1; i++)
	{
		DrawLine(x, kMenuMargin, x, y, 0xffffff);

		x += width;
	}
}

void OptionScene::DrawContent(std::vector<std::wstring> strs, int width) const
{
	// 選択している場所を描画
	int x = kMenuMargin * 2;

	DrawBox(x + width * m_currentMenuLine + 1, static_cast<int>(kMenuMargin + 1),
		x + width * (m_currentMenuLine + 1), static_cast<int>(kMenuMargin + kMenuMarginHeight),
		kFrameColor, true);

	int fontHandle = m_mgr.GetFont()->GetHandle(32);
	unsigned int color;

	int centerX = static_cast<int>(width / 2.0f);

	// メニューの文字列群
	for (int i = 0; i < strs.size(); i++)
	{
		color = kStrColor;
		if (m_currentMenuLine == i)
		{
			color = kRedColor;
		}

		auto& str = strs[i];
		
		// 文字を中心辺りに描画するように半分の位置に移動
		int subX = static_cast<int>(str.size()) / 2 * 32;
		DrawStringToHandle(x + width * i + centerX - subX, kStrPosY, str.c_str(), color, fontHandle);
	}
}

void OptionScene::ChangeScene(Input& input)
{
	int current = m_currentMenuLine;

	if (!m_isGame)
	{
		// タイトルだとステージ選択がないためはじめを飛ばす
		current++;
	}

	if (current == kStageSelect)
	{
		m_optionScn->ChangeScene(std::make_shared<StageSelectScene>(m_mgr, input));
	}
	else if (current == kOperat)
	{
		m_optionScn->ChangeScene(std::make_shared<ConfigScene>(m_mgr, input, m_optionScn));
	}
	else if (current == kValume)
	{
		m_optionScn->ChangeScene(std::make_shared<SoundOptionScene>(m_mgr, input));
	}
	else if (current == kOther)
	{
		m_optionScn->ChangeScene(std::make_shared<OtherOptionScene>(m_mgr, input, m_optionScn));
	}
}

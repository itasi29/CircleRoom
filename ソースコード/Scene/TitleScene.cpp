#include <DxLib.h>
#include <cassert>
#include <string>
#include <cmath>
#include "Application.h"
#include "Common/Input.h"

#include "GameManager.h"
#include "SceneManager.h"
#include "Stage/StageManager.h"
#include "FileSystem/BottansFile.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/ImageFile.h"
#include "FileSystem/BottansFile.h"
#include "FileSystem/KeyFile.h"

#include "GamePlayingScene.h"
#include "OptionScene.h"

#include "TitleScene.h"

#include "Enemy/EnemyBase.h"
#include "Enemy/EnemyMoveWall.h"
#include "Enemy/EnemyNormal.h"
#include "Enemy/EnemyLarge.h"
#include "Enemy/EnemyDash.h"
#include "Enemy/EnemyCreate.h"
#include "Enemy/EnemyDivision.h"
#include "Enemy/EnemyChild.h"
#include "Enemy/EnemySplit.h"
#include "Boss/BossBase.h"
#include "Boss/BossArmored.h"
#include "Boss/BossStrongArmored.h"

namespace
{
	// フェードフレーム
	constexpr int kFadeFrame = 60;

	// フレームの色
	constexpr unsigned int kFrameColor = 0xd2001a;
	// バックフレームの色
	constexpr unsigned int kBackFrameColor = 0x161a30;

	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	// 選択時文字列の色
	constexpr unsigned int kYellowColor = 0xffde00;

	// 基準生成タイミング
	constexpr int kCreateEnemy = 60 * 2;
	constexpr int kCreateBoss = 60 * 10;
	// タイミングの振れ幅
	constexpr int kCreateSwingWidth = static_cast<int>(60 * 1.5);

	// 動画再生までの待機フレーム
	constexpr int kPlayDemoMoveFrame = 60 * 8;

	// メニューラインの数
	constexpr int kMenuLineNum = 3;
	// ライン間隔
	constexpr int kMenuLineInterval = 160;
	// ラインの長さ
	constexpr int kMenuLength = 256;

	// フレームの左余白
	constexpr int kFrameMargin = 16;

	// 点滅間隔
	constexpr int kFlashInterval = 40;

	// 文字列のX座標
	constexpr int kStrDrawX = 640 + 128;

	// ロゴ描画位置
	constexpr int kLogoDrawX = 374;
	constexpr int kLogoDrawY = 336;
	// 上下に動かす幅
	constexpr int kLogoShitY = 25;
	// 1度ずつのアングル
	constexpr float kLogoAngle = DX_PI_F / 180.0f;

	const std::wstring kMenuStr[kMenuLineNum] = {
		L"スタート",
		L"オプション",
		L"エンド"
	};

	// スタートセレクト
	constexpr int kStartSelectNum = 2;

	const std::wstring kStartSelect[kStartSelectNum] =
	{
		L"続きから",
		L"初めから"
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

TitleScene::TitleScene(GameManager& mgr, Input& input) :
	Scene(mgr),
	m_input(input),
	m_createEnemyFrame(0),
	m_createBossFrame(0),
	m_createEnemyTiming(0),
	m_createBossTiming(0),
	m_playDemoMoveFrame(0),
	m_fadeFrame(kFadeFrame),
	m_currentLinePos(0),
	m_logoAngle(0),
	m_bgFrame(0),
	m_waveAngle(DX_PI_F),
	m_isWaveDraw(false)
{
	m_updateFunc = &TitleScene::FadeInUpdate;
	m_drawFunc = &TitleScene::FadeDraw;

	auto& file = mgr.GetFile();
	m_logo = file->LoadGraphic(L"logo.png");
	m_bg = file->LoadGraphic(L"BG/bg.png");
	m_frame = file->LoadGraphic(L"UI/normalFrame.png", true);
	m_startFrame = file->LoadGraphic(L"UI/startFrame.png");
	m_backFrame = file->LoadGraphic(L"UI/unselectedFrame.png");

	m_soundSys = mgr.GetSound();
	m_bgm = file->LoadSound(L"Bgm/title.mp3");
	m_selectSe = file->LoadSound(L"Se/select.mp3", true);
	m_cancelSe = file->LoadSound(L"Se/cancel.mp3", true);
	m_cursorSe = file->LoadSound(L"Se/cursor.mp3", true);

	m_bt = std::make_shared<BottansFile>(file);
	m_key = std::make_shared<KeyFile>(file);

	const auto& size = Application::GetInstance().GetWindowSize();
}

TitleScene::~TitleScene()
{
	OutputDebugString(L"タイトルシーンがdeleteされました\n");
}

void TitleScene::Update(Input& input)
{
	m_bgFrame--;
	m_logoAngle += kLogoAngle;

	for (const auto& enemy : m_enemy)
	{
		enemy->TitleUpdate();
	}
	m_enemy.remove_if(
		[](const auto& enemy)
		{
			return !enemy->IsExsit();
		}
	);
	for (const auto& boss : m_boss)
	{
		boss->TitleUpdate();
	}
	m_boss.remove_if(
		[](const auto& boss)
		{
			return !boss->IsExsit();
		}
	);

	m_isWaveDraw = true;
	m_waveAngle -= kWaveSpeed;

	(this->*m_updateFunc)(input);
}

void TitleScene::Draw() const
{
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	for (const auto& enemy : m_enemy)
	{
		enemy->TitleDraw();
	}
	for (const auto& boss : m_boss)
	{
		boss->TitleDraw();
	}
	SetDrawMode(DX_DRAWMODE_NEAREST);

	(this->*m_drawFunc)();
}

void TitleScene::FadeInUpdate(Input&)
{
	m_sound->PlayFadeBgm(m_bgm->GetHandle(), 1.0f - m_fadeFrame / static_cast<float>(kFadeFrame));
	m_fadeFrame--;
	if (m_fadeFrame <= 0) // 遷移条件
	{
		// 次の遷移先
		m_updateFunc = &TitleScene::NormalUpdate;
		m_drawFunc = &TitleScene::NormalDraw;

		m_fadeFrame = 0;
	}
}

void TitleScene::NormalUpdate(Input& input)
{
	m_sound->PlayBgm(m_bgm->GetHandle());
	m_fadeFrame++;

	if (input.IsTriggered("up"))
	{
		m_currentLinePos = (kMenuLineNum + m_currentLinePos - 1) % kMenuLineNum;
		m_fadeFrame = 0;
		m_sound->PlaySe(m_cursorSe->GetHandle());
	}
	if (input.IsTriggered("down"))
	{
		m_currentLinePos = (m_currentLinePos + 1) % kMenuLineNum;
		m_fadeFrame = 0;
		m_sound->PlaySe(m_cursorSe->GetHandle());
	}

	if (input.IsTriggered("pause"))
	{
		m_mgr.GetScene()->PushScene(std::make_shared<OptionScene>(m_mgr, input, false));
		return;
	}

	if (input.IsTriggered("OK"))
	{
		m_playDemoMoveFrame = 0;
		m_soundSys->PlaySe(m_selectSe->GetHandle());
		// 0番目のときはスタート処理
		if (m_currentLinePos == 0)
		{
			// チュートリアルをクリアしていたら続きからor初めから
			if (m_mgr.GetStage()->IsClearStage("練習"))
			{
				m_updateFunc = &TitleScene::StartSelectUpdate;
				m_drawFunc = &TitleScene::StartSelectDraw;
				m_fadeFrame = 0;
			}
			// してなければ強制スタート
			else
			{
				m_updateFunc = &TitleScene::FadeOutUpdate;
				m_drawFunc = &TitleScene::FadeDraw;
				m_fadeFrame = 0;

				// 画面外に飛ばす
				m_currentLinePos = -10;
			}
			return;
		}
		// 1番目のときはオプション処理
		if (m_currentLinePos == 1)
		{
			m_isWaveDraw = false;
			m_mgr.GetScene()->PushScene(std::make_shared<OptionScene>(m_mgr, input, false));
			return;
		}
		// 2番目のときは終了処理
		if (m_currentLinePos == 2)
		{
			Application& app = Application::GetInstance();
			app.End();
			return;
		}
	}

	CreateEnemy();
	CreateBoss();
	PlayDemoMove(input);
}

void TitleScene::StartSelectUpdate(Input& input)
{
	m_sound->PlayBgm(m_bgm->GetHandle());
	m_fadeFrame++;

	if (input.IsTriggered("cancel"))
	{
		m_soundSys->PlaySe(m_cancelSe->GetHandle());

		m_updateFunc = &TitleScene::NormalUpdate;
		m_drawFunc = &TitleScene::NormalDraw;

		m_fadeFrame = 0;
		m_currentLinePos = 0;
	}

	if (input.IsTriggered("up"))
	{
		// 現在のラインの位置をメニューのラインの数で繰り返す
		m_currentLinePos = (kStartSelectNum + m_currentLinePos - 1) % kStartSelectNum;
		m_fadeFrame = 0;
		m_sound->PlaySe(m_cursorSe->GetHandle());
	}
	if (input.IsTriggered("down"))
	{
		m_currentLinePos = (m_currentLinePos + 1) % kStartSelectNum;
		m_fadeFrame = 0;
		m_sound->PlaySe(m_cursorSe->GetHandle());
	}

	if (input.IsTriggered("OK"))
	{
		m_soundSys->PlaySe(m_selectSe->GetHandle());

		// 続きから
		if (m_currentLinePos == 0)
		{
			// MEMO:特にすることないと思う
		}
		// 初めから
		if (m_currentLinePos == 1)
		{
			// データの削除
			m_mgr.GetStage()->InitData();
			m_mgr.GetStage()->InitPos();
			m_mgr.InitData();
		}

		// 画面外に飛ばす
		m_currentLinePos = -10;

		m_updateFunc = &TitleScene::FadeOutUpdate;
		m_drawFunc = &TitleScene::FadeDraw;
		m_fadeFrame = 0;

		return;
	}

	CreateEnemy();
	CreateBoss();
	PlayDemoMove(input);
}

void TitleScene::FadeOutUpdate(Input& input)
{
	m_sound->PlayFadeBgm(m_bgm->GetHandle(), 1.0f - m_fadeFrame / static_cast<float>(kFadeFrame));
	m_fadeFrame++;
	if (m_fadeFrame > kFadeFrame)
	{
		m_sound->Stop();
		m_mgr.GetScene()->ChangeScene(std::make_shared<GamePlayingScene>(m_mgr, input), 0.5f);
	}
}

void TitleScene::FadeDraw() const
{
	// 通常の方の描画
	NormalDraw();

	const auto& size = Application::GetInstance().GetWindowSize();
	// その後にフェード暗幕を描画
	int alpha = static_cast<int>(255 * (static_cast<float>(m_fadeFrame) / 60.0f));
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, size.w, size.h, 0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void TitleScene::NormalDraw() const
{
	DrawLogo();
	
	int fontHandle = m_mgr.GetFont()->GetHandle(32);

	int y = 184;
	int frameY = 178;

	for (int i = 0; i < kMenuLineNum; i++)
	{
		if (m_currentLinePos == i)
		{
			// 選択フレーム描画
			DrawGraph(kStrDrawX + kMenuLength, frameY, m_frame->GetHandle(), true);
			DrawBox(kStrDrawX - kFrameMargin, frameY, kStrDrawX + kMenuLength, frameY + 44, kFrameColor, true);

			int frame = (m_fadeFrame % (kFlashInterval * 2)) - kFlashInterval;
			float rate = fabsf(static_cast<float>(frame)) / static_cast<float>(kFlashInterval);
			int alpha = static_cast <int>(255 * rate);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawStringToHandle(kStrDrawX, y, kMenuStr[i].c_str(), kYellowColor, fontHandle);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else
		{
			// 選択フレーム描画
			DrawGraph(kStrDrawX + kMenuLength, frameY, m_backFrame->GetHandle(), true);
			DrawBox(kStrDrawX - kFrameMargin, frameY, kStrDrawX + kMenuLength, frameY + 44, kBackFrameColor, true);

			DrawStringToHandle(kStrDrawX, y, kMenuStr[i].c_str(), kWhiteColor, fontHandle);
		}

		y += kMenuLineInterval;
		frameY += kMenuLineInterval;
	}

	DrawWave(kSelectWavePosX, kSelectWavePosY, "OK", kSelectWave, kSelectWaveNum);
}

void TitleScene::StartSelectDraw() const
{
	DrawLogo();

	int fontHandle = m_mgr.GetFont()->GetHandle(32);

	int add = 8;

	int y = 228 + add;
	int frameY = 222 + add;
	for (int i = 0; i < kStartSelectNum; i++)
	{
		if (m_currentLinePos == i)
		{
			// 選択フレーム描画
			DrawGraph(kStrDrawX + 64 + kMenuLength, frameY, m_frame->GetHandle(), true);
			DrawBox(kStrDrawX + 64 - kFrameMargin, frameY, kStrDrawX + 64 + kMenuLength, frameY + 44, kFrameColor, true);

			int frame = (m_fadeFrame % (kFlashInterval * 2)) - kFlashInterval;
			float rate = fabsf(static_cast<float>(frame)) / static_cast<float>(kFlashInterval);
			int alpha = static_cast <int>(255 * rate);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawStringToHandle(kStrDrawX + 64, y, kStartSelect[i].c_str(), kYellowColor, fontHandle);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else
		{
			// 選択フレーム描画
			DrawGraph(kStrDrawX + 64 + kMenuLength, frameY, m_backFrame->GetHandle(), true);
			DrawBox(kStrDrawX + 64 - kFrameMargin, frameY, kStrDrawX + 64 + kMenuLength, frameY + 44, kBackFrameColor, true);

			DrawStringToHandle(kStrDrawX + 64, y, kStartSelect[i].c_str(), kWhiteColor, fontHandle);
		}

		y += 56;
		frameY += 56;
	}

	y = 184;
	frameY = 178;
	for (int i = 0; i < kMenuLineNum; i++)
	{
		// 選択フレーム描画
		DrawGraph(kStrDrawX + kMenuLength, frameY, m_backFrame->GetHandle(), true);
		DrawBox(kStrDrawX - kFrameMargin, frameY, kStrDrawX + kMenuLength, frameY + 44, kBackFrameColor, true);

		DrawStringToHandle(kStrDrawX, y, kMenuStr[i].c_str(), kWhiteColor, fontHandle);

		y += kMenuLineInterval;
		frameY += kMenuLineInterval;
	}

	DrawWave(kSelectWavePosX, kSelectWavePosY, "OK", kSelectWave, kSelectWaveNum);
	DrawWave(kBackWavePosX, kBackWavePosY, "cancel", kBackWave, kBackWaveNum);
}

void TitleScene::CreateEnemy()
{
	m_createEnemyFrame++;

	if (m_createEnemyFrame > m_createEnemyTiming)
	{
		m_createEnemyFrame = 0;
		// 生成タイミングの変更
		m_createEnemyTiming = kCreateEnemy + GetRand(kCreateSwingWidth);

		// 敵の数で乱数生成
		// MEMO:出したい敵の数-1の値にする
		int rand = GetRand(7);
		// 生成したのを保存しておく用
		std::shared_ptr<EnemyBase> enemy;

		const auto& size = Application::GetInstance().GetWindowSize();

		if (rand == 0)
		{
			enemy = std::make_shared<EnemyNormal>(size, 0.0f);
		}
		else if (rand == 1)
		{
			enemy = std::make_shared<EnemyLarge>(size, 0.0f);
		}
		else if (rand == 2)
		{
			enemy = std::make_shared<EnemyDash>(size, 0.0f);
		}
		else if (rand == 3)
		{
			enemy = std::make_shared<EnemyCreate>(size, 0.0f);
		}
		else if (rand == 4)
		{
			enemy = std::make_shared<EnemyDivision>(size, 0.0f);
		}
		else if (rand == 5)
		{
			enemy = std::make_shared<EnemyMoveWall>(size, 0.0f);
		}
		else if (rand == 6)
		{
			enemy = std::make_shared<EnemyChild>(size, 0.0f);
		}
		else
		{
			enemy = std::make_shared<EnemySplit>(size, 0.0f);
		}

		// タイトル用初期化処理を行う
		enemy->TitleInit();
		// 追加
		m_enemy.push_back(enemy);
	}
}

void TitleScene::CreateBoss()
{
	m_createBossFrame++;

	if (m_createBossFrame > m_createBossTiming)
	{
		m_createBossFrame = 0;
		// 生成タイミングの変更
		m_createBossTiming = kCreateBoss + GetRand(kCreateSwingWidth);

		// 通常のが出やすくなるように調整している
		int rand = GetRand(3);
		// 生成したのを保存しておく用
		std::shared_ptr<BossBase> boss;

		const auto& size = Application::GetInstance().GetWindowSize();

		if (rand < 3)
		{
			boss = std::make_shared<BossArmored>(size, 0.0f);
		}
		else
		{
			boss = std::make_shared<BossStrongArmored>(size, 0.0f);
		}

		// タイトル用初期化処理を行う
		boss->TitleInit();
		// 追加
		m_boss.push_back(boss);
	}
}

void TitleScene::DrawLogo() const
{
	int y = kLogoDrawY + static_cast<int>(kLogoShitY * sinf(m_logoAngle));

	DrawRotaGraph(kLogoDrawX, y, 1.0, 0.0, m_logo->GetHandle(), true);
}

void TitleScene::DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const
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

void TitleScene::PlayDemoMove(Input& input)
{
	// 何かしらのボタン・キーが押されたらデモムービーまでの時間を初期化
	if (input.IsAnyPress())
	{
		m_playDemoMoveFrame = 0;
		return;
	}

	m_playDemoMoveFrame++;
	if (m_playDemoMoveFrame > kPlayDemoMoveFrame)
	{
		m_soundSys->Stop(m_bgm->GetHandle(), true);
		// ここに入ったらボタンが何かしら押されるまでデモムービーを繰り返す
		while (true)
		{
			PlayMovie(L"Data/Movie/demoMovie.mp4", 1, DX_MOVIEPLAYTYPE_BCANCEL);

			// もしもゲーム終了されていたら終了処理に移行する
			if (ProcessMessage() == -1)
			{
				Application::GetInstance().End();
				break;
			}

			input.Update();
			if (input.IsAnyPress())
			{
				m_soundSys->PlayBgm(m_bgm->GetHandle(), false, true);
				break;
			}
		}

		m_playDemoMoveFrame = 0;
	}
}

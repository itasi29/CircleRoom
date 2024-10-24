#include <DxLib.h>
#include <cassert>
#include "Application.h"
#include "Input.h"

#include "GameManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/SoundSystem.h"
#include "StageManager.h"
#include "Scene/SceneManager.h"
#include "Scene/OneShotScene.h"

#include "Player/Player.h"
#include "Enemy/EnemyNormal.h"

#include "StageTutorial.h"
#include "Stage1_1.h"

namespace
{
	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;

	const std::string kStageName = "サークル";
	constexpr int kExsitTime = 2;

	// 生成時間
	constexpr int kCreateFrame = 300;

	// 条件達成時の描画時間("○の条件達成の文字")
	constexpr int kAchivedFrame = 120;

	// サウンドのフェードフレーム
	constexpr int kSoundFade = 30;

	// プレイヤー死亡時の画面の揺れフレーム
	constexpr int kPlayerDeathShakeFrame = 10;

	// プレイヤー強調描画フレーム
	constexpr int kEmphasisFrame = 60 * 5;
	// 透明開始フレーム
	constexpr int kAlphaEmphasisFrame = 60 * 4;
	// 色変更間隔
	constexpr int kEmlhasisInterval = 20;

	// 揺れ角度
	float kShakeAngle = DX_PI_F / 180 * 8;
}

StageTutorial::StageTutorial(GameManager& mgr, Input& input) :
	StageBase(mgr, input),
	m_index(0),
	m_explanation(Explanation::kOperation),
	m_createFrame(0),
	m_isStart(true),
	m_emphasisFrame(0)
{
	m_handle[0] = m_mgr.GetFile()->LoadGraphic(L"UI/operationExplanation.png");
	m_handle[1] = m_mgr.GetFile()->LoadGraphic(L"UI/explanation0.png");
	m_handle[2] = m_mgr.GetFile()->LoadGraphic(L"UI/explanation1.png");
	m_handle[3] = m_mgr.GetFile()->LoadGraphic(L"UI/explanation2.png");
	m_handle[4] = m_mgr.GetFile()->LoadGraphic(L"UI/explanation3.png");

	m_arrowImg[0] = m_mgr.GetFile()->LoadGraphic(L"UI/playerEmphasis0.png");
	m_arrowImg[1] = m_mgr.GetFile()->LoadGraphic(L"UI/playerEmphasis1.png");

	m_stageName = "練習";
	m_player = std::make_shared<Player>(m_size, m_fieldSize);

	// データの生成
	m_mgr.GetStage()->CreateData(m_stageName);
}

StageTutorial::~StageTutorial()
{
	// 終了する際にチュートリアル完了という事にする
	m_mgr.GetStage()->SaveClear(m_stageName);
}

void StageTutorial::Init()
{
	StageBase::Init();
	m_achived.clear();

	m_timeFrame = 0;
	m_createFrame = 0;
	m_extRateFrame = 0;
	m_isUpdateBestTime = false;

	m_enemy.clear();
	m_enemy.push_back(std::make_shared<EnemyNormal>(m_size, m_fieldSize));
	m_enemy.back()->Init(m_centerPos);
}

void StageTutorial::ChangeStage(Input& input)
{
	if (m_player->IsExsit()) return;

	// 死亡直後は変わらないようにする
	if (m_waitFrame < kWaitChangeFrame) return;


	if (m_mgr.GetStage()->IsClearStage(kStageName) && input.IsTriggered("up"))
	{
		std::shared_ptr<Stage1_1> nextStage = std::make_shared<Stage1_1>(m_mgr, input);

		m_mgr.GetStage()->ChangeStage(nextStage);

		return;
	}
}

void StageTutorial::UpdateSelect(Input& input)
{
	if (m_soundFrame > kSoundFade)
	{
		m_soundSys->PlayBgm(m_selectBgm->GetHandle());
	}
	else
	{
		m_soundFrame++;
		m_soundSys->PlayFadeBgm(m_selectBgm->GetHandle(), m_soundFrame / static_cast<float>(kSoundFade));
	}

	m_waitFrame++;

	for (auto& achived : m_achived)
	{
		achived.frame++;
	}
	m_achived.remove_if(
		[](const auto& achived)
		{
			return achived.frame > kAchivedFrame;
		}
	);

	m_player->Update(input, Ability::kNone);

	for (auto& enemy : m_enemy)
	{
		enemy->Update();
	}

	if (m_explanation == Explanation::kOperation)
	{
		m_mgr.GetScene()->PushScene(std::make_shared<OneShotScene>(m_mgr, m_handle[m_index]->GetHandle()));
		m_index++;
		if (m_index >= 1)
		{
			m_explanation = Explanation::kClear;
		}
		m_isWaveDraw = false;
	}
	else if (m_explanation == Explanation::kClear)
	{
		m_mgr.GetScene()->PushScene(std::make_shared<OneShotScene>(m_mgr, m_handle[m_index]->GetHandle()));
		m_index++;
		if (m_index >= 5)
		{
			m_explanation = Explanation::kPlay;
		}
		m_isWaveDraw = false;
	}
	else if (m_explanation == Explanation::kPlay)
	{
		if (input.IsTriggered("OK"))
		{
			ChangePlayingFunc();
			Init();
			m_soundFrame = 0;
			m_soundSys->Stop();
		}
	}
}

void StageTutorial::UpdatePlaying(Input& input)
{
	m_soundFrame++;

	m_player->Update(input, m_mgr.GetStage()->GetAbility());

	// プレイヤーの情報を抜き取る
	bool playerIsDash = m_player->IsDash();
	bool playerIsExsit = m_player->IsExsit();
	const Collision& playerCol = m_player->GetRect();

	CreateEnemy();
	for (const auto& enemy : m_enemy)
	{
		enemy->Update();

		// プレイヤーとの当たり判定処理
		// ダッシュしていたら処理はしない
		if (!playerIsDash && playerCol.IsCollsion(enemy->GetRect()))
		{
			// プレイヤーの死亡処理
			m_player->Death();

			m_mgr.GetScene()->ShakeScreen(kPlayerDeathShakeFrame);

			// 殺したことがある敵情報の更新
			m_mgr.GetStage()->UpdateEnemyType(enemy->GetName());

			break;
		}
	}

	// 死亡した敵は消す
	m_enemy.remove_if(
		[](const auto& enemy)
		{
			return !enemy->IsExsit();
		});

	UpdateTime();

	if (!m_player->IsExsit())
	{
		ChangeSelectFunc();

		// フレームの初期化
		m_waitFrame = 0;
		m_waveAngle = 0.0;
		m_soundFrame = 0;

		m_soundSys->Stop();

		// ベストタイムの更新
		if (m_mgr.GetStage()->UpdateBestTime(m_stageName, m_timeFrame))
		{
			m_isUpdateBestTime = true;
		}

		// クリアしているかの確認
	}
	CheckStageConditions(m_timeFrame);

	if (m_isStart)
	{
		m_emphasisFrame++;

		if (m_emphasisFrame > kEmphasisFrame)
		{
			m_isStart = false;
		}
	}

	m_extRateFrame++;
}

void StageTutorial::DrawUnique() const
{
	if (m_soundFrame > kSoundFade)
	{
		m_soundSys->PlayBgm(m_playBgm->GetHandle());
	}
	else
	{
		m_soundSys->PlayFadeBgm(m_playBgm->GetHandle(), m_soundFrame / static_cast<float>(kSoundFade));
	}

	if (!m_isStart) return;

	if (m_emphasisFrame > kAlphaEmphasisFrame)
	{
		int alpha = static_cast<int>(255 * (1.0f - (m_emphasisFrame - kAlphaEmphasisFrame) / static_cast<float>(kEmphasisFrame - kAlphaEmphasisFrame)));
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	}

	float addX = sinf(m_emphasisFrame * kShakeAngle) * 5;

	int x = static_cast<int>(m_player->GetPos().x + 64 + addX);
	int y = static_cast<int>(m_player->GetPos().y);

	int index = (m_emphasisFrame / kEmlhasisInterval) % 2;

	DrawRotaGraph(x, y, 1.0, 0.0, m_arrowImg[index]->GetHandle(), true);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void StageTutorial::CheckStageConditions(int timeFrame)
{
	CheckConditionsTime(kStageName, timeFrame, kExsitTime, L"上");
}

int StageTutorial::DrawStageConditions(int drawY) const
{
	DrawTimeConditions(drawY, m_mgr.GetFont()->GetHandle(28), kExsitTime);

	return 0;
}

void StageTutorial::DrawArrow() const
{
	DrawUpArrow(false, kStageName);
}

void StageTutorial::CreateEnemy()
{
	m_createFrame++;

	if (m_createFrame > kCreateFrame)
	{
		m_createFrame = 0;

		m_enemy.push_back(std::make_shared<EnemyNormal>(m_size, m_fieldSize));
		m_enemy.back()->Init(m_centerPos);
	}
}

void StageTutorial::UpdateTime()
{
	m_timeFrame++;
}

#include <DxLib.h>
#include "Application.h"
#include "Input.h"

#include "GameManager.h"
#include "StageManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"

#include "Scene/OneShotScene.h"
#include "Player/Player.h"
#include "Boss/BossStrongArmored.h"
#include "Boss/BossArmored.h"
#include "Stage1_8.h"
#include "Stage1_7.h"
#include "Stage1_4.h"

namespace
{
	// ラジアンでの90度
	constexpr double kRad90 = DX_PI / 2;

	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	// 強調文字列の色
	constexpr unsigned int kYellowColor = 0xffde00;

	// 条件の描画基準位置
	constexpr int kConditionStrPosX = 20;
	// 生成間隔フレーム
	constexpr int kCreateFrame = static_cast<int>(60 * 5.5);

	// クリア時間
	constexpr int kRightExsitTime = 10;
	constexpr int kDownKilledNum = 4;

	const std::string kRightStName = "分離";
	const std::string kDownStName = "切断";
}

Stage1_8::Stage1_8(GameManager& mgr, Input& input) :
	StageBase(mgr, input)
{
	m_stageName = "Reaper";
	m_player = std::make_shared<Player>(m_size, m_fieldSize);

	// 1-8はボスのためBGMを変更する
	m_playBgm = m_mgr.GetFile()->LoadSound(L"Bgm/boss.mp3");
	m_explanation = m_mgr.GetFile()->LoadGraphic(L"UI/bossExplanation.png");

	// データの生成
	m_mgr.GetStage()->CreateData(m_stageName);
	CheckStageConditions(m_mgr.GetStage()->GetBestTime(m_stageName));

	StartCheck();
}

Stage1_8::~Stage1_8()
{
}

void Stage1_8::Init()
{
	StageBase::Init();
	m_isUpdateBestTime = false;

	CreateMoveWall();

	m_boss = std::make_shared<BossArmored>(m_size, m_fieldSize, this);
	m_boss->Init(m_centerPos);

	// ボスステージに入ったことがなければ説明
	if (!m_mgr.GetStage()->IsBossIn())
	{
		// 先にBGMをならして置かないと選択画面の音楽が流れるため流しておく
		m_soundSys->PlayFadeBgm(m_playBgm->GetHandle(), 0.8f);
		m_mgr.GetScene()->PushScene(std::make_shared<OneShotScene>(m_mgr, m_explanation->GetHandle()));
		m_mgr.GetStage()->BossStageIn();
	}
}

void Stage1_8::StartCheck()
{
	m_isRightClear = m_mgr.GetStage()->IsClearStage(kRightStName);
	m_isDownClear = m_mgr.GetStage()->IsClearStage(kDownStName);
}

void Stage1_8::ChangeStage(Input& input)
{
	// プレイヤーが生存している間は変わらないようにする
	if (m_player->IsExsit()) return;

	// 死亡直後は変わらないようにする
	if (m_waitFrame < kWaitChangeFrame) return;

	auto& stage = m_mgr.GetStage();
	if (stage->IsClearStage(kRightStName) && input.IsTriggered("right"))
	{
		std::shared_ptr<Stage1_7> nextStage;
		nextStage = std::make_shared<Stage1_7>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
	if (stage->IsClearStage(kDownStName) && input.IsTriggered("down"))
	{
		std::shared_ptr<Stage1_4> nextStage;
		nextStage = std::make_shared<Stage1_4>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
}

void Stage1_8::UpTime()
{
	m_timeFrame += 15;
}

void Stage1_8::UniqueEndProcessing()
{
	const auto& armored = std::dynamic_pointer_cast<BossArmored>(m_boss);

	armored->DeleteDamageObjects();
}

void Stage1_8::CheckStageConditions(int timeFrame)
{
	CheckConditionsTime(kRightStName, timeFrame, kRightExsitTime, L"右");
	CheckConditionsKilled(kDownStName, kDownKilledNum, L"下");
}

int Stage1_8::DrawStageConditions(int drawY) const
{
	int startY = drawY;
	int fontHandle = m_mgr.GetFont()->GetHandle(28);

	if (!m_isRightClear)
	{
		DrawArrowConditions(kRightStName, drawY, kRad90);
		DrawTimeConditions(drawY, fontHandle, kRightExsitTime);

		drawY += 68;
	}
	if (!m_isDownClear)
	{
		DrawArrowConditions(kDownStName, drawY, DX_PI);
		DrawKilledConditions(drawY, fontHandle, kDownKilledNum);

		drawY += 68;
	}

	if (!m_mgr.GetStage()->IsClearBoss("BossArmored"))
	{
		DrawStringToHandle(kConditionStrPosX, drawY + 14, L"ボスを倒せ！", kYellowColor, fontHandle);

		drawY += 70;
	}

	return drawY - startY - 68;
}

void Stage1_8::DrawArrow() const
{
	DrawRightArrow(m_isRightClear, kRightStName);
	DrawDownArrow(m_isDownClear, kDownStName);
}

void Stage1_8::DrawEnemyKilledInfo(int x, int y) const
{
	DrawKilledEnemy("MoveWall", x, y, 0);
	DrawKilledEnemy("BossArmored", x, y, 40, 20);
	DrawKilledEnemy("BossStrongArmored", x, y, 88, 20);
	DrawKilledEnemy("SplitTwoBound", x, y, 132, 12);
}

void Stage1_8::CreateEnemy()
{
}

void Stage1_8::CreateStrongBoss()
{
	std::shared_ptr<BossStrongArmored> strong;
	strong = std::make_shared<BossStrongArmored>(m_size, m_fieldSize, this);
	strong->Init(m_boss->GetPos());

	m_boss = strong;
}

void Stage1_8::UpdateTime()
{
}

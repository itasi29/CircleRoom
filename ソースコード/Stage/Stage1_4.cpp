#include <DxLib.h>
#include "Application.h"
#include "Input.h"

#include "GameManager.h"
#include "StageManager.h"
#include "FileSystem/FontSystem.h"
#include "Stage1_4.h"
#include "Stage1_3.h"
#include "Stage1_6.h"
#include "Stage1_8.h"
#include "Stage1_1.h"

#include "Player/Player.h"

#include "Enemy/EnemyMoveWall.h"
#include "Enemy/EnemyNormal.h"
#include "Enemy/EnemyCreate.h"

namespace
{
	// ラジアンでの90度
	constexpr double kRad90 = DX_PI / 2;

	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	// 生成間隔
	constexpr int kCreateNormalFrame = 60 * 10;
	constexpr int kCreateDashFrame = 60 * 15;
	constexpr int kCreateLargeFrame = 60 * 10;

	// ディレイフレーム
	constexpr int kDeleyFrame = 60 * -5;


	// クリア時間
	constexpr int kLeftExsitTime = 10;
	constexpr int kRightExsitTime = 5;
	constexpr int kUpExistTime = 60;
	constexpr int kDownKilledNum = 4;

	const std::string kLeftStName = "近接遭遇";
	const std::string kRightStName = "発生";
	const std::string kUpStName = "Reaper";
	const std::string kDownStName = "サークル";

	const std::vector<std::string> kNames =
	{
		"サークル",
		"巨壁",
		"近接遭遇",
		"切断",
		"ランナー",
		"発生",
		"分離"
	};
}

Stage1_4::Stage1_4(GameManager& mgr, Input& input) :
	StageBase(mgr, input)
{
	m_stageName = "切断";
	m_player = std::make_shared<Player>(m_size, m_fieldSize);

	// データの生成
	m_mgr.GetStage()->CreateData(m_stageName);
	CheckStageConditions(m_mgr.GetStage()->GetBestTime(m_stageName));

	StartCheck();
}

Stage1_4::~Stage1_4()
{
}

void Stage1_4::Init()
{
	StageBase::Init();

	// 生成フレームの初期化
	m_createNormalFrame = 0;
	m_createDashFrame = 0;
	m_createLargeFrame = 0;

	// 壁動く敵の作成
	CreateMoveWall();

	CreateNormal(m_createNormalFrame, true);
	CreateDash(m_createDashFrame, true);
	CreateLarge(m_createLargeFrame, true);

	m_createLargeFrame = kDeleyFrame;
}

void Stage1_4::StartCheck()
{
	auto& stage = m_mgr.GetStage();
	m_isLeftClear = stage->IsClearStage(kLeftStName);
	m_isRightClear = stage->IsClearStage(kRightStName);
	m_isUpClear = stage->IsClearStage(kUpStName);
	m_isDownClear = stage->IsClearStage(kDownStName);
}

void Stage1_4::ChangeStage(Input& input)
{
	// プレイヤーが生存している間は変わらないようにする
	if (m_player->IsExsit()) return;

	// 死亡直後は変わらないようにする
	if (m_waitFrame < kWaitChangeFrame) return;

	auto& stage = m_mgr.GetStage();
	if (stage->IsClearStage(kLeftStName) && input.IsTriggered("left"))
	{
		std::shared_ptr<Stage1_3> nextStage;
		nextStage = std::make_shared<Stage1_3>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
	if (stage->IsClearStage(kRightStName) && input.IsTriggered("right"))
	{
		std::shared_ptr<Stage1_6> nextStage;
		nextStage = std::make_shared<Stage1_6>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
	if (stage->IsClearStage(kUpStName) && input.IsTriggered("up"))
	{
		std::shared_ptr<Stage1_8> nextStage;
		nextStage = std::make_shared<Stage1_8>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
	if (stage->IsClearStage(kDownStName) && input.IsTriggered("down"))
	{
		std::shared_ptr<Stage1_1> nextStage;
		nextStage = std::make_shared<Stage1_1>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
}

void Stage1_4::CheckStageConditions(int timeFrame)
{
	CheckConditionsTime(kLeftStName, timeFrame, kLeftExsitTime, L"左");
	CheckConditionsTime(kRightStName, timeFrame, kRightExsitTime, L"右");
	CheckConditionsSumTime(kUpStName, kNames, timeFrame, kUpExistTime, L"上");
	CheckConditionsKilled(kDownStName, kDownKilledNum, L"下");
}

int Stage1_4::DrawStageConditions(int drawY) const
{
	int startY = drawY;
	int fontHandle = m_mgr.GetFont()->GetHandle(28);

	if (!m_isLeftClear)
	{
		DrawArrowConditions(kLeftStName, drawY, -kRad90);
		DrawTimeConditions(drawY, fontHandle, kLeftExsitTime);

		drawY += 68;
	}
	if (!m_isRightClear)
	{
		DrawArrowConditions(kRightStName, drawY, kRad90);
		DrawTimeConditions(drawY, fontHandle, kRightExsitTime);

		drawY += 68;
	}
	if (!m_isUpClear)
	{
		DrawArrowConditions(kUpStName, drawY, 0.0);
		DrawSumTimeConditions(kNames, drawY, fontHandle, kUpExistTime);

		drawY += 102;
	}
	if (!m_isDownClear)
	{
		DrawArrowConditions(kDownStName, drawY, DX_PI);
		DrawKilledConditions(drawY, fontHandle, kDownKilledNum);

		drawY += 68;
	}

	return drawY - startY - 68;
}

void Stage1_4::DrawArrow() const
{
	DrawLeftArrow(m_isLeftClear, kLeftStName);
	DrawRightArrow(m_isRightClear, kRightStName);
	DrawUpArrow(m_isUpClear, kUpStName, true, m_mgr.GetStage()->IsClearStage(kUpStName));
	DrawDownArrow(m_isDownClear, kDownStName);
}

void Stage1_4::DrawEnemyKilledInfo(int x, int y) const
{
	DrawKilledEnemy("Normal", x, y, 0);
	DrawKilledEnemy("MoveWall", x, y, 36);
	DrawKilledEnemy("Dash", x, y, 72);
	DrawKilledEnemy("Large", x, y, 112, 20);
}

void Stage1_4::CreateEnemy()
{
	m_createNormalFrame++;
	m_createDashFrame++;
	m_createLargeFrame++;

	if (m_createNormalFrame > kCreateNormalFrame)
	{
		CreateNormal(m_createNormalFrame);
	}

	if (m_createDashFrame > kCreateDashFrame)
	{
		CreateDash(m_createDashFrame);
	}

	if (m_createLargeFrame > kCreateLargeFrame)
	{
		CreateLarge(m_createLargeFrame);
	}
}

void Stage1_4::UpdateTime()
{
	m_timeFrame++;
}

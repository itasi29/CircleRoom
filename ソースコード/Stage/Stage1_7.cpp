#include <DxLib.h>
#include "Application.h"
#include "Input.h"

#include "GameManager.h"
#include "StageManager.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/FileBase.h"

#include "Player/Player.h"
#include "Stage1_7.h"
#include "Stage1_8.h"
#include "Stage1_6.h"

namespace
{
	// ラジアンでの90度
	constexpr double kRad90 = DX_PI / 2;

	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	// 生成間隔フレーム
	constexpr int kCreateFrame = 60 * 10;

	// ディレイフレーム
	constexpr int kDeleyFrame = 60 * -4;

	// 生成数
	constexpr int kCreateNum = 3;


	// クリア時間
	constexpr int kDownExsitTime = 10;
	constexpr int kLeftExsitTime = 60;

	const std::string kLeftStName = "Reaper";
	const std::string kDownStName = "発生";

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

Stage1_7::Stage1_7(GameManager& mgr, Input& input) :
	StageBase(mgr, input)
{
	m_stageName = "分離";
	m_player = std::make_shared<Player>(m_size, m_fieldSize);

	// データの生成
	m_mgr.GetStage()->CreateData(m_stageName);
	CheckStageConditions(m_mgr.GetStage()->GetBestTime(m_stageName));

	StartCheck();
}

Stage1_7::~Stage1_7()
{
}

void Stage1_7::Init()
{
	StageBase::Init();

	// 生成フレームの初期化
	m_createFrame = 0;
	// 生成数の初期化
	m_createNum = 0;

	// 壁動く敵の作成
	CreateMoveWall();
}

void Stage1_7::StartCheck()
{
	m_isLeftClear = m_mgr.GetStage()->IsClearStage(kLeftStName);
	m_isDownClear = m_mgr.GetStage()->IsClearStage(kDownStName);
}

void Stage1_7::ChangeStage(Input& input)
{
	// プレイヤーが生存している間は変わらないようにする
	if (m_player->IsExsit()) return;

	// 死亡直後は変わらないようにする
	if (m_waitFrame < kWaitChangeFrame) return;

	auto& stage = m_mgr.GetStage();
	if (stage->IsClearStage(kLeftStName) && input.IsTriggered("left"))
	{
		std::shared_ptr<Stage1_8> nextStage;
		nextStage = std::make_shared<Stage1_8>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
	if (stage->IsClearStage(kDownStName) && input.IsTriggered("down"))
	{
		std::shared_ptr<Stage1_6> nextStage;
		nextStage = std::make_shared<Stage1_6>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
}

void Stage1_7::CheckStageConditions(int timeFrame)
{
	CheckConditionsSumTime(kLeftStName, kNames, timeFrame, kLeftExsitTime, L"左");
	CheckConditionsKilled(kDownStName, kDownExsitTime, L"下");
}

int Stage1_7::DrawStageConditions(int drawY) const
{
	int startY = drawY;
	int fontHandle = m_mgr.GetFont()->GetHandle(28);
	if (!m_isLeftClear)
	{
		DrawArrowConditions(kLeftStName, drawY, -kRad90);
		DrawSumTimeConditions(kNames, drawY, fontHandle, kLeftExsitTime);

		drawY += 102;
	}
	if (!m_isDownClear)
	{
		DrawArrowConditions(kDownStName, drawY, DX_PI);
		DrawKilledConditions(drawY, fontHandle, kDownExsitTime);

		drawY += 68;
	}

	return drawY - startY - 68;
}

void Stage1_7::DrawArrow() const
{
	DrawLeftArrow(m_isLeftClear, kLeftStName, true, m_mgr.GetStage()->IsClearStage(kLeftStName));
	DrawDownArrow(m_isDownClear, kDownStName);
}

void Stage1_7::DrawEnemyKilledInfo(int x, int y) const
{
	DrawKilledEnemy("MoveWall", x, y, 0);
	DrawKilledEnemy("Division", x, y, 42);
	DrawKilledEnemy("Split", x, y, 84, 12);
}

void Stage1_7::CreateEnemy()
{
	m_createFrame++;

	if (m_createNum < kCreateNum)
	{
		m_createNum++;
		CreateDivision(m_createFrame, true);
		m_createFrame = kDeleyFrame;
		return;
	}

	if (m_createFrame > kCreateFrame)
	{
		CreateDivision(m_createFrame);
	}
}

void Stage1_7::UpdateTime()
{
	m_timeFrame++;
}

#include <DxLib.h>
#include "Application.h"
#include "Input.h"

#include "GameManager.h"
#include "StageManager.h"
#include "FileSystem/FontSystem.h"
#include "Stage1_5.h"
#include "Stage1_3.h"

#include "Player/Player.h"

namespace
{
	// ラジアンでの90度
	constexpr double kRad90 = DX_PI / 2;

	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;

	constexpr int kCreateFrame = 60 * 3;
	constexpr int kCreateNum = 5;


	constexpr int kDownExsitTime = 10;

	const std::string kDownStName = "近接遭遇";
}

Stage1_5::Stage1_5(GameManager& mgr, Input& input) :
	StageBase(mgr, input),
	m_createFrame(0)
{
	m_stageName = "ランナー";
	m_player = std::make_shared<Player>(m_size, m_fieldSize);

	// データの生成
	m_mgr.GetStage()->CreateData(m_stageName);

	StartCheck();
}

Stage1_5::~Stage1_5()
{
}

void Stage1_5::Init()
{
	StageBase::Init();

	// 生成フレームの初期化
	m_createFrame = 0;

	m_createNum = 0;

	CreateMoveWall();
}

void Stage1_5::StartCheck()
{
	m_isDownClear = m_mgr.GetStage()->IsClearStage(kDownStName);
}

void Stage1_5::ChangeStage(Input& input)
{
	// プレイヤーが生存している間は変わらないようにする
	if (m_player->IsExsit()) return;

	// 死亡直後は変わらないようにする
	if (m_waitFrame < kWaitChangeFrame) return;

	if (m_mgr.GetStage()->IsClearStage(kDownStName) && input.IsTriggered("down"))
	{
		std::shared_ptr<Stage1_3> nextStage;
		nextStage = std::make_shared<Stage1_3>(m_mgr, input);

		m_mgr.GetStage()->ChangeStage(nextStage);

		return;
	}
}

void Stage1_5::CheckStageConditions(int timeFrame)
{
	CheckConditionsTime(kDownStName, timeFrame, kDownExsitTime, L"下");
}

int Stage1_5::DrawStageConditions(int drawY) const
{
	int startY = drawY;
	int fontHandle = m_mgr.GetFont()->GetHandle(28);

	if (!m_isDownClear)
	{
		DrawArrowConditions(kDownStName, drawY, DX_PI);
		DrawTimeConditions(drawY, fontHandle, kDownExsitTime);

		drawY += 68;
	}


	return drawY - startY - 68;
}

void Stage1_5::DrawArrow() const
{
	DrawDownArrow(m_isDownClear, kDownStName);
}

void Stage1_5::DrawEnemyKilledInfo(int x, int y) const
{
	DrawKilledEnemy("MoveWall", x, y, 0);
	DrawKilledEnemy("Large", x, y, 40, 20);
}

void Stage1_5::CreateEnemy()
{
	m_createFrame++;

	if (m_createNum < kCreateNum)
	{
		m_createNum++;
		CreateLarge(m_createFrame, true);
		return;
	}

	if (m_createFrame > kCreateFrame)
	{
		CreateLarge(m_createFrame);
	}
}

void Stage1_5::UpdateTime()
{
	m_timeFrame++;
}
#include <DxLib.h>
#include "Application.h"
#include "Input.h"

#include "GameManager.h"
#include "StageManager.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/FileBase.h"
#include "Stage1_9.h"
#include "Stage1_6.h"

#include "Player/Player.h"

namespace
{
	// ラジアンでの90度
	constexpr double kRad90 = DX_PI / 2;

	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	// 黄色文字列の色
	constexpr unsigned int kYellowColor = 0xffde00;;

	// 初めに生成する敵の数
	constexpr int kCreatDashNum = 2;
	constexpr int kCreatEneCreateNum = 4;
	constexpr int kCreateLargeNum = 3;

	// 生成間隔
	constexpr int kCreateDashInterval = static_cast<int>(60 * 6.5);
	constexpr int kCreateEneCreateInterval = 60 * 12;
	constexpr int kCreateDivisionInterval = 60 * 8;
	constexpr int kCreateLargeInterval = 60 * 4;

	// ディレイフレーム
	constexpr int kStartDeleyFrame = 40;
	constexpr int kDeleyFrame = 60 * 6;

	// 上クリア条件　生存時間
	constexpr int kUpExistTime = 5;

	// 上の部屋の名前
	const std::string kUpStName = "発生";
}

Stage1_9::Stage1_9(GameManager& mgr, Input& input) :
	StageBase(mgr, input)
{
	m_stageName = "要警戒";
	m_player = std::make_shared<Player>(m_size, m_fieldSize);

	// データの生成
	m_mgr.GetStage()->CreateData(m_stageName);
	// 1-1に関しては初めからクリアしていることとする
	//m_mgr.GetStage()->SaveClear(m_stageName);
	CheckStageConditions(m_mgr.GetStage()->GetBestTime(m_stageName));

	StartCheck();
}

Stage1_9::~Stage1_9()
{
}

void Stage1_9::Init()
{
	StageBase::Init();

	// 生成関係の初期化
	m_createDashFrame = kStartDeleyFrame;
	m_createDashNum = 0;
	m_createEneCreateNum = 0;
	m_createLargeNum = 0;
	m_createDivisionFrame = kDeleyFrame;

	// 壁動く敵の作成
	CreateMoveWall();
}

void Stage1_9::StartCheck()
{
	auto& stage = m_mgr.GetStage();
	m_isUpClear = stage->IsClearStage(kUpStName);
}

void Stage1_9::ChangeStage(Input& input)
{
	// プレイヤーが生存している間は変わらないようにする
	if (m_player->IsExsit()) return;

	// 死亡直後は変わらないようにする
	if (m_waitFrame < kWaitChangeFrame) return;

	auto& stage = m_mgr.GetStage();
	if (stage->IsClearStage(kUpStName) && input.IsTriggered("up"))
	{
		std::shared_ptr<Stage1_6> nextStage;
		nextStage = std::make_shared<Stage1_6>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
}

void Stage1_9::CheckStageConditions(int timeFrame)
{
	CheckConditionsTime(kUpStName, timeFrame, kUpExistTime, L"上");
}

int Stage1_9::DrawStageConditions(int drawY) const
{
	int startY = drawY;
	int fontHandle = m_mgr.GetFont()->GetHandle(28);
	if (!m_isUpClear)
	{
		DrawArrowConditions(kUpStName, drawY, -kRad90);
		DrawTimeConditions(drawY, fontHandle, kUpExistTime);

		drawY += 68;
	}

	return drawY - startY - 68;
}

void Stage1_9::DrawArrow() const
{
	DrawUpArrow(m_isUpClear, kUpStName);
}

void Stage1_9::DrawEnemyKilledInfo(int x, int y) const
{
}

void Stage1_9::CreateEnemy()
{
	m_createDashFrame++;
	m_createEneCreateFrame++;
	m_createLargeFrame++;
	m_createDivisionFrame++;

	if (m_createDashNum < kCreatDashNum)
	{
		if (m_createDashFrame > kStartDeleyFrame)
		{
			m_createDashNum++;
			CreateDash(m_createDashFrame);
		}
	}
	else
	{
		if (m_createDashFrame > kCreateDashInterval)
		{
			CreateDash(m_createDashFrame);
		}
	}

	if (m_createEneCreateNum < kCreatEneCreateNum)
	{
		m_createEneCreateNum++;
		CreateEneCreate(m_createEneCreateFrame);
	}
	else
	{
		if (m_createEneCreateFrame > kCreateEneCreateInterval)
		{
			CreateEneCreate(m_createEneCreateFrame);
		}
	}

	if (m_createLargeNum < kCreateLargeNum)
	{
		m_createLargeNum++;
		CreateLarge(m_createLargeFrame);
	}
	else
	{
		if (m_createLargeFrame > kCreateLargeInterval)
		{
			CreateLarge(m_createLargeFrame);
		}
	}

	if (m_createDivisionFrame > kCreateDivisionInterval)
	{
		CreateDivision(m_createDivisionFrame);
	}
}

void Stage1_9::UpdateTime()
{
	m_timeFrame++;
}

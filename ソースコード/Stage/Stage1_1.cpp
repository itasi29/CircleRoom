#include <DxLib.h>
#include "Application.h"
#include "Input.h"

#include "GameManager.h"
#include "StageManager.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/FileBase.h"
#include "Stage1_1.h"
#include "Stage1_2.h"
#include "Stage1_4.h"

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
	constexpr int kStartCreatNum = 4;
	// 初めの生成間隔フレーム
	constexpr int kStartCreateFrame = 10;

	// 敵生成間隔フレーム
	constexpr int kCreateFrame = 60 * 6;

	// 左クリア条件　生存時間
	constexpr int kLeftExsitTime = 5;
	// 上クリア条件　生存時間
	constexpr int kUpKilledNum = 4;

	// 左の部屋の名前
	const std::string kLeftStName = "巨壁";
	// 上の部屋の名前
	const std::string kUpStName = "切断";
}

Stage1_1::Stage1_1(GameManager& mgr, Input& input) :
	StageBase(mgr, input),
	m_createFrame(0)
{
	m_stageName = "サークル";
	m_player = std::make_shared<Player>(m_size, m_fieldSize);

	// データの生成
	m_mgr.GetStage()->CreateData(m_stageName);
	// 1-1に関しては初めからクリアしていることとする
	//m_mgr.GetStage()->SaveClear(m_stageName);
	CheckStageConditions(m_mgr.GetStage()->GetBestTime(m_stageName));

	StartCheck();
}

Stage1_1::~Stage1_1()
{
}

void Stage1_1::Init()
{
	StageBase::Init();

	// 生成フレームの初期化
	m_createFrame = 0;
	// 生成数の初期化
	m_createNum = 0;

	// 壁動く敵の作成
	CreateMoveWall();
}

void Stage1_1::StartCheck()
{
	auto& stage = m_mgr.GetStage();
	m_isLeftClear = stage->IsClearStage(kLeftStName);
	m_isUpClear = stage->IsClearStage(kUpStName);
}

void Stage1_1::ChangeStage(Input& input)
{
	// プレイヤーが生存している間は変わらないようにする
	if (m_player->IsExsit()) return;

	// 死亡直後は変わらないようにする
	if (m_waitFrame < kWaitChangeFrame) return;

	auto& stage = m_mgr.GetStage();
	if (stage->IsClearStage(kLeftStName) && input.IsTriggered("left"))
	{
		std::shared_ptr<Stage1_2> nextStage;
		nextStage = std::make_shared<Stage1_2>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
	if (stage->IsClearStage(kUpStName) && input.IsTriggered("up"))
	{
		std::shared_ptr<Stage1_4> nextStage;
		nextStage = std::make_shared<Stage1_4>(m_mgr, input);

		stage->ChangeStage(nextStage);

		return;
	}
}

void Stage1_1::CheckStageConditions(int timeFrame)
{
	CheckConditionsTime(kLeftStName, timeFrame, kLeftExsitTime, L"左");
	CheckConditionsKilled(kUpStName, kUpKilledNum, L"上");
}

int Stage1_1::DrawStageConditions(int drawY) const
{
	int startY = drawY;
	int fontHandle = m_mgr.GetFont()->GetHandle(28);
	if (!m_isLeftClear)
	{
		DrawArrowConditions(kLeftStName, drawY, -kRad90);
		DrawTimeConditions(drawY, fontHandle, kLeftExsitTime);

		drawY += 68;
	}
	if (!m_isUpClear)
	{
		DrawArrowConditions(kUpStName, drawY, 0.0);
		DrawKilledConditions(drawY, fontHandle, kUpKilledNum);

		drawY += 68;
	}

	return drawY - startY - 68;
}

void Stage1_1::DrawArrow() const
{
	DrawLeftArrow(m_isLeftClear, kLeftStName);
	DrawUpArrow(m_isUpClear, kUpStName);
}

void Stage1_1::DrawEnemyKilledInfo(int x, int y) const
{
	DrawKilledEnemy("Normal", x, y, 0);
	DrawKilledEnemy("MoveWall", x, y, 36);
}

void Stage1_1::CreateEnemy()
{
	// 生成時間の更新
	m_createFrame++;

	// 初めは生成間隔が早め
	if (m_createNum < kStartCreatNum && m_createFrame > kStartCreateFrame)
	{
		CreateNormal(m_createFrame, true);
		m_createNum++;

		return;
	}

	if (m_createFrame > kCreateFrame)
	{
		CreateNormal(m_createFrame);
	}
}

void Stage1_1::UpdateTime()
{
	m_timeFrame++;
}


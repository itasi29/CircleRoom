#include "EnemySplit.h"

#include "GameManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"

namespace
{
	// 動くスピード
	constexpr float kSpeed = 6.0f;
	// 半径
	constexpr float kRadius = 20.0f;
}

EnemySplit::EnemySplit(const size& windowSize, float fieldSize) :
	EnemyBase(windowSize, fieldSize)
{
	m_name = "Split";
	m_radius = kRadius;

	auto& mgr = GameManager::GetInstance().GetFile();
	m_charImg = mgr->LoadGraphic(L"Enemy/Split.png");
	m_shadow = mgr->LoadGraphic(L"Enemy/ShadowSmall.png");
}

EnemySplit::~EnemySplit()
{
}

void EnemySplit::Init(const Vec2& pos, Vec2& vec)
{
	// 引数で渡された位置を初期位置に
	m_pos = pos;
	m_radius = kRadius;

	// 移動ベクトルを調整
	m_vec = vec;
	m_vec *= kSpeed;

	// 回転初期化
	m_angle = 0.0;

	// フレームの初期化
	m_frame = 0;

	// 初めから判定を持つ
	m_col.SetCenter(m_pos, m_radius);

	ChangeNormalFunc();
}

void EnemySplit::StartUpdate()
{
}

void EnemySplit::NormalUpdate()
{
	m_pos += m_vec;
	m_angle -= kRad;
	Reflection();

	m_col.SetCenter(m_pos, m_radius);
}

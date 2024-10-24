#include <DxLib.h>
#include "Application.h"

#include "GameManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"

#include "EnemyMoveWall.h"

namespace
{
	// “®‚­ƒXƒs[ƒh
	constexpr float kSpeed = 9.0f;
	// ”¼Œa
	constexpr float kRadius = 43.0f;
}

EnemyMoveWall::EnemyMoveWall(const size& windowSize, float fieldSize) :
	EnemyBase(windowSize, fieldSize)
{
	m_name = "MoveWall";
	m_radius = kRadius;

	auto& mgr = GameManager::GetInstance().GetFile();
	m_charImg = mgr->LoadGraphic(L"Enemy/Wall.png");
}

EnemyMoveWall::~EnemyMoveWall()
{
}

void EnemyMoveWall::Init(const Vec2& vec, bool isStart)
{
	// ƒtƒŒ[ƒ€‚Ì‰Šú‰»
	m_frame = 0;

	// ‰ñ“]‰Šú‰»
	m_angle = 0.0;

	// ”¼Œa‚ÌÝ’è
	m_radius = kRadius;

	float centerX = m_size.w * 0.5f;
	float centerY = m_size.h * 0.5f;

	m_pos = Vec2{centerX + m_fieldSize * -vec.y, centerY};

	m_vec = vec * kSpeed;
}

void EnemyMoveWall::StartUpdate()
{
	m_frame++;
	m_pos += m_vec;
	Reflection(0.0f, false);

	if (m_frame > kApeearFrame)
	{
		// •Ï‚í‚é‚Æ‚«‚É“–‚½‚è”»’è‚à“ü‚ê‚é
		m_col.SetCenter(m_pos, m_radius);

		EnemyBase::ChangeNormalFunc();
	}
}

void EnemyMoveWall::NormalUpdate()
{
	m_pos += m_vec;
	m_angle -= kRad;
	Reflection(0.0f, false);

	m_col.SetCenter(m_pos, m_radius);
}

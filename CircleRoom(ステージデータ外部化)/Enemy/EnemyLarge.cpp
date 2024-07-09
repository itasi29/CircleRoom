#include <DxLib.h>
#include <cmath>
#include "GameManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/SoundSystem.h"

#include "EnemyLarge.h"

namespace
{
	// 動くスピード
	constexpr float kSpeed = 2.5f;
	// 半径
	constexpr float kRadius = 80.0f;
}

EnemyLarge::EnemyLarge(const size& windowSize, float fieldSize) :
	EnemyBase(windowSize, fieldSize)
{
	m_name = "Large";
	m_radius = kRadius;

	auto& mgr = GameManager::GetInstance().GetFile();
	m_charImg = mgr->LoadGraphic(L"Enemy/Large.png");
	m_shadow = mgr->LoadGraphic(L"Enemy/ShadowLarge.png");
}

EnemyLarge::~EnemyLarge()
{
}

void EnemyLarge::Init(const Vec2& pos, bool isStart)
{
	// 引数で渡された位置を初期位置に
	m_pos = pos;
	m_radius = kRadius;

	// 回転初期化
	m_angle = 0.0;

	// フレームの初期化
	m_frame = 0;

	// 撃つ方向をランダムで決める
	do
	{
		float moveX = (GetRand(16) - 8) * 0.125f;
		float moveY = (GetRand(16) - 8) * 0.125f;

		if (isStart)
		{
			double angle = atan2(moveX, -moveY);

			if (angle >= (DX_PI / 180 * 135)) continue;
			if (angle <= -(DX_PI / 180 * 135)) continue;
		}

		m_vec = Vec2{ moveX, moveY };
	} while (false);

	// ゼロベクトルでないなら正規化
	if (m_vec.SqLength() > 0)
	{
		m_vec.Normalize();
	}
	// ゼロベクトルなら方向を真横にする
	else
	{
		m_vec = Vec2{ 1.0f, 0.0f };
	}

	// スピードを調整
	m_vec *= kSpeed;
}

void EnemyLarge::StartUpdate()
{
	m_frame++;

	if (m_frame > kApeearFrame)
	{
		// 変わるときに当たり判定も入れる
		m_col.SetCenter(m_pos, m_radius);
		auto& sound = GameManager::GetInstance().GetSound();
		sound->PlaySe(m_createSe->GetHandle());

		EnemyBase::ChangeNormalFunc();
	}
}

void EnemyLarge::NormalUpdate()
{
	m_pos += m_vec;
	m_angle -= kRad;
	Reflection(0.8f);

	m_col.SetCenter(m_pos, m_radius);
}

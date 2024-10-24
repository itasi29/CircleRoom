#include <DxLib.h>
#include <cmath>
#include <memory>

#include "Application.h"
#include "Stage/StageBase.h"
#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/SoundSystem.h"

#include "EnemyDivision.h"
#include "EnemySplit.h"

namespace
{
	// 動くスピード
	constexpr float kSpeed = 4.0f;
	// 半径
	constexpr float kRadius = 43.0f;

	// 分裂する数
	constexpr int kDivisionNum = 4;
	// 分裂するまでのフレーム
	constexpr int kDivisionFrame = 60 * 8;

	// 分裂する前の待機フレーム
	constexpr int kDivisionWaitFrame = 60;
	// 分裂前回転フレーム
	constexpr int kRotationFrame = 40;

	// 最後の円の線の大きさとスピード
	// MEMO:線の大きさとスピードは同じほうが見栄えが良い
	constexpr int kRipple = 6;
	// 波紋の最大大きさ
	// MEMO:予想だけどこれ以上いかないと思う値
	constexpr int kMaxRippleSize = 1400;
}

EnemyDivision::EnemyDivision(const size& windowSize, float fieldSize) :
	EnemyBase(windowSize, fieldSize)
{
	m_name = "Division";
	m_radius = kRadius;

	auto& mgr = GameManager::GetInstance().GetFile();
	m_charImg = mgr->LoadGraphic(L"Enemy/Division.png");
	m_shadow = mgr->LoadGraphic(L"Enemy/ShadowDivision.png");
}

EnemyDivision::EnemyDivision(const size& windowSize, float fieldSize, StageBase* stage) :
	EnemyBase(windowSize, fieldSize),
	m_stage(stage),
	m_divisionWaitFrame(0),
	m_ripple(static_cast<int>(kRadius))
{
	m_name = "Division";
	m_radius = kRadius;

	auto& mgr = GameManager::GetInstance().GetFile();
	m_charImg = mgr->LoadGraphic(L"Enemy/Division.png");
	m_shadow = mgr->LoadGraphic(L"Enemy/ShadowDivision.png");

	m_dUpdateFunc = &EnemyDivision::UsuallyUpdate;
	m_dDrawFunc = &EnemyDivision::UsuallyDraw;

	m_rippleScreen = MakeScreen(m_size.w, m_size.h, true);
}

EnemyDivision::~EnemyDivision()
{
	DeleteGraph(m_rippleScreen);
}

void EnemyDivision::Init(const Vec2& pos, bool isStart)
{
	// 引数で渡された位置を初期位置に
	m_pos = pos;
	m_radius = kRadius;

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

	// 回転初期化
	m_angle = GetRand(359) * kRad;

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

void EnemyDivision::StartUpdate()
{
	m_frame++;

	if (m_frame > kApeearFrame)
	{
		// 変わるときに当たり判定も入れる
		m_col.SetCenter(m_pos, m_radius);
		auto& sound = GameManager::GetInstance().GetSound();
		sound->PlaySe(m_createSe->GetHandle());

		EnemyBase::ChangeNormalFunc();

		m_frame = 0;
	}
}

void EnemyDivision::NormalUpdate()
{
	(this->*m_dUpdateFunc)();
}

void EnemyDivision::NormalDraw() const
{
	(this->*m_dDrawFunc)();
}

void EnemyDivision::UsuallyUpdate()
{
	m_pos += m_vec;
	m_angle -= kRad;
	Reflection();

	m_col.SetCenter(m_pos, m_radius);

	m_frame++;

	// 一定時間超えたら分裂前状態に入る
	if (m_frame > kDivisionFrame)
	{
		m_dUpdateFunc = &EnemyDivision::DivisionUpdate;
	}
}

void EnemyDivision::DivisionUpdate()
{
	m_divisionWaitFrame++;
	// 発射前に高速回転させる
	if (m_divisionWaitFrame < kRotationFrame)
	{
		m_angle -= (kRad * 20);
	}

	// 待機時間を超えたら分裂する
	if (m_divisionWaitFrame > kDivisionWaitFrame)
	{
		// ベクトルを保存する
		Vec2 vec = { cosf(static_cast<float>(m_angle)), sinf(static_cast<float>(m_angle)) };
		for (int i = 0; i < kDivisionNum; i++)
		{
			std::shared_ptr<EnemySplit> split;
			split = std::make_shared<EnemySplit>(m_size, m_fieldSize);
			split->Init(m_pos, vec);

			m_stage->GenericEnemy(split);

			// 回転させる
			vec = vec.Right();
		}

		// 判定をステージ外に持っていく
		m_col = {};

		m_dUpdateFunc = &EnemyDivision::EndUpdate;
		m_dDrawFunc = &EnemyDivision::EndDraw;
	}
}

void EnemyDivision::EndUpdate()
{
	m_ripple += kRipple;

	if (m_ripple > kMaxRippleSize)
	{
		m_isExsit = false;
	}
}

void EnemyDivision::UsuallyDraw() const
{
	// 影の描画
	DrawRotaGraph(static_cast<int>(m_pos.x + 10), static_cast<int>(m_pos.y + 10), 1.0, m_angle,
		m_shadow->GetHandle(), true);

	DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), 1.0, m_angle,
		m_charImg->GetHandle(), true);

	DrawHitWallEffect();

#ifdef _DEBUG
	// 当たり判定の描画
	m_col.Draw(0xff0000, false);
#endif
}

void EnemyDivision::EndDraw() const
{
	// 別のスクリーンに変更
	SetDrawScreen(m_rippleScreen);
	// 少し黒くする
	SetDrawBlendMode(DX_BLENDMODE_SUB, 16);
	DrawBox(0, 0, m_size.w, m_size.h, 0x010101, true);
	// 波紋の描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 64);
	DrawCircle(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), m_ripple, 0x0b60b0, false, kRipple);
	// 本体の方に描画
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	int handle = GameManager::GetInstance().GetScene()->GetScreenHandle();
	SetDrawScreen(handle);
	DrawGraph(0, 0, m_rippleScreen, true);
}

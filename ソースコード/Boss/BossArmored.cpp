#include <DxLib.h>
#include <cmath>

#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/ImageFile.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/SoundSystem.h"

#include "BossArmored.h"

#include "Stage/StageBase.h"
#include "BossDamageObject.h"
#include "Enemy/EnemySplitTwoBound.h"

namespace
{
	// 最大HP
#ifdef _DEBUG
	constexpr int kMaxHp = 4;
#else
	constexpr int kMaxHp = 40;
#endif

	// 動くスピード
	constexpr float kSpeed = 3.0f;
	// 揺れ幅
	constexpr float kSwing = 1.5f;

	// 半径
	constexpr float kRadius = 80.0f;

	// カラー
	constexpr int kColor = 0x08ff08;

	// 1フレームごとの角度
	constexpr float kRadian = 1.0f * (DX_PI_F / 180.0f);

	// 生成するダメージオブジェクトの数
	constexpr int kDamageObjectNum = 1;

	// 敵生成数
	constexpr int kCreateNum = 4;
	// 生成間隔フレーム
	constexpr int kCreateFrame = static_cast<int>(60 * 4.5);
	// 生成前待機フレーム
	constexpr int kWaitCreateFrame = 30;
	// 生成するときの角度
	constexpr float kCreateRadian = 45.0f * (DX_PI_F / 180.0f);

	// ダメージを受けた際画面を揺らす時間
	constexpr int kShakeFrame = 5;
	// 揺らすサイズ
	constexpr int kShakeSize = 20;
}

BossArmored::BossArmored(const size& windowSize, float fieldSize) :
	BossBase(windowSize, fieldSize, kMaxHp)
{
	InitData();
}

BossArmored::BossArmored(const size& windowSize, float fieldSize, StageBase* stage) :
	BossBase(windowSize, fieldSize, kMaxHp),
	m_stage(stage)
{
	InitData();
}

BossArmored::~BossArmored()
{
}

void BossArmored::Init(const Vec2& pos, bool isStart)
{
	// 引数で渡された位置を初期位置に
	m_pos = pos;
	m_radius = kRadius;

	// hpの初期化
	m_hp = m_maxHp;
	// バーの初期化
	m_hpWidth = kHpBarWidth;
	m_hpDownWidth = kHpBarWidth;

	// ラジアンの初期化
	m_radian = 0;

	// 回転初期化
	m_angle = {};

	// フレームの初期化
	m_frame = 0;
	m_createFrame = kCreateFrame;
	m_waitCreateFrame = kWaitCreateFrame;

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
	m_conversionVec = m_vec * kSpeed;

	m_objects.clear();
	m_objects.push_back(std::make_shared<BossDamageObject>(m_pos, this));
}

bool BossArmored::OnAttack(bool isDash, const Collision& col)
{
	// ダッシュ中であればこの処理はしない
	if (isDash) return false;
	bool isHit = false;

	int objAdd = 0;

	for (const auto& obj : m_objects)
	{
		if (obj->IsPickUp())	continue;

		if (col.IsCollsion(obj->GetRect()))
		{
			obj->OnUse();

			objAdd++;

			break;
		}
	}

	// 使用済みのものを削除する
	m_objects.remove_if(
		[](const auto& obj)
		{
			return obj->IsEnd();
		});

	for (int i = 0; i < objAdd; i++)
	{
		m_objects.push_back(std::make_shared<BossDamageObject>(m_size, m_fieldSize, this));
	}

	return isHit;
}

void BossArmored::OnDamage()
{
	auto& sound = GameManager::GetInstance().GetSound();
	sound->PlaySe(m_damageSe->GetHandle());

	StartJoypadVibration(DX_INPUT_PAD1, 400, 200, 1);

	m_stage->UpTime();

	m_radian = 0;

	m_hp--;
	m_hpWidth = static_cast<int>(kHpBarWidth * (m_hp / static_cast<float>(m_maxHp)));

	// HPがゼロになったら死亡とする
	if (m_hp <= 0)
	{
		// バーの描画問題でHPを0にしておく
		m_hp = 0;
		m_isExsit = false;
		OnDeath();

		return;
	}

	GameManager& mgr = GameManager::GetInstance();
	mgr.GetScene()->ShakeScreen(kShakeFrame, kShakeSize);
	//HitStop();
	return;
}

void BossArmored::DeleteDamageObjects()
{
	// ダメージオブジェクトの削除
	m_objects.clear();
}

void BossArmored::StartUpdate()
{
	m_frame++;

	if (m_frame > kApeearFrame)
	{
		// 変わるときに当たり判定も入れる
		m_col.SetCenter(m_pos, m_radius);
		auto& sound = GameManager::GetInstance().GetSound();
		sound->PlaySe(m_createSe->GetHandle());

		BossBase::ChangeNormalFunc();
	}
}

void BossArmored::NormalUpdate()
{
	m_angle[0] += -kRad * 2;
	m_angle[1] += kRad;
	m_angle[2] += -kRad;

	m_radian += kRadian;

	if (m_radian > DX_TWO_PI_F)
	{
		m_radian = 0;
	}
	m_conversionVec.x = m_vec.x * cosf(m_radian);
	m_conversionVec.y = m_vec.y * sinf(m_radian);

	m_conversionVec = m_vec * kSpeed + m_conversionVec * kSwing;

	CreateEnemy();

	m_pos += m_conversionVec;
	Reflection();

	m_col.SetCenter(m_pos, m_radius);

	for (const auto& obj : m_objects)
	{
		obj->Update();
	}

	if (m_hpDownWidth > m_hpWidth)
	{
		m_hpDownWidth--;
	}
}

void BossArmored::NormalDraw() const
{
	DrawRotaGraph(static_cast<int>(m_pos.x + 10), static_cast<int>(m_pos.y + 10), 1.0, m_angle[0],
		m_shadow->GetHandle(), true);

	for (int i = 0; i < kGraphNum; i++)
	{
		DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), 1.0, m_angle[i],
			m_char[i]->GetHandle(), true);
	}

	// ダメージオブジェクトの描画
	for (const auto& obj : m_objects)
	{
		obj->Draw();
	}

	DrawHitWallEffect();

	DrawHpBar();

#ifdef _DEBUG
	// 当たり判定の描画
	m_col.Draw(0xff0000, false);
#endif
}

void BossArmored::CreateEnemy()
{
	// 生成前なら以下if文の処理を行う
	if (m_waitCreateFrame > 0)
	{
		m_waitCreateFrame--;

		// 速度を落とす
		m_conversionVec *= 0.1f;

		if (m_waitCreateFrame <= 0)
		{
			// 生成間隔フレームの初期化
			m_createFrame = kCreateFrame;

			// ベクトルの生成
			Vec2 vec = m_conversionVec.GetNormalized();

			// 体力が半分未満なら量倍
			if (m_hp < m_maxHp * 0.5f)
			{
				// 現在の角度を得る
				float radian = atan2f(vec.x, vec.y);

				for (int i = 0; i < kCreateNum * 2 + 1; i++)
				{
					std::shared_ptr<EnemySplitTwoBound> split;
					split = std::make_shared<EnemySplitTwoBound>(m_size, m_fieldSize);
					split->Init(m_pos, vec);

					m_stage->GenericEnemy(split);

					radian += kCreateRadian;

					vec.x = cosf(radian);
					vec.y = sinf(radian);
				}
			}
			else
			{
				for (int i = 0; i < kCreateNum; i++)
				{
					std::shared_ptr<EnemySplitTwoBound> split;
					split = std::make_shared<EnemySplitTwoBound>(m_size, m_fieldSize);
					split->Init(m_pos, vec);

					m_stage->GenericEnemy(split);

					vec = vec.Right();
				}
			}
		}

		return;
	}

	m_createFrame--;

	if (m_createFrame < 0)
	{
		// 生成前待機フレームの初期化
		m_waitCreateFrame = kWaitCreateFrame;
	}
}

void BossArmored::InitData()
{
	m_name = "BossArmored";
	m_color = kColor;
	m_radius = kRadius;

	auto& mgr = GameManager::GetInstance().GetFile();
	m_charAll = mgr->LoadGraphic(L"Enemy/BossArmored.png");
	m_char[0] = mgr->LoadGraphic(L"Enemy/BossArmoredOutside.png");
	m_char[1] = mgr->LoadGraphic(L"Enemy/BossArmoredMiddle.png");
	m_char[2] = mgr->LoadGraphic(L"Enemy/BossArmoredCenter.png");
}

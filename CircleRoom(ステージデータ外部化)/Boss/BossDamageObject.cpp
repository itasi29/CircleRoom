#include <DxLib.h>
#include "Application.h"
#include "GameManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "BossDamageObject.h"
#include "BossBase.h"

namespace
{
	// 半径
	constexpr float kRadius = 16.0f;

	// ランダム幅
	constexpr int kRand = 10;

	// フラッシュ間隔
	constexpr int kFlashInterval = 30;

	// ミサイルのスピード
	constexpr float kMissileSpeed = 8.0f;
	// 半径
	constexpr int kMissileRadius = 8;

	// ミサイル着弾エフェクトのスピード
	constexpr float kMissileEffSpeed = 4.0f;
	// フレーム
	constexpr int kMissileEffFrame = 30;
}

BossDamageObject::BossDamageObject(const size& windowSize, float fieldSize, BossBase* boss) :
	m_boss(boss),
	m_isPickUp(false),
	m_isUsed(false),
	m_flashFrame(0)
{
	// 中心地からフィールド範囲内に出現するようにランダムに設置
	m_pos.x = windowSize.w * 0.5f + GetRand(static_cast<int>(fieldSize * 2 - kRadius * 2)) - fieldSize + kRadius;
	m_pos.y = windowSize.h * 0.5f + GetRand(static_cast<int>(fieldSize * 2 - kRadius * 2)) - fieldSize + kRadius;

	Init();
}

BossDamageObject::BossDamageObject(const Vec2& col, BossBase* boss) :
	m_boss(boss),
	m_isPickUp(false),
	m_isUsed(false),
	m_flashFrame(0)
{
	// 送られた値からランダムでずらして設置
	m_pos.x = col.x + GetRand(static_cast<int>(kRand * 0.5f)) - kRand;
	m_pos.y = col.y + GetRand(static_cast<int>(kRand * 0.5f)) - kRand;

	Init();
}

BossDamageObject::~BossDamageObject()
{
}

void BossDamageObject::Update()
{
	(this->*m_updateFunc)();
}

void BossDamageObject::Draw()
{
	(this->*m_drawFunc)();
}

void BossDamageObject::OnUse()
{
	m_isPickUp = true;

	m_missiles.push_back({});
	m_missiles.back().vec = { kMissileSpeed, 0 };
	m_missiles.push_back({});
	m_missiles.back().vec = { -kMissileSpeed, 0 };
	m_missiles.push_back({});
	m_missiles.back().vec = { 0, kMissileSpeed };
	m_missiles.push_back({});
	m_missiles.back().vec = { 0, -kMissileSpeed };

	for (auto& missile : m_missiles)
	{
		missile.pos = m_pos;
		missile.col.SetCenter(missile.pos, kMissileRadius);
	}

	m_missileEffs.resize(4);

	m_updateFunc = &BossDamageObject::AimUpdae;
	m_drawFunc = &BossDamageObject::AimDraw;
}

void BossDamageObject::Init()
{
	auto& file = GameManager::GetInstance().GetFile();
	m_missileEff = file->LoadGraphic(L"Enemy/missileEff.png");
	m_objShadow = file->LoadGraphic(L"Enemy/ObjShadow.png");
	m_missileShadow = file->LoadGraphic(L"Enemy/MissileShadow.png");

	m_col.SetCenter(m_pos, kRadius);
	m_updateFunc = &BossDamageObject::FlashUpdate;
	m_drawFunc = &BossDamageObject::FlashDraw;
}

void BossDamageObject::FlashUpdate()
{
	m_flashFrame++;
}

void BossDamageObject::AimUpdae()
{
	// ミサイル本体の更新
	for (auto& missile : m_missiles)
	{
		Move(missile);

		// 当たったら
		if (missile.col.IsCollsion(m_boss->GetRect()))
		{
			missile.isHit = true;
			m_boss->OnDamage();

			// エフェクトの追加
			for (auto& effs : m_missileEffs)
			{
				if (effs.isUse) continue;

				std::vector<MissileEff> missileEff;

				missileEff.resize(20);

				float x, y;
				for (auto& eff : missileEff)
				{
					// サイズの決定
					eff.size = 0.6 + GetRand(10) * 0.1 - 0.5;

					// 場所の決定
					eff.pos = missile.pos;

					// 進む方向・速度の決定
					x = GetRand(16) * 0.125f - 1.0f;
					y = GetRand(16) * 0.125f - 1.0f;
					eff.vec = { x, y };
					eff.vec.Normalize();
					eff.vec = eff.vec * static_cast<float>(eff.size) * kMissileEffSpeed;
				}

				effs.effs = missileEff;
				effs.isUse = true;

				break;
			}
		}
	}
	// 当たったミサイルは消去
	auto rmlt = std::remove_if(m_missiles.begin(), m_missiles.end(),
		[](const auto& missile)
		{
			return missile.isHit;
		}
	);
	m_missiles.erase(rmlt, m_missiles.end());

	// エフェクトの更新
	for (auto& effs : m_missileEffs)
	{
		if (!effs.isUse) continue;

		for (auto& eff : effs.effs)
		{
			eff.pos += eff.vec;
		}
		effs.frame++;

		if (effs.frame > kMissileEffFrame)
		{
			effs.isEnd = true;
		}
	}
	// 終了したものは消去
	auto rmlt2 = std::remove_if(m_missileEffs.begin(), m_missileEffs.end(),
		[](const auto& effs)
		{
			return effs.isEnd;
		}
	);

	// 全部ヒットしたら終了
	if (m_missileEffs.empty())
	{
		m_isUsed = true;
	}
}

void BossDamageObject::FlashDraw()
{
	DrawRotaGraph(static_cast<int>(m_pos.x + 5), static_cast<int>(m_pos.y + 5), 1.0, 0.0, m_objShadow->GetHandle(), true);
	DrawCircle(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), static_cast<int>(kRadius), 0xd2001a, true);

	if ((m_flashFrame / kFlashInterval) % 2 == 0)
	{
		DrawCircle(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), static_cast<int>(kRadius * 0.5), 0xff4500, true);
		DrawCircle(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), static_cast<int>(kRadius), 0xff4500, false, 2);
	}

#ifdef _DEBUG
	m_col.Draw(0x000000, false);
#endif
}

void BossDamageObject::AimDraw()
{
	for (const auto& missile : m_missiles)
	{
		int x = static_cast<int>(missile.pos.x);
		int y = static_cast<int>(missile.pos.y);
		DrawRotaGraph(x + 5, y + 5, 1.0, 0.0, m_missileShadow->GetHandle(), true);
		DrawCircle(x, y, kMissileRadius, 0xff4500, true);
	}

	for (const auto& effs : m_missileEffs)
	{
		if (!effs.isUse) return;

		int alpha = static_cast<int>(255 * (1.0f - (effs.frame / static_cast<float>(kMissileEffFrame))));

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		for (const auto& eff : effs.effs)
		{
			DrawRotaGraph(static_cast<int>(eff.pos.x), static_cast<int>(eff.pos.y), eff.size, 0.0, m_missileEff->GetHandle(), true);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void BossDamageObject::Move(Missile& missile)
{
	// 目標までのベクトル
	Vec2 targetVec = m_boss->GetPos() - missile.pos;

	// ターゲットまでの線形補間
	missile.vec += targetVec.GetNormalized() - missile.vec.GetNormalized() / 30;

	// 速度の調整
	missile.vec = missile.vec.GetNormalized() * kMissileSpeed;

	// 位置の更新
	missile.pos += missile.vec;

	// 当たり判定の更新
	missile.col.SetCenter(missile.pos, kMissileRadius);
}

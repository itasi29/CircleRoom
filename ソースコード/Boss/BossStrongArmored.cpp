#include <DxLib.h>

#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "Stage/StageBase.h"
#include "FileSystem/ImageFile.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/SoundSystem.h"

#include "Enemy/EnemySplitTwoBound.h"

#include "BossStrongArmored.h"
#include "BossDamageObject.h"

namespace
{
	// カラー
	constexpr int kColor = 0xaaffaa;

	// 生成するダメージオブジェクトの数
	constexpr int kDamageObjectNum = 1;

	// 敵生成数
	constexpr int kCreateNum = 9;
	// 生成間隔フレーム
	constexpr int kCreateFrame = static_cast<int>(60 * 2);
	// 生成前待機フレーム
	constexpr int kWaitCreateFrame = 15;
	// 生成するときの角度
	constexpr float kCreateRadian = 45.0f * (DX_PI_F / 180.0f);

	// ダメージを受けた際のフレーム
	constexpr int kOnDamageFrame = 33;
	// ダメージを受けた際画面を揺らす時間
	constexpr int kShakeFrame = 5;
	// 揺らすサイズ
	constexpr int kShakeSize = 20;


	// ダメージエフェクトの画像サイズ
	constexpr int kDamageGraphSize = 32;
	// ダメージエフェクトサイズ
	constexpr double kDamageSize = 2.0;
	// ダメージエフェクトの画像の縦切り取り位置
	constexpr int kSrcY = 8 * 64;
}

BossStrongArmored::BossStrongArmored(const size& windowSize, float fieldSize) :
	BossArmored(windowSize, fieldSize)
{
	InitData();
}

BossStrongArmored::BossStrongArmored(const size& windowSize, float fieldSize, StageBase* stage) :
	BossArmored(windowSize, fieldSize, stage)
{
	InitData();
}

BossStrongArmored::~BossStrongArmored()
{
}

bool BossStrongArmored::OnAttack(bool isDash, const Collision& col)
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
			
			objAdd++;

			obj->OnUse();

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

void BossStrongArmored::OnDamage()
{
	auto& sound = GameManager::GetInstance().GetSound();
	sound->PlaySe(m_damageSe->GetHandle());

	m_stage->UpTime();

	m_radian = 0;

	GameManager& mgr = GameManager::GetInstance();
	mgr.GetScene()->ShakeScreen(kShakeFrame, kShakeSize);
//	HitStop();

	return;
}

void BossStrongArmored::InitData()
{
	m_name = "BossStrongArmored";
	m_color = kColor;

	auto& mgr = GameManager::GetInstance().GetFile();
	m_char[0] = mgr->LoadGraphic(L"Enemy/BossStrongArmoredOutside.png");
	m_char[1] = mgr->LoadGraphic(L"Enemy/BossStrongArmoredMiddle.png");
	m_char[2] = mgr->LoadGraphic(L"Enemy/BossStrongArmoredCenter.png");
}

void BossStrongArmored::NormalDraw() const
{
	// 影の描画
	DrawRotaGraph(static_cast<int>(m_pos.x + 10), static_cast<int>(m_pos.y + 10), 1.0, m_angle[0],
		m_shadow->GetHandle(), true);

	for (int i = 0; i < kGraphNum; i++)
	{
		DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), 1.0, m_angle[i],
			m_char[i]->GetHandle(), true);
	}

	DrawHitWallEffect();

	// ダメージオブジェクトの描画
	for (const auto& obj : m_objects)
	{
		obj->Draw();
	}

#ifdef _DEBUG
	// 当たり判定の描画
	m_col.Draw(0xff0000, false);
#endif
}

void BossStrongArmored::CreateEnemy()
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
			// 現在の角度を得る
			float radian = atan2f(vec.x, vec.y);

			for (int i = 0; i < kCreateNum; i++)
			{
				std::shared_ptr<EnemySplitTwoBound> split;
				split = std::make_shared<EnemySplitTwoBound>(m_size, m_fieldSize);
				split->Init(m_pos, vec);

				m_stage->GenericEnemy(split);

				// 回転させる
				radian += kCreateRadian;

				vec.x = cosf(radian);
				vec.y = sinf(radian);
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

#include <DxLib.h>
#include <cmath>
#include "Application.h"

#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/SoundSystem.h"

#include "EnemyDash.h"

#include "Player/Player.h"

namespace
{
	// 動くスピード
	constexpr float kSpeed = 4.0f;
	// 半径
	constexpr float kRadius = 43.0f;

	// ログ数
	constexpr int kDashLogNum = 8;
	// ダッシュ時のスピード
	constexpr float kDashSpeed = 15.0f;
	// ダッシュ開始までのスピード
	constexpr float kStartWaitSpeed = 2.5f;

	// ダッシュ待機時間
	constexpr int kWaitDashFrame = 60 * 5;
	// ダッシュを開始するまでのフレーム
	constexpr int kStartWaitDashFrame = 30;

	// ダッシュエフェクトの間隔
	constexpr int kDasshEffInterval = 3;
	// 行数
	constexpr int kDashRow = 10;
	// ダッシュエフェクトフレーム
	constexpr int kDashEffFrame = kDasshEffInterval * kDashRow;
	// 画像サイズ
	constexpr int kDashGraphSize = 64;
	// 拡大率
	constexpr double kDashExtRate = 2.0;
	// Y座標の切り抜き位置
	constexpr int kDashSrcY = kDashGraphSize * 8;

	// 最後の円の線の大きさとスピード
	// MEMO:線の大きさとスピードは同じほうが見栄えが良い
	constexpr int kRipple = 6;
	// 波紋の最大大きさ
	// MEMO:予想だけどこれ以上いかないと思う値
	constexpr int kMaxRippleSize = 1400;

	// 縮ませる最小大きさ
	constexpr double kShrinkSize = 0.5;
	// 縮ませるフレーム
	constexpr int kShrinkFrame = static_cast<int>(kStartWaitDashFrame * 0.2f);
	// 縮ませる大きさ
	constexpr double kShirnkSub = (1.0 - kShrinkSize) / (kStartWaitDashFrame - kShrinkFrame);
}

EnemyDash::EnemyDash(const size& windowSize, float fieldSize) :
	EnemyBase(windowSize, fieldSize),
	m_player(nullptr),
	m_circleSize(1.0)
{
	m_name = "Dash";
	m_radius = kRadius;

	auto& mgr = GameManager::GetInstance().GetFile();
	m_charImg = mgr->LoadGraphic(L"Enemy/Dash.png");
}

EnemyDash::EnemyDash(const size& windowSize, float fieldSize, std::shared_ptr<Player>& player) :
	EnemyBase(windowSize, fieldSize),
	m_player(player),
	m_logFrame(kDashLogNum),
	m_isDash(false),
	m_waitDashFrame(kWaitDashFrame),
	m_startWaitDashFrame(kStartWaitDashFrame),
	m_dashEffRipper(kMaxRippleSize),
	m_isDashEff(false),
	m_isShrink(false),
	m_circleSize(1.0)
{
	m_name = "Dash";
	m_radius = kRadius;
	m_posLog.resize(kDashLogNum);
	m_dashEffScreen = MakeScreen(m_size.w, m_size.h, true);

	auto& mgr = GameManager::GetInstance().GetFile();
	m_charImg = mgr->LoadGraphic(L"Enemy/Dash.png");

	m_dashSe = mgr->LoadSound(L"Se/enemyDash.mp3");
}

EnemyDash::~EnemyDash()
{
	DeleteGraph(m_dashEffScreen);
}

void EnemyDash::Init(const Vec2& pos, bool isStart)
{
	// 引数で渡された位置を初期位置に
	m_pos = pos;
	m_radius = kRadius;

	// 回転初期化
	m_angle = 0.0;

	// フレームの初期化
	m_frame = 0;
	m_isDash = false;
	m_isShrink = false;

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

void EnemyDash::StartUpdate()
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

void EnemyDash::NormalUpdate()
{
	Dash();
	m_pos += m_vec;

	// ダッシュ後の円を描画する時
	if (m_isDashEff)
	{
		// 半径の増加
		m_dashEffRipper += kRipple;

		// 描画終了判定
		m_isDashEff = (m_dashEffRipper < kMaxRippleSize);
	}

	// 反射したかつ、ダッシュ状態なら通常状態に戻す
	if (Reflection() && m_isDash)
	{
		// ダッシュ終了
		m_isDash = false;

		m_dashEffPos = m_pos;
		m_dashEffRipper = kRipple;
		m_isDashEff = true;

		// ダッシュ状態から通常速度に戻る
		m_vec = m_vec.GetNormalized() * kSpeed;

		// 待機時間を戻す
		m_waitDashFrame = kWaitDashFrame;
		// スタート待機時間を戻す
		m_startWaitDashFrame = kStartWaitDashFrame;
	}

	m_col.SetCenter(m_pos, m_radius * static_cast<float>(m_circleSize));
}

void EnemyDash::NormalDraw() const
{
	if (m_logFrame < kDashLogNum)
	{
		for (int i = kDashLogNum - 1; i > -1; i--)
		{
			//auto alpha = (255 - (m_logFrame + i) * (255 / kDashLogNum));
			auto alpha = 255 / (i + m_logFrame + 1);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawRotaGraph(static_cast<int>(m_posLog[i].x), static_cast<int>(m_posLog[i].y), 1.0, m_angle,
				m_charImg->GetHandle(), true);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// 影の描画
	DrawRotaGraph(static_cast<int>(m_pos.x) + 10, static_cast<int>(m_pos.y) + 10, m_circleSize, m_angle,
		m_shadow->GetHandle(), true);

	DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), m_circleSize, m_angle, m_charImg->GetHandle(), true);
	

	DrawDashEff();

	DrawHitWallEffect();

	DrawDashEff();

#ifdef _DEBUG
	// 当たり判定の描画
	m_col.Draw(0xff0000, false);
#endif
}

void EnemyDash::Dash()
{
	m_logFrame++;
	// ダッシュ中
	if (m_isDash)
	{
		m_angle -= kRad * 0.25;

		for (int i = kDashLogNum - 1; i > 0; i--)
		{
			m_posLog[i] = m_posLog[i - 1];
		}
		m_posLog[0] = m_pos;
		m_logFrame = 0;

		return;
	}
	else
	{
		m_angle -= kRad;
	}

	// 待機時間待ち処理
	if (m_waitDashFrame > 0)
	{
		m_waitDashFrame--;
		

		// 待機時間が終わったら
		if (m_waitDashFrame <= 0)
		{
			m_isShrink = true;
			// 正規化
			m_vec.Normalize();
			// 速度の変更
			m_vec *= kStartWaitSpeed;
		}
		return;
	}
	
	// スタートまでのフレームを減らす
	m_startWaitDashFrame--;
	// スタート前揺らし
	if (m_isShrink)
	{
		if (m_startWaitDashFrame < kShrinkFrame)
		{
			m_isShrink = false;
			m_vec *= 0.1f;
		}
		else
		{
			m_circleSize -= kShirnkSub;
		}
	}

	// スタート待機が終わったら
	if (m_startWaitDashFrame <= 0)
	{
		auto& sound = GameManager::GetInstance().GetSound();
		sound->PlaySe(m_dashSe->GetHandle());
		m_isDash = true;
		m_circleSize = 1.0;

		// 現在の位置からプレイヤーまでのベクトルの生成
		m_vec = m_player->GetPos() - m_pos;
		// 正規化
		m_vec.Normalize();
		// スピードに変換
		m_vec *= kDashSpeed;

		// ログの更新
		for (auto& log : m_posLog)
		{
			log = m_pos;
		}
		m_logFrame = 0;
	}
}

void EnemyDash::DrawDashEff() const
{
	if (!m_isDashEff) return;

	// 別のスクリーンに変更
	SetDrawScreen(m_dashEffScreen);
	// 少し黒くする
	SetDrawBlendMode(DX_BLENDMODE_SUB, 16);
	DrawBox(0, 0, m_size.w, m_size.h, 0x010101, true);
	// 半径を描画する
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 64);
	DrawCircle(static_cast<int>(m_dashEffPos.x), static_cast<int>(m_dashEffPos.y), m_dashEffRipper, 0x0b60b0, false, kRipple);
	// 本体の方に描画する
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	int handle = GameManager::GetInstance().GetScene()->GetScreenHandle();
	SetDrawScreen(handle);
	DrawGraph(0, 0, m_dashEffScreen, true);
}

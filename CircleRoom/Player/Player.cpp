#include <DxLib.h>
#include "Application.h"
#include "Common/Input.h"
#include "Stage/StageManager.h"
#include "Stage/GameData.h"

#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/SoundSystem.h"

#include "Player.h"

namespace
{
	// 色
	//constexpr unsigned int kExsitColor = 0xFFFAE7;
	constexpr unsigned int kExsitColor = 0xF45050;
	constexpr unsigned int kDeathColor = 0xD2001A;

	// 影のずれ具合
	constexpr int kShadowShift = 5;

	// 当たり判定の半径の大きさ
	constexpr float kColRadius = 6.0f;
	// オブジェクトとの当たり判定半径
	constexpr float kObjColRadius = 12.0f;

	// プレイヤーの大きさ
	constexpr float kSize = 24.0f;
	// プレイヤーのスピード
	constexpr float kSpeed = 4.0f;
	// プレイヤーの中心から判定をどのくらい動かすか
	constexpr float kColShift = kSize * 0.32f;

	// 移動エフェクトフレーム
	constexpr int kEffFrame = 20;
	// エフェクト時のランダムサイズ
	constexpr int kEffRandSize = static_cast<int>(kSize * 0.8f);

	// ダッシュログ数
	constexpr int kDashLogNum = 8;
	// ダッシュ時のスピード
	constexpr float kDashSpeed = 12.0f;
	// ダッシュ可能時間
	constexpr int kDashFrame = 25;
	// ダッシュ待機時間
	constexpr int kDashWaitFrame = 25;

	// 線形補間を行うフレーム
	constexpr int kInterpolatedFrame = 50;

	// 死亡エフェクト1つの出ているフレーム数
	constexpr int kDeathEffectFrame = 3;
	// 死亡エフェクトのフレーム
	constexpr int kDeathFrame = 24 * kDeathEffectFrame;
	// 画像サイズ
	constexpr int kDeathGraphSize = 100;
	// 画像の行数
	constexpr int kRow = 6;
	// 画像の列数
	constexpr int kLine = 6;
}

Player::Player(const size& windowSize, float fieldSize) :
	m_size(windowSize),
	m_fieldSize(fieldSize),
	m_angle(0),
	m_logFrame(kDashLogNum),
	m_dashFrame(0),
	m_dashWaitFrame(0),
	m_isDash(false),
	m_isExsit(false),
	m_deathFrame(0),
	m_isDeathEffect(false),
	m_isInvivisible(false)
{
	Init();
	m_isExsit = false;
	m_posLog.resize(kDashLogNum);
	m_dirLog.resize(kDashLogNum);
	m_angleLog.resize(kDashLogNum);

	auto& mgr = GameManager::GetInstance().GetFile();
	m_bloodImg = mgr->LoadGraphic(L"Player/blood.png");
	m_charEffImg = mgr->LoadGraphic(L"Player/PlayerEff.png");

	m_deathSe = mgr->LoadSound(L"Se/playerDeath.mp3");
	m_dashSe = mgr->LoadSound(L"Se/enemyDash.mp3");
}

Player::~Player()
{
}

void Player::Init()
{
	// 初期化処理
	m_effs.clear();
	m_angle = 0;
	m_dashFrame = 0;
	m_dashWaitFrame = 0;
	m_isDash = false;
	m_isExsit = true;
	m_isDeathEffect = false;
	m_isInvivisible = false;

	// 位置の設定
	m_pos = Vec2{ m_size.w / 2.0f, m_size.h - m_fieldSize * 0.5f };

	// 方向の設定
	m_front = Vec2::Up();
	m_dir.front = m_front * kSize;
	m_dir.right = m_front.Right() * kSize * 0.5f;
	m_dir.left = m_front.Left() * kSize * 0.5f;

	// 当たり判定の更新
	m_col.SetCenter(m_pos, kColRadius, m_front.x * kColShift, m_front.y * kColShift);
	m_objCol.SetCenter(m_pos, kObjColRadius, m_front.x * kColShift, m_front.y * kColShift);
}

void Player::Update(Input& input, Ability ability)
{
	// 死亡エフェクト
	if (m_isDeathEffect)
	{
		m_deathFrame++;

		if (m_deathFrame > kDeathFrame)
		{
			m_isDeathEffect = false;
		}
	}

	// 死んでいれば処理をしない
	if (!m_isExsit) return;

	Move(input);
	// アビリティ処理
	if (ability == Ability::kDash)
	{
		Dash(input);
	}
	// 移動エフェクトの更新
	for (auto& eff : m_effs)
	{
		eff.frame++;

		if (eff.frame > kEffFrame)
		{
			eff.isEnd = true;
		}

		eff.pos += eff.vec;

		eff.angle += DX_PI_F / 180.0f;
	}

	m_effs.remove_if(
		[](const auto& eff) {
			return eff.isEnd;
		});
	// ログの更新
	for (int i = kDashLogNum - 1; i > 0; i--)
	{
		m_posLog[i] = m_posLog[static_cast<std::vector<Vec2, std::allocator<Vec2>>::size_type>(i) - 1];
		m_dirLog[i] = m_dirLog[static_cast<std::vector<DirectionVec, std::allocator<DirectionVec>>::size_type>(i) - 1];
		m_angleLog[i] = m_angleLog[static_cast<std::vector<double, std::allocator<double>>::size_type>(i) - 1];
	}
	m_posLog[0] = m_pos;
	m_dirLog[0] = m_dir;
	m_angleLog[0] = m_angle;
	// 位置の更新
	m_pos += m_vec;

	InRange();

	// 当たり判定の更新
	m_col.SetCenter(m_pos, kColRadius, m_front.x * kColShift, m_front.y * kColShift);
	m_objCol.SetCenter(m_pos, kObjColRadius, m_front.x * kColShift, m_front.y * kColShift);
}

void Player::Draw() const
{
	if (m_isInvivisible) return;

	if (m_isExsit)
	{
		float rate;
		int alpha;

		// 移動時のエフェクトを描画
		for (const auto& eff : m_effs)
		{
			rate = 1.0f - eff.frame / static_cast<float>(kEffFrame);
			alpha = static_cast<int>(153 * rate);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawRotaGraph(static_cast<int>(eff.pos.x), static_cast<int>(eff.pos.y), 1.0, eff.angle, m_charEffImg->GetHandle(), true);
		}
		// ダッシュした時のログを描画
		if (m_logFrame < kDashLogNum)
		{
			for (int i = 0; i < kDashLogNum; i++)
			{
				rate = 1.0f - (m_logFrame + i + 1) / static_cast<float>(m_logFrame + kDashLogNum);
				alpha = static_cast<int>(128 * rate);
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
				DrawTriangle(static_cast<int>(m_dirLog[i].front.x + m_posLog[i].x), static_cast<int>(m_dirLog[i].front.y + m_posLog[i].y),
					static_cast<int>(m_dirLog[i].left.x + m_posLog[i].x), static_cast<int>(m_dirLog[i].left.y + m_posLog[i].y),
					static_cast<int>(m_dirLog[i].right.x + m_posLog[i].x), static_cast<int>(m_dirLog[i].right.y + m_posLog[i].y),
					kExsitColor, true);
			}
		}

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 影の描画
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
		DrawTriangleAA(m_dir.front.x + m_pos.x + kShadowShift, m_dir.front.y + m_pos.y + kShadowShift,
			m_dir.left.x + m_pos.x + kShadowShift, m_dir.left.y + m_pos.y + kShadowShift,
			m_dir.right.x + m_pos.x + kShadowShift, m_dir.right.y + m_pos.y + kShadowShift,
			0x000000, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 現在のプレイヤーを描画
		DrawTriangleAA(m_dir.front.x + m_pos.x, m_dir.front.y + m_pos.y,
			m_dir.left.x + m_pos.x, m_dir.left.y + m_pos.y,
			m_dir.right.x + m_pos.x, m_dir.right.y + m_pos.y,
			kExsitColor, true);
		// 枠線の描画
		DrawTriangleAA(m_dir.front.x + m_pos.x, m_dir.front.y + m_pos.y,
			m_dir.left.x + m_pos.x, m_dir.left.y + m_pos.y,
			m_dir.right.x + m_pos.x, m_dir.right.y + m_pos.y,
			0x161a30, false, 2.0f);
	}

	// 死亡時のエフェクト
	if (m_isDeathEffect)
	{
		int x = static_cast<int>(m_pos.x - kDeathGraphSize * 0.5f);
		int y = static_cast<int>(m_pos.y - kDeathGraphSize * 0.5f);
		int index = m_deathFrame / kDeathEffectFrame;
		int srcX = kDeathGraphSize * (index % kRow);
		int srcY = kDeathGraphSize * (index / kLine);

		DrawRectGraph(x, y, srcX, srcY, kDeathGraphSize, kDeathGraphSize, m_bloodImg->GetHandle(), true);

		float rate = (1.0f - (m_deathFrame / static_cast<float>(kDeathFrame)));
		int alpha = static_cast<int>(128 * rate);

		// 影の描画
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawTriangleAA(m_dir.front.x + m_pos.x + kShadowShift, m_dir.front.y + m_pos.y + kShadowShift,
			m_dir.left.x + m_pos.x + kShadowShift, m_dir.left.y + m_pos.y + kShadowShift,
			m_dir.right.x + m_pos.x + kShadowShift, m_dir.right.y + m_pos.y + kShadowShift,
			0x000000, true);

		alpha = static_cast<int>(255 * rate);
		// 本体の描画
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawTriangleAA(m_dir.front.x + m_pos.x, m_dir.front.y + m_pos.y,
			m_dir.left.x + m_pos.x, m_dir.left.y + m_pos.y,
			m_dir.right.x + m_pos.x, m_dir.right.y + m_pos.y,
			kDeathColor, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

#ifdef _DEBUG
	// 走っていない場合当たり判定の描画
	if (!m_isDash)
	{
		m_col.Draw(0x00ff00, false);
		m_objCol.Draw(0x0000ff, false);
	}
#endif
}

void Player::Death()
{
	auto& sound = GameManager::GetInstance().GetSound();
	sound->PlaySe(m_deathSe->GetHandle());

	// 強さ 0~1000
	// 時間 ミリ秒
	// 位置
	StartJoypadVibration(DX_INPUT_PAD1, 1000, 200, 1);

	m_isExsit = false;

	m_deathFrame = 0;
	m_isDeathEffect = true;
}

void Player::Move(Input& input)
{
	// ゼロベクトルに戻す
	m_vec = Vec2::Zero();

	m_vec = input.GetStickDate();

	if (input.IsPress("up"))
	{
		m_vec.y--;
	}
	if (input.IsPress("down"))
	{
		m_vec.y++;
	}
	if (input.IsPress("left"))
	{
		m_vec.x--;
	}
	if (input.IsPress("right"))
	{
		m_vec.x++;
	}

	// 移動ベクトルを正規化
	m_vec.Normalize();

	if (m_vec.SqLength() > 0)
	{
		m_front = m_vec;

		AddMoveEff();

		m_front = m_vec;
		m_dir.front = m_front * kSize;
		m_dir.right = m_front.Right() * kSize * 0.5f;
		m_dir.left = m_front.Left() * kSize * 0.5f;
	}

	m_vec *= kSpeed;
}

void Player::Dash(Input& input)
{
	m_logFrame++;

	// ダッシュ待機時間中なら待機時間を減らして処理終了
	if (m_dashWaitFrame > 0)
	{
		m_dashWaitFrame--;
		return;
	}

	// ダッシュコマンドが押されたら
	if (input.IsTriggered("dash"))
	{
		auto& sound = GameManager::GetInstance().GetSound();
		sound->PlaySe(m_dashSe->GetHandle());

		// ダッシュするようにする
		m_isDash = true;
		// 使用時間の初期化
		m_dashFrame = kDashFrame;

		// ログを現在の位置にする
		for (int i = 0; i < kDashLogNum; i++)
		{
			m_posLog[i] = m_pos;
			m_dirLog[i] = m_dir;
			m_angleLog[i] = m_angle;
		}

		GameManager::GetInstance().UpdateDashCount();
	}

	// 現在ダッシュ中でないなら処理終了
	if (!m_isDash) return;

	// ダッシュさせる
	m_vec = m_front * kDashSpeed;
	AddMoveEff();

	// ログフレームの更新
	m_logFrame = 0;

	// 使用時間を減らす
	m_dashFrame--;
	// ダッシュを一定時間押し続けるか離したら終了
	if (m_dashFrame <= 0 || input.IsReleased("dash"))
	{
		m_isDash = false;
		// 待機時間の初期化
		m_dashWaitFrame = kDashWaitFrame;
	}
}

void Player::InRange()
{
	float centerX = m_size.w *0.5f;
	float centerY = m_size.h * 0.5f;

	// 左処理
	// 現在の位置から正面方向側の向きに中心をずらし(当たり判定の位置に持っていく)
	// 当たり判定の半径分ずらした位置が場外で位置をずらす
	if (m_pos.x + m_front.x * kColShift - kColRadius < centerX - m_fieldSize)
	{
		// 場外の端から当たり判定の位置分ずらし
		// 半径分右にずらす
		m_pos.x = centerX - m_fieldSize - m_front.x * kColShift + kColRadius;
	}
	// 右処理
	if (m_pos.x + m_front.x * kColShift + kColRadius > centerX + m_fieldSize)
	{
		m_pos.x = centerX + m_fieldSize - m_front.x * kColShift - kColRadius;
	}
	// 上処理
	if (m_pos.y + m_front.y * kColShift - kColRadius < centerY - m_fieldSize)
	{
		m_pos.y = centerY - m_fieldSize - m_front.y * kColShift + kColRadius;
	}
	// 下処理
	if (m_pos.y + m_front.y * kColShift + kColRadius> centerY + m_fieldSize)
	{
		m_pos.y = centerY + m_fieldSize - m_front.y * kColShift - kColRadius;
	}
}

void Player::AddMoveEff()
{
	float effX = -m_vec.x * 0.5f + (GetRand(30) * 0.1f - 1.5f);
	float effY = -m_vec.y * 0.5f + (GetRand(30) * 0.1f - 1.5f);

	Vec2 pos = m_pos + m_pos.Right().GetNormalized() * (GetRand(kEffRandSize) - kEffRandSize * 0.5f);

	m_effs.push_back({ {effX, effY}, pos });
}

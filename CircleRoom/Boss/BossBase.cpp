#include <DxLib.h>
#include "Application.h"
#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/ImageFile.h"
#include "FileSystem/FontSystem.h"

#include "Enemy/EnemyBase.h"
#include "BossBase.h"

namespace
{
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	constexpr unsigned int kYellowColor = 0xffde00;
	constexpr unsigned int kRedColor = 0xd2001a;
	constexpr unsigned int kBackFrameColor = 0x161a30;

	// タイトルでの動くスピード
	constexpr float kTitleSpeed = 4.0f;
	// タイトルでの大きさ
	constexpr double kTitleSize = 2.0;

	// 壁からの法線ベクトル
	const Vec2 kNorVecLeft = Vec2{ 1.0f,  0.0f };
	const Vec2 kNorVecRight = Vec2{ -1.0f,  0.0f };
	const Vec2 kNorVecUp = Vec2{ 0.0f,  1.0f };
	const Vec2 kNorVecDown = Vec2{ 0.0f, -1.0f };

	// ずらす方向
	const Vec2 kShiftSide = Vec2{ 0.0f, 0.2f };
	const Vec2 kShiftVert = Vec2{ 0.2f, 0.0f };

	// HPバーの描画位置
	constexpr int kDrawHpBarX = 960;
	constexpr int kDrawHpBarY = 512;
	// HPバーの縦幅
	constexpr int kHpBarHeight = 56;

	// 背景HPバーの前後幅
	constexpr int kBackHpBarWidth = 10;
	// 背景HPバーの前後高さ
	constexpr int kBackHpBarHeight = 10;

	// 画像サイズ
	constexpr int kGraphSize = 162;
	// 横のカット数
	constexpr int kRectRow = 4;
	// 縦のカット数
	constexpr int kRectLine = 4;
	// カット幅
	constexpr int kRectWidth = kGraphSize / kRectRow;
	constexpr int kRectHeight = kGraphSize / kRectLine;
	// カット時のスピード集
	constexpr float kRectSpeed[kRectRow] = { -1.0f, -0.5f, 0.5f, 1.0f };
	// 何かいい感じに速度調整するよう
	constexpr float kRectAngle = DX_PI_F / 180 * 4;



	// エフェクトのスピード
	constexpr float kWallEffSpeed = 4.0f;
	// エフェクト数
	constexpr int kWallEffNum = 5;
	// 壁に当たったエフェクトをするフレーム
	constexpr int kWallEffFrame = 30;

	// HitStopフレーム
	constexpr int kHitStopFrame = 1;

	/*死亡時の演出について*/
	// 演出数
	constexpr int kPerformaceNum = 4;
	// 演出の間隔
	constexpr int kPerformanceInterval = 68;
	// 震わせるフレーム
	constexpr int kShakeFrame = 60;
	// エフェクトインターバル
	constexpr int kPerEffInterval = 3;
	// エフェクト終了フレーム
	constexpr int kEndPerformanceFrame = 60;
	// 震わせる最初の大きさ
	constexpr int kShakeStartSize = 2;
	// 震わせる最大大きさ
	constexpr int kShakeMaxSize = 50;
	// 増やしていく大きさ
	constexpr int kShakeAdd = static_cast<int>((kShakeMaxSize - kShakeStartSize) / kPerformaceNum);

	// 最後の円の線の大きさとスピード
	// MEMO:線の大きさとスピードは同じほうが見栄えが良い
	constexpr int kRipple = 8;
}

BossBase::BossBase(const size& windowSize, float fieldSize, int maxHp) :
	m_angle({}),
	m_size(windowSize),
	m_fieldSize(fieldSize),
	m_maxHp(maxHp),
	m_isExsit(true),
	m_hp(maxHp),
	m_hitStopFrame(kHitStopFrame),
	m_isEndPerformance(false)
{
	m_rippleScreen = MakeScreen(m_size.w, m_size.h, true);

	m_updateFunc = &BossBase::StartUpdate;
	m_drawFunc = &BossBase::StartDraw;
	m_deathUpdateFunc = &BossBase::ExplotionUpdate;
	m_deathDrawFunc = &BossBase::ExplotionDraw;

	auto& mgr = GameManager::GetInstance().GetFile();
	m_shadow = mgr->LoadGraphic(L"Enemy/ShadowBoss.png");
	m_wallEffect = mgr->LoadGraphic(L"Enemy/wallEff.png");
	m_hpBar = mgr->LoadGraphic(L"UI/HpBar.png");
	m_hpBarBack = mgr->LoadGraphic(L"UI/HpBarBack.png");
	m_hpBarDown = mgr->LoadGraphic(L"UI/HpBarDown.png");
	m_hpBarFrame = mgr->LoadGraphic(L"UI/HpBarFrame.png");
	m_particle = mgr->LoadGraphic(L"Enemy/missileEff.png");

	m_createSe = mgr->LoadSound(L"Se/create.mp3");
	m_damageSe = mgr->LoadSound(L"Se/bossDamage.mp3");
	m_explosionSe = mgr->LoadSound(L"Se/bossExplosion.mp3");
	m_explosionLastSe = mgr->LoadSound(L"Se/bossExplosionLast.mp3");
}

BossBase::~BossBase()
{
	DeleteGraph(m_rippleScreen);
}

void BossBase::TitleInit()
{
	m_radius *= kTitleSize;
	// 出現場所の作成
	int rand = GetRand(3);
	int width = static_cast<int>(m_size.w - 1);
	int height = static_cast<int>(m_size.h - 1);
	// 上から
	if (rand == 0)
	{
		m_pos = { static_cast<float>(GetRand(width)), -m_radius };
	}
	// 下から
	else if (rand == 1)
	{
		m_pos = { static_cast<float>(GetRand(width)), m_radius };
	}
	// 左から
	else if (rand == 2)
	{
		m_pos = { -m_radius, static_cast<float>(GetRand(height)) };
	}
	// 右から
	else
	{
		m_pos = { m_radius, static_cast<float>(GetRand(height)) };
	}

	// ベクトルの作成
	m_vec = Vec2{m_size.w * 0.5f, m_size.h * 0.5f} - m_pos;
	// 速度の調整
	m_vec = m_vec.GetNormalized() * kTitleSpeed;
}

void BossBase::TitleUpdate()
{
	m_pos += m_vec;
	m_angle[0] += -kRad * 2;
	m_angle[1] += kRad;
	m_angle[2] += -kRad;
	
	// 画面外判定
	// FIXME:時間があれば関数化
	// 左に動いているとき
	if (m_vec.x < 0)
	{
		// 下に動いているとき
		if (m_vec.y > 0)
		{
			if (m_pos.x + m_radius < 0 || m_pos.y - m_radius > m_size.h)
			{
				m_isExsit = false;
				return;
			}
		}
		// 上に動いているとき
		else
		{
			if (m_pos.x + m_radius < 0 || m_pos.y + m_radius < 0)
			{
				m_isExsit = false;
				return;
			}
		}
	}
	// 右に動いているとき
	else
	{
		// 下に動いているとき
		if (m_vec.y > 0)
		{
			if (m_pos.x - m_radius > m_size.w || m_pos.y - m_radius > m_size.h)
			{
				m_isExsit = false;
				return;
			}
		}
		// 上に動いているとき
		else
		{
			if (m_pos.x - m_radius > m_size.w || m_pos.y + m_radius < 0)
			{
				m_isExsit = false;
				return;
			}
		}
	}
}

void BossBase::TitleDraw() const
{
	DrawRotaGraph(static_cast<int>(m_pos.x + 10), static_cast<int>(m_pos.y + 10), 1.0, m_angle[0],
		m_shadow->GetHandle(), true);

	for (int i = 0; i < kGraphNum; i++)
	{
		DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), 1.0, m_angle[i],
			m_char[i]->GetHandle(), true);
	}
}

void BossBase::Update()
{
	for (auto& walls : m_wallEff)
	{
		for (auto& eff : walls.effs)
		{
			eff.pos += eff.vec;
		}
		walls.frame++;

		if (walls.frame > kWallEffFrame)
		{
			walls.isUse = false;
		}
	}
	m_wallEff.remove_if(
		[](const auto& walls)
		{
			return !walls.isUse;
		}

	);

	(this->*m_updateFunc)();
}

void BossBase::Draw() const
{
	(this->*m_drawFunc)();
}

bool BossBase::OnAttack(bool isDash, const Collision& rect)
{
	if (isDash) return false;
	
	m_hp--;

	// HPがゼロになったら死亡とする
	if (m_hp <= 0)
	{
		// バーの描画問題でHPを0にしておく
		m_hp = 0;
		m_isExsit = false;
	}

	return true;
}

bool BossBase::Reflection()
{
	float centerX = m_size.w * 0.5f;
	float centerY = m_size.h * 0.5f;

	// 左
	if (m_pos.x - m_radius < centerX - m_fieldSize)
	{
		m_pos.x = centerX - m_fieldSize + m_radius;

		AddWallEff({ centerX - m_fieldSize, m_pos.y }, 4, 6, 16, 8);
		ReflectionCal(kNorVecLeft);
		ShiftReflection(kShiftSide);

		return true;
	}
	// 右
	if (m_pos.x + m_radius > centerX + m_fieldSize)
	{
		m_pos.x = centerX + m_fieldSize - m_radius;

		AddWallEff({ centerX + m_fieldSize, m_pos.y }, 4, -2, 16, 8);
		ReflectionCal(kNorVecRight);
		ShiftReflection(kShiftSide);

		return true;
	}
	// 上
	if (m_pos.y - m_radius < centerY - m_fieldSize)
	{
		m_pos.y = centerY - m_fieldSize + m_radius;

		AddWallEff({ m_pos.x, centerY - m_fieldSize }, 16, 8, 4, 6);
		ReflectionCal(kNorVecUp);
		ShiftReflection(kShiftVert);

		return true;
	}
	// 下
	if (m_pos.y + m_radius > centerY + m_fieldSize)
	{
		m_pos.y = centerY + m_fieldSize - m_radius;

		AddWallEff({ m_pos.x, centerY + m_fieldSize }, 16, 8, 4, -2);
		ReflectionCal(kNorVecDown);
		ShiftReflection(kShiftVert);

		return true;
	}

	return false;
}

void BossBase::ReflectionCal(const Vec2& norVec)
{
	// 法線ベクトルの2倍から現在のベクトルを引く
	m_vec = m_vec + norVec * norVec.Dot(-m_vec) * 2.0f;
}

void BossBase::ShiftReflection(const Vec2& shift)
{
	Vec2 temp = m_vec;

	// 進んでいる方向にshift分進ませる
	if (temp.x > 0)
	{
		temp.x += shift.x;
	}
	else
	{
		temp.x -= shift.x;
	}

	if (temp.y > 0)
	{
		temp.y += shift.y;
	}
	else
	{
		temp.y -= shift.y;
	}

	m_vec = temp.GetNormalized() * m_vec.Length();
}

void BossBase::ChangeNormalFunc()
{
	m_updateFunc = &BossBase::NormalUpdate;
	m_drawFunc = &BossBase::NormalDraw;
}

void BossBase::HitStop()
{
	m_hitStopFrame = 0;
	m_updateFunc = &BossBase::HitStopUpdate;
}

void BossBase::OnDeath()
{
	// 中心に向かうようにベクトルを調整
	m_vec = Vec2{ m_size.w * 0.5f, m_size.h * 0.5f } - m_pos;
	// ちょうど爆発(？)エフェクト終了時に中心辺りに来るように調整
	m_vec = m_vec / (kPerformaceNum * kPerformanceInterval);

	// それぞれの初期化
	m_endPerformanceFrame = 0;
	m_performanceNum = 0;
	m_isShake = false;
	m_isScatter = false;

	m_ripple1 = 64;
	m_ripple2 = 32;
	m_ripple3 = 16;

	m_updateFunc = &BossBase::DeathUpdate;
	m_drawFunc = &BossBase::DeathDraw;

	m_particles.clear();
	m_shakeSize = kShakeStartSize;
}

void BossBase::HitStopUpdate()
{
	if (m_hitStopFrame > kHitStopFrame)
	{
		m_updateFunc = &BossBase::NormalUpdate;
	}
	m_hitStopFrame++;
}

void BossBase::DeathUpdate()
{
	(this->*m_deathUpdateFunc)();

	m_angle[0] += -kRad * 0.5;
	m_angle[1] += kRad * 0.25;
	m_angle[2] += -kRad * 0.25;

	for (auto& eff : m_particles)
	{
		eff.frame++;
		eff.pos += eff.vec;

		if (eff.frame > 30)
		{
			eff.isEnd = true;
		}
	}
	m_particles.remove_if(
		[](const auto& eff)
		{
			return eff.isEnd;
		}
	);
}

void BossBase::ExplotionUpdate()
{
	m_pos += m_vec;
	m_endPerformanceFrame++;

	AddParticleEff();

	if (m_endPerformanceFrame > 10)
	{
		m_isShake = false;
	}
	m_isShake = true;

	if (m_endPerformanceFrame > kPerformanceInterval)
	{
		m_performanceNum++;
		m_endPerformanceFrame = 0;
		// 震わせる大きさ増加
		m_isShake = true;
		m_shakeSize += kShakeAdd;
		// 爆発音ならす
		auto& sound = GameManager::GetInstance().GetSound();
		sound->PlaySe(m_explosionSe->GetHandle());

		StartJoypadVibration(DX_INPUT_PAD1, 200 * m_performanceNum, 200, 1);

		// 最後の爆発エフェクト立った場合
		if (m_performanceNum >= kPerformaceNum)
		{
			m_deathUpdateFunc = &BossBase::ShakeUpdate;
			m_deathDrawFunc = &BossBase::ShakeDraw;
		}
	}
}

void BossBase::ShakeUpdate()
{
	m_endPerformanceFrame++;
	AddParticleEff();

	if (m_endPerformanceFrame > kShakeFrame)
	{
		m_endPerformanceFrame = 0;

		auto& mgr = GameManager::GetInstance();

		// 最後の爆発音鳴らす
		auto& sound = mgr.GetSound();
		sound->PlaySe(m_explosionLastSe->GetHandle());

		StartJoypadVibration(DX_INPUT_PAD1, 1000, 1000, 1);

		// シェーダーの開始
		mgr.GetScene()->OnShader();

		m_deathUpdateFunc = &BossBase::LastUpdate;
		m_deathDrawFunc = &BossBase::LastDraw;
	}
}

void BossBase::LastUpdate()
{
	m_endPerformanceFrame++;

	m_ripple1 += kRipple;
	m_ripple2 += kRipple;
	m_ripple3 += kRipple;

	if (m_endPerformanceFrame > kEndPerformanceFrame)
	{
		m_isEndPerformance = true;
	}

}

void BossBase::StartDraw() const
{
	float rate = static_cast<float>(m_frame) / static_cast<float>(kApeearFrame);
	int alpha = static_cast<int>(255 * rate);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	// 影の描画
	DrawRotaGraph(static_cast<int>(m_pos.x + 10), static_cast<int>(m_pos.y + 10), 1.0, m_angle[0],
		m_shadow->GetHandle(), true);

	for (int i = 0; i < kGraphNum; i++)
	{
		DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), 1.0, m_angle[i],
			m_char[i]->GetHandle(), true);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossBase::NormalDraw() const
{// 影の描画
	DrawRotaGraph(static_cast<int>(m_pos.x + 10), static_cast<int>(m_pos.y + 10), 1.0, m_angle[0],
		m_shadow->GetHandle(), true);

	for (int i = 0; i < kGraphNum; i++)
	{
		DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), 1.0, m_angle[i],
			m_char[i]->GetHandle(), true);
	}

	DrawHitWallEffect();

#ifdef _DEBUG
	// 当たり判定の描画
	m_col.Draw(0xff0000, false);
#endif
}

void BossBase::DeathDraw() const
{
	(this->*m_deathDrawFunc)();

	for (const auto& eff : m_particles)
	{
		int alpha = static_cast<int>(255 * (1.0f - eff.frame / 30.0f));

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawRotaGraph(static_cast<int>(eff.pos.x), static_cast<int>(eff.pos.y), eff.size, 0.0, m_particle->GetHandle(), true);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossBase::ExplotionDraw() const
{
	int x = static_cast<int>(m_pos.x);
	int y = static_cast<int>(m_pos.y);

	x += GetRand(m_shakeSize) - static_cast<int>(m_shakeSize * 0.5);
	y += GetRand(m_shakeSize) - static_cast<int>(m_shakeSize * 0.5);

	// 影の描画
	DrawRotaGraph(x + 10, y + 10, 1.0, m_angle[0],
		m_shadow->GetHandle(), true);
	for (int i = 0; i < kGraphNum; i++)
	{
		DrawRotaGraph(x, y, 1.0, m_angle[i],
			m_char[i]->GetHandle(), true);
	}
}

void BossBase::ShakeDraw() const
{
	int x = static_cast<int>(m_pos.x);
	int y = static_cast<int>(m_pos.y);

	x += GetRand(kShakeMaxSize) - static_cast<int>(kShakeMaxSize * 0.5);
	y += GetRand(kShakeMaxSize) - static_cast<int>(kShakeMaxSize * 0.5);

	// 影の描画
	DrawRotaGraph(x + 10, y + 10, 1.0, m_angle[0],
		m_shadow->GetHandle(), true);
	for (int i = 0; i < kGraphNum; i++)
	{
		DrawRotaGraph(x, y, 1.0, m_angle[i],
			m_char[i]->GetHandle(), true);
	}
}

void BossBase::LastDraw() const
{
	int x = static_cast<int>(m_pos.x - kGraphSize * 0.5f);
	int y;

	float angle = m_endPerformanceFrame * kRectAngle;

	int alpha = static_cast<int>(255 * (1.0f - m_endPerformanceFrame / static_cast<float>(kEndPerformanceFrame)));

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	for (int rectX = 0; rectX < kRectRow; rectX++)
	{
		y = static_cast<int>(m_pos.y - kGraphSize * 0.5f);

		for (int rectY = 0; rectY < kRectLine; rectY++)
		{
			int drawX = x + static_cast<int>(kRectSpeed[rectX] * (m_endPerformanceFrame + abs(sinf(angle))));
			int drawY = y + static_cast<int>(kRectSpeed[rectY] * (m_endPerformanceFrame + abs(sinf(angle))));

			DrawRectRotaGraph(drawX, drawY, rectX * kRectWidth, rectY * kRectHeight, kRectWidth, kRectHeight,
				1.0, angle * (rectX * kRectRow + rectY), m_charAll->GetHandle(), true);

			y += kRectHeight;
		}
		
		x += kRectWidth;
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 波紋の描画
	x = static_cast<int>(m_pos.x);
	y = static_cast<int>(m_pos.y);

	SetDrawScreen(m_rippleScreen);
	SetDrawBlendMode(DX_BLENDMODE_SUB, 16);
	DrawBox(0, 0, m_size.w, m_size.h, 0x010101, true);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 96);
	DrawCircle(x, y, m_ripple3, 0x789461, false, kRipple);
	DrawCircle(x, y, m_ripple2, 0x50623a, false, kRipple);
	DrawCircle(x, y, m_ripple1, 0x294b29, false, kRipple);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int screenHandle = GameManager::GetInstance().GetScene()->GetScreenHandle();
	SetDrawScreen(screenHandle);

	DrawGraph(0, 0, m_rippleScreen, true);
}

void BossBase::DrawHpBar() const
{
	// フレームよりバーの方が小さいため位置調整
	int barX = kDrawHpBarX + 2;
	int barY = kDrawHpBarY + 4;

	// 背景のHPバー
	DrawGraph(barX, barY, m_hpBarBack->GetHandle(), true);
	// 赤いHPバーの描画
	DrawExtendGraph(barX, barY, 
		barX + m_hpDownWidth, barY + kHpBarHeight, 
		m_hpBarDown->GetHandle(), true);
	// 通常のHPバー
	DrawExtendGraph(barX, barY, 
		barX + m_hpWidth, barY + kHpBarHeight,
		m_hpBar->GetHandle(), true);
	// フレームの描画
	DrawGraph(kDrawHpBarX, kDrawHpBarY, m_hpBarFrame->GetHandle(), true);

	// 文字見やすくするように背景描画
	DrawBox(kDrawHpBarX - 4, kDrawHpBarY - 52, kDrawHpBarX + 140, kDrawHpBarY - 12, kBackFrameColor, true);
	// 残量HPによって文字の色を変更
	unsigned int color = kWhiteColor;
	if (m_hp < static_cast<int>(m_maxHp * 0.2f))
	{
		color = kRedColor;
	}
	else if (m_hp < static_cast<int>(m_maxHp * 0.5f))
	{
		color = kYellowColor;
	}
	auto& font = GameManager::GetInstance().GetFont();
	DrawFormatStringToHandle(kDrawHpBarX, kDrawHpBarY -48, color, font->GetHandle(32),
		L"BOSS : %02d / %02d", m_hp, m_maxHp);
}

void BossBase::DrawHitWallEffect() const
{
	// 壁に当たったエフェクトの描画
	for (const auto& walls : m_wallEff)
	{
		int alpha = static_cast<int>(255 * (1.0f - walls.frame / static_cast<float>(kWallEffFrame)));

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		for (const auto& eff : walls.effs)
		{
			DrawRotaGraph(static_cast<int>(eff.pos.x), static_cast<int>(eff.pos.y), eff.size, 0.0, m_wallEffect->GetHandle(), true);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}


void BossBase::AddWallEff(const Vec2& pos, int sizeX, float shiftX, int sizeY, float shiftY)
{
	std::vector<WallEff> effs;
	effs.resize(kWallEffNum);

	float x, y;

	for (auto& eff : effs)
	{
		eff.size = 0.6 + GetRand(10) * 0.1 - 0.5;

		eff.pos = pos;

		x = (GetRand(sizeX) - shiftX) * 0.125f;
		y = (GetRand(sizeY) - shiftY) * 0.125f;

		eff.vec = { x, y };
		eff.vec.Normalize();

		eff.vec = eff.vec * static_cast<float>(eff.size) * kWallEffSpeed;
	}

	m_wallEff.push_back({ effs, true, 0 });
}

void BossBase::AddParticleEff()
{
	ParticleEff eff;

	// MEMO:初回と二回目が０スタートだと変わらないため
	int num = -1;

	do
	{
		eff.size = GetRand(10) * 0.1 + 0.2;

		float x = GetRand(16) * 0.125f - 1.0f;
		float y = GetRand(16) * 0.125f - 1.0f;

		eff.vec = { x, y };

		eff.vec.Normalize();

		eff.pos = m_pos + eff.vec * m_radius;

		eff.vec = eff.vec * static_cast<float>(eff.size) * 8.0f;

		m_particles.push_back(eff);

		num++;
	} while (num < m_performanceNum);
}


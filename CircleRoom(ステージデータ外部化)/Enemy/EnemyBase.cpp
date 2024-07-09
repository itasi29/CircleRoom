#include <DxLib.h>
#include "Application.h"
#include "EnemyBase.h"

#include "GameManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/ImageFile.h"

namespace
{
	// タイトルでのスピード
	constexpr float kTitleSpeed = 4.0f;

	// 壁からの法線ベクトル
	const Vec2 kNorVecLeft = Vec2{ 1.0f,  0.0f };
	const Vec2 kNorVecRight = Vec2{ -1.0f,  0.0f };
	const Vec2 kNorVecUp = Vec2{ 0.0f,  1.0f };
	const Vec2 kNorVecDown = Vec2{ 0.0f, -1.0f };

	// ずらす方向
	const Vec2 kShiftSide = Vec2{ 0.0f, 0.2f };
	const Vec2 kShiftVert = Vec2{ 0.2f, 0.0f };

	// エフェクトのスピード
	constexpr float kWallEffSpeed = 4.0f;
	// エフェクト数
	constexpr int kWallEffNum = 5;

	// 壁に当たったエフェクトをするフレーム
	constexpr int kWallEffFrame = 30;
}

EnemyBase::EnemyBase(const size& windowSize, float fieldSize) :
	m_size(windowSize),
	m_fieldSize(fieldSize),
	m_radius(0),
	m_isExsit(true),
	m_frame(0),
	m_angle(0)
{
	m_updateFunc = &EnemyBase::StartUpdate;
	m_drawFunc = &EnemyBase::StartDraw;

	auto& file = GameManager::GetInstance().GetFile();
	m_wallEffect = file->LoadGraphic(L"Enemy/wallEff.png");
	m_shadow = file->LoadGraphic(L"Enemy/ShadowNormal.png");
	m_createSe = file->LoadSound(L"Se/create.mp3");
}

EnemyBase::~EnemyBase()
{
}

void EnemyBase::TitleInit()
{
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
	m_vec = Vec2{ m_size.w * 0.5f, m_size.h * 0.5f } - m_pos;
	// 速度の調整
	m_vec = m_vec.GetNormalized() * kTitleSpeed;
}

void EnemyBase::TitleUpdate()
{
	m_pos += m_vec;
	m_angle -= kRad;

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

void EnemyBase::TitleDraw() const
{
	// 影の描画
	DrawRotaGraph(static_cast<int>(m_pos.x + 10), static_cast<int>(m_pos.y + 10), 1.0, m_angle,
		m_shadow->GetHandle(), true);

	// 本体の描画
	DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), 1.0, m_angle,
		m_charImg->GetHandle(), true);
}

void EnemyBase::Update()
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

void EnemyBase::Draw() const
{
	(this->*m_drawFunc)();
}

bool EnemyBase::Reflection(float scale, bool isShift)
{
	float centerX = m_size.w * 0.5f;
	float centerY = m_size.h * 0.5f;

	// 左
	if (m_pos.x - m_radius * scale < centerX - m_fieldSize)
	{
		// 位置の修正
		m_pos.x = centerX - m_fieldSize + m_radius * scale;

		if (isShift)
		{
			AddWallEff({ centerX - m_fieldSize, m_pos.y }, 4, 6, 16, 8);
			ReflectionCal(kNorVecLeft);
			ShiftReflection(kShiftSide);
		}
		else
		{
			m_vec = { m_vec.y, -m_vec.x};
		}

		return true;
	}
	// 右
	if (m_pos.x + m_radius * scale > centerX + m_fieldSize)
	{
		m_pos.x = centerX + m_fieldSize - m_radius * scale;

		if (isShift)
		{
			AddWallEff( { centerX + m_fieldSize, m_pos.y }, 4, -2, 16, 8);
			ReflectionCal(kNorVecRight);
			ShiftReflection(kShiftSide);
		}
		else
		{
			m_vec = { m_vec.y, -m_vec.x };
		}

		return true;
	}
	// 上
	if (m_pos.y - m_radius * scale < centerY - m_fieldSize)
	{
		m_pos.y = centerY - m_fieldSize + m_radius * scale;

		if (isShift)
		{
			AddWallEff( { m_pos.x, centerY - m_fieldSize } , 16, 8, 4, 6);
			ReflectionCal(kNorVecUp);
			ShiftReflection(kShiftVert);
		}
		else
		{
			m_vec = { m_vec.y, -m_vec.x };
		}

		return true;
	}
	// 下
	if (m_pos.y + m_radius * scale > centerY + m_fieldSize)
	{
		m_pos.y = centerY + m_fieldSize - m_radius * scale;

		if (isShift)
		{
			AddWallEff( { m_pos.x, centerY + m_fieldSize }, 16, 8, 4, -2);
			ReflectionCal(kNorVecDown);
			ShiftReflection(kShiftVert);
		}
		else
		{
			m_vec = { m_vec.y, -m_vec.x };
		}

		return true;
	}

	return false;
}

void EnemyBase::ReflectionCal(const Vec2& norVec)
{
	// 法線ベクトルの2倍から現在のベクトルを引く
	m_vec = m_vec + norVec * norVec.Dot(-m_vec) * 2.0f;
}

void EnemyBase::ShiftReflection(const Vec2& shift)
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

void EnemyBase::ChangeNormalFunc()
{
	m_updateFunc = &EnemyBase::NormalUpdate;
	m_drawFunc = &EnemyBase::NormalDraw;
}

void EnemyBase::StartDraw() const
{
	float rate = static_cast<float>(m_frame) / static_cast<float>(kApeearFrame);
	int alpha = static_cast<int>(96 * rate);
	alpha = static_cast<int>(255 * rate);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	// 影の描画
	DrawRotaGraph(static_cast<int>(m_pos.x + 10), static_cast<int>(m_pos.y + 10), 1.0, m_angle,
		m_shadow->GetHandle(), true);
	// 本体の描画
	DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), 1.0, m_angle,
		m_charImg->GetHandle(), true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void EnemyBase::NormalDraw() const
{
	// 影の描画
	DrawRotaGraph(static_cast<int>(m_pos.x + 10), static_cast<int>(m_pos.y + 10), 1.0, m_angle,
		m_shadow->GetHandle(), true);

	// 本体の描画
	DrawRotaGraph(static_cast<int>(m_pos.x), static_cast<int>(m_pos.y), 1.0, m_angle,
		m_charImg->GetHandle(), true);

	DrawHitWallEffect();

#ifdef _DEBUG
	// 当たり判定の描画
	m_col.Draw(0xff0000, false);
#endif
}

void EnemyBase::DrawHitWallEffect() const
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

void EnemyBase::AddWallEff(const Vec2& pos, int sizeX, float shiftX, int sizeY, float shiftY)
{
	std::vector<WallEff> effs;
	// エフェクト数でリサイズ
	effs.resize(kWallEffNum);

	float x, y;

	// エフェクトの追加
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

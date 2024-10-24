#pragma once
#include <vector>
#include <memory>
#include "Vec2.h"
#include "Collision.h"

struct size;
class BossBase;
class FileBase;

// 使用後のボスに飛んでいくやつ
struct Missile
{
	// 場所
	Vec2 pos;
	// 当たり判定
	Collision col;
	// ベクトル
	Vec2 vec;
	// ボスに当たったか
	bool isHit = false;
};

struct MissileEff
{
	Vec2 pos;
	Vec2 vec;

	double size;
};

struct MissileEffMass
{
	std::vector<MissileEff> effs;
	bool isUse = false;
	bool isEnd = false;
	int frame = 0;
};

/// <summary>
/// ボスにダメージを与える物体
/// </summary>
class BossDamageObject
{
public:
	BossDamageObject(const size& windowSize, float fieldSize, BossBase* boss);
	BossDamageObject(const Vec2& col, BossBase* boss);
	~BossDamageObject();

	void Update();
	void Draw();

	/// <summary>
	/// 当たり判定の中心座標を取得
	/// </summary>
	/// <returns>当たり判定の中心座標</returns>
	Collision GetRect() const { return m_col; }

	/// <summary>
	/// 攻撃しよう
	/// </summary>
	void OnUse();

	/// <summary>
	/// 既に使用済みになっていないか
	/// </summary>
	/// <returns>true: 使用済み / false:未使用</returns>
	bool IsEnd() const { return m_isUsed; }

	bool IsPickUp() const { return m_isPickUp; }

private:
	/// <summary>
	/// 共通初期化部分
	/// </summary>
	void Init();

	// 拾われるまでのアップデート
	void FlashUpdate();
	// 拾ったあとのアップデート
	void AimUpdae();

	void FlashDraw();
	void AimDraw();

	void Move(Missile& missile);

private:
	using UpdateFunc_t = void(BossDamageObject::*)();
	using DrawFunc_t = void(BossDamageObject::*)();

	UpdateFunc_t m_updateFunc;
	DrawFunc_t m_drawFunc;

	// 画像
	std::shared_ptr<FileBase> m_missileEff;
	std::shared_ptr<FileBase> m_objShadow;
	std::shared_ptr<FileBase> m_missileShadow;

	// ボスのポインタ
	BossBase* m_boss;

	// 場所
	Vec2 m_pos;
	Collision m_col;

	// 拾ったか(触れたか)
	bool m_isPickUp;
	// 全て使ったか
	bool m_isUsed;
	// 点滅フレーム
	int m_flashFrame;

	// ミサイル系の配列
	std::vector<Missile> m_missiles;
	std::vector<MissileEffMass> m_missileEffs;
};


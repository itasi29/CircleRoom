#pragma once
#include "EnemyBase.h"

/// <summary>
/// 壁に沿って動く敵
/// </summary>
class EnemyMoveWall : public EnemyBase
{
public:
	EnemyMoveWall(const size& windowSize, float fieldSize);
	virtual ~EnemyMoveWall();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="vec">撃つ方向 (上or下)</param>
	void Init(const Vec2& vec, bool isStart = false) override;

private:
	// 更新関数
	void StartUpdate() override;
	void NormalUpdate() override;
};


#pragma once
#include "EnemyBase.h"

/// <summary>
/// 通常より大きいサイズの敵
/// </summary>
class EnemyLarge : public EnemyBase
{
public:
	EnemyLarge(const size& windowSize, float fieldSize);
	virtual ~EnemyLarge();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="pos">初期位置</param>
	void Init(const Vec2& pos, bool isStart = false) override;

private:
	// 更新関数
	void StartUpdate() override;
	void NormalUpdate() override;
};


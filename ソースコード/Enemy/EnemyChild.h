#pragma once
#include "EnemyBase.h"

/// <summary>
/// 自身の足元に待機させる敵
/// 動かず時間経過で消えるs
/// </summary>
class EnemyChild : public EnemyBase
{
public:
	EnemyChild(const size& windowSize, float fieldSize);
	virtual ~EnemyChild();

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

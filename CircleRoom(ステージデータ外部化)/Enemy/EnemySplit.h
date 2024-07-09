#pragma once
#include "EnemyBase.h"

/// <summary>
/// 分裂後の敵
/// </summary>
class EnemySplit : public EnemyBase
{
public:
	EnemySplit(const size& windowSize, float fieldSize);
	virtual ~EnemySplit();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="pos">初期位置</param>
	/// <param name="vec">移動ベクトル</param>
	virtual void Init(const Vec2& pos, Vec2& vec);

private:
	// 更新関数
	void StartUpdate() override;
	virtual void NormalUpdate() override;
};


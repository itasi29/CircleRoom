#pragma once

#include "EnemyBase.h"

/// <summary>
/// ’Êí‚Ì“G
/// </summary>
class EnemyNormal : public EnemyBase
{
public:
	EnemyNormal(const size& windowSize, float fieldSize);
	virtual ~EnemyNormal();

	/// <summary>
	/// ‰Šú‰»ˆ—
	/// </summary>
	/// <param name="pos">‰ŠúˆÊ’u</param>
	void Init(const Vec2& pos, bool isStart = false) override;

protected:
	// XVŠÖ”
	void StartUpdate() override;
	void NormalUpdate() override;
};


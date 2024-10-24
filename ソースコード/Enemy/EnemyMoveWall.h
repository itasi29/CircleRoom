#pragma once
#include "EnemyBase.h"

/// <summary>
/// •Ç‚É‰ˆ‚Á‚Ä“®‚­“G
/// </summary>
class EnemyMoveWall : public EnemyBase
{
public:
	EnemyMoveWall(const size& windowSize, float fieldSize);
	virtual ~EnemyMoveWall();

	/// <summary>
	/// ‰Šú‰»ˆ—
	/// </summary>
	/// <param name="vec">Œ‚‚Â•ûŒü (ãor‰º)</param>
	void Init(const Vec2& vec, bool isStart = false) override;

private:
	// XVŠÖ”
	void StartUpdate() override;
	void NormalUpdate() override;
};


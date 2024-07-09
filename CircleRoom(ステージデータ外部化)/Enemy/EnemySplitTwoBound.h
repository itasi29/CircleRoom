#pragma once
#include "EnemySplit.h"
class EnemySplitTwoBound : public EnemySplit
{
public:
	EnemySplitTwoBound(const size& windowSize, float fieldSize);
	virtual ~EnemySplitTwoBound();

	void Init(const Vec2& pos, Vec2& vec) override;

private:
	void NormalUpdate() override;

private:
	int m_boundNo;
};


#pragma once
#include "BossArmored.h"
class BossStrongArmored : public BossArmored
{
public:
	BossStrongArmored(const size& windowSize, float fieldSize);
	BossStrongArmored(const size& windowSize, float fieldSize, StageBase* stage);
	~BossStrongArmored();

	bool OnAttack(bool isDash, const Collision& col) override;
	void OnDamage() override;

private:
	void InitData();

	void NormalDraw() const override;

	void CreateEnemy() override;
};


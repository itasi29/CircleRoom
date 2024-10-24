#pragma once
#include "StageBase.h"

class Stage1_2 : public StageBase
{
public:
	Stage1_2(GameManager& mgr, Input& input);
	~Stage1_2();

	void Init() override;
	void StartCheck() override;
	void ChangeStage(Input& input) override;

private:
	void CheckStageConditions(int timeFrame) override;
	int DrawStageConditions(int drawY) const override;
	void DrawArrow() const override;
	void DrawEnemyKilledInfo(int x, int y) const override;

	void CreateEnemy() override;
	void UpdateTime() override;

private:
	// 敵(Lage)生成フレーム
	int m_createLageFrame;
	// 敵(Normal)生成フレーム1
	int m_createNormalFrame1;
	// 敵(Normal)生成フレーム2
	int m_createNormalFrame2;

	// 初回生成数(Normal)
	int m_createNum;

	// ステージに入った時点でのクリア情報を保持
	bool m_isRightClear;
	bool m_isUpClear;
};

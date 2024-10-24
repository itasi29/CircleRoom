#pragma once
#include "StageBase.h"

class Stage1_9 : public StageBase
{
public:
	Stage1_9(GameManager& mgr, Input& input);
	~Stage1_9();

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
	// 生成数
	int m_createDashNum;
	int m_createEneCreateNum;
	int m_createLargeNum;

	// 敵生成時間
	int m_createDashFrame;
	int m_createEneCreateFrame;
	int m_createDivisionFrame;
	int m_createLargeFrame;

	// ステージに入った時点でのクリア情報を保持
	bool m_isUpClear;
};

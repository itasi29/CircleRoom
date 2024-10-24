#pragma once
#include "StageBase.h"

class Stage1_8 : public StageBase
{
public:
	Stage1_8(GameManager& mgr, Input& input);
	~Stage1_8();

	void Init() override;
	void StartCheck() override;
	void ChangeStage(Input& input) override;
	void UpTime() override;

private:
	void UniqueEndProcessing() override;

	void CheckStageConditions(int timeFrame) override;
	int DrawStageConditions(int drawY) const override;
	void DrawArrow() const override;
	void DrawEnemyKilledInfo(int x, int y) const override;

	void CreateEnemy() override;
	void CreateStrongBoss() override;
	void UpdateTime() override;

private:
	std::shared_ptr<FileBase> m_explanation;

	// ステージに入った時点でのクリア情報を保持
	bool m_isDownClear;
	bool m_isRightClear;
};


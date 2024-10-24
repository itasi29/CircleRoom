#pragma once
#include <array>
#include <memory>

class FileBase;

enum class Explanation
{
	kOperation,	// 操作説明
	kClear,		// クリア方法
	kPlay,		// 実プレイ
	kEnd,		// 説明終了
};

#include "StageBase.h"
class StageTutorial : public StageBase
{
public:
	StageTutorial(GameManager& mgr, Input& input);
	~StageTutorial();

	void Init() override;
	void StartCheck() override {}
	void ChangeStage(Input& input) override;

private:
	void UpdateSelect(Input& input);
	void UpdatePlaying(Input& input);

	void DrawUnique() const override;

	void CheckStageConditions(int timeFrame) override;
	int DrawStageConditions(int drawY) const override;
	void DrawArrow() const override;
	void DrawEnemyKilledInfo(int x, int y) const override {}
	void CreateEnemy() override;

	void UpdateTime() override;

private:
	UpdateFunc_t m_updateFunc;

	int  m_index;
	std::array<std::shared_ptr<FileBase>, 5> m_handle;
	std::array<std::shared_ptr<FileBase>, 2> m_arrowImg;

	Explanation m_explanation;
	int m_createFrame;
	bool m_isStart;
	int m_emphasisFrame;
};


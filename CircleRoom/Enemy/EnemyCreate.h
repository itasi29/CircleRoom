#pragma once
#include "EnemyBase.h"

class Stage;

/// <summary>
/// EnemyChildを生成していく敵
/// </summary>
class EnemyCreate : public EnemyBase
{
public:
	EnemyCreate(const size& windowSize, float fieldSize);
	EnemyCreate(const size& windowSize, float fieldSize, Stage* stage);
	virtual ~EnemyCreate();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="pos">初期位置</param>
	void Init(const Vec2& pos, bool isStart = false) override;

private:
	// 更新関数
	void StartUpdate() override;
	void NormalUpdate() override;

private:
	// ステージのポインタ
	Stage* m_stage;

	// 変換後の移動ベクトル
	Vec2 m_conversionVec;

	// 角度
	float m_radian;
};


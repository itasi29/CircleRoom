#pragma once
#include <list>
#include <memory>
#include "BossBase.h"

class StageBase;
class BossDamageObject;

/// <summary>
/// 一面ボス
/// </summary>
class BossArmored : public BossBase
{
public:
	BossArmored(const size& windowSize, float fieldSize);
	BossArmored(const size& windowSize, float fieldSize, StageBase* stage);
	virtual ~BossArmored();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="pos">初期位置</param>
	void Init(const Vec2& pos, bool isStart = false) override;

	/// <summary>
	/// ダメージ処理
	/// </summary>
	/// <param name="isDash">プレイヤーのダッシュ状態</param>
	/// <param name="col">プレイヤーの当たり判定</param>
	/// <returns>true: ダメージを与えた / false:ダメージを与えていない</returns>
	virtual bool OnAttack(bool isDash, const Collision& col) override;

	virtual void OnDamage() override;

	/// <summary>
	/// ダメージオブジェクトの消去
	/// </summary>
	void DeleteDamageObjects();

protected:
	// 更新関数
	void StartUpdate() override;
	void NormalUpdate() override;

	// 描画関数
	virtual void NormalDraw() const override;

	virtual void CreateEnemy();

private:
	void InitData();

protected:
	// ステージのポインタ
	StageBase* m_stage;

	// 実際の移動は別の動きとなるので変数としてもっておく
	Vec2 m_conversionVec;

	// 移動時のいびつな移動用
	float m_radian;

	// ダメージを与える物体のリスト
	std::list<std::shared_ptr<BossDamageObject>> m_objects;

	// 生成間隔フレーム
	int m_createFrame;
	// 生成前待機フレーム
	int m_waitCreateFrame;
};


#pragma once
#include "Vec2.h"

/// <summary>
/// 円を管理するためのクラス
/// </summary>
class Collision
{
public:
	Collision();
	virtual ~Collision();

	/// <summary>
	/// 当たり判定の描画
	/// </summary>
	/// <param name="color">色</param>
	/// <param name="isFill">true:全体描画 / false:外枠のみ描画</param>
	void Draw(unsigned int color, bool isFill) const;
	
	/// <summary>
	/// 中心地の更新
	/// </summary>
	/// <param name="pos">中心地</param>
	/// <param name="radius">半径</param>
	/// <param name="shiftX">X方向に中心からどれくらい動いているか</param>
	/// <param name="shiftY">Y方向に中心からどれくらい動いているか</param>
	void SetCenter(Vec2 pos, float radius, float shiftX = 0.0f, float shiftY = 0.0f);

	// 必要な情報を取得する
	float GetRadius() const { return m_radius; }
	Vec2 GetCenter() const { return m_pos; }

	/// <summary>
	/// 矩形同士の当たり判定
	/// </summary>
	/// <param name="target">判定側</param>
	/// <returns>true : 当たった, false : 当たってない</returns>
	bool IsCollsion(const Collision& target) const;

private:
	// 中心座標
	Vec2 m_pos;
	// 半径
	float m_radius;
};


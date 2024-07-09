#pragma once
#include <cmath>

/// ベクトルを便利に使うためのクラス(2次元ベクトル)
class Vec2
{
public:
	float x;
	float y;

public:
	Vec2() :
		x(0.0f),
		y(0.0f)
	{
	}

	Vec2(float posX, float posY) :
		x(posX),
		y(posY)
	{
	}

	static Vec2 Zero()
	{
		return Vec2{ 0, 0 };
	}
	static Vec2 Up()
	{
		return Vec2{ 0.0f, -1.0f };
	}
	// 自身から-90度したベクトル
	Vec2 Right() const
	{
		return Vec2{ y, -x };
	}
	// 自身から90度したベクトル
	Vec2 Left() const
	{
		return Vec2{ -y, x };
	}

	/// 単項演算子+
	Vec2 operator +() const
	{
		// thisポインタ : 自分自身のアドレスを指す
		return *this;
	}

	/// 単項演算子- 逆ベクトル
	Vec2 operator -() const
	{
		return Vec2{ -x, -y };
	}

	/// 足し算
	Vec2 operator +(Vec2 vec) const
	{
		return Vec2{ x + vec.x, y + vec.y };
	}
	Vec2 operator +=(Vec2 vec)
	{
		x += vec.x;
		y += vec.y;
		return *this;
	}

	/// 引き算
	Vec2 operator -(Vec2 vec) const
	{
		return Vec2{ x - vec.x, y - vec.y };
	}
	Vec2 operator -=(Vec2 vec)
	{
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	/// 掛け算
	Vec2 operator *(float scale) const
	{
		return Vec2{ x * scale, y * scale };
	}
	Vec2 operator *=(float scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}

	/// 割り算
	Vec2 operator /(float scale) const
	{
		return Vec2{ x / scale, y / scale };
	}
	Vec2 operator /=(float scale)
	{
		x /= scale;
		y /= scale;
		return *this;
	}

	Vec2 operator *(Vec2 left) const
	{
		Vec2 temp;

		temp.x = x * left.x - y * left.y;
		temp.y = x * left.y + y * left.x;

		return temp;
	}
	Vec2 operator *=(Vec2 left)
	{
		Vec2 temp = this->GetNormalized();

		x = temp.x * left.x - temp.y * left.y;
		y = temp.x * left.y + temp.y * left.x;

		return *this;		
	}

	bool operator ==(Vec2 left)
	{
		return x == left.x && y == left.y;
	}
	bool operator !=(Vec2 left)
	{
		return !(*this == left);
	}

	/// <summary>
	/// ベクトルの長さの2乗を求める
	/// </summary>
	/// <returns>長さの2乗</returns>
	float SqLength() const
	{
		return x * x + y * y;
	}

	/// <summary>
	/// ベクトルの長さを求める
	/// </summary>
	/// <returns>長さ</returns>
	float Length() const
	{
		return sqrtf(SqLength());
	}

	/// <summary>
	/// 自身の正規化を行う
	/// </summary>
	void Normalize()
	{
		float len = Length();

		// 長さ0以下なら処理を行わない
		if (len <= 0.0f) return;

		x /= len;
		y /= len;
	}

	/// <summary>
	/// 自身を正規化したベクトルを取得する
	/// </summary>
	/// <returns>正規化したベクトル</returns>
	Vec2 GetNormalized() const
	{
		float len = Length();

		// 長さ0以下なら(0, 0)を返す
		/* 引数ありにしている理由 */
		// 引数なしコンストラクタの修正に強くするため
		if (len <= 0.0f) return Vec2{ 0.0f, 0.0f };

		return Vec2{ x / len, y / len };
	}

	/// <summary>
	/// 内積を取得する
	/// </summary>
	/// <param name="vec2">求める相手のベクトル</param>
	/// <returns>内積</returns>
	float Dot(Vec2 vec2) const
	{
		return x * vec2.x + y * vec2.y;
	}
};
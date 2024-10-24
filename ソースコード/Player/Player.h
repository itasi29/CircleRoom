#pragma once
#include <vector>
#include <list>
#include "Vec2.h"
#include "Utility/Collision.h"

class Input;
class Application;
class FileBase;
struct size;
enum class Ability;

struct DirectionVec
{
	Vec2 front;
	Vec2 right;
	Vec2 left;
};

struct EffectData
{
	Vec2 vec;
	Vec2 pos;
	double angle = 0.0;
	int frame = 0;
	bool isEnd = false;
};


class Player
{
public:
	Player(const size& windowSize, float fieldSize);
	~Player();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init();
	void Update(Input& input, Ability ability);
	void Draw() const;

	/// <summary>
	/// 正面ベクトルを取得
	/// </summary>
	/// <returns>正面ベクトル</returns>
	Vec2 GetFront() const { return m_front; }

	/// <summary>
	/// 中心座標を取得
	/// </summary>
	/// <returns>中心座標</returns>
	Vec2 GetPos() const { return m_pos; }

	/// <summary>
	/// 当たり判定の中心座標を取得
	/// </summary>
	/// <returns>当たり判定の中心座標</returns>
	Collision GetRect() const { return m_col; }
	/// <summary>
	/// オブジェクトの当たり判定を取得
	/// </summary>
	/// <returns>オブジェクト判定の中心座標</returns>
	Collision GetObjRect() const { return m_objCol; }
	bool IsDash() const { return m_isDash; }
	/// <summary>
	/// 生存しているか
	/// </summary>
	/// <returns>生存判定</returns>
	bool IsExsit() const { return m_isExsit; }

	/// <summary>
	/// 死亡処理
	/// </summary>
	void Death();

private:
	/// <summary>
	/// プレイヤーの移動
	/// </summary>
	/// <param name="input">入力情報</param>
	void Move(Input& input);
	/// <summary>
	/// ダッシュ処理
	/// </summary>
	/// <param name="input">入力情報</param>
	void Dash(Input& input);
	/// <summary>
	/// 場外判定処理
	/// </summary>
	void InRange();
	/// <summary>
	/// 移動エフェクトの追加
	/// </summary>
	void AddMoveEff();

private:
	std::shared_ptr<FileBase> m_bloodImg;

	// スクリーンサイズ
	const size& m_size;
	// フィールドのサイズ
	float m_fieldSize;

	// 画像
	std::shared_ptr<FileBase> m_charImg;
	std::shared_ptr<FileBase> m_charDeathImg;
	std::shared_ptr<FileBase> m_charEffImg;

	// 死亡時Se
	std::shared_ptr<FileBase> m_deathSe;
	// ダッシュ時Se
	std::shared_ptr<FileBase> m_dashSe;

	// 中心座標
	Vec2 m_pos;
	// 正面方向
	Vec2 m_front;
	// 移動ベクトル
	Vec2 m_vec;
	// 方向ベクトル
	DirectionVec m_dir;

	// 移動エフェクト
	std::list<EffectData> m_effs;

	// 角度
	double m_angle;

	// 当たり判定
	Collision m_col;
	// ダメージオブジェクト当たり判定
	Collision m_objCol;

	// ダッシュログ
	std::vector<Vec2> m_posLog;
	std::vector<DirectionVec> m_dirLog;
	std::vector<double> m_angleLog;
	// ログフレーム
	int m_logFrame;
	// ダッシュするフレーム
	int m_dashFrame;
	// ダッシュ再利用可能までのフレーム
	int m_dashWaitFrame;
	// ダッシュ判定
	bool m_isDash;

	// 生存しているか
	bool m_isExsit;

	// 死亡エフェクトのフレーム
	int m_deathFrame;
	// 死亡エフェクを行うかどうか
	bool m_isDeathEffect;
};


#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "Vec2.h"

class Input;
class SceneManager;
class Stage;
class StageMap;
class GameData;
struct size;

enum class StageDir
{
	kStageLeft,
	kStageRight,
	kStageUp,
	kStageDown,
	kStageMax,
};

/// <summary>
/// ステージの遷移をコントロール
/// </summary>
class StageManager
{
public:

	StageManager(std::shared_ptr<SceneManager>& mgr);
	~StageManager();

	void Init(Input& input);
	void InitPos();

	void Update(Input& input);
	void Draw() const;

	std::string GetStageName() const;

	bool IsClear() const { return m_isClear; }
	void OnClear() { m_isClear = true; }
	void ResetClear() { m_isClear = false; }
	void OffDrawWave() const;

	std::shared_ptr<StageMap> GetMap() const { return m_map; }
	std::shared_ptr<GameData> GetData() const { return m_data; }

	/// <summary>
	///  ステージの切り替え
	/// </summary>
	/// <param name="nextStage">次のステージ名</param>
	/// <param name="isGameScene">ゲームシーンからの変更か</param>
	void ChangeStage(const std::string& nextStage, bool isGameScene = false);

	/// <summary>
	/// ステージの即時変更
	/// </summary>
	void ImmediatelyChange();

private:
	// 通常の更新
	void NormalUpdate(Input& input);
	// ステージ移動の更新
	void StageMoveUpdate(Input& input);
	// 移動かつプレイの更新
	void MoveGamePlaingUpdate(Input& input);

	// 通常の描画
	void NormalDraw() const;
	// ステージ移動時の描画
	void StageMoveDraw() const;
	// 移動かつプレイの描画
	void MoveGamePlaingDraw() const;

	void CheckEnd();

private:
	using UpdateFunc_t = void(StageManager::*)(Input&);
	using DrawFunc_t = void(StageManager::*)() const;

	UpdateFunc_t m_updateFunc;
	DrawFunc_t m_drawFunc;

	const size& m_size;

	std::shared_ptr<SceneManager>& m_mgr;

	// 画面描画用ハンドル
	int m_drawScreen;
	// 移動時の画面ハンドル
	int m_screen;
	// キープ用画面ハンドル
	int m_keepScreen;

	// ゲームクリア
	bool m_isClear;

	// ステージのポインタ
	std::shared_ptr<Stage> m_stage;
	// マップ情報のポインタ
	std::shared_ptr<StageMap> m_map;
	// ゲームデータのポインタ
	std::shared_ptr<GameData> m_data;

	// 画面が動いているか
	bool m_isMove;

	// 画面を動かすベクトル
	Vec2 m_vec;
	// 現在のステージの場所
	Vec2 m_pos;
	// 次のステージの場所
	Vec2 m_targetPos;
};


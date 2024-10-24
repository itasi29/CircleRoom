#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "Vec2.h"

class Input;
class SceneManager;
class StageBase;
class FileBase;
struct size;

struct StageData
{
	int bestTime = 0;
	bool isClear = false;
};

enum class Ability
{
	kNone,
	kDash,
};

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

	void Init();
	void InitData();
	void InitPos();

	void Update(Input& input);
	void Draw() const;

	std::string GetStageName() const;

	bool IsClear() const { return m_isClear; }
	void OnClear() { m_isClear = true; }
	void ResetClear() { m_isClear = false; }
	void OffDrawWave() const;

	/// <summary>
	///  ステージの切り替え
	/// </summary>
	/// <param name="nextStage">次のステージ</param>
	/// <param name="isGameScene">ゲームシーンからの変更か</param>
	void ChangeStage(std::shared_ptr<StageBase> nextStage, bool isGameScene = false);

	/// <summary>
	/// ステージの即時変更
	/// </summary>
	void ImmediatelyChange();

	/// <summary>
	/// クリア情報を保存する
	/// </summary>
	/// <param name="path">パス</param>
	void Save(const std::string& path);

	/// <summary>
	/// クリア情報を読み込む
	/// </summary>
	/// <param name="path">パス</param>
	void Load(const std::wstring& path);

	/// <summary>
	/// そのステージ名のステージデータがない場合作成する
	/// </summary>
	/// <param name="stgName">ステージ名</param>
	void CreateData(const std::string& stgName);

	/// <summary>
	/// クリア情報
	/// </summary>
	/// <param name="stgName">ステージ名</param>
	/// <returns>true:クリア済み / false:未クリア</returns>
	bool IsClearStage(const std::string& stgName);

	/// <summary>
	/// すでにクリアしたことがあるボスか
	/// </summary>
	/// <param name="name">ボスの名前</param>
	/// <returns>true: クリアしている / false:クリアしていない</returns>
	bool IsClearBoss(const std::string& name) const;

	/// <summary>
	/// すでに殺されたことがある敵か
	/// </summary>
	/// <param name="name">敵の名前</param>
	/// <returns>true: 殺されたことがある / false: 殺されたことがない</returns>
	bool IsKilledEnemy(const std::string& name) const;

	/// <summary>
	/// ステージ名に対応するベストクリアタイムを持ってくる
	/// </summary>
	/// <param name="stgName">ステージ名</param>
	/// <returns>ベストタイム</returns>
	int GetBestTime(const std::string& stgName) const;

	/// <summary>
	/// プレイヤーを殺した敵の種類数を返す
	/// </summary>
	/// <returns>種類数</returns>
	int GetEnemyTypeCount() const;

	/// <summary>
	/// アビリティを返す
	/// </summary>
	/// <returns>アビリティ</returns>
	Ability GetAbility() const;

	/// <summary>
	/// ボスステージに入ったことがあるか
	/// </summary>
	/// <returns>true: ある / false: ない</returns>
	bool IsBossIn() const { return m_isBossIn; }

	/// <summary>
	/// クリア情報の保存
	/// クリア済みとする
	/// </summary>
	/// <param name="stgName">ステージ名</param>
	void SaveClear(const std::string& stgName);

	/// <summary>
	/// ボスをクリアしたことがあるとする
	/// </summary>
	/// <param name="name">ボスの名前</param>
	void UpdateClearBoss(const std::string& name);

	/// <summary>
	/// ステージ名に対応するベストタイムの更新
	/// </summary>
	/// <param name="stgName">ステージ名</param>
	/// <param name="bestTime">更新タイム</param>
	bool UpdateBestTime(const std::string& stgName, int bestTime);

	/// <summary>
	/// プレイヤーを殺した敵がすでに殺したことがあるかの確認
	/// もし、殺したことがなければ名前を保存し、種類数カウントを増やす
	/// </summary>
	/// <param name="name">敵の名前</param>
	void UpdateEnemyType(const std::string& name);

	/// <summary>
	/// アビリティの変更
	/// </summary>
	/// <param name="ability">アビリティ番号</param>
	void ChangeAbility(Ability ability);

	/// <summary>
	/// ボスステージに入ったことがあるとする
	/// </summary>
	void BossStageIn() { m_isBossIn = true; }

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

	Vec2 GetPos(const std::string& stage) const;

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

	// ダッシュ説明画像クラス
	std::shared_ptr<FileBase> m_dashImg;

	// ステージのクリア情報群
	std::unordered_map<std::string, StageData> m_stageSaveData;
	// 殺された敵の情報群
	std::vector<std::string> m_killedEnemyNameTable;
	// プレイヤー殺した種類の数
	int m_killedEnemyCount;
	// クリアしたボス情報群
	std::vector<std::string> m_clearBossTable;
	// アビリティ
	Ability m_ability;
	// アビリティの有効無効
	std::unordered_map<Ability, bool> m_abilityActive;
	// ボスステージに入ったことがあるか
	bool m_isBossIn;
	// ゲームクリア
	bool m_isClear;

	// ステージのポインタ
	std::shared_ptr<StageBase> m_stage;
	// 次のステージのポインタ
	std::shared_ptr<StageBase> m_nextStage;

	// 画面が動いているか
	bool m_isMove;

	// 画面を動かすベクトル
	Vec2 m_vec;
	// 現在のステージの場所
	Vec2 m_pos;
	// 次のステージの場所
	Vec2 m_targetPos;
};


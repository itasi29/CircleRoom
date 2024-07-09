#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class FileBase;

struct SaveData
{
	int bestTime = 0;
	bool isClear = false;
};

enum class Ability
{
	kNone,
	kDash,
};

/// <summary>
/// ステージデータを扱うクラス
/// </summary>
class GameData
{
public:
	GameData();
	~GameData();

	// 初期化
	void Init();
	void InitData();

	// 取得系
	int GetBestTime(const std::string& name) const;
	int GetEnemyTypeCount() const { return m_killedEnemyTypeNum; }
	Ability GetAbility() const { return m_ability; }

	// 判断系
	bool IsClearStage(const std::string& name);
	bool IsClearBoss(const std::string& name) const;
	bool IsKilledEnemy(const std::string& name) const;
	bool IsInBoss() const { return m_isInBoss; };

	// データ作成
	void CreateData(const std::string& name);

	// 保存系
	void SaveClearStage(const std::string& name);
	void SaveClearBoss(const std::string& name);
	void SaveKilledEnemyType(const std::string& name);
	bool SaveBestTime(const std::string& name, int frame);
	void SaveInBossStage() { m_isInBoss = true; }

	// 変更系
	void ChangeAbility(Ability ability);

	// 情報ロード・セーブ
	void Save();
	void Load();

private:
	// セーブデータ情報群
	std::unordered_map<std::string, SaveData> m_saveData;
	// クリアしたボスの情報群
	std::vector<std::string> m_clearBossData;
	// 殺されたことのある敵の情報群
	std::vector<std::string> m_killedEnemyNameData;
	// プレイヤーを殺したことがある敵の種類数
	int m_killedEnemyTypeNum;
	// アビリティ
	Ability m_ability;
	// アビリティ有効判断
	std::unordered_map<Ability, bool> m_isActiveAbilitys;

	// ボスステージ入場判定
	bool m_isInBoss;

	// ダッシュ説明画像
	std::shared_ptr<FileBase> m_explanationDash;
};


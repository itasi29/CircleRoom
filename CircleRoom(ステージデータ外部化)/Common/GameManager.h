#pragma once
#include <string>
#include <memory>

class FileManager;
class SceneManager;
class StageManager;
class SoundSystem;
class FontSystem;
class Input;

struct VolumeData
{
	// Bgmボリューム
	int bgmVol;
	// Seボリューム
	int seVol;
};

struct AccountData
{
	// プレイ時間
	int playTime;
	// 死亡数
	int deathCount;
	// ダッシュ使用回数
	int dashCount;

	VolumeData volume;
};

/// <summary>
/// ゲーム全体のマネージャーを一括管理する
/// </summary>
class GameManager
{
private:
	GameManager();
	// コピー代入の禁止
	GameManager(const GameManager& game) = delete;
	void operator=(const GameManager& game) = delete;

public:
	~GameManager();

	void Init(Input& input);
	void InitData();

	/// <summary>
	/// ゲームマネージャークラスのインスタンスを返す
	/// </summary>
	/// <returns>ゲームマネージャークラスのインスタンス</returns>
	static GameManager& GetInstance();

	void SetFileManager(std::shared_ptr<FileManager>& file);

	std::shared_ptr<FileManager>& GetFile();
	std::shared_ptr<SceneManager>& GetScene();
	std::shared_ptr<StageManager>& GetStage();
	std::shared_ptr<SoundSystem>& GetSound();
	std::shared_ptr<FontSystem>& GetFont();

	VolumeData GetVolume() const { return m_data.volume; }
	AccountData GetData() const;
	const std::string& GetNowStage() const { return m_nowStage; }

	void UpdateNowStage(const std::string& name) { m_nowStage = name; }
	void UpdateVolume(const VolumeData& data) { m_data.volume = data; }
	void UpdatePlaytime() { m_data.playTime++; }
	void UpdateDeathCcount() { m_data.deathCount++; }
	void UpdateDashCount() { m_data.dashCount++; }

private:
	void Save();
	void Load();

private:
	std::shared_ptr<FileManager> m_file;
	std::shared_ptr<SceneManager> m_scene;
	std::shared_ptr<StageManager> m_stage;
	std::shared_ptr<SoundSystem> m_sound;
	std::shared_ptr<FontSystem> m_font;

	AccountData m_data;
	std::string m_nowStage;
};


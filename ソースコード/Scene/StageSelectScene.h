#pragma once
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include "Scene.h"

class FileBase;
class StageBase;
class SoundSystem;

/// <summary>
/// ステージ選択シーン
/// </summary>
class StageSelectScene : public Scene
{
public:
	StageSelectScene(GameManager& mgr, Input& input);
	~StageSelectScene();

	void Update(Input& input);
	void Draw() const;

private:
	void DrawInf(const std::string& st) const;

	/// <summary>
	/// 選択等の文字・画像描画
	/// </summary>
	/// /// <param name="cmd">コマンド名</param>
	/// <param name="str">ウェーブさせる文字列</param>
	/// <param name="num">文字列数</param>
	void DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const;

	void CurrosrPos();

private:
	int m_indexRow;
	int m_indexLine;

	int m_fadeFrame;

	std::unordered_map<std::string, std::shared_ptr<StageBase>> m_stageData;
	// 音マネージャー
	std::shared_ptr<SoundSystem> m_soundSys;

	// 画像関係
	std::shared_ptr<FileBase> m_frame;
	std::shared_ptr<FileBase> m_nowPos;
	std::shared_ptr<FileBase> m_lock;
	std::shared_ptr<FileBase> m_startFrame;
	std::unordered_map<std::string, std::array<std::shared_ptr<FileBase>, 2>> m_stage;
	// 選択時Se
	std::shared_ptr<FileBase> m_selectSe;

	// 文字ウェーブ用の角度
	float m_waveAngle;

	std::shared_ptr<BottansFile> m_bt;
	std::shared_ptr<KeyFile> m_key;

	const Input& m_input;
};


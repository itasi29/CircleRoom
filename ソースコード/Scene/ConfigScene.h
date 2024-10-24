#pragma once
#include <memory>
#include "Scene.h"

class FileBase;
class BottansFile;
class KeyFile;
class SoundSystem;

class ConfigScene : public Scene
{
public:
	ConfigScene(GameManager& mgr, Input& input, std::shared_ptr<SceneManager> scene);
	~ConfigScene();

	void Update(Input& input);
	void Draw() const;

private:
	void DrawName(int drawY, int index, std::wstring str) const;

	/// <summary>
	/// 選択等の文字・画像描画
	/// </summary>
	/// /// <param name="cmd">コマンド名</param>
	/// <param name="str">ウェーブさせる文字列</param>
	/// <param name="num">文字列数</param>
	void DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const;

private:
	// OptionSceneが持っているScene
	std::shared_ptr<SceneManager> m_optionScn;

	int m_currentLineIndex;

	int m_fadeFrame;

	// 画像関係
	std::shared_ptr<FileBase> m_frame;
	// 音関係
	std::shared_ptr<SoundSystem> m_soundSys;
	std::shared_ptr<FileBase> m_startFrame;
	// 選択時Se
	std::shared_ptr<FileBase> m_selectSe;
	std::shared_ptr<FileBase> m_cursorSe;

	// 文字ウェーブ用の角度
	float m_waveAngle;
	// ウエーブのやつ描画するか
	bool m_isWaveDraw;

	std::shared_ptr<BottansFile> m_bt;
	std::shared_ptr<KeyFile> m_key;

	const Input& m_input;
};


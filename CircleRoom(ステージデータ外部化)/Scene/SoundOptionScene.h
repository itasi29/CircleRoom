#pragma once
#include "Scene.h"

class FileBase;
class SoundSystem;
class BottansFile;
class KeyFile;

/// <summary>
/// 音量調整シーン
/// </summary>
class SoundOptionScene :
    public Scene
{
public:
    SoundOptionScene(GameManager& mgr, Input& input);
    ~SoundOptionScene();

    void Update(Input& input);
    void Draw() const;

private:
	using updateFunc_t = void(SoundOptionScene::*)(Input&);

	void NormalUpdate(Input& input);
	void EditUpdate(Input& input);

	void DrawName(int drawY, int index, std::wstring str) const;

	/// <summary>
	/// ゲージの描画
	/// </summary>
	/// <param name="drawX">左端描画位置</param>
	/// <param name="drawY">上端描画位置</param>
	/// <param name="rate">ボリューム割合</param>
	void DrawGauge(int drawX, int drawY, float rate) const;

	/// <summary>
	/// 選択等の文字・画像描画
	/// </summary>
	/// /// <param name="cmd">コマンド名</param>
	/// <param name="str">ウェーブさせる文字列</param>
	/// <param name="num">文字列数</param>
	void DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const;

private:
	updateFunc_t m_updateFunc;

	int m_currentLineIndex;
	bool m_isEdit;

	// フェードフレーム
	int m_fadeFrame;
	// リピート確認フレーム
	int m_repeatUpFrame;
	int m_repeatDownFrame;

	// 画像関係
	std::shared_ptr<FileBase> m_frame;
	std::shared_ptr<FileBase> m_addFrame;
	std::shared_ptr<FileBase> m_startFrame;

	// 音関係
	std::shared_ptr<SoundSystem> m_soundSys;
	// 選択時Se
	std::shared_ptr<FileBase> m_selectSe;
	std::shared_ptr<FileBase> m_cursorSe;

	// 文字ウェーブ用の角度
	float m_waveAngle;

	std::shared_ptr<BottansFile> m_bt;
	std::shared_ptr<KeyFile> m_key;

	const Input& m_input;
};


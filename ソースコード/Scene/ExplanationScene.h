#pragma once
#include "Scene.h"
#include <array>

class SceneManager;
class FileBase;
class BottansFile;
class KeyFile;

class ExplanationScene : public Scene
{
public:
	ExplanationScene(GameManager& mgr, Input& input, std::shared_ptr<SceneManager> scn);
	~ExplanationScene();

	void Update(Input& input);
	void Draw() const;

private:
	void SelectUpdate(Input& input);
	void OperationUpdate(Input&);
	void ProceedUpdate(Input&);
	void DashUpdate(Input&);
	void BossUpdate(Input&);

	/// <summary>
	/// 選択等の文字・画像描画
	/// </summary>
	/// /// <param name="cmd">コマンド名</param>
	/// <param name="str">ウェーブさせる文字列</param>
	/// <param name="num">文字列数</param>
	void DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const;

private:
	using UpdateFunc_t = void(ExplanationScene::*)(Input&);

	UpdateFunc_t m_updateFunc;

	// 画像
	std::shared_ptr<FileBase> m_frame;
	std::shared_ptr<FileBase> m_startFrame;
	std::shared_ptr<FileBase> m_operator;
	std::array<std::shared_ptr<FileBase>, 3> m_proceed;
	std::shared_ptr<FileBase> m_dash;
	std::shared_ptr<FileBase> m_boss;
	// SE
	std::shared_ptr<FileBase> m_selectSe;
	std::shared_ptr<FileBase> m_cancelSe;
	std::shared_ptr<FileBase> m_cursorSe;

	// optionのシーン
	std::shared_ptr<SceneManager> m_optionScn;

	int m_currentLineIndex;

	int m_fadeFrame;

	int m_index;

	// 文字ウェーブ用の角度
	float m_waveAngle;
	// ウエーブのやつ描画するか
	bool m_isWaveDraw;

	std::shared_ptr<BottansFile> m_bt;
	std::shared_ptr<KeyFile> m_key;

	const Input& m_input;
};


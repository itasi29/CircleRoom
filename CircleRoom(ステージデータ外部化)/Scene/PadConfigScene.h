#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "Scene.h"
#include "Input.h"

class KeyFile;
class BottansFile;
class SceneManager;

/// <summary>
/// PADコンフィグのシーン
/// </summary>
class PadConfigScene : public Scene
{
public:
	PadConfigScene(GameManager& mgr, Input& input, std::shared_ptr<SceneManager> scn);
	~PadConfigScene();

	void Update(Input& input);
	void Draw() const;

private:
	void NormalUpdate(Input& input);
	void EditUpdate(Input& input);
	void EditEndUpdate(Input& input);

	void DrawCommandList() const;

	/// <summary>
	/// 選択等の文字・画像描画
	/// </summary>
	/// /// <param name="cmd">コマンド名</param>
	/// <param name="str">ウェーブさせる文字列</param>
	/// <param name="num">文字列数</param>
	void DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const;

private:
	using updateFunc_t = void (PadConfigScene::*)(Input&);
	updateFunc_t m_updateFunc;

	// optionのシーン
	std::shared_ptr<SceneManager> m_optionScn;

	// PADの画像用
	std::shared_ptr<BottansFile> m_btImg;

	// Inputクラスを元から持っておく
	Input& m_input;
	// Inputクラスのテーブル
	InputTable_t m_commandTable;

	// 選択位置
	int m_currentLineIndex;
	// 編集中
	bool m_isEdit;
	int m_fadeFrame;
	// キャンセル確認フレーム
	int m_cancleFrame;

	// 画像
	std::shared_ptr<FileBase> m_frame;
	std::shared_ptr<FileBase> m_addFrame;
	std::shared_ptr<FileBase> m_startFrame;
	// Se
	std::shared_ptr<FileBase> m_selectSe;
	std::shared_ptr<FileBase> m_cancelSe;
	std::shared_ptr<FileBase> m_cursorSe;

	// 文字ウェーブ用の角度
	float m_waveAngle;

	std::shared_ptr<BottansFile> m_bt;
	std::shared_ptr<KeyFile> m_key;
};


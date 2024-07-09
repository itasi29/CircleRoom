#pragma once
#include "Scene.h"
#include <unordered_map>
#include <memory>
#include "Input.h"

class BottansFile;
class KeyFile;
class SceneManager;

/// <summary>
/// キーコンフィグのシーン
/// </summary>
class KeyConfigScene : public Scene
{
public:
	KeyConfigScene(GameManager& mgr, Input& input, std::shared_ptr<SceneManager> scn);
	~KeyConfigScene();

	void Update(Input& input);
	void Draw() const;

private:	
	// 更新関数
	void NormalUpdate(Input&);		// 通常状態
	void EditUpdate(Input&);		// 編集状態
	void EditEndUpdate(Input&);		// 編集終了状態

	void DrawCommandList() const;	// コマンドリストの描画(テキスト描画)

	void CommitCurrenKeySetting();

	/// <summary>
	/// 選択等の文字・画像描画
	/// </summary>
	/// /// <param name="cmd">コマンド名</param>
	/// <param name="str">ウェーブさせる文字列</param>
	/// <param name="num">文字列数</param>
	void DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const;

private:
	// 更新メンバ関数ポインタ
	using UpdateFunc_t = void(KeyConfigScene::*)(Input& input);
	UpdateFunc_t  m_updateFunc;

	// Optionのシーン
	std::shared_ptr<SceneManager> m_optionScn;

	// KEYの画像用
	std::shared_ptr<KeyFile> m_keyImg;

	// Inputクラスの参照を持っておく
	Input& m_input;
	// Inputクラスのテーブル
	InputTable_t m_commandTable;

	// 現在行のインデックス
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


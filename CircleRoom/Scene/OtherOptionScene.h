#pragma once
#include "Scene.h"

class SceneManager;
class FileBase;
class BottansFile;
class KeyFile;

class OtherOptionScene : public Scene
{
public:
	OtherOptionScene(GameManager& mgr, Input& input, std::shared_ptr<SceneManager> scn);
	~OtherOptionScene();

	void Update(Input& input);
	void Draw() const;

private:
	void FadeUpdate(Input& input);
	void NormalUpdate(Input& input);

	void FadeDraw() const;
	void NormalDraw() const;

	void DrawWindowMode(int index, int handle, int y, unsigned int color) const;

	/// <summary>
	/// 選択等の文字・画像描画
	/// </summary>
	/// /// <param name="cmd">コマンド名</param>
	/// <param name="str">ウェーブさせる文字列</param>
	/// <param name="num">文字列数</param>
	void DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const;

private:
	using UpdateFunc_t = void(OtherOptionScene::*)(Input&);
	using DrawFunc_t = void(OtherOptionScene::*)() const;

	UpdateFunc_t m_updateFunc;
	DrawFunc_t m_drawFunc;

	// 画像
	std::shared_ptr<FileBase> m_frame;
	std::shared_ptr<FileBase> m_rightNotationImg;
	std::shared_ptr<FileBase> m_startFrame;
	// Se
	std::shared_ptr<FileBase> m_cursorSe;
	std::shared_ptr<FileBase> m_selectSe;

	// optionのシーン
	std::shared_ptr<SceneManager> m_optionScn;

	int m_currentLineIndex;

	int m_fadeFrame;

	// 文字ウェーブ用の角度
	float m_waveAngle;
	// ウエーブのやつ描画するか
	bool m_isWaveDraw;

	std::shared_ptr<BottansFile> m_bt;
	std::shared_ptr<KeyFile> m_key;

	const Input& m_input;
};


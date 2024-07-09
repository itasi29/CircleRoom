#pragma once
#include <memory>
#include "Scene.h"

class FileBase;

/// <summary>
/// 説明等の一画面を写す用のクラス
/// </summary>
class OneShotScene : public Scene
{
public:
	/// <summary>
	/// インストラクタ
	/// </summary>
	/// <param name="mgr">ゲームマネージャー</param>
	/// <param name="handle">写したい画像のハンドル</param>
	OneShotScene(GameManager& mgr, int handle);
	~OneShotScene();

	void Update(Input& input);
	void Draw() const;

private:
	void AppearUpdate(Input& input);
	void NormalUpdate(Input& input);
	void DisAppearUpdate(Input& input);

	void MoveDraw() const;
	void NormalDraw() const;

private:
	using UpdateFunc_t = void(OneShotScene::*)(Input&);
	using DrawFunc_t = void(OneShotScene::*)() const;
	UpdateFunc_t m_updateFunc;
	DrawFunc_t m_drawFunc;

	std::shared_ptr<FileBase> m_selectSe;

	int m_handle;
	int m_frame;

	// 画像サイズ固定化用
	double m_sizeW;
	double m_sizeH;
};


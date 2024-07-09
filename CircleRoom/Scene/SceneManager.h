#pragma once
#include <memory>
#include <list>
#include "Vec2.h"

class Application;
class Input;
class Scene;
class FileBase;
class BackgroundScene;

/// <summary>
/// 各シーンの遷移をコントロールするクラス
/// </summary>
class SceneManager
{
public:
	SceneManager(bool isDrawBg = true);
	~SceneManager();

	void Init();

	/// <summary>
	/// 持っているシーンのUpdate関数を呼び出す
	/// </summary>
	/// <param name="input">入力クラス</param>
	void Update(Input& input);
	/// <summary>
	/// 持っているシーンの描画Draw関数を呼び出す
	/// </summary>
	void Draw() const;

	/// <summary>
	/// 現在末尾で実行中のシーンを引数で指定されたシーンに切り替える
	/// </summary>
	/// <param name="nextScene">次のシーン</param>
	/// <param name="isMoveBg">背景を動かすか</param>
	void ChangeScene(std::shared_ptr<Scene> nextScene, float speed = -1);

	/// <summary>
	/// 現在あるすべてのシーンを削除しシーンを切り替える
	/// </summary>
	/// <param name="nextScene">次のシーン</param>
	/// <param name="isMoveBg">背景を動かすか</param>
	void ChangeSceneWithClear(std::shared_ptr<Scene> nextScene, float speed = -1);

	/// <summary>
	/// 現在のシーンの上にシーンを乗っけます
	/// </summary>
	/// <param name="scene">上に乗っけたいシーン</param>
	void PushScene(std::shared_ptr<Scene> scene);

	/// <summary>
	/// 現在の末尾シーンをポップします。
	/// </summary>
	void PopScene();

	/// <summary>
	/// 画面全体を揺らす
	/// </summary>
	/// <param name="frame">揺らすフレーム</param>
	void ShakeScreen(int frame);
	/// <summary>
	/// 画面全体を揺らす
	/// </summary>
	/// <param name="frame">揺らすフレーム</param>
	/// <param name="size">揺らす際の幅</param>
	void ShakeScreen(int frame, int size);

	/// <summary>
	/// シェーダー開始
	/// </summary>
	void OnShader();

	/// <summary>
	/// ガウスぼかし開始
	/// </summary>
	void OnGaussianBlur(bool isAll = false);

	/// <summary>
	/// 通常描画へ
	/// </summary>
	void OnNormal();

	/// <summary>
	/// 末尾のSceneを取得
	/// </summary>
	std::shared_ptr<Scene> GetTopScene();

	/// <summary>
	/// 現在描画している先を入手
	/// </summary>
	/// <returns>描画先ハンドル</returns>
	int GetScreenHandle() const;

private:
	void NormalUpdate(Input& input);
	void ShakeUpdate(Input& input);
	void ShaderUpdate(Input& input);
	void GaussianBlurUpdate(Input& input);

	void NormalDraw() const;
	void ShakeDraw() const;
	void ShaderDraw() const;
	void GaussianBlurDraw() const;

	/// <summary>
	/// シェーダー描くよう
	/// </summary>
	/// <param name="x">X座標</param>
	/// <param name="y">Y座標</param>
	/// <param name="width">幅</param>
	/// <param name="height">高さ</param>
	void MyDraw(int x, int y, int width, int height) const;

private:
	using UpdateFunc_t = void(SceneManager::*)(Input&);
	using DrawFunc_t = void(SceneManager::*)() const;

	UpdateFunc_t m_updateFunc;
	DrawFunc_t m_drawFunc;

	// シーンを入れる
	std::list<std::shared_ptr<Scene>> m_scenes;
	// 背景用シーン
	std::shared_ptr<BackgroundScene> m_bg;

	// 画面を揺らすときに使う画面
	int m_shakeHandle;
	// 画面を揺らすフレーム
	int m_shakeFrame;
	// 画面を揺らす際のサイズ
	int m_shakeSize;
	// 画面を揺しているか
	bool m_isShake;

	// 背景を描画するか
	bool m_isDrawBg;

	// シェーダーしているか
	bool m_isShader;
	// シェーダーに使う用
	int m_cbuffer;
	float m_angle;
	float* m_gAngle;
	int m_wavePs;
	int m_ps;
	int m_shaderScreen;
	int m_drawScreen;

	// ガウスぼかしに使うよう
	int m_gaussianBlurScreen;
	bool m_isAllGaussianBlur;
};


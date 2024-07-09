#pragma once
#include <list>
#include <memory>
#include "Scene.h"

class FileBase;
class SoundSystem;
class EnemyBase;
class BossBase;
class BottansFile;
class KeyFile;
struct size;

/// <summary>
/// タイトルシーンクラス
/// </summary>
class TitleScene : public Scene
{
public:
	TitleScene(GameManager& mgr, Input& input);
	~TitleScene();
	virtual void Update(Input& input);
	virtual void Draw() const;

private:
	// 更新関数
	void FadeInUpdate(Input&);	// フェードイン状態
	void NormalUpdate(Input&);	// 通常状態
	void StartSelectUpdate(Input&);	// 続きからor初めから
	void FadeOutUpdate(Input&);	// フェードアウト状態

	// 描画関数
	void FadeDraw() const;	// フェード中描画
	void NormalDraw() const;	// 非フェード描画
	void StartSelectDraw() const;	// スタート選択状態描画

	// 敵の生成関数
	void CreateEnemy();
	// ボスの生成関数
	void CreateBoss();

	/// <summary>
	/// ロゴの描画
	/// </summary>
	void DrawLogo() const;

	/// <summary>
	/// 選択等の文字・画像描画
	/// </summary>
	/// /// <param name="cmd">コマンド名</param>
	/// <param name="str">ウェーブさせる文字列</param>
	/// <param name="num">文字列数</param>
	void DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const;

	/// <summary>
	/// デモムービーの再生処理
	/// </summary>
	void PlayDemoMove(Input& input);

private:
	// 更新メンバ関数ポインタ
	void(TitleScene::* m_updateFunc)(Input& input);
	// 描画メンバ関数ポインタ
	using DrawFunc_t = void (TitleScene::*)() const;
	DrawFunc_t m_drawFunc;

	// 画像群
	std::shared_ptr<FileBase> m_logo;
	std::shared_ptr<FileBase> m_bg;
	std::shared_ptr<FileBase> m_frame;
	std::shared_ptr<FileBase> m_startFrame;
	std::shared_ptr<FileBase> m_backFrame;

	// 音関係
	std::shared_ptr<SoundSystem> m_soundSys;
	// Bgm
	std::shared_ptr<FileBase> m_bgm;
	// 選択時Se
	std::shared_ptr<FileBase> m_selectSe;
	std::shared_ptr<FileBase> m_cancelSe;
	std::shared_ptr<FileBase> m_cursorSe;

	// タイトルで動く敵
	std::list<std::shared_ptr<EnemyBase>> m_enemy;
	std::list<std::shared_ptr<BossBase>> m_boss;

	// タイトルで動く敵の生成フレーム
	int m_createEnemyFrame;
	int m_createBossFrame;
	// 敵の生成タイミング
	int m_createEnemyTiming;
	int m_createBossTiming;

	// 動画再生フレーム
	int m_playDemoMoveFrame;

	// 全体フレーム
	int m_fadeFrame;
	// メニュー選択カーソル位置
	int m_currentLinePos;

	// ロゴアングル
	float m_logoAngle;

	// 背景フレーム
	int m_bgFrame;

	// 文字ウェーブ用の角度
	float m_waveAngle;
	// ウエーブのやつ描画するか
	bool m_isWaveDraw;

	std::shared_ptr<BottansFile> m_bt;
	std::shared_ptr<KeyFile> m_key;

	const Input& m_input;
};


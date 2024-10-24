#pragma once
#include "Scene.h"
#include <memory>
#include <vector>

struct m_size;
class Player;
class EnemyBase;
class FileBase;

class GamePlayingScene : public Scene
{
public:
    GamePlayingScene(GameManager& mgr, Input& input);
    ~GamePlayingScene();
    virtual void Update(Input& input);
    virtual void Draw() const;

private:
    /* 更新処理 */
    /// <summary>
    /// フェードインするときの更新処理
    /// </summary>
    /// <param name="input">入力情報</param>
    void UpdateFadeIn(Input& input);
    /// <summary>
    /// フェードアウトするときの更新処理
    /// </summary>
    /// <param name="input">入力情報</param>
    void UpdateFadeOut(Input& input);
    /// <summary>
    /// 通常時の更新処理
    /// </summary>
    /// <param name="input">入力情報</param>
    void UpdateNormal(Input& input);

    /* 描画関数 */
    /// <summary>
    /// フェード時の描画
    /// </summary>
    void DrawFade() const;
    /// <summary>
    /// 通常時の描画
    /// </summary>
    void DrawNormal() const;

    void StartStage(Input& input);

private:
    // メンバ関数ポインタの宣言
    using UpdateFunc_t = void (GamePlayingScene::*)(Input&);
    using DrawFunc_t = void (GamePlayingScene::*)() const;

    UpdateFunc_t m_updateFunc;
    DrawFunc_t m_drawFunc;

    // ステージを動かすときの画面のハンドル
    int m_screenHandle;

    // 経過フレーム
    int m_frame;

    // ステージ内で多く使うデータ
    std::vector<std::shared_ptr<FileBase>> m_stgData;
    // BGM
    std::shared_ptr<FileBase> m_bgm;
};


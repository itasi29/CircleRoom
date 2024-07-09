#pragma once
#include "Scene.h"

class GameClearScene : public Scene
{
public:
    GameClearScene(GameManager& mgr);
    ~GameClearScene();
    virtual void Update(Input& input);
    virtual void Draw() const;

private:

    // 更新状態を表す関数
    void FadeInUpdate(Input&);
    void NormalUpdate(Input& input);
    void FadeOutUpdate(Input&);

    // 描画状態を表す関数
    void FadeDraw() const;
    void NormalDraw() const;

    void DrawInf(int index, int drawY, int handle) const;
private:
    // メンバ関数ポインタの宣言
    using UpdateFunc_t = void (GameClearScene::*)(Input&);
    using DrawFunc_t = void (GameClearScene::*)() const;

    UpdateFunc_t m_updateFunc;
    DrawFunc_t m_drawFunc;

    std::shared_ptr<FileBase> m_result;
    std::shared_ptr<FileBase> m_bgm;

    int m_frame;
    int m_textFrame;
    int m_index;
};


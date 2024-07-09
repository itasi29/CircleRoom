#include <DxLib.h>
#include <cassert>
#include <memory>

#include "Application.h"
#include "Input.h"
#include "GameManager.h"
#include "FileSystem/FileManager.h"

#include "Scene/SceneManager.h"
#include "Scene/TitleScene.h"

#ifdef _DEBUG
#include "Scene/DebugScene.h"
#endif


namespace 
{
    // 画面サイズ
    constexpr int kScreenWidth = 1280;
    constexpr int kScreenHeight = 720;

    // FPS固定値
    constexpr int kFpsFrame = 16667;
}

int MyLoadGraph(const wchar_t* path)
{
    int handle = LoadGraph(path);
    assert(handle != -1);
    return handle;
}

Application::Application() :
    m_time(0),
    m_isEnd(false),
#ifdef _DEBUG
    m_isWindows(true)
#else
    m_isWindows(false)
#endif
{
    m_size = size{kScreenWidth, kScreenHeight};
}

void Application::Terminate()
{
    DxLib_End();
}

bool Application::Init()
{
#ifdef _DEBUG
    ChangeWindowMode(true); // ウィンドウモードにします
#endif

    // ウィンドウモードを変更したときに画像が消去されないようにする
    SetChangeScreenModeGraphicsSystemResetFlag(false);

    SetWindowIconID(111);

    // 背景色の指定 5f6976
//    SetBackgroundColor(0x5f, 0x69, 0x76);

    SetGraphMode(m_size.w, m_size.h, 32);
    SetWindowText(L"CircleRoom");

    if (DxLib_Init() == -1)
    {
        return false;
    }
    SetDrawScreen(DX_SCREEN_BACK);

    return true;
}

void Application::Run()
{
    std::shared_ptr<FileManager> fileManager = std::make_shared<FileManager>();
    {
        GameManager& manager = GameManager::GetInstance();
        manager.SetFileManager(fileManager);
        Input input;

        manager.Init(input);

#ifdef _DEBUG
        manager.GetScene()->ChangeScene(std::make_shared<DebugScene>(manager));
#else
        manager.GetScene()->ChangeScene(std::make_shared<TitleScene>(manager, input));
#endif

        while (ProcessMessage() != -1)
        {
            // 新しいゲームループを始めた時間を記憶
            m_time = GetNowHiPerformanceCount();

            ClearDrawScreen();
            // 入力を更新
            input.Update();
            manager.GetScene()->Update(input);
            manager.GetScene()->Draw();


            ScreenFlip();

            // 終了フラグが立っていれば終了する
            if (m_isEnd)
            {
                break;
            }

            // FPSの固定化
            while (kFpsFrame > GetNowHiPerformanceCount() - m_time);
        }
    }
}

void Application::End()
{
    m_isEnd = true;
}

const size& Application::GetWindowSize() const
{
    return m_size;
}

void Application::ChangeWindows()
{
    m_isWindows = !m_isWindows;
    ChangeWindowMode(m_isWindows);
}

#include <DxLib.h>
#include <cassert>
#include "Input.h"
#include "Application.h"
#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "Stage/StageManager.h"
#include "Stage/GameData.h"
#include "FileSystem/SoundSystem.h"

#include "GamePlayingScene.h"
#include "GameClearScene.h"
#include "OptionScene.h"

#include "FileSystem/FileManager.h"
#include "FileSystem/ImageFile.h"


namespace
{
	enum class PathType
	{
		kGraph,
		kSound
	};

	struct PathData
	{
		std::wstring path;
		PathType type;
	};

	// フェードのフレーム時間
	constexpr int kFadeFrame = 60;

	// 事前に読み込んでおくデータのpath
	const std::vector<PathData> kPath =
	{
		//{L"Enemy/wallEffect.png", kGraph},
		{L"Player/blood.png", PathType::kGraph},
		{L"UI/backFrame.png", PathType::kGraph},
		{L"Bgm/provisionalBgm.mp3", PathType::kSound},
		{L"Bgm/fieldFight.mp3", PathType::kSound},
		{L"Bgm/boss.mp3", PathType::kSound}
	};

}

GamePlayingScene::GamePlayingScene(GameManager& mgr, Input& input) :
	Scene(mgr),
	m_screenHandle(0),
	m_frame(kFadeFrame)
{
	// メンバ関数ポインタの設定
	m_updateFunc = &GamePlayingScene::UpdateFadeIn;
	m_drawFunc = &GamePlayingScene::DrawFade;

	// ステージの設定
	// チュートリアルステージを通常ステージに
	if (m_mgr.GetStage()->GetData()->IsClearStage("練習"))
	{
		StartStage(input);
	}
	// していなければチュートリアルステージに
	else
	{
		m_mgr.GetStage()->ChangeStage("練習", true);
	}

	m_mgr.GetStage()->ResetClear();

	m_bgm = m_mgr.GetFile()->LoadSound(L"Bgm/provisionalBgm.mp3");

	// 事前にステージ内で多く使うものはここで読み込んでおく
	auto size = kPath.size();
	m_stgData.resize(size);
	for (int i = 0; i < size; i++)
	{
		if (kPath[i].type == PathType::kGraph)
		{
			m_stgData[i] = m_mgr.GetFile()->LoadGraphic(kPath[i].path);
		}
		else if (kPath[i].type == PathType::kSound)
		{
			m_stgData[i] = m_mgr.GetFile()->LoadSound(kPath[i].path);
		}
	}
}

GamePlayingScene::~GamePlayingScene()
{
}

void GamePlayingScene::Update(Input& input)
{
	(this->*m_updateFunc)(input);
}

void GamePlayingScene::Draw() const
{
	(this->*m_drawFunc)();
}

void GamePlayingScene::UpdateFadeIn(Input& input)
{
	m_sound->PlayFadeBgm(m_bgm->GetHandle(), 1.0f - m_frame / static_cast<float>(kFadeFrame));
	m_frame--;
	if (m_frame <= 0)
	{
		m_updateFunc = &GamePlayingScene::UpdateNormal;
		m_drawFunc = &GamePlayingScene::DrawNormal;
		m_frame = 0;
	}
}

void GamePlayingScene::UpdateFadeOut(Input& input)
{
	m_mgr.GetStage()->Update(input);
	m_frame++;
	if (m_frame > kFadeFrame)
	{
		m_sound->Stop();
		m_mgr.GetScene()->OnNormal();

		m_mgr.GetScene()->ChangeScene(std::make_shared<GameClearScene>(m_mgr));
	}
}

void GamePlayingScene::UpdateNormal(Input& input)
{
	// pauseボタンが押されたらポーズ画面を開く
	if (input.IsTriggered("pause"))
	{
		m_mgr.GetScene()->PushScene(std::make_shared<OptionScene>(m_mgr, input));
		m_mgr.GetStage()->OffDrawWave();
		return;
	}

	m_mgr.GetStage()->Update(input);
	// プレイ時間のアップデート
	m_mgr.UpdatePlaytime();

	// クリアしたら
	if (m_mgr.GetStage()->IsClear())
	{
		m_updateFunc = &GamePlayingScene::UpdateFadeOut;
		m_drawFunc = &GamePlayingScene::DrawFade;
		m_frame = 0;
	}
#if _DEBUG
	// 速攻終了（クリア画面への移行）
	if (CheckHitKey(KEY_INPUT_RSHIFT))
	{
		m_updateFunc = &GamePlayingScene::UpdateFadeOut;
		m_drawFunc = &GamePlayingScene::DrawFade;
		m_frame = 0;
	}
#endif
}

void GamePlayingScene::DrawFade() const
{
	m_mgr.GetStage()->Draw();

	const auto& m_size = Application::GetInstance().GetWindowSize();
	int alpha = static_cast<int>(255 * (static_cast<float>(m_frame) / 60.0f));
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, m_size.w, m_size.h, 0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GamePlayingScene::DrawNormal() const
{
	m_mgr.GetStage()->Draw();
}

void GamePlayingScene::StartStage(Input& input)
{
	const auto& nowStage = m_mgr.GetNowStage();
	std::string name = nowStage;
	
	if (nowStage == "")
	{
		// どこにも入らなかったら1-1に入るようにする
		name = "サークル";
	}

	m_mgr.GetStage()->ChangeStage(name, true);
	return;
}


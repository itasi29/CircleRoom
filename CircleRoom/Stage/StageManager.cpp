#include <DxLib.h>
#include <cassert>
#include <cmath>
#include "Application.h"

#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "Scene/SceneManager.h"
#include "Scene/OneShotScene.h"

#include "StageManager.h"
#include "Stage.h"
#include "StageMap.h"
#include "GameData.h"

namespace
{
	constexpr float kVersion = 1.3f;

	// ファイル用
	struct StageInfHeader
	{
		const char* const id = "sti"; // 最後に'\0'入ってるので4バイト
		float version = kVersion;
		size_t dataCount = 0;
	};

	// 移動スピード
	constexpr float kSpeed = 100.0f;

	// 書き換え認識距離
	constexpr float kRecognitionDis = 1280.0f;
	// ゆっくりになる距離
	constexpr float kSlowDis = 640.0f;

	// 下限速度
	constexpr float kLimitSlowSpeed = 10.0f;

	// ステージの縦横数
	constexpr int kLine = 4;
	constexpr int kRow = 3;

	// ステージ間の間隔
	constexpr int kStageMarginX = 1280;
	constexpr int kStageMarginY = 720;

	// 名前の場所
	const std::string kStName[kLine][kRow] =
	{
		{"ランナー", "Reaper", "分離"},
		{"近接遭遇", "切断", "発生"},
		{"巨壁", "サークル", "要警戒"},
		{"none", "練習", "none"}
	};
}

StageManager::StageManager(std::shared_ptr<SceneManager>& mgr) :
	m_mgr(mgr),
	m_size(Application::GetInstance().GetWindowSize()),
	m_isMove(false)
{
	m_map = std::make_shared<StageMap>();
	m_data = std::make_shared<GameData>();

	m_drawScreen = MakeScreen(m_size.w, m_size.h, true);
	m_screen = MakeScreen(m_size.w * kRow + kStageMarginX * kRow, m_size.h * kLine + kStageMarginY * kLine, true);
	m_keepScreen = MakeScreen(m_size.w * kRow + kStageMarginX * kRow, m_size.h * kLine + kStageMarginY * kLine, true);

	m_updateFunc = &StageManager::NormalUpdate;
	m_drawFunc = &StageManager::NormalDraw;
}

StageManager::~StageManager()
{
	DeleteGraph(m_drawScreen);
	DeleteGraph(m_screen);
	DeleteGraph(m_keepScreen);
}

void StageManager::Init(Input& input)
{
	m_data->Init();
	InitPos();
	m_stage = std::make_shared<Stage>(GameManager::GetInstance(), input, m_map);
}

void StageManager::InitPos()
{
	if (m_data->IsClearStage("練習"))
	{
		const auto& stageName = GameManager::GetInstance().GetNowStage();
		if (stageName == "")
		{
			m_pos = m_map->GetPos("サークル");
		}
		else
		{
			m_pos = m_map->GetPos(stageName);
		}
	}
	else
	{
		m_pos = m_map->GetPos("練習");
	}
	m_targetPos = {};
	m_vec = {};
}

void StageManager::Update(Input& input)
{
	(this->*m_updateFunc)(input);
}

void StageManager::Draw() const
{
	(this->*m_drawFunc)();
}

std::string StageManager::GetStageName() const
{
	return m_stage->GetStageName();
}

void StageManager::OffDrawWave() const
{
	m_stage->OffDrawWave();
}

void StageManager::ChangeStage(const std::string& nextStage, bool isGameScene)
{
	// 初回またはゲームシーンからのみ、ただ入れるだけ
	if (isGameScene)
	{
//		m_stage = nextStage;
		m_stage->ChangeStageData(nextStage);
		m_stage->Init();
		return;
	}

	Vec2 pos;

	float dis = (m_targetPos - m_pos).Length();

	if (!m_isMove || dis < kRecognitionDis)
	{
		// 描画用スクリーンに現在いるステージを描画
		SetDrawScreen(m_drawScreen);
		ClearDrawScreen();
		m_stage->Draw();

		// キープ用のスクリーンに場所を指定してさっきのを描画
		SetDrawScreen(m_keepScreen);
		ClearDrawScreen();
		pos = m_map->GetPos(m_stage->GetStageName());

		DrawGraph(static_cast<int>(pos.x), static_cast<int>(pos.y), m_drawScreen, true);

	}
	m_stage->ChangeStageData(nextStage);
	m_stage->Init();

	// 描画用スクリーンに次のステージの描画
	SetDrawScreen(m_drawScreen);
	ClearDrawScreen();
	m_stage->Draw();

	// 移動中描画スクリーンに場所を指定してさっきのを描画
	SetDrawScreen(m_screen);
	ClearDrawScreen();
	pos = m_map->GetPos(m_stage->GetStageName());

	DrawGraph(static_cast<int>(pos.x), static_cast<int>(pos.y), m_drawScreen, true);

	// キープにあるものを描画
	DrawGraph(0, 0, m_keepScreen, true);

	// 移動ベクトルの計算
	m_vec = (pos - m_pos).GetNormalized() * kSpeed;

	// 向かう先の場所の保存
	m_targetPos = pos;

	// 次のステージの保存
	m_data->SaveClearStage(nextStage);

	m_isMove = true;

	GameManager::GetInstance().UpdateNowStage(nextStage);

	// 現在の描画先へと戻す(本来は)
	SetDrawScreen(DX_SCREEN_BACK);

	// メンバ関数ポインタの更新
	m_updateFunc = &StageManager::StageMoveUpdate;
	m_drawFunc = &StageManager::StageMoveDraw;
}

void StageManager::ImmediatelyChange()
{
	if (!m_isMove) return;

	m_updateFunc = &StageManager::MoveGamePlaingUpdate;
	m_drawFunc = &StageManager::MoveGamePlaingDraw;
}

void StageManager::NormalUpdate(Input& input)
{
	m_stage->Update(input);
	m_stage->ChangeStage(input);
}

void StageManager::StageMoveUpdate(Input& input)
{
	m_stage->Update(input);
	m_stage->ChangeStage(input);

	// 場所の更新
	m_pos += m_vec;

	CheckEnd();
}

void StageManager::MoveGamePlaingUpdate(Input& input)
{
	m_stage->Update(input);

	m_pos += m_vec;

	CheckEnd();
}

void StageManager::NormalDraw() const
{
	m_stage->Draw();
}

void StageManager::StageMoveDraw() const
{
	// MEMO:画面上を動かすからマイナスにしておく
	DrawGraph(static_cast <int>(-m_pos.x), static_cast <int>(-m_pos.y), m_screen, true);

#ifdef _DEBUG
	// MEMO:デバッグなので未定数化
	DrawFormatString(100, 100, 0xffffff, L"dis:%.2f", (m_targetPos - m_pos).Length());
#endif
}

void StageManager::MoveGamePlaingDraw() const
{
	// 描画用スクリーンに次のステージの描画
	SetDrawScreen(m_drawScreen);
	ClearDrawScreen();
	m_stage->Draw();

	// 移動中描画スクリーンに場所を指定してさっきのを描画
	SetDrawScreen(m_screen);
	ClearDrawScreen();
	Vec2 pos = m_map->GetPos(m_stage->GetStageName());

	DrawGraph(static_cast<int>(pos.x), static_cast<int>(pos.y), m_drawScreen, true);

	// キープにあるものを描画
	DrawGraph(0, 0, m_keepScreen, true);

	SetDrawScreen(m_mgr->GetScreenHandle());

	DrawGraph(static_cast<int>(-m_pos.x), static_cast<int>(-m_pos.y), m_screen, true);
}

void StageManager::CheckEnd()
{
	// 距離の計算
	Vec2 vel = (m_targetPos - m_pos);
	float dis = vel.Length();

	// おそくする距離になったら
	if (dis < kSlowDis)
	{
		// 速度が0以下になったら移動終了
		if (dis < kLimitSlowSpeed)
		{
			m_updateFunc = &StageManager::NormalUpdate;
			m_drawFunc = &StageManager::NormalDraw;

			m_pos = m_targetPos;

			m_isMove = false;
		}

		// スピードの計算
		float speed = (dis / kSlowDis) * kSpeed;
		// スピードの調整
		m_vec = vel.GetNormalized() * speed;
	}
}

#include <DxLib.h>
#include <cassert>
#include <sstream>

#include "StringUtility.h"
#include "Vec2.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "Stage/StageManager.h"
#include "Stage/GameData.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/BottansFile.h"
#include "FileSystem/KeyFile.h"
#include "Input.h"

#include "Stage/Stage.h"
#include "StageSelectScene.h"

namespace
{
	constexpr int kAppeaInterval = 5;
	constexpr int kMenuMargin = 120;

	// 通常文字列の色
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	// 強調文字列の色
	constexpr unsigned int kYellowColor = 0xffde00;

	constexpr int kStageNum = 5;

	// ステージ枠の大きさ
	constexpr int kStageSize = 64;
	// フレームの太さ
	constexpr int kStageFrameThickness = 4;
	// ステージ間の空白
	constexpr int kStageMargine = 128;

	// スタート位置
	constexpr int kStartX = 512;
	constexpr int kStartY = 256;

	// 情報文字列描画位置
	constexpr int kDrawStringX = 200;
	constexpr int kDrawStringY = 216;

	// ステージの縦横の数
	constexpr int kLineNum = 3;
	constexpr int kRowNum = 3;
	// ステージ名簿
	const std::string kStageName[kLineNum][kRowNum] =
	{
		{"ランナー", "Reaper", "分離"},
		{"近接遭遇", "切断", "発生"},
		{"巨壁", "サークル", "要警戒"}
	};
	const std::wstring kFileName[kLineNum][kRowNum] = {
		{ L"Stage1-5", L"StageBoss", L"Stage1-7" },
		{ L"Stage1-3", L"Stage1-4", L"Stage1-6" },
		{ L"Stage1-2", L"Stage1-1", L"Master"}
	};

	// スタート文字のウェーブスピード
	constexpr float kWaveSpeed = DX_PI_F / 180 * 5;
	// ウェーブの間隔
	constexpr float kWaveInterval = DX_PI_F / 15.0f;

	// ウェーブ文字列
	int kSelectWaveNum = 4;
	int kSelectWavePosX = 1064;
	int kSelectWavePosY = 592;
	const wchar_t* const kSelectWave[] = { L"け", L"っ", L"て", L"い" };
}

StageSelectScene::StageSelectScene(GameManager& mgr, Input& input) :
	Scene(mgr),
	m_indexRow(0),
	m_indexLine(0),
	m_fadeFrame(0),
	m_waveAngle(0.0),
	m_input(input)
{
	CurrosrPos();

	m_soundSys = mgr.GetSound();

	auto& file = mgr.GetFile();
	m_frame = file->LoadGraphic(L"UI/selectFrame.png");
	m_nowPos = file->LoadGraphic(L"UI/nowPos.png");
	m_lock = file->LoadGraphic(L"UI/lock.png");
	m_startFrame = file->LoadGraphic(L"UI/startFrame.png");
	
	for (int x = 0; x < kRowNum; x++)
	{
		for (int y = 0; y < kLineNum; y++)
		{
			const auto& stgName = kStageName[y][x];
			const auto& fileName = kFileName[y][x];
			std::wostringstream oss1;
			oss1 << L"UI/" << fileName << L".png";
			m_stage[stgName][0] = file->LoadGraphic(oss1.str());

			std::wostringstream oss2;
			oss2 << L"UI/n" << fileName << L".png";
			m_stage[stgName][1] = file->LoadGraphic(oss2.str());
		}
	}

	m_selectSe = file->LoadSound(L"Se/select.mp3");

	m_bt = std::make_shared<BottansFile>(file);
	m_key = std::make_shared<KeyFile>(file);
}

StageSelectScene::~StageSelectScene()
{
}

void StageSelectScene::Update(Input& input)
{
	m_waveAngle -= kWaveSpeed;
	m_fadeFrame++;

	// MEMO:展示会用コード
	if (CheckHitKey(KEY_INPUT_M))
	{
		const auto& stage = m_mgr.GetStage()->GetData();
		for (const auto& name : m_stageData)
		{
			stage->SaveClearStage(name.first);
		}
	}

	// 上下のインデックスの変更
	if (input.IsTriggered("up"))
	{
		m_indexLine = (m_indexLine - 1 + kLineNum) % kLineNum;
		m_fadeFrame = 0;
	}
	if (input.IsTriggered("down"))
	{
		m_indexLine = (m_indexLine + 1) % kLineNum;
		m_fadeFrame = 0;
	}

	// 左右のインデックスの変更
	if (input.IsTriggered("left"))
	{
		m_indexRow = (m_indexRow - 1 + kRowNum) % kRowNum;
		m_fadeFrame = 0;
	}
	if (input.IsTriggered("right"))
	{
		m_indexRow = (m_indexRow + 1) % kRowNum;
		m_fadeFrame = 0;
	}

	if (input.IsTriggered("OK"))
	{
		// ステージがないなら終了
		if (kStageName[m_indexLine][m_indexRow] == "None") return;
		auto stgName = kStageName[m_indexLine][m_indexRow];

		// そのステージがクリアされていなければ終了
		if (!m_mgr.GetStage()->GetData()->IsClearStage(stgName)) return;

		// 止めないと音が二重になる
		m_soundSys->Stop();

		m_soundSys->PlaySe(m_selectSe->GetHandle());

		m_mgr.GetStage()->ChangeStage(stgName);

		m_mgr.GetScene()->PopScene();
	}
}

void StageSelectScene::Draw() const
{
	for(int x = 0; x < kRowNum; x++)
	{
		int drawX = static_cast<int>(kStageMargine * x) + kStartX;
		bool isIndexRow = (m_indexRow == x);

		for (int y = 0; y < kLineNum; y++)
		{
			int drawY = static_cast<int>(kStartY + kStageMargine * y);
			// 現在選択しているステージ
			const std::string& stageName = kStageName[y][x];
			// フレームの描画
			if (isIndexRow && m_indexLine == y)
			{
				int frame = (m_fadeFrame % 80) - 40;
				float rate = fabsf(static_cast<float>(frame)) / 40.0f;
				int alpha = static_cast<int>(255 * rate);
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
				DrawRotaGraph(drawX, drawY, 1.0, 0.0, m_frame->GetHandle(), true);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);


				// 情報の描画
				DrawInf(stageName);
			}

			// ステージの描画が無しの場合は以下のは描画しない
			if (kStageName[y][x] == "None") continue;



			// ステージの描画
			if (m_mgr.GetStage()->GetData()->IsClearStage(stageName))
			{
				DrawRotaGraph(drawX, drawY, 1.0, 0.0, m_stage.at(stageName)[0]->GetHandle(), true);
			}
			else
			{
				DrawRotaGraph(drawX, drawY, 1.0, 0.0, m_stage.at(stageName)[1]->GetHandle(), true);
				DrawGraph(drawX + 16, drawY + 12, m_lock->GetHandle(), true);
			}

			// 現在いるステージの描画
			if ((stageName) == m_mgr.GetStage()->GetStageName())
			{
				DrawRotaGraph(static_cast<int>(drawX), static_cast<int>(drawY),
					1.0, 0.0, m_nowPos->GetHandle(), true);

			}
		}
	}

	DrawWave(kSelectWavePosX, kSelectWavePosY, "OK", kSelectWave, kSelectWaveNum);
}

void StageSelectScene::DrawInf(const std::string& str) const
{
	int font = m_mgr.GetFont()->GetHandle(32);

	const auto& wstr = StringUtility::StringToWString(str);

	int y = kDrawStringY;

	// ステージ名の描画
	DrawStringToHandle(kDrawStringX, y, wstr.c_str(), kWhiteColor, font);
	// ベストタイムの描画
	y += 96;
	DrawStringToHandle(kDrawStringX, y, L"ベストタイム", kWhiteColor, font);
	y += 48;

	int bestTime = m_mgr.GetStage()->GetData()->GetBestTime(str);
	int minSec = (bestTime * 1000 / 60) % 1000;
	int sec = (bestTime / 60) % 60;
	int min = bestTime / 3600;
	DrawFormatStringToHandle(kDrawStringX, y, kYellowColor, font, L"%02d:%02d.%03d", min, sec, minSec);

	y += 48;

	// 殺された種類の描画
	m_stageData.at(str)->DrawEnemyKilledInfo(kDrawStringX, y);

}

void StageSelectScene::DrawWave(int x, int y, const char* const cmd, const wchar_t* const str[], int num) const
{
	DrawGraph(x - 84, y - 5, m_startFrame->GetHandle(), true);

	const auto& type = m_input.GetType();
	if (type == InputType::keybd)
	{
		m_key->DrawKey(m_input.GetHardDataName(cmd, InputType::keybd), x - 48, y, 2.0);
	}
	else if (type == InputType::pad)
	{
		m_bt->DrawBottan(m_input.GetHardDataName(cmd, InputType::pad), x - 48, y, 2.0);
	}

	int handle = m_mgr.GetFont()->GetHandle(32);

	int strX = x;

	for (int i = 0; i < num; i++)
	{
		int add = static_cast<int>(sinf(m_waveAngle + kWaveInterval * i) * -10);

		if (add > 0)
		{
			add = 0;
		}

		int strY = y + add;


		DrawStringToHandle(strX, strY, str[i], kWhiteColor, handle);
		strX += 24;
	}
}

void StageSelectScene::CurrosrPos()
{
	const auto& nowStage = m_mgr.GetNowStage();

	for (int x = 0; x < kRowNum; x++)
	{
		for (int y = 0; y < kLineNum; y++)
		{
			if (nowStage == kStageName[y][x])
			{
				m_indexRow = x;
				m_indexLine = y;

				return;
			}
		}
	}
}

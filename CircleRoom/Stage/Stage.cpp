#include <DxLib.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include "Application.h"
#include "Input.h"
#include "StringUtility.h"

#include "Stage.h"
#include "StageMap.h"
#include "StageManager.h"
#include "GameData.h"
#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "FileSystem/FontSystem.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/BottansFile.h"
#include "FileSystem/KeyFile.h"

#include "Player/Player.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/EnemyMoveWall.h"
#include "Enemy/EnemyNormal.h"
#include "Enemy/EnemyLarge.h"
#include "Enemy/EnemyDash.h"
#include "Enemy/EnemyCreate.h"
#include "Enemy/EnemyDivision.h"
#include "Boss/BossBase.h"

#include "Boss/BossArmored.h"
#include "Boss/BossStrongArmored.h"
#include "Scene/OneShotScene.h"

namespace
{
	// ステージデータインデックス番号
	enum Index : int
	{
		kStageName,
		kEnemyTypeNum,
		kEnemyName,
		kEnemyInfoNum,
		kEnemyStartCreatNm,
		kEnemyStartCreateInterval,
		kEnmeyStartDelayFrame,
		kEnemyCreateInterval,
		kIsCreateBoss,
		kUpdateTimeType,
		kAddTimeFrame,
		kInfoType,
		kInfo,
		kInfoGroupNum,
		kInfoGroup
	};
	// 説明インデックス番号
	constexpr int kIndexBossExplanation = 5;
	// チュートリアル説明総数
	constexpr int kExplanationNum = 5;

	// フィールドサイズの倍率　(半分の大きさ)
	constexpr float kSizeScale = 0.4f;

	// プレイヤー死亡時の画面の揺れフレーム
	constexpr int kPlayerDeathShakeFrame = 10;

	// サウンドのフェードフレーム
	constexpr int kSoundFade = 30;

	// 色定数
	constexpr unsigned int kFrameColor = 0xd80032;
	constexpr unsigned int kBackFrameColor = 0x161a30;
	constexpr unsigned int kWhiteColor = 0xf0ece5;
	constexpr unsigned int kYellowColor = 0xffde00;
	constexpr unsigned int kRedColor = 0xd2001a;

	// 矢印画像定数
	constexpr int kArrowFlashInterval = 20;
	constexpr double kRad90 = DX_PI / 2;

	// ウェーブ文字定数
	constexpr float kWaveSpeed = DX_PI_F / 180 * 5;
	constexpr float kWaveInterval = DX_PI_F / 15.0f;
	int kTitleWaveNum = 4;
	const wchar_t* const kTitleWave[] = {L"ス", L"タ", L"ー", L"ト"};
	int kDashWaveNum = 4;
	const wchar_t* const kDashWave[] = {L"ダ", L"ッ", L"シ", L"ュ"};

	// 拡大文字定数
	constexpr float kExtRateSize = 2.0f;
	constexpr int kExtRateFrame = 45;
	constexpr int kWaitExtRateFrame = 30;
	constexpr int kExtRateAlpha = 224;
	constexpr int kExtShiftWidth = 256;
	constexpr int kExtShiftRight = 320;
	constexpr int kExtShiftUp = 360;

	// ステージ名座標定数
	constexpr int kNamePosX = 64;
	constexpr int kNamePosY = 32;
	constexpr int kNameFramePosX = 0;
	constexpr int kNameFramePosY = 24;
	constexpr int kNameFrameWidth = 288;
	constexpr int kNameFrameHeight = 80;

	// 時間座標定数
	constexpr int kTimePosX = 20;
	constexpr int kTimePosY = 160;
	constexpr int kTimePlayingDiff = -16;
	constexpr int kTimeAddY = 24;

	// 敵種類定数
	constexpr int kKilledStrPosX = 136;
	constexpr int kKilledStrPosY = 160;
	constexpr int kKillTypePosX = 156;
	constexpr int kKillTypePosY = 200;
	constexpr int kKillTypeAlpha = 192;
	constexpr double kKillTypeDefExtRate = 0.372;
	constexpr double kKillTypeLargeExtRate = 0.6;
	const std::vector<std::string> kLargeTypeName = {
		"Child",
		"Split",
		"SplitTwoBound"
	};
	constexpr double kKillTypeSmallExtRate = 0.25;
	const std::vector<std::string> kSmallTypeName = {
		"Large",
		"BossArmored",
		"BossStrongArmored"
	};

	// 情報定数
	constexpr int kInfoFramePosX = 155;
	constexpr int kInfoFramePosY = 168;
	constexpr int kInfoBoxPosX = 0;
	constexpr int kInfoBoxPosY = 198;
	constexpr int kInfoBoxWidth = 310;
	constexpr int kInfoBoxHeight = 30;
	constexpr int kInfoBoxPlayingDiff = 6;

	// 条件定数
	constexpr int kConditionStrPosX = 20;
	constexpr int kConditionPlayingDiff = 24;
	constexpr int kConditionPosY = 256;
	constexpr int kConditionBoxPosX = 0;
	constexpr int kConditionBoxPosY = 244;
	constexpr int kConditionBoxWidth = 310;

	// 条件側矢印定数
	constexpr int kArrowSize = 28;
	// ステージ側矢印定数
	constexpr int kArrowShiftPos = 150;

	// １秒のフレーム数
	constexpr int kFrameToSec = 60;

	// ベストタイム座標定数
	constexpr int kBestTimeFrameSubX = 128;
	constexpr int kBestTimeFramePosY = 128;
	constexpr int kBestTimeFrameBoxPosY = 158;
	constexpr int kBestTimeFrameBoxSizeW = 155;
	constexpr int kBestTimeFrameBoxSizeH = 66;
	constexpr int kBestTimeStrSubX = 256;
	constexpr int kBestTimeStrPosY = 112;
	constexpr int kBestTimeStrAddY = 48;
	constexpr int kBestTimeFlashInterval = 30;
	constexpr int kBestTimeBoxPosX = 344;
	constexpr int kBestTimeBoxPosY = 248;
	constexpr int kBestTimeBoxHeight = 64;
	constexpr int kBestTimeUpStrPosX = 336;
	constexpr int kBestTimeUpStrPosY = 256;

	// 達成文字描画定数
	constexpr int kAchivedDrawFrame = 120;
	constexpr int kAchivedStrShiftPosY = 344;
	constexpr int kAchivedStrPosX = 12;
	constexpr int kAchivedFrameWidth = 352;
	constexpr int kAchivedFrameHeight = 68;
	constexpr int kAchivedStrAdd = 64;
	constexpr int kAchivedFrameDiff = 4;

	// ウェーブ文字描画定数
	constexpr int kWaveFramePosX = 980;
	constexpr int kWaveFramePosY = 595;
	constexpr int kWaveStrPosX = 1064;
	constexpr int kWaveStrPosY = 600;
	constexpr int kWaveStrAdd = 24;
	constexpr int kWaveImgPosX = 1016;
	constexpr int kWaveImgPosY = 600;

	/* チュートリアル関連定数 */
	// 自機指し矢印
	constexpr int kEmphasisFrame = 60 * 5;
	constexpr int kStartAlphaEmphasisFrame = 60 * 4;
	constexpr int kEmphasisInterval = 20;
	constexpr float kEmphasisAngle = DX_PI_F / 180 * 8;
}

Stage::Stage(GameManager& mgr, Input& input, std::shared_ptr<StageMap> map) :
	m_mgr(mgr),
	m_soundSys(m_mgr.GetSound()),
	m_input(input),
	m_size(Application::GetInstance().GetWindowSize()),
	m_fieldSize(m_size.h* kSizeScale),
	m_centerPos({m_size.w * 0.5f, m_size.h * 0.5f}),
	m_stageName({}),
	m_achived({}),
	m_soundFrame(kSoundFade),
	m_timeFrame(0),
	m_waitFrame(kWaitChangeFrame),
	m_extRateFrame(0),
	m_isUpdateBestTime(false),
	m_isExtRate(true),
	m_isWaveDraw(true),
	m_waveAngle(DX_PI_F),
	m_explanationIndex(0),
	m_emphasisFrame(0),
	m_map(map)
{
	m_updateFunc = &Stage::UpdateSelect;
	m_drawFunc = &Stage::DrawSelect;

	// ステージ情報読み込み
	LoadStageInfo();

	// 画面の作成
	m_strScreen = MakeScreen(m_size.w, m_size.h, true);
	m_extScreen = MakeScreen(m_size.w, m_size.h, true);

	// 読み込み
	auto& file = m_mgr.GetFile();
	// 画像
	m_fieldImg = file->LoadGraphic(L"field.png");
	m_arrowImg = file->LoadGraphic(L"UI/arrow.png");
	m_arrowFlashImg = file->LoadGraphic(L"UI/arrowFlash.png");
	m_arrowNoImg = file->LoadGraphic(L"UI/arrowNo.png");
	m_arrowLockImg = file->LoadGraphic(L"UI/lock.png");
	m_arrowConditionsImg = file->LoadGraphic(L"UI/arrowConditions.png");
	m_frameImg = file->LoadGraphic(L"UI/startFrame.png");
	m_backFrameImg = file->LoadGraphic(L"UI/backFrame.png");
	m_enemysImg["Normal"] = file->LoadGraphic(L"Enemy/Normal.png");
	m_enemysImg["MoveWall"] = file->LoadGraphic(L"Enemy/Wall.png");
	m_enemysImg["Large"] = file->LoadGraphic(L"Enemy/Large.png");
	m_enemysImg["Division"] = file->LoadGraphic(L"Enemy/Division.png");
	m_enemysImg["Dash"] = file->LoadGraphic(L"Enemy/Dash.png");
	m_enemysImg["Create"] = file->LoadGraphic(L"Enemy/Create.png");
	m_enemysImg["Child"] = file->LoadGraphic(L"Enemy/Child.png");
	m_enemysImg["Split"] = file->LoadGraphic(L"Enemy/Split.png");
	m_enemysImg["SplitTwoBound"] = file->LoadGraphic(L"Enemy/SplitTwoBound.png");
	m_enemysImg["BossArmored"] = file->LoadGraphic(L"Enemy/BossArmored.png");
	m_enemysImg["BossStrongArmored"] = file->LoadGraphic(L"Enemy/BossStrongArmored.png");
	m_checkImg = file->LoadGraphic(L"UI/check.png");
	m_explanation.push_back(file->LoadGraphic(L"UI/operationExplanation.png"));
	m_explanation.push_back(file->LoadGraphic(L"UI/explanation0.png"));
	m_explanation.push_back(file->LoadGraphic(L"UI/explanation1.png"));
	m_explanation.push_back(file->LoadGraphic(L"UI/explanation2.png"));
	m_explanation.push_back(file->LoadGraphic(L"UI/explanation3.png"));
	m_explanation.push_back(file->LoadGraphic(L"UI/bossExplanation.png"));
	m_emphasisArrow.push_back(file->LoadGraphic(L"UI/playerEmphasis0.png"));
	m_emphasisArrow.push_back(file->LoadGraphic(L"UI/playerEmphasis1.png"));
	// BGM
	m_selectBgm = file->LoadSound(L"Bgm/provisionalBgm.mp3");
	m_playBgm = file->LoadSound(L"Bgm/fieldFight.mp3");
	// SE
	m_clearSe = file->LoadSound(L"Se/clear.mp3");
	// キー・ボタン
	m_bt = std::make_shared<BottansFile>(file);
	m_key = std::make_shared<KeyFile>(file);

	// プレイヤー生成
	m_player = std::make_shared<Player>(m_size, m_fieldSize);
}

Stage::~Stage()
{
	DeleteGraph(m_strScreen);
	DeleteGraph(m_extScreen);
}

void Stage::Init()
{
	auto& data = m_stageData[m_stageName];

	// 敵の総数を確認
	m_enemyNum = data.enemyNum;
	int temp = 0;
	for (int i = 0; i < m_enemyNum; i++)
	{
		for (int j = 1; j < data.enemyInfo[i].num; j++)
		{
			temp++;
		}
	}
	m_enemyNum += temp;
	// サイズ更新
	m_enemyStarCreateNum.resize(m_enemyNum);
	m_enemyCreateFrame.resize(m_enemyNum);

	// クリア情報更新
	CheckStageConditions(m_mgr.GetStage()->GetData()->GetBestTime(m_stageName));
	// 進行方向のクリアチェック
#if false
	for (const auto& stage : data.stageInfo)
	{
		bool isCreal = m_mgr.GetStage()->IsClearStage(stage.name);

		m_isClear[stage.dir] = isCreal;
	}
#else
	for (int i = 0; i < static_cast<int>(MapDir::kMax); i++)
	{
		const MapDir& dir = static_cast<MapDir>(i);

		// 接続してなかったら次へ
		if (!m_map->IsConect(m_stageName, dir)) continue;

		bool isCreal = m_mgr.GetStage()->GetData()->IsClearStage(m_map->GetConectName(m_stageName, dir));

		m_isClear[dir] = isCreal;
	}
#endif

	// BGM変更
	auto& file = m_mgr.GetFile();
	if (m_stageData[m_stageName].isBoss)
	{
		m_playBgm = file->LoadSound(L"Bgm/boss.mp3");
	}
	else
	{
		m_playBgm = file->LoadSound(L"Bgm/fieldFight.mp3");
	}
}

void Stage::Update(Input& input)
{
	// ウェーブ文字更新
	m_isWaveDraw = true;
	m_waveAngle -= kWaveSpeed;
	(this->*m_updateFunc)(input);
}

void Stage::Draw() const
{
	(this->*m_drawFunc)();
}

void Stage::GenericEnemy(const std::shared_ptr<EnemyBase>& enemy)
{
	m_backEnemy.push_back(enemy);
}

void Stage::UpTime()
{
	m_timeFrame += m_stageData[m_stageName].addTimeFrame;
}

void Stage::DrawEnemyKilledInfo(int x, int y) const
{
	int addX = 0;
	const auto& data = m_stageData.at(m_stageName);
	int bossNum = 0;

	for (const auto& enemy : data.enemyInfo)
	{
		DrawKilledEnemy(enemy.name, x, y, addX);

		addX += 36;

		if (enemy.name == "Create")
		{
			addX += 2;
			DrawKilledEnemy("Child", x, y, addX, 12);
			addX += 36 + 2;
		}
		else if (enemy.name == "Division")
		{
			addX += 2;
			DrawKilledEnemy("Split", x, y, addX, 12);
			addX += 36 + 2;
		}
		else if (enemy.name == "Large")
		{
			addX += 4;
		}
		else if (data.isBoss)
		{
			addX += 6;
			DrawKilledEnemy("BossArmored", x, y, addX);
			addX += 36 + 6;
			DrawKilledEnemy("BossStrongArmored", x, y, addX);
			addX += 36 + 6;
			DrawKilledEnemy("SplitTwoBound", x, y, addX, 12);
			addX += 36 + 2;
		}
	}
}

void Stage::ChangeStage(Input& input)
{
	// プレイヤーが生存している間は変わらないようにする
	if (m_player->IsExsit()) return;

	// 死亡直後は変わらないようにする
	if (m_waitFrame < kWaitChangeFrame) return;

	const auto& data = m_mgr.GetStage()->GetData();

	for (int i = 0; i < static_cast<int>(MapDir::kMax); i++)
	{
		const MapDir& dir = static_cast<MapDir>(i);

		// 接続してなかったら次へ
		if (!m_map->IsConect(m_stageName, dir)) continue;

		const auto& conectName = m_map->GetConectName(m_stageName, dir);

		// 接続ステージをクリアしてなかったら次へ
		if (!data->IsClearStage(conectName)) continue;

		if (dir == MapDir::kUp && input.IsTriggered("up"))
		{
			m_mgr.GetStage()->ChangeStage(conectName);
		}
		else if (dir == MapDir::kDown && input.IsTriggered("down"))
		{
			m_mgr.GetStage()->ChangeStage(conectName);
		}
		else if (dir == MapDir::kRight && input.IsTriggered("right"))
		{
			m_mgr.GetStage()->ChangeStage(conectName);
		}
		else if (dir == MapDir::kLeft && input.IsTriggered("left"))
		{
			m_mgr.GetStage()->ChangeStage(conectName);
		}
	}
}

void Stage::ChangeStageData(const std::string& name)
{ 
	// 名前の変更
	m_stageName = name;

	// 敵の削除
	m_enemy.clear();
	m_frontEnemy.clear();
	m_backEnemy.clear();
	m_boss = nullptr;

	// プレイヤー透明化
	m_player->OnInvisible();
}

/// <summary>
/// 選択中の更新処理
/// </summary>
/// <param name="input">入力情報</param>
void Stage::UpdateSelect(Input& input)
{
	if (m_soundFrame > kSoundFade)
	{
		m_soundSys->PlayBgm(m_selectBgm->GetHandle());
	}
	else
	{
		m_soundFrame++;
		m_soundSys->PlayFadeBgm(m_selectBgm->GetHandle(), m_soundFrame / static_cast<float>(kSoundFade));
	}

	m_waitFrame++;

	// プレイヤー更新
	m_player->Update(input, Ability::kNone);
	// 敵更新
	for (const auto& enemy : m_backEnemy)
	{
		enemy->Update();

	}
	for (const auto& enemy : m_enemy)
	{
		enemy->Update();
	}
	for (const auto& enemy : m_frontEnemy)
	{
		enemy->Update();
	}
	// 死亡した敵の処理
	m_backEnemy.remove_if(
		[](const auto& enemy)
		{
			return !enemy->IsExsit();
		});
	m_enemy.remove_if(
		[](const auto& enemy)
		{
			return !enemy->IsExsit();
		});
	m_frontEnemy.remove_if(
		[](const auto& enemy)
		{
			return !enemy->IsExsit();
		});
	// ボス更新
	if (m_boss)
	{
		m_boss->Update();
	}

	// 条件達成文字更新
	for (auto& achived : m_achived)
	{
		achived.frame++;
	}
	m_achived.remove_if(
		[](const auto& achived)
		{
			return achived.frame > kAchivedDrawFrame;
		}
	);

	
	if (UpdateTutorial()) return;

	if (input.IsTriggered("OK"))
	{
		// 移動中であっても即時移動
		m_mgr.GetStage()->ImmediatelyChange();

		// BGM変更
		m_soundFrame = 0;
		m_soundSys->Stop(m_selectBgm->GetHandle());
		m_soundSys->PlayFadeBgm(m_playBgm->GetHandle(), 0);

		PlayStart();

		m_updateFunc = &Stage::UpdatePlaying;
		m_drawFunc = &Stage::DrawPlaying;
	}

	// アビリティ変更
	m_mgr.GetStage()->GetData()->ChangeAbility(Ability::kDash);
}

/// <summary>
/// プレイ中の更新処理
/// </summary>
/// <param name="input">入力情報</param>
void Stage::UpdatePlaying(Input& input)
{
	if (m_soundFrame > kSoundFade)
	{
		m_soundSys->PlayBgm(m_playBgm->GetHandle());
	}
	else
	{
		m_soundFrame++;
		m_soundSys->PlayFadeBgm(m_playBgm->GetHandle(), m_soundFrame / static_cast<float>(kSoundFade));
	}

	const auto& data = m_mgr.GetStage()->GetData();

	m_emphasisFrame++;
	m_extRateFrame++;
	if (m_isUpdateBestTime)
	{
		// ベストタイムの常時更新
		m_waitFrame++;
		data->SaveBestTime(m_stageName, m_timeFrame);
	}
	else if (data->SaveBestTime(m_stageName, m_timeFrame))
	{
		m_isUpdateBestTime = true;
	}

	// プレイヤー更新
	m_player->Update(input, data->GetAbility());
	// プレイヤーの情報を抜き取る
	bool playerIsDash = m_player->IsDash();
	bool playerIsExsit = m_player->IsExsit();
	const Collision& playerCol = m_player->GetRect();
#ifdef _DEBUG
	// デバッグ用無敵モード
	if (CheckHitKey(KEY_INPUT_M))
	{
		playerIsDash = true;
	}
#endif

	// 敵生成
	CreateEnemy();
	// 敵更新
	UpdateEnemy(m_frontEnemy, playerIsDash, playerCol);
	UpdateEnemy(m_enemy, playerIsDash, playerCol);
	UpdateEnemy(m_backEnemy, playerIsDash, playerCol);

	if (m_boss)
	{
		// ボス更新
		m_boss->Update();
		m_boss->OnAttack(playerIsDash, m_player->GetObjRect());

		if (!m_boss->IsExsit())
		{
			DeathBoss();
		}
		else if (!playerIsDash && playerCol.IsCollsion(m_boss->GetRect()))
		{
			m_player->Death();

			m_mgr.UpdateDeathCcount();
			data->SaveKilledEnemyType(m_boss->GetName());
			m_mgr.GetScene()->ShakeScreen(kPlayerDeathShakeFrame);
		}
	}

#ifdef _DEBUG
	// デバッグ用として高速で時間が進むように
	if (CheckHitKey(KEY_INPUT_U))
	{
		for (int i = 0; i < 60; i++)
		{
			UpdateTime();
		}
	}
#endif
	UpdateTime();
	CheckStageConditions(m_timeFrame);

	if (!m_player->IsExsit())
	{
		// BGM変更
		m_soundFrame = 0;
		m_soundSys->Stop(m_playBgm->GetHandle());
		m_soundSys->PlayFadeBgm(m_selectBgm->GetHandle(), 0);

		m_waitFrame = 0;
		m_waveAngle = 0.0;
		m_isExtRate = false;

		// ベストタイムの更新
		if (data->SaveBestTime(m_stageName, m_timeFrame))
		{
			m_isUpdateBestTime = true;
		}

		m_updateFunc = &Stage::UpdateSelect;
		m_drawFunc = &Stage::DrawSelect;

		return;
	}

	DrawTutrial();
}

/// <summary>
/// ボス倒した後の更新処理
/// </summary>
/// <param name="input">入力情報</param>
void Stage::UpdateAfterBossDeath(Input& input)
{
	if (m_soundFrame > kSoundFade)
	{
		m_soundSys->PlayBgm(m_playBgm->GetHandle());
	}
	else
	{
		m_soundFrame++;
		m_soundSys->PlayFadeBgm(m_playBgm->GetHandle(), m_soundFrame / static_cast<float>(kSoundFade));
	}

	m_boss->Update();

	if (m_boss->IsEndPerformance())
	{
		m_mgr.GetStage()->OnClear();
	}
}

/// <summary>
/// 選択中の描画処理
/// </summary>
void Stage::DrawSelect() const
{
	DrawWall();

	SetDrawMode(DX_DRAWMODE_BILINEAR);
	for (const auto& enemy : m_backEnemy)
	{
		enemy->Draw();
	}
	for (const auto& enemy : m_enemy)
	{
		enemy->Draw();
	}
	for (const auto& enemy : m_frontEnemy)
	{
		enemy->Draw();
	}	
	if (m_boss)
	{
		m_boss->Draw();
	}
	SetDrawMode(DX_DRAWMODE_NEAREST);
	m_player->Draw();

	int drawScreenHandle = m_mgr.GetScene()->GetScreenHandle();

	// ステージ名の描画
	const auto& name = StringUtility::StringToWString(m_stageName);
	DrawBox(kNameFramePosX, kNameFramePosY, kNameFramePosX + kNameFrameWidth, kNameFramePosY + kNameFrameHeight, kFrameColor, true);
	DrawFormatStringToHandle(kNamePosX, kNamePosY, kYellowColor, m_mgr.GetFont()->GetHandle(64), L"%s", name.c_str());

	// ステージ情報の描画
	SetDrawScreen(m_strScreen);
	ClearDrawScreen();
	// 時間
	DrawTime(kTimePosX, kTimePosY, m_mgr.GetFont()->GetHandle(32));
	// 種類
	DrawStringToHandle(kKilledStrPosX, kKilledStrPosY, L"> Circle", kWhiteColor, m_mgr.GetFont()->GetHandle(24));
	DrawEnemyKilledInfo(kKillTypePosX, kKillTypePosY);
	SetDrawScreen(drawScreenHandle);
	// 背景フレーム
	DrawRotaGraph(kInfoFramePosX, kInfoFramePosY, 1.0, 0.0, m_backFrameImg->GetHandle(), true, false, true);
	DrawBox(kInfoBoxPosX, kInfoBoxPosY, kInfoBoxPosX + kInfoBoxWidth, kInfoBoxPosY + kInfoBoxHeight, kBackFrameColor, true);
	DrawGraph(0, 0, m_strScreen, true);

	// ステージ条件の描画
	SetDrawScreen(m_strScreen);
	ClearDrawScreen();
	auto y = DrawStageConditions(kConditionPosY);
	SetDrawScreen(drawScreenHandle);
	// Y座標の変動があれば背景フレームを描画
	if (y >= 0)
	{
		DrawBox(kConditionBoxPosX, kConditionBoxPosY, kConditionBoxPosX + kConditionBoxWidth, kConditionBoxPosY + y, kBackFrameColor, true);
		DrawGraph(kConditionBoxPosX, kConditionBoxPosY + y, m_backFrameImg->GetHandle(), true);
	}
	DrawGraph(0, 0, m_strScreen, true);

	DrawBestTime();
	DrawArrow();
	DrawWave("OK", kTitleWave, kTitleWaveNum);
	DrawConditionsAchived(y);
}

/// <summary>
/// プレイ中の描画処理
/// </summary>
void Stage::DrawPlaying() const
{
	DrawWall();

	SetDrawMode(DX_DRAWMODE_BILINEAR);
	for (const auto& enemy : m_backEnemy)
	{
		enemy->Draw();
	}
	for (const auto& enemy : m_enemy)
	{
		enemy->Draw();
	}
	for (const auto& enemy : m_frontEnemy)
	{
		enemy->Draw();
	}
	if (m_boss)
	{
		m_boss->Draw();
	}
	SetDrawMode(DX_DRAWMODE_NEAREST);
	m_player->Draw();

	int drawScreenHandle = m_mgr.GetScene()->GetScreenHandle();

	// ステージ情報の描画
	SetDrawScreen(m_strScreen);
	ClearDrawScreen();
	// 時間
	DrawTime(kTimePosX, kTimePosY + kTimePlayingDiff, m_mgr.GetFont()->GetHandle(64));
	// 背景フレーム
	SetDrawScreen(drawScreenHandle);
	DrawRotaGraph(kInfoFramePosX, kInfoFramePosY, 1.0, 0.0, m_backFrameImg->GetHandle(), true, false, true);
	DrawBox(kInfoBoxPosX, kInfoBoxPosY, kInfoBoxPosX + kInfoBoxWidth, kInfoBoxPosY + kInfoBoxHeight + kInfoBoxPlayingDiff, kBackFrameColor, true);
	DrawGraph(0, 0, m_strScreen, true);

	// 条件の描画
	SetDrawScreen(m_strScreen);
	ClearDrawScreen();
	auto y = DrawStageConditions(kConditionPosY + kConditionPlayingDiff);
	// Y座標の変動があれば背景フレームを描画
	SetDrawScreen(m_extScreen);
	ClearDrawScreen();
	if (y >= 0)
	{
		DrawBox(kConditionBoxPosX, kConditionBoxPosY + kConditionPlayingDiff, kConditionBoxPosX + kConditionBoxWidth, kConditionBoxPosY + kConditionPlayingDiff + y, kBackFrameColor, true);
		DrawGraph(kConditionBoxPosX, kConditionBoxPosY + kConditionPlayingDiff + y, m_backFrameImg->GetHandle(), true);
	}
	DrawGraph(0, 0, m_strScreen, true);
	// 本体に描画
	SetDrawScreen(drawScreenHandle);
	if (m_isExtRate && m_extRateFrame < kExtRateFrame)
	{
		DrawExpansion();
	}
	else
	{
		DrawGraph(0, 0, m_extScreen, true);
	}

	if (m_mgr.GetStage()->GetData()->GetAbility() == Ability::kDash)
	{
		DrawWave("dash", kDashWave, kDashWaveNum);
	}

	DrawBestTime();
	DrawConditionsAchived(y + kConditionPlayingDiff);
}

/// <summary>
/// ボス倒した後の描画処理
/// </summary>
void Stage::DrawAfterBossDeath() const
{
	DrawWall();

	SetDrawMode(DX_DRAWMODE_BILINEAR);
	m_boss->Draw();
	SetDrawMode(DX_DRAWMODE_NEAREST);
}

/// <summary>
/// プレイ開始時の処理
/// </summary>
void Stage::PlayStart()
{
	// ゲーム動作側初期化
	m_emphasisFrame = 0;
	m_timeFrame = 0;
	m_waitFrame = 0;
	m_extRateFrame = 0;
	m_isUpdateBestTime = false;
	m_waveAngle = 0;
	m_achived.clear();

	// キャラ初期化
	m_player->Init();
	m_enemy.clear();
	m_backEnemy.clear();
	m_frontEnemy.clear();
	m_boss = nullptr;

	// 敵の生成フレーム初期化
	for (int i = 0; i < static_cast<int>(m_enemyCreateFrame.size()); i++)
	{
		m_enemyCreateFrame[i] = 0;
		m_enemyStarCreateNum[i] = 0;
	}


	if (m_stageData[m_stageName].isBoss)
	{
		m_boss = std::make_shared<BossArmored>(m_size, m_fieldSize, this);
		m_boss->Init(m_centerPos);

		// ボスステージに入ったことがなければ説明
		const auto& data = m_mgr.GetStage()->GetData();
		if (!data->IsInBoss())
		{
			// 先にBGMをならして置かないと選択画面の音楽が流れるため流しておく
			m_soundSys->PlayFadeBgm(m_playBgm->GetHandle(), 0.8f);
			m_mgr.GetScene()->PushScene(std::make_shared<OneShotScene>(m_mgr, m_explanation[kIndexBossExplanation]->GetHandle()));
			data->SaveInBossStage();
		}
	}
}

/// <summary>
/// チュートリアルでの処理
/// </summary>
/// <returns>true: チュートリアル途中 / false: チュートリアル終了済み</returns>
bool Stage::UpdateTutorial()
{
	// チュートリアルステージでなければ早期リターン
	if (m_stageName != "練習") return false;

	// 説明総数を超えていたら早期リターン
	if (m_explanationIndex >= kExplanationNum) return false;

	m_mgr.GetScene()->PushScene(std::make_shared<OneShotScene>(m_mgr, m_explanation[m_explanationIndex]->GetHandle()));
	m_explanationIndex++;
	m_isWaveDraw = false;

	return true;
}

/// <summary>
/// 敵の更新
/// </summary>
/// <param name="enemys">敵の配列</param>
/// <param name="isDash">プレイヤーがダッシュしているか</param>
/// <param name="col">プレイヤーの当たり判定</param>
void Stage::UpdateEnemy(std::list<std::shared_ptr<EnemyBase>>& enemys, bool isDash, const Collision& col)
{
	for (const auto& enemy : enemys)
	{
		enemy->Update();

		if (!isDash && col.IsCollsion(enemy->GetRect()))
		{
			m_player->Death();

			m_mgr.UpdateDeathCcount();
			m_mgr.GetStage()->GetData()->SaveKilledEnemyType(enemy->GetName());

			m_mgr.GetScene()->ShakeScreen(kPlayerDeathShakeFrame);

			break;
		}
	}

	enemys.remove_if(
		[](const auto& enemy)
		{
			return !enemy->IsExsit();
		}
	);
}

/// <summary>
/// 時間の更新
/// </summary>
void Stage::UpdateTime()
{
	auto& data = m_stageData[m_stageName];

	if (data.timeType == UpTimeType::kNormal)
	{
		UpTime();
	}
	else if (data.timeType == UpTimeType::kAttack)
	{
		// 弾を拾った時なのでここでは特に処理はしない
	}
}

/// <summary>
/// チュートリアルでの描画
/// </summary>
void Stage::DrawTutrial()
{
	if (m_emphasisFrame > kEmphasisFrame) return;

	if (m_emphasisFrame > kStartAlphaEmphasisFrame)
	{
		int alpha = static_cast<int>(255 * (1.0f - (m_emphasisFrame - kStartAlphaEmphasisFrame) / static_cast<float>(kEmphasisFrame - kStartAlphaEmphasisFrame)));
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	}

	float addX = sinf(m_emphasisFrame * kEmphasisAngle) * 5;

	int x = static_cast<int>(m_player->GetPos().x + 64 + addX);
	int y = static_cast<int>(m_player->GetPos().y);

	int index = (m_emphasisFrame / kEmphasisInterval) % 2;

	DrawRotaGraph(x, y, 1.0, 0.0, m_emphasisArrow[index]->GetHandle(), true);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

/// <summary>
/// 壁の描画
/// </summary>
void Stage::DrawWall() const
{
	int centerX = static_cast<int>(m_size.w * 0.5f);
	int centerY = static_cast<int>(m_size.h * 0.5f);

	// 画像中心を元にした描画をするために
	// Rotaにしている
	DrawRotaGraph(centerX, centerY, 1.0, 0.0, m_fieldImg->GetHandle(), true);
}

/// <summary>
/// ステージ条件描画
/// </summary>
/// <param name="drawY">描画位置Y</param>
/// <returns>フレーム描画追加Y</returns>
int Stage::DrawStageConditions(int drawY) const
{
	int startY = drawY;
	int fontHandle = m_mgr.GetFont()->GetHandle(28);
	int arrowHandle = m_arrowConditionsImg->GetHandle();
	int arrowX = kConditionStrPosX + static_cast<int>(kArrowSize * 0.5);
	int arrowY = drawY + static_cast<int>(kArrowSize * 0.5);

	for (int i = 0; i < static_cast<int>(MapDir::kMax); i++)
	{
		const MapDir& dir = static_cast<MapDir>(i);

		// 接続してなかったら次へ
		if (!m_map->IsConect(m_stageName, dir)) continue;

		const auto& conectName = m_map->GetConectName(m_stageName, dir);

		// 既にクリアしていたら次へ
		if (m_isClear.at(dir)) continue;
		// チュートリアルステージなら次へ
		if (conectName == "練習") continue;

		// 矢印描画
		bool isClear = m_mgr.GetStage()->GetData()->IsClearStage(conectName);
		double angle = 0.0;
		if (dir == MapDir::kUp)
		{
			angle = 0.0;
		}
		else if (dir == MapDir::kDown)
		{
			angle = DX_PI;
		}
		else if (dir == MapDir::kRight)
		{
			angle = kRad90;
		}
		else if (dir == MapDir::kLeft)
		{
			angle = -kRad90;
		}
		if (isClear && (m_waitFrame / kArrowFlashInterval) % 2 != 0)
		{
			DrawBox(kConditionStrPosX, drawY, kConditionStrPosX + kArrowSize, drawY + kArrowSize, kYellowColor, true);
		}
		DrawRotaGraph(arrowX, arrowY, 1.0, angle, arrowHandle, true);


		// 条件描画
		auto& stage = m_stageData.at(conectName).stageInfo;
		auto& type = stage.type;
		if (type == ConditionsType::kTime)
		{
			DrawTimeConditions(drawY, fontHandle, stage.info);
		}
		else if (type == ConditionsType::kKilled)
		{
			DrawKilledConditions(drawY, fontHandle, stage.info);
		}
		else if (type == ConditionsType::kSumTime)
		{
			DrawSumTimeConditions(stage.infoGroup, drawY, fontHandle, stage.info);
		}

		drawY += 68;
	}

	return drawY - startY - 68;
}

/// <summary>
/// クリア条件のタイムを描画
/// </summary>
/// <param name="y">描画Y座標</param>
/// <param name="handle">フォントハンドル</param>
/// <param name="existTime">クリア時間(秒)</param>
void Stage::DrawTimeConditions(int y, int handle, int existTime) const
{
	int bestTime = m_mgr.GetStage()->GetData()->GetBestTime(m_stageName);

	DrawStringToHandle(kConditionStrPosX, y, L"　　   秒間生き残る\n　　(           )", kWhiteColor, handle);
	DrawFormatStringToHandle(kConditionStrPosX, y, kYellowColor, handle, L"　　%2d\n　　  %2d / %2d",
		existTime, bestTime / kFrameToSec, existTime);
}

/// <summary>
/// クリア条件の合計時間のやつ描画
/// </summary>
/// <param name="names">確認するステージ群情報</param>
/// <param name="y">描画Y座標</param>
/// <param name="handle">フォントハンドル</param>
/// <param name="existTime">合計クリア時間(秒)</param>
void Stage::DrawSumTimeConditions(const std::vector<std::string>& names, int y, int handle, int existTime) const
{
	const auto& stage = m_mgr.GetStage()->GetData();
	int sumTime = 0;

	// 確認するステージのすべてのタイムを加算する
	for (const auto& name : names)
	{
		sumTime += stage->GetBestTime(name);
	}

	// 秒に戻す
	sumTime /= kFrameToSec;

	DrawStringToHandle(kConditionStrPosX, y, L"　　全ステージ合計\n　　    秒間生き残る\n　　(           )", kWhiteColor, handle);
	DrawFormatStringToHandle(kConditionStrPosX, y, kYellowColor, handle, L"\n　　%02d\n　　  %2d / %2d",
		existTime, sumTime, existTime);
}

/// <summary>
/// クリア条件の敵に殺される種類数を描画
/// </summary>
/// <param name="y">描画Y座標</param>
/// <param name="handle">フォントハンドル</param>
/// <param name="killedNum">倒される種類数</param>
void Stage::DrawKilledConditions(int y, int handle, int killedNum) const
{
	int typeCount = m_mgr.GetStage()->GetData()->GetEnemyTypeCount();

	DrawStringToHandle(kConditionStrPosX, y, L"　　   種類の敵に殺される\n　　(          )", kWhiteColor, handle);
	DrawFormatStringToHandle(kConditionStrPosX, y, kYellowColor, handle, L"　　%2d\n　　  %2d / %2d",
		killedNum, typeCount, killedNum);
}

/// <summary>
/// 条件達成の描画
/// </summary>
/// <param name="y">描画Y座標</param>
void Stage::DrawConditionsAchived(int y) const
{
	// 送られてきた座標をずらす
	y += kAchivedStrShiftPosY;
	int backFrameY = y - kAchivedFrameDiff;

	for (const auto& achived : m_achived)
	{
		if (achived.frame < static_cast<int>(kAchivedDrawFrame * 0.5f))
		{
			DrawBox(0, backFrameY, kAchivedFrameWidth, backFrameY + kAchivedFrameHeight, kBackFrameColor, true);
			DrawStringToHandle(kAchivedStrPosX, y, achived.str.c_str(), kRedColor, m_mgr.GetFont()->GetHandle(64));
		}
		// 少しずつ透明に描画
		else
		{
			float rate = (kAchivedDrawFrame - achived.frame) / (kAchivedDrawFrame * 0.5f);
			int alpha = static_cast<int>(255 * rate);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawBox(0, backFrameY, kAchivedFrameWidth, backFrameY + kAchivedFrameHeight, kBackFrameColor, true);
			DrawStringToHandle(kAchivedStrPosX, y, achived.str.c_str(), kRedColor, m_mgr.GetFont()->GetHandle(64));
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		// 描画位置の更新
		y += kAchivedStrAdd;
		backFrameY += kAchivedStrAdd + kAchivedFrameDiff;
	}
}

/// <summary>
/// 条件の拡大描画
/// </summary>
void Stage::DrawExpansion() const
{
	int width = m_size.w;
	int height = m_size.h;

	int posX = 0;

	int left = 0;
	int right = width + static_cast<int>((width - kExtShiftRight) * kExtRateSize);
	int top = static_cast<int>(-kExtShiftUp * kExtRateSize);
	int bottom = height + static_cast<int>((height - kExtShiftUp) * kExtRateSize);

	if (m_extRateFrame > kWaitExtRateFrame)
	{
		float rate = 1.0f - ((m_extRateFrame - kWaitExtRateFrame) / static_cast<float>(kExtRateFrame - kWaitExtRateFrame));

		posX = static_cast<int>(kExtShiftWidth * rate);

		right = width + static_cast<int>((width - kExtShiftRight) * kExtRateSize * rate);
		top = static_cast<int>(-kExtShiftUp * kExtRateSize * rate);
		bottom = height + static_cast<int>((height - kExtShiftUp) * kExtRateSize * rate);

		int alpha = kExtRateAlpha + static_cast<int>((255 - kExtRateAlpha) * (1.0f - rate));
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	}
	else
	{
		posX = kExtShiftWidth;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, kExtRateAlpha);
	}

	DrawExtendGraph(posX + left, top, posX + right, bottom, m_extScreen, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

/// <summary>
/// 現在のタイムを描画
/// </summary>
/// <param name="x">描画X座標</param>
/// <param name="y">描画Y座標</param>
/// <param name="handle">フォントハンドル</param>
void Stage::DrawTime(int x, int y, int handle) const
{
	DrawStringToHandle(x, y, L"> タイム", kWhiteColor, m_mgr.GetFont()->GetHandle(24));
	y += kTimeAddY;
	int minSec = (m_timeFrame * 1000 / kFrameToSec) % 1000;
	int sec = (m_timeFrame / kFrameToSec) % kFrameToSec;
	int min = m_timeFrame / (kFrameToSec * kFrameToSec);
	DrawFormatStringToHandle(x, y, kYellowColor, handle, L"%01d:%02d.%03d", min, sec, minSec);
}

/// <summary>
/// ベストタイム関連の描画
/// </summary>
void Stage::DrawBestTime() const
{
	// フレーム描画
	DrawRotaGraph(m_size.w - kBestTimeFrameSubX, kBestTimeFramePosY, 1.0, 0.0, m_backFrameImg->GetHandle(), true, true, true);
	DrawBox(m_size.w - 128 - 155, 158, m_size.w, 224, kBackFrameColor, true);
	DrawBox(m_size.w - kBestTimeFrameSubX - kBestTimeFrameBoxSizeW, kBestTimeFrameBoxPosY,
		m_size.w, kBestTimeFramePosY + kBestTimeFrameBoxSizeH, kBackFrameColor, true);
	// ベストタイムの描画
	int bestTime = m_mgr.GetStage()->GetData()->GetBestTime(m_stageName);
	int minSec = (bestTime * 1000 / kFrameToSec) % 1000;
	int sec = (bestTime / kFrameToSec) % kFrameToSec;
	int min = bestTime / (kFrameToSec * kFrameToSec);
	DrawStringToHandle(m_size.w - kBestTimeStrSubX, kBestTimeStrPosY, L"> ベストタイム", kWhiteColor, m_mgr.GetFont()->GetHandle(32));
	DrawFormatStringToHandle(m_size.w - kBestTimeStrSubX, kBestTimeStrPosY + kBestTimeStrAddY, kYellowColor, m_mgr.GetFont()->GetHandle(64), L"%02d:%02d.%03d", min, sec, minSec);

	if (!m_isUpdateBestTime) return;

	// 点滅描画
	if (((m_waitFrame / kBestTimeFlashInterval) % 2) == 1)
	{
		DrawFormatStringToHandle(m_size.w - kBestTimeStrSubX, kBestTimeStrPosY + kBestTimeStrAddY, kRedColor, m_mgr.GetFont()->GetHandle(64), L"%02d:%02d.%03d", min, sec, minSec);
	}

	if (m_waitFrame > kAchivedDrawFrame) return;

	// 更新した場合教える用
	if (m_waitFrame > static_cast<int>(kAchivedDrawFrame * 0.5f))
	{
		float rate = 1.0f - (m_waitFrame - (kAchivedDrawFrame * 0.5f)) / (kAchivedDrawFrame * 0.5f);
		int alpha = static_cast<int>(255 * rate);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawBox(m_size.w - kBestTimeBoxPosX, kBestTimeBoxPosY, m_size.w, kBestTimeBoxPosY + kBestTimeBoxHeight, kBackFrameColor, true);
		DrawStringToHandle(m_size.w - kBestTimeUpStrPosX, kBestTimeUpStrPosY, L"ベストタイム更新！", kYellowColor, m_mgr.GetFont()->GetHandle(48));
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		return;
	}

	DrawBox(m_size.w - kBestTimeBoxPosX, kBestTimeBoxPosY, m_size.w, kBestTimeBoxPosY + kBestTimeBoxHeight, kBackFrameColor, true);
	DrawStringToHandle(m_size.w - kBestTimeUpStrPosX, kBestTimeUpStrPosY, L"ベストタイム更新！", kYellowColor, m_mgr.GetFont()->GetHandle(48));
}

/// <summary>
/// ステージ移動の矢印描画
/// </summary>
void Stage::DrawArrow() const
{
	for (int i = 0; i < static_cast<int>(MapDir::kMax); i++)
	{
		const MapDir& dir = static_cast<MapDir>(i);

		// 接続してなかったら次へ
		if (!m_map->IsConect(m_stageName, dir)) continue;

		const auto& conectName = m_map->GetConectName(m_stageName, dir);

		// チュートリアルステージなら次へ
		if (conectName == "練習") continue;

		bool isAlreadyClear = m_isClear.at(dir);
		int handle = GetArrowHandle(isAlreadyClear, conectName);

		int x = static_cast<int>(m_size.w * 0.5f);
		int y = static_cast<int>(m_size.h * 0.5f);
		double angle = 0.0;
		bool isReverseX = false;
		bool isReverseY = false;

		// 矢印描画
		if (dir == MapDir::kUp)
		{
			y -= kArrowShiftPos;
		}
		else if (dir == MapDir::kDown)
		{
			y += kArrowShiftPos;
			isReverseY = true;
		}
		else if (dir == MapDir::kRight)
		{
			x += kArrowShiftPos;
			angle = kRad90;
			isReverseX = true;
		}
		else if (dir == MapDir::kLeft)
		{
			x -= kArrowShiftPos;
			angle = -kRad90;
		}

		DrawRotaGraph(x, y, 1.0, angle, handle, true, isReverseX, isReverseY);

		bool isBoss = m_stageData.at(conectName).isBoss;
		// ボスステージではないまたは既にクリアしていたら描画しない
		if (!isBoss || isAlreadyClear) continue;

		// 鍵描画
		DrawRotaGraph(x, y, 1.0, 0.0, m_arrowLockImg->GetHandle(), true);
	}
}

/// <summary>
/// 敵タイプの描画
/// </summary>
/// <param name="enemyName">名前</param>
/// <param name="addX">X座標の追加値</param>
/// <param name="radius">半径 : def = 16</param>
void Stage::DrawKilledEnemy(const std::string& enemyName, int x, int y, int addX, int radius) const
{
	double enemyExtRate = kKillTypeDefExtRate;
	// 小さくする敵の場合は小さく
	for (const auto& name : kSmallTypeName)
	{
		if (name == enemyName)
		{
			enemyExtRate = kKillTypeSmallExtRate;
			break;
		}
	}
	// 大きくする敵の場合は大きく
	for (const auto& name : kLargeTypeName)
	{
		if (name == enemyName)
		{
			enemyExtRate = kKillTypeLargeExtRate;
			break;
		}
	}

	auto& file = m_enemysImg.at(enemyName);
	bool isKilled = m_mgr.GetStage()->GetData()->IsKilledEnemy(enemyName);
	if (isKilled)
	{
		DrawRotaGraph(x + addX, y, enemyExtRate, 0.0, file->GetHandle(), true);
		DrawRotaGraph(x + addX, y, 0.5, 0.0, m_checkImg->GetHandle(), true);
	}
	else
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, kKillTypeAlpha);
		DrawRotaGraph(x + addX, y, enemyExtRate, 0.0, file->GetHandle(), true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

/// <summary>
/// 選択等の文字・画像描画
/// </summary>
/// /// <param name="cmd">コマンド名</param>
/// <param name="str">ウェーブさせる文字列</param>
/// <param name="num">文字列数</param>
void Stage::DrawWave(const char* const cmd, const wchar_t* const str[], int num) const
{
	if (!m_isWaveDraw) return;

	DrawGraph(kWaveFramePosX, kWaveFramePosY, m_frameImg->GetHandle(), true);

	// 事前に入力したタイプで画像変更
	const auto& type = m_input.GetType();
	if (type == InputType::keybd)
	{
		m_key->DrawKey(m_input.GetHardDataName(cmd, InputType::keybd), kWaveImgPosX, kWaveImgPosY, 2.0);
	}
	else if (type == InputType::pad)
	{
		m_bt->DrawBottan(m_input.GetHardDataName(cmd, InputType::pad), kWaveImgPosX, kWaveImgPosY, 2.0);
	}

	int handle = m_mgr.GetFont()->GetHandle(32);

	int x = kWaveStrPosX;

	// 文字一つずつ描画
	for (int i = 0; i < num; i++)
	{
		int add = static_cast<int>(sinf(m_waveAngle + kWaveInterval * i) * -10);

		if (add > 0)
		{
			add = 0;
		}

		int y = kWaveStrPosY + add;


		DrawStringToHandle(x, y, str[i], kWhiteColor, handle);
		x += kWaveStrAdd;
	}
}

/// <summary>
/// 敵の生成
/// </summary>
void Stage::CreateEnemy()
{
	auto& data = m_stageData[m_stageName];

	int index = 0;
	// 敵種類全部回す
	for (int i = 0; i < data.enemyNum; i++)
	{
		auto& enemy = data.enemyInfo[i];

		// その種類のすべてのパターンを回す
		for (const auto& info : enemy.info)
		{
			auto& startNum = m_enemyStarCreateNum[index];
			auto& createFrame = m_enemyCreateFrame[index];

			createFrame++;
			// 初回生成
			if (startNum < info.startNum && createFrame > info.startInterval)
			{
				CreateEnemyType(enemy.name, createFrame, true);
				createFrame = info.startDelayFrame;

				startNum++;
			}
			// 通常生成  生成間隔が０より大きいことが条件
			else if (createFrame > info.createInterval && info.createInterval > 0)
			{
				CreateEnemyType(enemy.name, createFrame);
			}

			// 要素番号更新
			index++;
		}
	}
}

/// <summary>
/// 種類によって生成する敵を変更する
/// </summary>
/// <param name="name">名前</param>
/// <param name="frame">生成フレーム</param>
/// <param name="isStart">開始時に生成する敵か</param>
void Stage::CreateEnemyType(const std::string& name, int& frame, bool isStart)
{
	if (name == "MoveWall")
	{
		// 上側
		auto enemy = std::make_shared<EnemyMoveWall>(m_size, m_fieldSize);
		enemy->Init({ 0, -1 });
		m_enemy.push_back(enemy);
		// 下側
		enemy = std::make_shared<EnemyMoveWall>(m_size, m_fieldSize);
		enemy->Init({ 0, 1 });
		m_enemy.push_back(enemy);

		return;
	}

	// 生成
	std::shared_ptr<EnemyBase> enemy;
	if (name == "Normal")
	{
		enemy = std::make_shared<EnemyNormal>(m_size, m_fieldSize);
	}
	else if (name == "Large")
	{
		enemy = std::make_shared<EnemyLarge>(m_size, m_fieldSize);
	}
	else if (name == "Dash")
	{
		enemy = std::make_shared<EnemyDash>(m_size, m_fieldSize, m_player);
	}
	else if (name == "Create")
	{
		enemy = std::make_shared<EnemyCreate>(m_size, m_fieldSize, this);
	}
	else if (name == "Division")
	{
		enemy = std::make_shared<EnemyDivision>(m_size, m_fieldSize, this);
	}

	// 初期化
	frame = 0;
	enemy->Init(m_centerPos, isStart);
	// 追加
	m_enemy.push_back(enemy);
}

/// <summary>
/// 強化ボスの生成
/// </summary>
void Stage::CreateStrongBoss()
{
	std::shared_ptr<BossStrongArmored> strong;
	strong = std::make_shared<BossStrongArmored>(m_size, m_fieldSize, this);
	strong->Init(m_boss->GetPos());

	m_boss = strong;
}

/// <summary>
/// ボスの死亡処理
/// </summary>
void Stage::DeathBoss()
{
	const auto& data = m_mgr.GetStage()->GetData();

	// すでにクリアされていた場合は強化ボスを出す
	if (data->IsClearBoss(m_boss->GetName()))
	{
		CreateStrongBoss();
		return;
	}

	// ベストタイムの更新
	if (data->SaveBestTime(m_stageName, m_timeFrame))
	{
		m_isUpdateBestTime = true;
	}

	// クリアしているかの確認
	CheckStageConditions(data->GetBestTime(m_stageName));

	// 倒した情報の追加
	data->SaveClearBoss(m_boss->GetName());

	// 敵全て消す
	m_enemy.clear();
	m_frontEnemy.clear();
	m_backEnemy.clear();

	// 死亡後はダメージオブジェクトをすべて消す
	const auto& armored = std::dynamic_pointer_cast<BossArmored>(m_boss);
	armored->DeleteDamageObjects();

	m_updateFunc = &Stage::UpdateAfterBossDeath;
	m_drawFunc = &Stage::DrawAfterBossDeath;
}

/// <summary>
/// 条件確認
/// </summary>
/// <param name="frame">経過時間</param>
void Stage::CheckStageConditions(int frame)
{
	for (int i = 0; i < static_cast<int>(MapDir::kMax); i++)
	{
		const MapDir& dir = static_cast<MapDir>(i);

		// 接続してなかったら次へ
		if (!m_map->IsConect(m_stageName, dir)) continue;

		const auto& conectName = m_map->GetConectName(m_stageName, dir);
		auto& stage = m_stageData[conectName].stageInfo;

		auto& type = stage.type;
		if (type == ConditionsType::kTime)
		{
			CheckConditionsTime(conectName, frame, stage.info, GetDirName(dir));
		}
		else if (type == ConditionsType::kKilled)
		{
			CheckConditionsKilled(conectName, stage.info, GetDirName(dir));
		}
		else if (type == ConditionsType::kSumTime)
		{
			CheckConditionsSumTime(conectName, stage.info, frame, GetDirName(dir));
		}
	}
}

/// <summary>
/// クリア時間を超えているかの確認
/// </summary>
/// <param name="stageName">確認するステージ名</param>
/// <param name="timeFrame">タイムフレーム</param>
/// <param name="exsitTime">クリア時間(秒)</param>
/// <param name="dir">方向名</param>
void Stage::CheckConditionsTime(const std::string& stageName, int timeFrame, int exsitTime, const std::wstring& dir)
{
	const auto& stage = m_mgr.GetStage()->GetData();

	if (stage->IsClearStage(stageName)) return;

	if (timeFrame > exsitTime * kFrameToSec)
	{
		m_soundSys->PlaySe(m_clearSe->GetHandle());
		stage->SaveClearStage(stageName);
		AddAchivedStr(dir);
	}
}

/// <summary>
/// あるステージ全ての生存時間がクリア時間を超えているか
/// </summary>
/// <param name="stageName">確認するステージ名</param>
/// <param name="exsitTime">クリア時間(秒)</param>
/// <param name="dir">方向名</param>
void Stage::CheckConditionsSumTime(const std::string& stageName, int timeFrame, int exsitTime, const std::wstring& dir)
{
	const auto& stage = m_mgr.GetStage()->GetData();
	int sumTime = 0;

	if (stage->IsClearStage(stageName)) return;

	// 確認するステージのすべてのタイムを加算する
	for (const auto& name : m_stageData[stageName].stageInfo.infoGroup)
	{
		if (name == m_stageName)
		{
			sumTime += timeFrame;
		}
		else
		{
			sumTime += stage->GetBestTime(name);
		}
	}

	if (sumTime >= exsitTime * kFrameToSec)
	{
		m_soundSys->PlaySe(m_clearSe->GetHandle());
		stage->SaveClearStage(stageName);
		AddAchivedStr(dir);
	}
}

/// <summary>
/// 殺された数が超えているかの確認
/// </summary>
/// <param name="stageName">確認するステージ名</param>
/// <param name="killedNum">殺される種類数</param>
/// <param name="dir">方向名</param>
void Stage::CheckConditionsKilled(const std::string& stageName, int killedNum, const std::wstring& dir)
{
	const auto& stage = m_mgr.GetStage()->GetData();

	if (stage->IsClearStage(stageName)) return;

	if (stage->GetEnemyTypeCount() >= killedNum)
	{
		m_soundSys->PlaySe(m_clearSe->GetHandle());
		stage->SaveClearStage(stageName);
		AddAchivedStr(dir);
	}
}

/// <summary>
/// クリアした文字の描画群に追加
/// </summary>
/// <param name="dir">方向</param>
void Stage::AddAchivedStr(const std::wstring& dir)
{
	m_achived.push_back({ dir + L"の条件達成！", 0 });
}

/// <summary>
/// 矢印方向の文字を取得
/// </summary>
/// <param name="dir">方向</param>
/// <returns>方向文字</returns>
std::wstring Stage::GetDirName(MapDir dir)
{
	if (dir == MapDir::kUp)
	{
		return L"上";
	}
	else if (dir == MapDir::kDown)
	{
		return L"下";
	}
	else if (dir == MapDir::kRight)
	{
		return L"右";
	}
	else if (dir == MapDir::kLeft)
	{
		return L"左";
	}

	return L"";
}

/// <summary>
/// 矢印の画像をどれにするか決める
/// </summary>
/// <param name="isAlreadyClear">既にステージをクリアしたことがあるか</param>
/// <param name="nextStName">次のステージ名</param>
/// <returns>対応した画像ハンドル</returns>
int Stage::GetArrowHandle(bool isAlreadyClear, const std::string& nextStName) const
{
	int handle;

	bool isClear = m_mgr.GetStage()->GetData()->IsClearStage(nextStName);
	if (isClear)
	{
		if (isAlreadyClear || (m_waitFrame / kArrowFlashInterval) % 2 == 0)
		{
			handle = m_arrowImg->GetHandle();
		}
		else
		{
			handle = m_arrowFlashImg->GetHandle();
		}
	}
	else
	{
		handle = m_arrowNoImg->GetHandle();
	}

	return handle;
}

/// <summary>
/// ステージデータのロード
/// </summary>
void Stage::LoadStageInfo()
{
	// 一時保存用string
	std::string strBuf;
	// カンマ分け一時保存用string
	std::vector<std::string> strConmaBuf;

	// ファイル読み込み
	std::ifstream ifs("Data/Stage/StageData.csv");
	if (!ifs)
	{
		assert(false);
		return;
	}

	// ステージ名保存
	std::string stageName;
	// 敵種類数情報
	int enemyTypeIndex = 0;
	bool isLoadAllEnemys = true;
	// 敵1体の情報
	int enemyInfoIndex = 0;
	bool isLoadAllEnmeyInfo = true;

	// 最初は対応表情報が入っているだけなので無視する
	std::getline(ifs, strBuf);
	// 情報を読み切るまでループ
	while (std::getline(ifs, strBuf))
	{
		strConmaBuf = StringUtility::Split(strBuf, ',');

		LoadImportantStageInfo(strConmaBuf, stageName, isLoadAllEnemys, enemyTypeIndex);
		auto& data = m_stageData[stageName];
		LoadEnemys(strConmaBuf, data, isLoadAllEnemys, enemyTypeIndex, isLoadAllEnmeyInfo, enemyInfoIndex);
	}
}

/// <summary>
/// ステージの詳細データ取得
/// </summary>
/// <param name="strConmaBuf">カンマ区切り済データ</param>
/// <param name="stageName">ステージ名</param>
/// <param name="isLoadAllEnemys">全敵ロードフラグ</param>
/// <param name="enemyTypeIndex">敵種類要素番号</param>
void Stage::LoadImportantStageInfo(std::vector<std::string>& strConmaBuf, std::string& stageName, bool& isLoadAllEnemys, int& enemyTypeIndex)
{
	// 全ての情報を読み込んでいる場合のみ次の情報群に移行する
	if (!isLoadAllEnemys) return;

	// ステージ名読み込み
	stageName = strConmaBuf[kStageName];
	// 敵種類数読み込み
	int enemyTypeNum = std::stoi(strConmaBuf[kEnemyTypeNum]);
	// ボス生成フラグ読み込み
	bool isCreateBoss = static_cast<bool>(std::stoi(strConmaBuf[kIsCreateBoss]));
	// 更新タイプ読み込み
	UpTimeType timeType = static_cast<UpTimeType>(std::stoi(strConmaBuf[kUpdateTimeType]));
	// 更新追加フレーム読み込み
	int addTimeFrame = std::stoi(strConmaBuf[kAddTimeFrame]);
	// 条件タイプ読み込み
	ConditionsType infoType = static_cast<ConditionsType>(std::stoi(strConmaBuf[kInfoType]));
	// 条件情報読み込み
	int info = std::stoi(strConmaBuf[kInfo]);

	// 情報の代入
	auto& data = m_stageData[stageName];
	data.timeType = timeType;
	data.addTimeFrame = addTimeFrame;
	data.enemyNum = enemyTypeNum;
	data.enemyInfo.resize(enemyTypeNum);
	data.isBoss = isCreateBoss;
	auto& stage = data.stageInfo;
	stage.type = infoType;
	stage.info = info;

	// 合計時間系のみ追加情報読み込み
	if (static_cast<ConditionsType>(infoType) == ConditionsType::kSumTime)
	{
		// 情報群数読み込み
		int infoNum = std::stoi(strConmaBuf[kInfoGroupNum]);
		stage.infoGroup.resize(infoNum);

		// 情報群読み込み
		for (int i = 0; i < infoNum; i++)
		{
			stage.infoGroup[i] = strConmaBuf[i + kInfoGroup];
		}
	}

	// 情報読み込んでいないとする
	isLoadAllEnemys = false;
	enemyTypeIndex = 0;
}

/// <summary>
/// 生成敵データ取得
/// </summary>
/// <param name="strConmaBuf">カンマ区切り済データ</param>
/// <param name="data">ステージデータ</param>
/// <param name="isLoadAllEnemys">全敵ロードフラグ</param>
/// <param name="enemyTypeIndex">敵種類要素番号</param>
/// <param name="isLoadAllEnmeyInfo">敵ロードフラグ</param>
/// <param name="enemyInfoIndex">敵単体要素番号</param>
void Stage::LoadEnemys(std::vector<std::string>& strConmaBuf, StageData& data, bool& isLoadAllEnemys, int& enemyTypeIndex, bool& isLoadAllEnmeyInfo, int& enemyInfoIndex)
{
	// 敵情報すべて読み込んでいたら早期リターン
	if (isLoadAllEnemys) return;

	auto& enemy = data.enemyInfo[enemyTypeIndex];

	if (isLoadAllEnmeyInfo)
	{
		// 名前読み込み
		std::string enemyName = strConmaBuf[kEnemyName];
		// 同名別条件数読み込み
		int enemyInfoNum = std::stoi(strConmaBuf[kEnemyInfoNum]);

		// 情報代入
		enemy.name = enemyName;
		enemy.num = enemyInfoNum;
		enemy.info.resize(enemyInfoNum);

		// 情報読み込み初期化
		isLoadAllEnmeyInfo = false;
		enemyInfoIndex = 0;
	}

	auto& info = enemy.info[enemyInfoIndex];

	// 初期生成数読み込み
	int startNum = std::stoi(strConmaBuf[kEnemyStartCreatNm]);
	// 初期生成間隔読み込み
	int startCreateFrame = std::stoi(strConmaBuf[kEnemyStartCreateInterval]);
	// 初期遅延フレーム
	int startDelayFrame = std::stoi(strConmaBuf[kEnmeyStartDelayFrame]);
	// 生成間隔
	int CreateFrame = std::stoi(strConmaBuf[kEnemyCreateInterval]);

	// 情報代入
	info.startNum = startNum;
	info.startInterval = startCreateFrame;
	info.startDelayFrame = startDelayFrame;
	info.createInterval = CreateFrame;

	// 情報更新
	enemyInfoIndex++;
	if (enemyInfoIndex >= enemy.num)
	{
		isLoadAllEnmeyInfo = true;

		enemyTypeIndex++;
		if (enemyTypeIndex >= data.enemyNum)
		{
			isLoadAllEnemys = true;
		}
	}
}
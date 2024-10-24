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
#include "StageBase.h"

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
	// 初期化(読み込みに失敗した場合は初プレイと同じとする)
	InitData();
	Load(L"Data/Bin/stg.inf");

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
	Save("Data/Bin/stg.inf");
}

void StageManager::Init()
{
	auto& mgr = GameManager::GetInstance().GetFile();
	m_dashImg = mgr->LoadGraphic(L"UI/dashExplanation.png");

	InitPos();
}

void StageManager::InitData()
{
	m_stageSaveData.clear();
	m_killedEnemyNameTable.clear();
	m_killedEnemyCount = 0;
	m_clearBossTable.clear();
	m_ability = Ability::kNone;
	m_abilityActive.clear();
	m_isBossIn = false;
}

void StageManager::InitPos()
{
	m_stage = nullptr;
	m_nextStage = nullptr;

	if (IsClearStage("練習"))
	{
		const auto& stageName = GameManager::GetInstance().GetNowStage();
		if (stageName == "")
		{
			m_pos = GetPos("サークル");
		}
		else
		{
			m_pos = GetPos(stageName);
		}
	}
	else
	{
		m_pos = GetPos("練習");
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

void StageManager::ChangeStage(std::shared_ptr<StageBase> nextStage, bool isGameScene)
{
	// 初回またはゲームシーンからのみ、ただ入れるだけ
	if (!m_stage || isGameScene)
	{
		m_stage = nextStage;
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
		pos = GetPos(m_stage->GetStageName());

		DrawGraph(static_cast<int>(pos.x), static_cast<int>(pos.y), m_drawScreen, true);

		// 現在のステージのポインタを変更する
		m_stage = nextStage;
	}

	// 描画用スクリーンに次のステージの描画
	SetDrawScreen(m_drawScreen);
	ClearDrawScreen();
	nextStage->Draw();

	// 移動中描画スクリーンに場所を指定してさっきのを描画
	SetDrawScreen(m_screen);
	ClearDrawScreen();
	pos = GetPos(nextStage->GetStageName());

	DrawGraph(static_cast<int>(pos.x), static_cast<int>(pos.y), m_drawScreen, true);

	// キープにあるものを描画
	DrawGraph(0, 0, m_keepScreen, true);

	// 移動ベクトルの計算
	m_vec = (pos - m_pos).GetNormalized() * kSpeed;

	// 向かう先の場所の保存
	m_targetPos = pos;

	// 次のステージの保存
	m_nextStage = nextStage;
	SaveClear(m_nextStage->GetStageName());
	m_nextStage->StartCheck();

	m_isMove = true;

	GameManager::GetInstance().UpdateNowStage(m_nextStage->GetStageName());

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

	m_stage = m_nextStage;
}

void StageManager::Save(const std::string& path)
{
	// ファイルポインタ
	FILE* fp = nullptr;
	auto err = fopen_s(&fp, path.c_str(), "wb");
	if (err != 0)
	{
		assert(false);
		return;
	}
	StageInfHeader header;
	header.dataCount = m_stageSaveData.size();
	fwrite(&header, sizeof(header), 1, fp);

	// クリアデータの書き込み
	for (const auto& stage : m_stageSaveData)
	{
		// ステージ名文字列のサイズを取得
		const auto& stageStr = stage.first; 
		uint8_t size = static_cast<uint8_t>(stageStr.size());
		// ステージ名文字列のバイト数を書き込む
		fwrite(&size, sizeof(size), 1, fp); 
		// 文字列の書き込み
		fwrite(stageStr.data(), stageStr.size(), 1, fp);    

		// データ情報の書き込み
		fwrite(&stage.second, sizeof(stage.second), 1, fp);
	}

	// データ本体を書き込んでいく
	// 殺された敵の種類数の書き込み
	fwrite(&m_killedEnemyCount, sizeof(m_killedEnemyCount), 1, fp);
	// 名前データの書き込み
	for (const auto& name : m_killedEnemyNameTable)
	{
		// 名前の文字列数を取得
		uint8_t nameSize = static_cast<uint8_t>(name.size());
		// 文字列数を書き込み
		fwrite(&nameSize, sizeof(nameSize), 1, fp);
		// 文字列の書き込み
		fwrite(name.data(), name.size(), 1, fp);
	}

	// ボスクリア情報の書き込み
	uint8_t size = static_cast<uint8_t>(m_clearBossTable.size());
	fwrite(&size, sizeof(size), 1, fp);
	// データの書き込み
	for (const auto& name : m_clearBossTable)
	{
		uint8_t nameSize = static_cast<uint8_t>(name.size());
		fwrite(&nameSize, sizeof(nameSize), 1, fp);
		fwrite(name.data(), name.size(), 1, fp);
	}

	// アビリティの書き込み
	fwrite(&m_ability, sizeof(m_ability), 1, fp);

	// アビリティの有効テーブルの書き込み
	size = static_cast<uint8_t>(m_abilityActive.size());
	fwrite(&size, sizeof(size), 1, fp);
	for (const auto& active : m_abilityActive)
	{
		// アビリティタイプ書き込み
		fwrite(&active.first, sizeof(active.first), 1, fp);
		// 有効化どうか書き込み
		fwrite(&active.second, sizeof(active.second), 1, fp);
	}

	// ボスステージに入ったことがるかの情報
	fwrite(&m_isBossIn, sizeof(m_isBossIn), 1, fp);

	fclose(fp);
}

void StageManager::Load(const std::wstring& path)
{
	auto handle = FileRead_open(path.c_str());
	// エラー(ファイルがない)場合は処理しない
	if (handle == 0)
	{
		return;
	}
	// ヘッダ情報の読み込み
	StageInfHeader header;
	FileRead_read(&header, sizeof(header), handle);

	if (header.version != kVersion)
	{
		FileRead_close(handle);
		return;
	}

	// データの読み込み
	for (int i = 0; i < header.dataCount; i++)
	{
		// ステージ名文字列サイズを入れる変数
		uint8_t stgStrSize = 0;
		// ステージ名文字列サイズ読み込む
		FileRead_read(&stgStrSize, sizeof(stgStrSize), handle);
		// ステージ名文字列を入れる変数
		std::string stgStr;
		// ステージ名文字列サイズでリサイズ
		stgStr.resize(stgStrSize);
		// ステージ名読み込み
		FileRead_read(stgStr.data(), static_cast<int>(stgStr.size() * sizeof(char)), handle);

		// ステージクリアテーブルから情報群のvector<StageData>の参照を取得
		auto& data = m_stageSaveData[stgStr];

		// データ情報の読み込み
		FileRead_read(&data, sizeof(data), handle);
	}

	// 殺された敵の種類数の取得
	FileRead_read(&m_killedEnemyCount, sizeof(m_killedEnemyCount), handle);
	// 名前情報群の数を種類数でリサイズ
	m_killedEnemyNameTable.resize(m_killedEnemyCount);
	for (int i = 0; i < m_killedEnemyCount; i++)
	{
		// 名前文字列サイズを入れる変数
		uint8_t nameSize;
		// サイズの読み込み
		FileRead_read(&nameSize, sizeof(nameSize), handle);

		auto& name = m_killedEnemyNameTable[i];
		// サイズで文字列をリサイズ
		name.resize(nameSize);
		// 文字列の読み込み
		FileRead_read(name.data(), static_cast<int>(name.size() * sizeof(char)), handle);
	}

	// ボスクリア情報の取得
	uint8_t size;
	FileRead_read(&size, sizeof(size), handle);
	m_clearBossTable.resize(size);
	for (int i = 0; i < size; i++)
	{
		uint8_t nameSize;
		FileRead_read(&nameSize, sizeof(nameSize), handle);
		auto& name = m_clearBossTable[i];
		name.resize(nameSize);
		FileRead_read(name.data(), static_cast<int>(name.size() * sizeof(char)), handle);
	}

	// アビリティの取得
	FileRead_read(&m_ability, sizeof(m_ability), handle);

	// アビリティ有効テーブルの取得
	FileRead_read(&size, sizeof(size), handle);
	for (int i = 0; i < size; i++)
	{
		Ability ability;
		bool active;
		FileRead_read(&ability, sizeof(ability), handle);
		FileRead_read(&active, sizeof(active), handle);

		m_abilityActive[ability] = active;
	}

	// ボスステージに入ったことがるかの情報
	FileRead_read(&m_isBossIn, sizeof(m_isBossIn), handle);

	// ファイルは閉じる
	FileRead_close(handle);
}

void StageManager::CreateData(const std::string& stgName)
{
	auto it = m_stageSaveData.find(stgName);
	// ステージを見つけたら何もしない
	if (it != m_stageSaveData.end())
	{
		return;
	}

	// なければ作成する
	auto& data = m_stageSaveData[stgName];

	// 情報の初期化
	data.bestTime = 0;
	data.isClear = false;
}

bool StageManager::IsClearStage(const std::string& stgName)
{
	auto it = m_stageSaveData.find(stgName);
	// ステージを見つけられなかったら0を返す
	if (it == m_stageSaveData.end())
	{
		CreateData(stgName);
		return false;
	}

	// 見つかったらクリア情報を返す
	return m_stageSaveData.at(stgName).isClear;
}

bool StageManager::IsClearBoss(const std::string& name) const
{
	for (const auto& boss : m_clearBossTable)
	{
		// 名前が一致したらクリアしたことがある
		if (boss == name)
		{
			return true;
		}
	}

	// 一致しなかったらクリアしたことがない
	return false;
}

bool StageManager::IsKilledEnemy(const std::string& name) const
{
	for (const auto& killedName : m_killedEnemyNameTable)
	{
		// 一致すれば殺されたことがある
		if (killedName == name)
		{
			return true;
		}
	}

	// 一致しないければ殺されたことがない
	return false;
}

int StageManager::GetBestTime(const std::string& stgName) const
{
	auto it = m_stageSaveData.find(stgName);
	// ステージを見つけられなかったら0を返す
	if (it == m_stageSaveData.end())
	{
		return 0;
	}

	// 見つかったらベストタイムを返す
	return m_stageSaveData.at(stgName).bestTime;
}

int StageManager::GetEnemyTypeCount() const
{
	return m_killedEnemyCount;
}

Ability StageManager::GetAbility() const
{
	return m_ability;
}

void StageManager::SaveClear(const std::string& stgName)
{
	auto it = m_stageSaveData.find(stgName);
	// ステージを見つけられなかったら何もしない
	if (it == m_stageSaveData.end())
	{
		assert(false);
		return;
	}

	// 指定のものをクリアとする
	m_stageSaveData[stgName].isClear = true;
}

void StageManager::UpdateClearBoss(const std::string& name)
{
	for (const auto& boss : m_clearBossTable)
	{
		// クリアしたことがあれば何もしない
		if (boss == name)
		{
			return;
		}
	}

	// 情報が乗っていない場合は追加する
	m_clearBossTable.push_back(name);
	return;
}

bool StageManager::UpdateBestTime(const std::string& stgName, int bestTime)
{
	auto it = m_stageSaveData.find(stgName);
	// ステージを見つけられなかったら何もしない
	if (it == m_stageSaveData.end())
	{
		assert(false);
		return false;
	}

	auto& time = m_stageSaveData[stgName].bestTime;

	// 現在保存されているタイムが更新タイムより大きければ更新は行わない
	if (time > bestTime)
	{
		return false;
	}

	// タイムの更新
	time = bestTime;
	return true;
}

void StageManager::UpdateEnemyType(const std::string& name)
{
	// 配列全部繰り返す
	for (const auto& tableName : m_killedEnemyNameTable)
	{
		// 現在のテーブルの名前と殺した敵の名前が同じ場合は一度殺したことがあるから
		// この処理を終了する
		if (tableName == name)
		{
			return;
		}
	}

	// ここまで来たら殺したことがなかった敵である
	// 種類数カウントの増加
	m_killedEnemyCount++;
	// テーブルに登録
	m_killedEnemyNameTable.push_back(name);

	// アビリティの有効化
	if (name == "Dash")
	{
		m_abilityActive[Ability::kDash] = true;

		// ダッシュ説明画像
		auto& mgr = GameManager::GetInstance();
		mgr.GetScene()->PushScene(std::make_shared<OneShotScene>(mgr, m_dashImg->GetHandle()));
	}
}

void StageManager::ChangeAbility(Ability ability)
{
	// アビリティが有効になっていたら変更させる
	if (m_abilityActive[ability])
	{
		m_ability = ability;
	}
}

void StageManager::NormalUpdate(Input& input)
{
	m_stage->Update(input);
	m_stage->ChangeStage(input);
}

void StageManager::StageMoveUpdate(Input& input)
{
	m_nextStage->Update(input);
	m_nextStage->ChangeStage(input);

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
	Vec2 pos = GetPos(m_stage->GetStageName());

	DrawGraph(static_cast<int>(pos.x), static_cast<int>(pos.y), m_drawScreen, true);

	// キープにあるものを描画
	DrawGraph(0, 0, m_keepScreen, true);

	SetDrawScreen(m_mgr->GetScreenHandle());

	DrawGraph(static_cast<int>(-m_pos.x), static_cast<int>(-m_pos.y), m_screen, true);
}

Vec2 StageManager::GetPos(const std::string& stage) const
{

	Vec2 pos;

	for (int x = 0; x < kRow; x++)
	{
		for (int y = 0; y < kLine; y++)
		{
			// ステージ名が一致したら
			if (kStName[y][x] == stage)
			{
				// 場所の保存
				pos.x = static_cast <float>(m_size.w * x + kStageMarginX * x);
				pos.y = static_cast <float>(m_size.h * y + kStageMarginY * y);

				// 探すの終了
				break;
			}
		}
	}

	return pos;
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

			m_stage = m_nextStage;

			m_isMove = false;
		}

		// スピードの計算
		float speed = (dis / kSlowDis) * kSpeed;
		// スピードの調整
		m_vec = vel.GetNormalized() * speed;
	}
}

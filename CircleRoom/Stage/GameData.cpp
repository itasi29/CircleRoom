#include "GameData.h"

#include <DxLib.h>
#include <cassert>

#include "StringUtility.h"
#include "GameManager.h"
#include "Scene/SceneManager.h"
#include "Scene/OneShotScene.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"

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

	const char* const kPath = "Data/Bin/stg.inf";
}

GameData::GameData() :
	m_ability(Ability::kNone),
	m_isInBoss(false),
	m_killedEnemyTypeNum(0)
{
}

GameData::~GameData()
{
	Save();
}

void GameData::Init()
{
	// 画像読み込み
	auto& mgr = GameManager::GetInstance().GetFile();
	m_explanationDash = mgr->LoadGraphic(L"UI/dashExplanation.png");

	// データ初期化
	InitData();
	// データ読み込み
	Load();
}

void GameData::InitData()
{
	m_saveData.clear();
	m_killedEnemyNameData.clear();
	m_killedEnemyTypeNum = 0;
	m_clearBossData.clear();
	m_ability = Ability::kNone;
	m_isActiveAbilitys.clear();
	m_isInBoss = false;
}

int GameData::GetBestTime(const std::string& name) const
{
	auto it = m_saveData.find(name);
	// ステージを見つけられなかったら0を返す
	if (it == m_saveData.end())
	{
		return 0;
	}

	// 見つかったらベストタイムを返す
	return m_saveData.at(name).bestTime;
}

bool GameData::IsClearStage(const std::string& name)
{
	auto it = m_saveData.find(name);
	// ステージを見つけられなかったら0を返す
	if (it == m_saveData.end())
	{
		CreateData(name);
		return false;
	}

	// 見つかったらクリア情報を返す
	return m_saveData.at(name).isClear;
}

bool GameData::IsClearBoss(const std::string& name) const
{
	for (const auto& boss : m_clearBossData)
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

bool GameData::IsKilledEnemy(const std::string& name) const
{
	for (const auto& killedName : m_killedEnemyNameData)
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

void GameData::CreateData(const std::string& name)
{
	auto it = m_saveData.find(name);
	// ステージを見つけたら何もしない
	if (it != m_saveData.end())
	{
		return;
	}

	// なければ作成する
	auto& data = m_saveData[name];

	// 情報の初期化
	data.bestTime = 0;
	data.isClear = false;
}

void GameData::SaveClearStage(const std::string& name)
{
	auto it = m_saveData.find(name);
	// ステージを見つけられなかったら何もしない
	if (it == m_saveData.end())
	{
		assert(false);
		return;
	}

	// 指定のものをクリアとする
	m_saveData[name].isClear = true;
}


void GameData::SaveClearBoss(const std::string& name)
{
	for (const auto& boss : m_clearBossData)
	{
		// クリアしたことがあれば何もしない
		if (boss == name)
		{
			return;
		}
	}

	// 情報が乗っていない場合は追加する
	m_clearBossData.push_back(name);
	return;
}

void GameData::SaveKilledEnemyType(const std::string& name)
{
	// 配列全部繰り返す
	for (const auto& tableName : m_killedEnemyNameData)
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
	m_killedEnemyTypeNum++;
	// テーブルに登録
	m_killedEnemyNameData.push_back(name);

	// アビリティの有効化
	if (name == "Dash")
	{
		m_isActiveAbilitys[Ability::kDash] = true;

		// ダッシュ説明画像
		auto& mgr = GameManager::GetInstance();
		mgr.GetScene()->PushScene(std::make_shared<OneShotScene>(mgr, m_explanationDash->GetHandle()));
	}
}

bool GameData::SaveBestTime(const std::string& name, int bestTime)
{
	auto it = m_saveData.find(name);
	// ステージを見つけられなかったら何もしない
	if (it == m_saveData.end())
	{
		assert(false);
		return false;
	}

	auto& time = m_saveData[name].bestTime;

	// 現在保存されているタイムが更新タイムより大きければ更新は行わない
	if (time > bestTime)
	{
		return false;
	}

	// タイムの更新
	time = bestTime;
	return true;
}

void GameData::ChangeAbility(Ability ability)
{
	// アビリティが有効になっていたら変更させる
	if (m_isActiveAbilitys[ability])
	{
		m_ability = ability;
	}
}

void GameData::Save()
{
	// ファイルポインタ
	FILE* fp = nullptr;
	auto err = fopen_s(&fp, kPath, "wb");
	if (err != 0)
	{
		assert(false);
		return;
	}
	StageInfHeader header;
	header.dataCount = m_saveData.size();
	fwrite(&header, sizeof(header), 1, fp);

	// クリアデータの書き込み
	for (const auto& stage : m_saveData)
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
	fwrite(&m_killedEnemyTypeNum, sizeof(m_killedEnemyTypeNum), 1, fp);
	// 名前データの書き込み
	for (const auto& name : m_killedEnemyNameData)
	{
		// 名前の文字列数を取得
		uint8_t nameSize = static_cast<uint8_t>(name.size());
		// 文字列数を書き込み
		fwrite(&nameSize, sizeof(nameSize), 1, fp);
		// 文字列の書き込み
		fwrite(name.data(), name.size(), 1, fp);
	}

	// ボスクリア情報の書き込み
	uint8_t size = static_cast<uint8_t>(m_clearBossData.size());
	fwrite(&size, sizeof(size), 1, fp);
	// データの書き込み
	for (const auto& name : m_clearBossData)
	{
		uint8_t nameSize = static_cast<uint8_t>(name.size());
		fwrite(&nameSize, sizeof(nameSize), 1, fp);
		fwrite(name.data(), name.size(), 1, fp);
	}

	// アビリティの書き込み
	fwrite(&m_ability, sizeof(m_ability), 1, fp);

	// アビリティの有効テーブルの書き込み
	size = static_cast<uint8_t>(m_isActiveAbilitys.size());
	fwrite(&size, sizeof(size), 1, fp);
	for (const auto& active : m_isActiveAbilitys)
	{
		// アビリティタイプ書き込み
		fwrite(&active.first, sizeof(active.first), 1, fp);
		// 有効化どうか書き込み
		fwrite(&active.second, sizeof(active.second), 1, fp);
	}

	// ボスステージに入ったことがるかの情報
	fwrite(&m_isInBoss, sizeof(m_isInBoss), 1, fp);

	fclose(fp);
}

void GameData::Load()
{
	const auto& wPath = StringUtility::StringToWString(kPath);

	auto handle = FileRead_open(wPath.c_str());
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
		auto& data = m_saveData[stgStr];

		// データ情報の読み込み
		FileRead_read(&data, sizeof(data), handle);
	}

	// 殺された敵の種類数の取得
	FileRead_read(&m_killedEnemyTypeNum, sizeof(m_killedEnemyTypeNum), handle);
	// 名前情報群の数を種類数でリサイズ
	m_killedEnemyNameData.resize(m_killedEnemyTypeNum);
	for (int i = 0; i < m_killedEnemyTypeNum; i++)
	{
		// 名前文字列サイズを入れる変数
		uint8_t nameSize;
		// サイズの読み込み
		FileRead_read(&nameSize, sizeof(nameSize), handle);

		auto& name = m_killedEnemyNameData[i];
		// サイズで文字列をリサイズ
		name.resize(nameSize);
		// 文字列の読み込み
		FileRead_read(name.data(), static_cast<int>(name.size() * sizeof(char)), handle);
	}

	// ボスクリア情報の取得
	uint8_t size;
	FileRead_read(&size, sizeof(size), handle);
	m_clearBossData.resize(size);
	for (int i = 0; i < size; i++)
	{
		uint8_t nameSize;
		FileRead_read(&nameSize, sizeof(nameSize), handle);
		auto& name = m_clearBossData[i];
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

		m_isActiveAbilitys[ability] = active;
	}

	// ボスステージに入ったことがるかの情報
	FileRead_read(&m_isInBoss, sizeof(m_isInBoss), handle);

	// ファイルは閉じる
	FileRead_close(handle);
}

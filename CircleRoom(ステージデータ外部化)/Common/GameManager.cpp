#include "GameManager.h"
#include <DxLib.h>
#include <cassert>

#include "Scene/SceneManager.h"
#include "Stage/StageManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/SoundSystem.h"
#include "FileSystem/FontSystem.h"

GameManager::GameManager() :
	m_scene(std::make_shared<SceneManager>()),
	m_stage(std::make_shared<StageManager>(m_scene)),
	m_sound(std::make_shared<SoundSystem>()),
	m_font(std::make_shared<FontSystem>())
{
	// 初期データ
	InitData();
	m_data.volume = { 255, 255 };
	Load();
}

GameManager::~GameManager()
{
	m_sound->End();

	Save();
}

void GameManager::Init(Input& input)
{
	m_stage->Init(input);
	m_sound->Init();
	m_font->Init();
	m_scene->Init();
}

void GameManager::InitData()
{
	// プレイデータの初期化
	m_nowStage = "練習";
	m_data.playTime = 0;
	m_data.deathCount = 0;
	m_data.dashCount = 0;	
}

GameManager& GameManager::GetInstance()
{
	static GameManager instance;
	return instance;
}

void GameManager::SetFileManager(std::shared_ptr<FileManager>& file)
{
	// 元から入っているのに再度入れられたら止める
	if (m_file)
	{
		assert(false);
	}

	m_file = file;
}

std::shared_ptr<FileManager>& GameManager::GetFile()
{
	return m_file;
}

std::shared_ptr<SceneManager>& GameManager::GetScene()
{
	return m_scene;
}

std::shared_ptr<StageManager>& GameManager::GetStage()
{
	return m_stage;
}

std::shared_ptr<SoundSystem>& GameManager::GetSound()
{
	return m_sound;
}

std::shared_ptr<FontSystem>& GameManager::GetFont()
{
	return m_font;
}

AccountData GameManager::GetData() const
{
	return m_data;
}

void GameManager::Save()
{
	FILE* fp = nullptr;
	auto err = fopen_s(&fp, "Data/Bin/sdt.inf", "wb");
	if (err != 0)
	{
		// 読み込みに失敗したため終了
		assert(false);
		return;
	}

	// 現在のステージの書き込み
	uint8_t size = static_cast<uint8_t>(m_nowStage.size());
	fwrite(&size, sizeof(size), 1, fp);
	fwrite(m_nowStage.data(), size, 1, fp);

	// アカウントデータの書き込み
	fwrite(&m_data, sizeof(m_data), 1, fp);

	fclose(fp);
}

void GameManager::Load()
{
	auto handle = FileRead_open(L"Data/Bin/sdt.inf");
	// エラー(ファイルがない)場合は処理しない
	if (handle == 0)
	{
		return;
	}

	// 現在のステージの読み込み
	uint8_t size;
	FileRead_read(&size, sizeof(size), handle);
	m_nowStage.resize(size);
	FileRead_read(m_nowStage.data(), size, handle);

	// アカウントデータの読み込み
	FileRead_read(&m_data, sizeof(m_data), handle);

	FileRead_close(handle);
}

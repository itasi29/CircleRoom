#include <DxLib.h>
#include <cassert>

#include "ImageFile.h"
#include "SoundFile.h"
#include "FileManager.h"

FileManager::~FileManager()
{
    End();
}

void FileManager::End()
{
    std::wstring path;

    for (auto& file : m_fileTable)
    {
        file.second->Delete();
        file.second->End();
        path = file.first;
    }
}

std::shared_ptr<FileBase> FileManager::LoadGraphic(const std::wstring& path, bool isEternal)
{
    // 二重ロード防止
    // もし、ロード済みのファイルがあったらそのファイルを返す
    if (m_fileTable.find(path) != m_fileTable.end())
    {
        auto file = m_fileTable.at(path);

        // ImageFileにキャスト
        std::shared_ptr<ImageFile> imgFile = 
            std::dynamic_pointer_cast<ImageFile>(file);
        assert(imgFile != nullptr);
        // 所有者カウントを増加させます
        file->m_count++;
        // テーブルにあるファイルオブジェクトをコピーして使う
        // コピーコンストラクトを使用
        return std::make_shared<ImageFile>(*imgFile);
    }

    std::wstring dPath = L"Data/Image/" + path;
    // もし、テーブルにロード済みパスがない場合には
    // こちらを通る
    int handle = LoadGraph(dPath.c_str());
    assert(handle >= 0);
    m_fileTable[path] = std::make_shared<ImageFile>(*this);
    // 内部のファイルテーブルに情報を登録する
    auto& file = m_fileTable[path];
    file->m_handle = handle;
    file->m_path = path;
    file->m_count = 1;
    file->m_isEternal = isEternal;
    // テーブルにあるファイルオブジェクトのコピーを返す
    // デストラクタを機能させるためあえて「コピー」にしてる
    auto imgFile = std::dynamic_pointer_cast<ImageFile>(file);
    return std::make_shared<ImageFile>(*imgFile);
    //return std::make_shared<ImageFile>
    //    (*std::dynamic_pointer_cast<ImageFile>(file));
}

std::shared_ptr<FileBase> FileManager::LoadSound(const std::wstring& path, bool isEternal)
{
    // 二重ロード防止
// もし、ロード済みのファイルがあったらそのファイルを返す
    if (m_fileTable.find(path) != m_fileTable.end())
    {
        auto file = m_fileTable.at(path);

        // SoundFileにキャスト
        std::shared_ptr<SoundFile> sndFile =
            std::dynamic_pointer_cast<SoundFile>(file);
        assert(sndFile != nullptr);
        // 所有者カウントを増加させます
        file->m_count++;
        // テーブルにあるファイルオブジェクトをコピーして使う
        // コピーコンストラクトを使用
        return std::make_shared<SoundFile>(*sndFile);
    }
    std::wstring dPath = L"Data/Sound/" + path;
    // もし、テーブルにロード済みパスがない場合には
    // こちらを通る
    int handle = LoadSoundMem(dPath.c_str());
    assert(handle >= 0);
    m_fileTable[path] = std::make_shared<SoundFile>(*this);
    // 内部のファイルテーブルに情報を登録する
    auto& file = m_fileTable[path];
    file->m_handle = handle;
    file->m_path = path;
    file->m_count = 1;
    file->m_isEternal = isEternal;
    // テーブルにあるファイルオブジェクトのコピーを返す
    // デストラクタを機能させるためあえて「コピー」にしてる
    auto sndFile = std::dynamic_pointer_cast<SoundFile>(file);
    return std::make_shared<SoundFile>(*sndFile);
}

void FileManager::Delete(const std::wstring& path)
{
    // なかったら無視
    if (m_fileTable.find(path) == m_fileTable.end()) return;

    auto file = m_fileTable.at(path);
    if (file->m_isEternal) return;

    file->m_count--;    // 所有者カウントをデクリメント
    // 所有者ゼロなら本当に削除
    if (file->m_count == 0)
    {
        m_fileTable.erase(path);
        file->Delete();
        file->End();
    }
}
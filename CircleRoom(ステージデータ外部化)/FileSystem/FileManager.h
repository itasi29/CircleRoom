#pragma once

#include <unordered_map>	// マップ用(ファイルパステーブル用)
#include <string>			// ファイルパスを覚えとくよう
#include <memory>			// Fileのshared_ptrを保持しておくよう

class FileBase;

/// <summary>
/// ファイルのマップを内包し、ファイルを管理する
/// </summary>
class FileManager
{
public:
	// ファイルパス文字列をキー、値をFileオブジェクトとしたテーブル型
	using FileTable_t = std::unordered_map<std::wstring,
											std::shared_ptr<FileBase>>;
	FileTable_t m_fileTable;

public:
	~FileManager();
	void End();

	/// <summary>
	/// 指定のパスの画像データをロードしてそのファイルオブジェクトを返す
	/// </summary>
	/// <param name="path">ファイルパス</param>
	/// <param name="isEternal">常駐フラグ</param>
	/// <returns>ファイルオブジェクト</returns>
	std::shared_ptr<FileBase> LoadGraphic(const std::wstring& path, bool isEternal = false);
	/// <summary>
	/// 指定のパスのサウンドデータをロードしてそのファイルオブジェクトを返す
	/// </summary>
	/// <param name="path">ファイルパス</param>
	/// <param name="isEternal">常駐フラグ</param>
	/// <returns>ファイルオブジェクト</returns>
	std::shared_ptr<FileBase> LoadSound(const std::wstring& path, bool isEternal = false);

	/// <summary>
	/// リソースの削除を行う
	/// 実際には内部カウントを見て、0になったらそれぞれのリソースの
	/// 削除を命令します。
	/// </summary>
	/// <param name="path">ファイルパス</param>
	/// <memo>ファイルパスを渡すのはマップから検索するため</memo>
	void Delete(const std::wstring& path);
};


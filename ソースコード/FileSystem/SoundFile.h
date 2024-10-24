#pragma once
#include "FileBase.h"
/// <summary>
/// サウンド系のファイルのハンドルを内包し
/// 削除処理を適切に行う
/// </summary>
class SoundFile : public FileBase
{
public:
	SoundFile(FileManager& manager);
	~SoundFile();

private:
	void Delete() override;
};


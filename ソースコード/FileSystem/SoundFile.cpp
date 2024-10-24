#include "SoundFile.h"
#include "FileManager.h"
#include <DxLib.h>

SoundFile::SoundFile(FileManager& manager) :
	FileBase(manager)
{
}

SoundFile::~SoundFile()
{
	if (m_isEnd) return;
	if (m_isEternal) return;

	m_mgr.Delete(m_path);
}

void SoundFile::Delete()
{
	DeleteSoundMem(m_handle);
}

#include "ImageFile.h"
#include <DxLib.h>
#include "FileManager.h"

ImageFile::ImageFile(FileManager& manager) :
	FileBase(manager)
{
}

ImageFile::~ImageFile()
{
	if (m_isEnd) return;
	if (m_isEternal) return;

	m_mgr.Delete(m_path);
}

void ImageFile::Delete()
{
	DeleteGraph(m_handle);
}

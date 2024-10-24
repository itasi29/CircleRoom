#include "FileBase.h"

FileBase::FileBase(FileManager& manager) :
	m_mgr(manager),
	m_handle(-1),
	m_count(0),
	m_isEternal(false),
	m_isEnd(false)
{
}

FileBase::~FileBase()
{
	// ‰½‚à‚µ‚È‚¢
}

int FileBase::GetHandle() const
{
	return m_handle;
}

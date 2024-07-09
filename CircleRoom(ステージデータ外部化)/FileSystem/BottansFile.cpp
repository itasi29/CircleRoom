#include <DxLib.h>
#include <cassert>
#include "StringUtility.h"

#include "BottansFile.h"

#include "FileManager.h"
#include "FileBase.h"

namespace
{
	constexpr int kGraphSize = 16;
}

BottansFile::BottansFile(std::shared_ptr<FileManager>& mgr)
{
	m_handle = mgr->LoadGraphic(L"UI/buttons.png", true)->GetHandle();

	m_bottans[L"‚`Bottan"] = { 0, 10 };
	m_bottans[L"‚aBottan"] = { 1, 10 };
	m_bottans[L"‚wBottan"] = { 2, 10 };
	m_bottans[L"‚xBottan"] = { 3, 10 };
	m_bottans[L"‚kBottan"] = { 4, 10 };
	m_bottans[L"‚qBottan"] = { 5, 10 };
	m_bottans[L"‚r‚s‚`‚q‚sBottan"] = { 3, 11 };
	m_bottans[L"‚lBottan"] = { 2, 11 };
}

BottansFile::~BottansFile()
{
}

void BottansFile::DrawBottan(std::wstring name, int drawX, int drawY, double rate) const
{
	auto it = m_bottans.find(name);
	if (it == m_bottans.end())
	{
		//assert(false);
		return;
	}

	int offset = static_cast<int>(kGraphSize * 0.5 * rate);
	drawX += offset;
	drawY += offset;

	auto& src = m_bottans.at(name);

	// ‰æ‘œ‚Ì•`‰æ
	DrawRectRotaGraph(drawX, drawY, src.x * kGraphSize, src.y * kGraphSize, 
		kGraphSize, kGraphSize, rate, 0.0, m_handle, true);
}

void BottansFile::DrawBottanAndString(std::wstring name, int drawGraphX, int drawGraphY, int drawStrX, int drawStrY, double rate) const
{
	DrawBottan(name, drawGraphX, drawGraphX, rate);

	// •¶Žš‚Ì•`‰æ
	DrawFormatString(drawStrX, drawStrY, 0xffffff, L"%s", name.c_str());
}

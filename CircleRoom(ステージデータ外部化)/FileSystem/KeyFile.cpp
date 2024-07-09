#include <DxLib.h>
#include <cassert>
#include "StringUtility.h"

#include "FileManager.h"
#include "FileBase.h"

#include "KeyFile.h"

namespace
{
	constexpr int kGraphSize = 16;
}

KeyFile::KeyFile(std::shared_ptr<FileManager>& mgr)
{
	m_handle = mgr->LoadGraphic(L"UI/Keyboard.png", true)->GetHandle();

	m_key[L"Ａキー"] = { 0, 2 };
	m_key[L"Ｂキー"] = { 1, 2 };
	m_key[L"Ｃキー"] = { 2, 2 };
	m_key[L"Ｄキー"] = { 3, 2 };
	m_key[L"Ｅキー"] = { 4, 2 };
	m_key[L"Ｆキー"] = { 5, 2 };
	m_key[L"Ｇキー"] = { 6, 2 };
	m_key[L"Ｈキー"] = { 7, 2 };
	m_key[L"Ｉキー"] = { 0, 3 };
	m_key[L"Ｊキー"] = { 1, 3 };
	m_key[L"Ｋキー"] = { 2, 3 };
	m_key[L"Ｌキー"] = { 3, 3 };
	m_key[L"Ｍキー"] = { 4, 3 };
	m_key[L"Ｎキー"] = { 5, 3 };
	m_key[L"Ｏキー"] = { 6, 3 };
	m_key[L"Ｐキー"] = { 7, 3 };
	m_key[L"Ｑキー"] = { 0, 4 };
	m_key[L"Ｒキー"] = { 1, 4 };
	m_key[L"Ｓキー"] = { 2, 4 };
	m_key[L"Ｔキー"] = { 3, 4 };
	m_key[L"Ｕキー"] = { 4, 4 };
	m_key[L"Ｖキー"] = { 5, 4 };
	m_key[L"Ｗキー"] = { 6, 4 };
	m_key[L"Ｘキー"] = { 7, 4 };
	m_key[L"Ｙキー"] = { 0, 5 };
	m_key[L"Ｚキー"] = { 1, 5 };
	m_key[L"BSキー"] = { 14, 0, true };
	m_key[L"Tabキー"] = { 8, 0, true };
	m_key[L"Enterキー"] = { 14, 1, true };
	m_key[L"左Shiftキー"] = { 8, 1, true };
	m_key[L"右Shiftキー"] = { 8, 1, true };
	m_key[L"左Ctrlキー"] = { 8, 2, true };
	m_key[L"右Ctrlキー"] = { 8, 2, true };
	m_key[L"Escキー"] = { 10, 0, true };
	m_key[L"スペースキー"] = { 12, 2, true };
}

KeyFile::~KeyFile()
{
}

void KeyFile::DrawKey(std::wstring name, int drawX, int drawY, double rate) const
{
	auto it = m_key.find(name);
	if (it == m_key.end())
	{
		//assert(false);
		return;
	}

	int offset = static_cast<int>(kGraphSize * 0.5 * rate);
	drawX += offset;
	drawY += offset;

	// 画像の描画
	int width = kGraphSize;
	if (it->second.isLink)
	{
		width += kGraphSize;
	}

	DrawRectRotaGraph(drawX, drawY, it->second.x * kGraphSize, it->second.y * kGraphSize,
		width, kGraphSize, rate, 0.0, m_handle, true);
}

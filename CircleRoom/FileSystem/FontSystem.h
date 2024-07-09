#pragma once
#include <unordered_map>
#include <windows.h>

/// <summary>
/// フォントを扱うクラス
/// </summary>
class FontSystem
{
public:
	FontSystem();
	~FontSystem();

	void Init();

	int GetHandle(int size) const;

private:
	LPCWSTR m_fontPath;

	// フォントのハンドル
	// Key:フォントのサイズ　Value:フォントハンドル
	std::unordered_map<int, int> m_handle;
};


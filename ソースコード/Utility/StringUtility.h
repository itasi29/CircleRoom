#pragma once
#include <string>
class StringUtility
{
public:
	/// <summary>
	/// マルチバイト文字列からワイド文字列への変換
	/// </summary>
	/// <param name="str">変換元のマルチバイト文字列</param>
	/// <returns>変換後のワイド文字列</returns>
	static std::wstring  
		StringToWString(const std::string& str);
};


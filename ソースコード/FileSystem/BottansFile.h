#pragma once
#include <string>
#include <unordered_map>
#include <memory>

/// <summary>
/// ΔpBottan
/// </summary>

class FileManager;

struct PadSrcData
{
	int x;
	int y;
};

class BottansFile
{
public:
	BottansFile(std::shared_ptr<FileManager>& mgr);
	~BottansFile();

	/// <summary>
	/// {^ΜζΜέ`ζ
	/// </summary>
	/// <param name="name">BottanΌ</param>
	/// <param name="drawX">Ά[</param>
	/// <param name="drawY">γ[</param>
	/// <param name="rate">{¦</param>
	void DrawBottan(std::wstring name, int drawX, int drawY, double rate = 1.0) const;
	/// <summary>
	/// {^Μζ{Ά`ζ
	/// </summary>
	/// <param name="name">BottanΌ</param>
	/// <param name="drawGraphX">ζΆ[</param>
	/// <param name="drawGraphY">ζγ[</param>
	/// <param name="drawStrX">ΆΆ[</param>
	/// <param name="drawStrY">Άγ[</param>
	/// <param name="rate">{¦</param>
	void DrawBottanAndString(std::wstring name, int drawGraphX, int drawGraphY, int drawStrX, int drawStrY, double rate = 1.0) const;

private:
	int m_handle;

	// {^Ό@Ψθ²«ΝΝ
	std::unordered_map<std::wstring, PadSrcData> m_bottans;	
};


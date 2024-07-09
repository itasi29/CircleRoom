#pragma once
#include <vector>
#include <string>
#include "Vec2.h"

enum class MapDir
{
	kUp,
	kDown,
	kRight,
	kLeft,
	kMax
};

struct LinkInfo
{
	bool isUp = false;
	std::string upName = {};
	bool isDown = false;
	std::string downName = {};
	bool isRight = false;
	std::string rightName = {};
	bool isLeft = false;
	std::string leftName = {};
};

struct MapInfo
{
	std::string name;
	LinkInfo info;
};

class StageMap
{
public:
	StageMap();

	/// <summary>
	/// 接続確認
	/// </summary>
	/// <param name="stage">現在ステージ名</param>
	/// <param name="dir">方向</param>
	/// <returns>true: 接続している / false:接続していない</returns>
	bool IsConect(const std::string& stage, MapDir dir) const;
	/// <summary>
	/// 接続しているところの名前を取得
	/// </summary>
	/// <param name="stage">現在ステージ名</param>
	/// <param name="dir">方向</param>
	/// <returns>ステージ名</returns>
	std::string GetConectName(const std::string& stage, MapDir dir) const;

	// ステージ位置を返す
	Vec2 GetPos(const std::string& stage) const;

private:
	// 要素番号取得
	int GetIndex(const std::string& stage) const;

	void Load();

	// 方向別情報確認
	void CheckUp(MapInfo& data, bool isStart, int index, const std::vector<std::string>& strInfo, const std::vector<std::string>& strName);
	void CheckDown(MapInfo& data, bool isLoop, int index, const std::vector<std::string>& strInfo, const std::vector<std::string>& strName);
	void CheckLeft(MapInfo& data, int index, const std::vector<std::string>& str);
	void CheckRight(MapInfo& data, int index, const std::vector<std::string>& str);

private:
	std::vector<MapInfo> m_data;
	int m_line;
	int m_row;
};


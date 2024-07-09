#include <cassert>
#include <fstream>
#include <sstream>
#include "StringUtility.h"
#include "Application.h"
#include "StageMap.h"

namespace
{
	constexpr int kStageMarginW = 1280;
	constexpr int kStageMarginH = 720;
}

StageMap::StageMap() :
	m_line(0),
	m_row(0)
{
	Load();
}

bool StageMap::IsConect(const std::string& stage, MapDir dir) const
{
	auto& data = m_data[GetIndex(stage)].info;

	if (dir == MapDir::kUp)
	{
		return data.isUp;
	}
	else if (dir == MapDir::kDown)
	{
		return data.isDown;
	}
	else if (dir == MapDir::kRight)
	{
		return data.isRight;
	}
	else if (dir == MapDir::kLeft)
	{
		return data.isLeft;
	}
}

std::string StageMap::GetConectName(const std::string& stage, MapDir dir) const
{
#ifdef _DEBUG
	assert(IsConect(stage, dir));
#endif
	auto& data = m_data[GetIndex(stage)].info;

	if (dir == MapDir::kUp)
	{
		return data.upName;
	}
	else if (dir == MapDir::kDown)
	{
		return data.downName;
	}
	else if (dir == MapDir::kRight)
	{
		return data.rightName;
	}
	else if (dir == MapDir::kLeft)
	{
		return data.leftName;
	}
}

/// <summary>
/// ステージ位置を返す
/// </summary>
/// <param name="stage">ステージ名</param>
/// <returns>位置</returns>
Vec2 StageMap::GetPos(const std::string& stage) const
{
	Vec2 pos = {};
	bool isEnd = false;

	for (int x = 0; x < m_line; x++)
	{
		for (int y = 0; y < m_row; y++)
		{
			if (m_data[x + y * m_line].name == stage)
			{
				const auto& wSize = Application::GetInstance().GetWindowSize();

				pos.x = static_cast<float>(wSize.w * x + kStageMarginW * x);
				pos.y = static_cast<float>(wSize.h * y + kStageMarginH * y);

				isEnd = true;
				break;
			}
		}

		if (isEnd) break;
	}

	return pos;
}

/// <summary>
/// ステージ名に対応する要素番号を返す
/// </summary>
/// <param name="stage">ステージ名</param>
/// <returns>要素番号</returns>
int StageMap::GetIndex(const std::string& stage) const
{
	for (int i = 0; i < static_cast<int>(m_data.size()); i++)
	{
		if (m_data[i].name != stage) continue;

		return i;
	}
}

void StageMap::Load()
{
	// 一時保存用string
	std::string strBuf;
	// カンマ分け一時保存用string
	std::vector<std::string> strUpConectNameBuf;
	std::vector<std::string> strUpConectInfoBuf;
	std::vector<std::string> strNowBuf;
	std::vector<std::string> strDownConectInfoBuf;
	std::vector<std::string> strDownConectNameBuf;

	// ファイル読み込み
	std::ifstream ifs("Data/Stage/MapData.csv");
	if (!ifs)
	{
		assert(false);
		return;
	}

	std::getline(ifs, strBuf);
	strNowBuf = StringUtility::Split(strBuf, ',');

	// 行列数取得
	m_line = std::stoi(strNowBuf[1]);
	m_row = std::stoi(strNowBuf[3]);
	// サイズ変更
	m_data.resize(m_line * m_row);

	// 要素
	int index = 0;
	// 列数
	int line = 0;
	// 継続フラグ
	bool isLoop = true;
	// 最初フラグ
	bool isStart = true;

	while (isLoop)
	{
		if (isStart)
		{
			// 現在情報
			std::getline(ifs, strBuf);
			strNowBuf = StringUtility::Split(strBuf, ',');
		}
		else
		{
			// 情報更新
			strUpConectNameBuf = strNowBuf;
			strUpConectInfoBuf = strDownConectInfoBuf;
			strNowBuf = strDownConectNameBuf;
		}

		// 下接続情報
		std::getline(ifs, strBuf);
		// 文字が入っていたら継続
		isLoop = !(strBuf.empty());
		if (isLoop)
		{
			strDownConectInfoBuf = StringUtility::Split(strBuf, ',');
			// 下名前情報
			std::getline(ifs, strBuf);
			strDownConectNameBuf = StringUtility::Split(strBuf, ',');
		}
		
		// データ取得
		for (int i = 0; i < m_line; i++)
		{
			int namePos = i * 2;

			auto& data = m_data[index];
			data.name = strNowBuf[namePos];

			CheckUp(data, isStart, namePos, strUpConectInfoBuf, strUpConectNameBuf);
			CheckDown(data, isLoop, namePos, strDownConectInfoBuf, strDownConectNameBuf);
			CheckLeft(data, namePos, strNowBuf);
			CheckRight(data, namePos, strNowBuf);

			// 要素番号更新
			index++;
		}

		isStart = false;
	}

	return;
}

/// <summary>
/// 上方向確認
/// </summary>
/// <param name="data">マップデータ</param>
/// <param name="isStart">ループ最初か</param>
/// <param name="index">要素番号</param>
/// <param name="strInfo">接続情報文字列</param>
/// <param name="strName">接続名前文字列</param>
void StageMap::CheckUp(MapInfo& data, bool isStart, int index, const std::vector<std::string>& strInfo, const std::vector<std::string>& strName)
{
	auto& flag = data.info.isUp;
	if (isStart)
	{
		flag = false;
	}
	else
	{
		flag = static_cast<bool>(std::stoi(strInfo[index]));
		if (!flag) return;
		data.info.upName = strName[index];
	}
}

/// <summary>
/// 下方向確認
/// </summary>
/// <param name="data">マップデータ</param>
/// <param name="isStart">ループ継続か</param>
/// <param name="index">要素番号</param>
/// <param name="strInfo">接続情報文字列</param>
/// <param name="strName">接続名前文字列</param>
void StageMap::CheckDown(MapInfo& data, bool isLoop, int index, const std::vector<std::string>& strInfo, const std::vector<std::string>& strName)
{
	auto& flag = data.info.isDown;
	if (!isLoop)
	{
		flag = false;
	}
	else
	{
		flag = static_cast<bool>(std::stoi(strInfo[index]));
		if (!flag) return;
		data.info.downName = strName[index];
	}
}

/// <summary>
/// 左方向確認
/// </summary>
/// <param name="data">マップデータ</param>
/// <param name="index">要素番号</param>
/// <param name="str">現在情報文字列</param>
void StageMap::CheckLeft(MapInfo& data, int index, const std::vector<std::string>& str)
{
	auto& flag = data.info.isLeft;
	if (index - 1 < 0)
	{
		flag = false;
	}
	else
	{
		flag = static_cast<bool>(std::stoi(str[index - 1]));
		if (!flag) return;
		data.info.leftName = str[index - 2];
	}
}

/// <summary>
/// 右方向確認
/// </summary>
/// <param name="data">マップデータ</param>
/// <param name="index">要素番号</param>
/// <param name="str">現在情報文字列</param>
void StageMap::CheckRight(MapInfo& data, int index, const std::vector<std::string>& str)
{
	auto& flag = data.info.isRight;
	if (index + 1 >= m_line * 2 - 1)
	{
		flag = false;
	}
	else
	{
		flag = static_cast<bool>(std::stoi(str[index + 1]));
		if (!flag) return;
		data.info.rightName = str[index + 2];
	}
}

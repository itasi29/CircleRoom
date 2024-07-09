#pragma once
#include <array>
#include <map>
#include <unordered_map>
#include <string>
#include "Vec2.h"

enum class InputType
{
	keybd,	// キーボード
	pad		// パッド
};

using InputTable_t = std::unordered_map<std::string, std::map<InputType, std::vector<int>>>;

class KeyConfigScene;
class PadConfigScene;

class Input
{
	friend KeyConfigScene;
	friend PadConfigScene;
private:
	// コマンド名と入力をペアにしたテーブル
	// キーボードの時は keybd&KEY_INPUT_○○で、
	// パッドの時はpad&PAD_INPUT_○○
	// をチェックする
	InputTable_t m_commandTable;
	std::vector<std::string> m_exclusiveKeyConfigCommands;

	// 値と名前の対応表
	std::unordered_map<InputType, std::unordered_map<int , std::wstring>> m_corrTable;

	// コマンドの入力を覚えておく
	std::map<std::string, bool> m_inputDate;		// 現在の入力
	std::map<std::string, bool> m_lastInputDate;	// 直前の入力

	// スティック情報
	Vec2 m_inputStickDate;
	// 何かしらのボタン押された情報
	bool m_isAnyPush;
	bool m_isLastAnyPush;

	// 最後に押されたのがキーボードかパッドかを覚えておく
	InputType m_lastType;

	const InputTable_t GetCommandTable() const;
	std::vector<std::string> GetExclusiveCommandTable() const { return m_exclusiveKeyConfigCommands; }

public:
	Input();
	/// <summary>
	/// 入力の情報を更新する
	/// </summary>
	void Update();
	/// <summary>
	/// 指定のコマンドが押された瞬間なのか
	/// </summary>
	/// <param name="command">コマンド文字列</param>
	/// <returns>true:押された瞬間 / false:押されていないか押しっぱ</returns>
	bool IsTriggered(const char* command) const;
	/// <summary>
	/// 何かしらのボタンが押されたか
	/// </summary>
	/// <returns>true:押された瞬間 / false:押されていないか押しっぱ</returns>
	bool IsAnyTriggerd();
	/// <summary>
	/// 指定のコマンドが押されているか
	/// </summary>
	/// <param name="command">コマンド文字列</param>
	/// <returns>true:押されている / false:押されていない</returns>
	bool IsPress(const char* command) const;
	/// <summary>
	/// 何かしらのボタンが押されているか
	/// </summary>
	/// <returns>true:押されている / false:押されていない</returns>
	bool IsAnyPress() const;
	/// <summary>
	/// 指定のコマンドが押されていないか
	/// </summary>
	/// <param name="command">コマンド文字列</param>
	/// <returns>true:押されていない / false:押されている</returns>
	bool IsNotPress(const char* command) const;
	/// <summary>
	/// 指定のコマンドが離された瞬間なのか
	/// </summary>
	/// <param name="command">コマンド文字列</param>
	/// <returns>true:離された瞬間 / false:それ以外</returns>
	bool IsReleased(const char* command) const;
	/// <summary>
	/// 押された瞬間＋押されて行って時間が経過されている
	/// </summary>
	/// <param name="command">コマンド文字列</param>
	/// <param name="frame">フレームの参照</param>
	/// <param name="frameInterval">フレームの間隔</param>
	/// <returns></returns>
	bool IsReepat(const char* command, int& frame, int frameInterval) const;

	InputType GetType() const { return m_lastType; }
	std::wstring GetHardDataName(const std::string cmd, InputType type) const;

	void Save(const std::string& path);
	void Load(const std::wstring& path);

	/// <summary>
	/// スティックの情報を渡す
	/// </summary>
	/// <returns>スティック情報</returns>
	Vec2 GetStickDate() const { return m_inputStickDate; }

	float GetStickTilt() const { return m_inputStickDate.Length(); }
};


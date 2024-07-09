#pragma once

struct size
{
	int w;
	int h;
};

/// <summary>
/// アプリケーション全体を管理するクラス
/// シングルトンクラス
/// </summary>
class Application
{
private:
	Application();	// シングルトンのためにprivateに
	// コピー＆代入の禁止
	Application(const Application& app) = delete;		// コピーコンストラクタの廃止
	void operator =(const Application& app) = delete;	// 代入コンストラクタの廃止

public:
	/// <summary>
	/// Applicationのシングルトンインスタンスを返す
	/// </summary>
	/// <returns>Applicationインスタンスの参照を返す</returns>
	static Application& GetInstance()
	{
		static Application instance;	// 自分自身の静的オブジェクトを作ります。
		return instance;
	}

	bool Init();
	void Run();
	/// <summary>
	/// 後処理を行う
	/// </summary>
	void Terminate();

	/// <summary>
	/// ゲームの終了
	/// </summary>
	void End();

	const size& GetWindowSize() const;

	/// <summary>
	/// ウィンドモードの切り替え
	/// </summary>
	void ChangeWindows();

	/// <summary>
	/// 現在のウィンドモードの状態を取得
	/// </summary>
	/// <returns>ウィンドウモード</returns>
	bool IsWindows() const { return m_isWindows; }

private:
	size m_size;
	// FPS固定用
	long long m_time;

	// ゲーム終了フラグ
	bool m_isEnd;

	// ウィンドウフラグ
	bool m_isWindows;
};


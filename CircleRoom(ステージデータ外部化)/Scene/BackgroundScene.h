#pragma once
#include <vector>
#include <memory>

class FileBase;
class FileManager;

class BackgroundScene
{
public:
	BackgroundScene(std::shared_ptr<FileManager> mgr, bool isMove = true, bool isDraw = true);
	~BackgroundScene();

	void Update();
	void Draw() const;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="speed">速度(何も入れなければデフォルトスピードで動く)</param>
	void Init(float speed = -1);

private:
	std::vector<std::shared_ptr<FileBase>> m_bg;

	float m_scroll;
	float m_scrollSpeed;
	bool m_isMove;

	bool m_isDraw;
};


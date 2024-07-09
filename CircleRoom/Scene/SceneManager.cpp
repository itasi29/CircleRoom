#include <DxLib.h>
#include <array>
#include "Application.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "FileSystem/FileManager.h"
#include "FileSystem/FileBase.h"
#include "Scene.h"
#include "BackgroundScene.h"

namespace
{
	constexpr int kShakeSize = 40;

	// 一度に進ませる量
	constexpr float kRad = DX_PI_F / 180 * 2.5f;
}

SceneManager::SceneManager(bool isDrawBg) :
	m_isShake(false),
	m_shakeFrame(0),
	m_shakeSize(kShakeSize),
	m_isDrawBg(isDrawBg),
	m_isShader(false),
	m_angle(0.0),
	m_isAllGaussianBlur(false)
{
	m_updateFunc = &SceneManager::NormalUpdate;
	m_drawFunc = &SceneManager::NormalDraw;

	const size size = Application::GetInstance().GetWindowSize();
	m_shakeHandle = MakeScreen(size.w, size.h);
	m_drawScreen = MakeScreen(size.w, size.h);
	m_shaderScreen = MakeScreen(size.w, size.h);
	m_gaussianBlurScreen = MakeScreen(size.w, size.h, true);

	m_wavePs = LoadPixelShader(L"Data/Shader/wavePs.pso");
	m_ps = LoadPixelShader(L"Data/Shader/ps.pso");

	m_cbuffer = CreateShaderConstantBuffer(sizeof(float) * 4);
	m_gAngle = static_cast<float*>(GetBufferShaderConstantBuffer(m_cbuffer));
}

SceneManager::~SceneManager()
{
	DeleteGraph(m_shakeHandle);
	DeleteGraph(m_drawScreen);
	DeleteGraph(m_shaderScreen);
	DeleteGraph(m_gaussianBlurScreen);

	DeleteShader(m_wavePs);
	DeleteShader(m_ps);

	DeleteShaderConstantBuffer(m_cbuffer);
}

void SceneManager::Init()
{
	if (m_bg) return;

	auto& file = GameManager::GetInstance().GetFile();
	m_bg = std::make_shared<BackgroundScene>(file, true, m_isDrawBg);
}

void SceneManager::Update(Input& input)
{
	(this->*m_updateFunc)(input);
}

void SceneManager::Draw() const
{
	(this->*m_drawFunc)();
}

void SceneManager::ChangeScene(std::shared_ptr<Scene>nextScene, float speed)
{
	m_bg->Init(speed);
	if (m_scenes.empty()) {	// リストが空っぽだったら入れ替えるのではなく
		m_scenes.push_back(nextScene);	// 末尾に追加する
	}
	else {
		m_scenes.back() = nextScene;	// すでに一つ以上あったら末尾のものを入れ替える
	}
}

void SceneManager::ChangeSceneWithClear(std::shared_ptr<Scene> nextScene, float speed)
{
	m_bg->Init(speed);
	// 一度すべて消す
	m_scenes.clear();
	// シーンを追加
	m_scenes.push_back(nextScene);
}

void SceneManager::PushScene(std::shared_ptr<Scene> scene)
{
	m_scenes.push_back(scene);
}

void SceneManager::PopScene()
{
	m_scenes.pop_back();
}

void SceneManager::ShakeScreen(int frame)
{
	ShakeScreen(frame, kShakeSize);
}

void SceneManager::ShakeScreen(int frame, int size = kShakeSize)
{
	m_shakeFrame = frame;
	m_shakeSize = size;
	m_isShake = true;

	m_updateFunc = &SceneManager::ShakeUpdate;
	m_drawFunc = &SceneManager::ShakeDraw;
}

void SceneManager::OnShader()
{
	m_angle = 3.14f;
	m_isShader = true;

	m_updateFunc = &SceneManager::ShaderUpdate;
	m_drawFunc = &SceneManager::ShaderDraw;
}

void SceneManager::OnGaussianBlur(bool isAll)
{
	m_isAllGaussianBlur = isAll;

	m_updateFunc = &SceneManager::GaussianBlurUpdate;
	m_drawFunc = &SceneManager::GaussianBlurDraw;
}

void SceneManager::OnNormal()
{
	m_isShake = false;
	m_isShader = false;
	m_updateFunc = &SceneManager::NormalUpdate;
	m_drawFunc = &SceneManager::NormalDraw;
}

std::shared_ptr<Scene> SceneManager::GetTopScene()
{
	return m_scenes.back();
}

int SceneManager::GetScreenHandle() const
{
	if (m_isShake)
	{
		return m_shakeHandle;
	}
	else if (m_isShader)
	{
		return m_drawScreen;
	}
	else
	{
		return DX_SCREEN_BACK;
	}
}

void SceneManager::NormalUpdate(Input& input)
{
	// 背景の更新
	m_bg->Update();

	// 末尾のみ実行
	m_scenes.back()->Update(input);
}

void SceneManager::ShakeUpdate(Input& input)
{
	NormalUpdate(input);

	m_shakeFrame--;

	if (m_shakeFrame < 0)
	{
		m_isShake = false;

		m_updateFunc = &SceneManager::NormalUpdate;
		m_drawFunc = &SceneManager::NormalDraw;
	}
}

void SceneManager::ShaderUpdate(Input& input)
{
	NormalUpdate(input);

	m_angle += kRad;

	*m_gAngle = m_angle;

	UpdateShaderConstantBuffer(m_cbuffer);
	SetShaderConstantBuffer(m_cbuffer, DX_SHADERTYPE_PIXEL, 3);
}

void SceneManager::GaussianBlurUpdate(Input& input)
{
	NormalUpdate(input);
}

void SceneManager::NormalDraw() const
{
	m_bg->Draw();
	// 先頭から順に描画(最後に積んだものが最後に描画される)
	for (auto& scene : m_scenes)
	{
		scene->Draw();
	}
}

void SceneManager::ShakeDraw() const
{
	SetDrawScreen(m_shakeHandle);
	ClearDrawScreen();

	NormalDraw();

	SetDrawScreen(DX_SCREEN_BACK);

	int x = GetRand(m_shakeSize) - static_cast<int>(m_shakeSize * 0.5f);
	int y = GetRand(m_shakeSize) - static_cast<int>(m_shakeSize * 0.5f);
	DrawGraph(x, y, m_shakeHandle, true);
}


void SceneManager::ShaderDraw() const
{
	const size& size = Application::GetInstance().GetWindowSize();

	SetDrawScreen(m_drawScreen);
	ClearDrawScreen();
	NormalDraw();

	SetDrawScreen(m_shaderScreen);
	ClearDrawScreen();
	SetUsePixelShader(m_wavePs);
	MyDraw(0, 0, size.w, size.h);

	SetDrawScreen(DX_SCREEN_BACK);
	ClearDrawScreen();
	SetUsePixelShader(m_ps);
	SetUseTextureToShader(0, m_drawScreen);
	SetUseTextureToShader(1, m_shaderScreen);
	MyDraw(0, 0, size.w, size.h);

}

void SceneManager::GaussianBlurDraw() const
{
	SetDrawScreen(m_gaussianBlurScreen);
	ClearDrawScreen();

	m_bg->Draw();

	if (m_isAllGaussianBlur)
	{
		for (const auto& scene : m_scenes)
		{
			scene->Draw();
		}
	}
	else
	{
		auto itr = m_scenes.begin();
		// 最後のシーン以外を描画させたいからサイズ-1
		int index = static_cast<int>(m_scenes.size()) - 1;
		for (int i = 0; i < index; i++)
		{
			itr->get()->Draw();
			itr++;
		}
	}

	// ぼかす
	GraphFilter(m_gaussianBlurScreen, DX_GRAPH_FILTER_GAUSS, 32, 1400);

	SetDrawScreen(DX_SCREEN_BACK);
	DrawGraph(0, 0, m_gaussianBlurScreen, false);
	// 全体ぼかす場合は既に書いている
	if (!m_isAllGaussianBlur)
	{
		m_scenes.back()->Draw();
	}

}

void SceneManager::MyDraw(int x, int y, int width, int height) const
{
	std::array<VERTEX2DSHADER, 4> verts;
	for (auto& v : verts) {
		// これだけよくわからなかったが基本1でいいらしい
		v.rhw = 1.0f;
		// 拡散反射光
		v.dif = GetColorU8(0xff, 0xff, 0xff, 0xff);
		// テクスチャ座標０
		v.u = 0.0f;
		v.v = 0.0f;
		v.su = 0.0f;
		// テクスチャ座標１
		v.sv = 0.0f;
		v.pos.z = 0.0f;
		// 鏡面反射光
		v.spc = GetColorU8(0, 0, 0, 0);
	}
	// 左上
	verts[0].pos.x = static_cast<float>(x);
	verts[0].pos.y = static_cast<float>(y);
	// 右上
	verts[1].pos.x = static_cast<float>(x + width);
	verts[1].pos.y = static_cast<float>(y);
	verts[1].u = 1.0f;
	// 左下
	verts[2].pos.x = static_cast<float>(x);
	verts[2].pos.y = static_cast<float>(y + height);
	verts[2].v = 1.0f;
	// 右下
	verts[3].pos.x = static_cast<float>(x + width);
	verts[3].pos.y = static_cast<float>(y + height);
	verts[3].u = 1.0f;
	verts[3].v = 1.0f;
	// 描画
	DrawPrimitive2DToShader(verts.data(), static_cast<int>(verts.size()), DX_PRIMTYPE_TRIANGLESTRIP);
}

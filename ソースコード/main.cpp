#include "Application.h"
#include <DxLib.h>

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) 
{
	Application& application = Application::GetInstance();

	if (!application.Init())
	{
		// このアプリは不正終了しました。
		return -1;
	}
	application.Run();
	application.Terminate();

	// このアプリは正常終了しました。
	return 0;
}
#include"GameApp.h"
#include<windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	auto& game = GameApp::Instance();

	if (!game.Initialize()) {
		return -1;
	}

	game.Run();
	game.Delete();

	return 0;
}
#include "App.hpp"
#include <stdio.h>

// handle windows release gui version of the application (no console, so entry point has to be WinMain)
#if !defined(__APPLE__) && defined(NEDBUG)
#include <Windows.h>
int main(int argc, char** argv);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	main(0,0);
}
#endif

int main(int, char**)
{
	App app;
	app.Run();

    return 0;
}

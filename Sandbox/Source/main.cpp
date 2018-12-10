#include "Game.h"
#include <Windows.h>

#include "Dependencies/LivePP/API/LPP_API.h"

using namespace lpp;

int main(int argc, char **argv)
{
	// HINSTANCE hGetProcIDDLL = LoadLibrary("LPP_x86.dll");
	// 
	// if (!hGetProcIDDLL) {
	// 	std::cout << "could not load the dynamic library" << std::endl;
	// 	return EXIT_FAILURE;
	// }

	HMODULE livePP = lppLoadAndRegister(L"../../CodeLibraries/LivePP/LivePP", "ZeroEngineGroup");

	lppRegisterProcessGroup(livePP, "AGroupName");
	lppEnableAllCallingModulesSync(livePP);

	Game game;
	int ret = game.Execute();

	FreeLibrary(livePP);

	return ret;
}

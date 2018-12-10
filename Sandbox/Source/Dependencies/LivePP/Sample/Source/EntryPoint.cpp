
#include <Windows.h>
#include "SDL.h"
#include "Vec.h"
#include "Ray.h"
#include "Update.h"

// Live++ API. needed to load the DLL and install hooks.
#include "../../API/LPP_API.h"


static const unsigned int SCREEN_WIDTH = 512u;
static const unsigned int SCREEN_HEIGHT = 512u;
static const unsigned int PIXEL_COUNT = SCREEN_WIDTH * SCREEN_HEIGHT;

Vec* g_colorData = NULL;
unsigned char* g_rgbaData = NULL;
unsigned int* g_sampleCountPerPixel = NULL;
uint64_t g_raysTraced = 0ull;
bool g_showCompileAnimation = false;

void ClearImageData(void)
{
	if (g_colorData)
		memset(g_colorData, 0, sizeof(Vec)*PIXEL_COUNT);

	if (g_rgbaData)
		memset(g_rgbaData, 0, sizeof(unsigned char)*PIXEL_COUNT*4u);

	if (g_sampleCountPerPixel)
		memset(g_sampleCountPerPixel, 0, sizeof(unsigned int)*PIXEL_COUNT);

	g_raysTraced = 0ull;

	// Live++: try doing other things in the hook.
	// or try commenting the above lines, then the image and trace data won't be cleared when a new patch is installed
}

// Live++: install a hook that automatically clears image data whenever Live++ installs a new code patch
LPP_POSTPATCH_HOOK(ClearImageData);


void OnCompileStart(void)
{
	SDL_Log("Live++ compilation started\n");
	g_showCompileAnimation = true;
}

// Live++: install a hook that will be called when compilation starts.
// this is used to show a simple animation in the lower right corner.
LPP_COMPILE_START_HOOK(OnCompileStart);


void OnCompileSuccess(void)
{
	SDL_Log("Live++ compilation successful\n");
	g_showCompileAnimation = false;
}

// Live++: install a hook that will be called when compilation succeeds
LPP_COMPILE_SUCCESS_HOOK(OnCompileSuccess);


void OnCompileError(void)
{
	SDL_Log("Live++ compilation failed\n");
	g_showCompileAnimation = false;
}

// Live++: install a hook that will be called when compilation fails
LPP_COMPILE_ERROR_HOOK(OnCompileError);


extern bool MainLoop(unsigned int width, unsigned int height, SDL_Renderer* renderer, SDL_Texture* texture);

// main entry point
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
	)
{
	// console for logging
	::AllocConsole();

	// initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		return 1;
	}

	// Live++.
	// first, load the Live++ DLL and register a process group named "Sample".
	// second, enable Live++ for this executable only (no import DLLs).
	// third, install Live++'s exception handler to recover from errors.
	HMODULE livePP = lpp::lppLoadAndRegister(L"../..", "Sample");
	lpp::lppEnableCallingModuleSync(livePP);
	lpp::lppInstallExceptionHandler(livePP);

	SDL_Window* window = SDL_CreateWindow("Sample - Hit 'C' to clear", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	// initialize image data
	g_colorData = new Vec [PIXEL_COUNT]();
	g_rgbaData = new unsigned char [PIXEL_COUNT * 4u]();
	g_sampleCountPerPixel = new unsigned int [PIXEL_COUNT]();

	// run the main loop
	while (MainLoop(SCREEN_WIDTH, SCREEN_HEIGHT, renderer, texture))
	{
		// Live++: having a sync point ensures that all pixels are traced using the same code, even when code was changed mid-frame.
		// try removing this sync point, restart the application, and then do a few of the changes marked with "Live++".
		lpp::lppSyncPoint(livePP);
	}

	// clean up
	delete[] g_sampleCountPerPixel;
	delete[] g_rgbaData;
	delete[] g_colorData;

	// clean up SDL
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

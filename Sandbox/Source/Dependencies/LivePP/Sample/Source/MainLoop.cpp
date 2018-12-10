
#include <Windows.h>
#include <stdio.h>
#include "SDL.h"
#include "Ray.h"
#include "Update.h"

extern Vec* g_colorData;
extern unsigned char* g_rgbaData;
extern unsigned int* g_sampleCountPerPixel;
extern uint64_t g_raysTraced;
extern bool g_showCompileAnimation;

bool MainLoop(unsigned int width, unsigned int height, SDL_Renderer* renderer, SDL_Texture* texture)
{
	// Live++: try changing camera position and direction
	Ray cameraRay(
		Vec(50, 52, 295.6), 
		Vec(0, -0.042612, -1).normalize()
	);

	// poll SDL events, listen for keyboard event
	SDL_Event event = {};
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			// clear image rendered so far when 'C' is pressed
			case SDL_KEYDOWN:
				// Live++: try listening to a different key
				if (event.key.keysym.sym == SDLK_c)
				{
					extern void ClearImageData(void);
					ClearImageData();
				}
				break;

			case SDL_QUIT:
				return false;
		}
	}

	// Live++: try changing the number of rays traced per iteration
	const unsigned int RAY_COUNT = 4096u;
	pathTrace::Trace(cameraRay, RAY_COUNT, width, height, g_sampleCountPerPixel, g_colorData, g_rgbaData);

	// accumulate average number of samples
	g_raysTraced += RAY_COUNT;

	SDL_Log("average number of samples per pixel: %.3f", static_cast<double>(g_raysTraced) / (width*height));

	// render texture on screen and present
	SDL_UpdateTexture(texture, NULL, g_rgbaData, width * 4);
	SDL_RenderCopy(renderer, texture, NULL, NULL);

	// show compile progress, if any
	if (g_showCompileAnimation)
	{
		const unsigned int ANIMATION_SPEED = 2u;
		const unsigned int FRAME_COUNT = 5u;
		const int SIZE = 8;

		static unsigned int counter = 0u;
		++counter;
		if (counter >= (FRAME_COUNT) * ANIMATION_SPEED)
		{
			counter = 0;
		}

		SDL_Rect rect;
		rect.x = 512 - SIZE*(FRAME_COUNT + 1) + (counter / ANIMATION_SPEED)*SIZE;
		rect.y = 512 - SIZE*2;
		rect.w = SIZE;
		rect.h = SIZE;

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &rect);
	}

	SDL_RenderPresent(renderer);

	return true;
}

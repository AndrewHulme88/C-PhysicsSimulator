#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
	// Initialize SDL video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
		return -1;
	}

	// Create a window
	SDL_Window* window = SDL_CreateWindow("C++ Physics Simulator", 800, 600, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	
	if (!window || !renderer)
	{
		std::cerr << "Window/Renderer creation error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}

	// Main event loop
	bool running = true;
	SDL_Event event;
	
	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT) 
				running = false;
		}
		SDL_Delay(16); // 60 FPS cap

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderPoint(renderer, 320.0f, 240.0f);

		SDL_RenderPresent(renderer);
	}

	// Cleanup
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
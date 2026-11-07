#include <SDL3/SDL.h>
#include <iostream>

int main()
{
	// Initialize SDL video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
		return -1;
	}

	// Create a window
	SDL_Window* window = SDL_CreateWindow("C++ Physics Simulator", 800, 600, SDL_WINDOW_RESIZABLE);
	
	if (!window)
	{
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
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

		//SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		//SDL_RenderClear(renderer);

		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		//SDL_Rect rect = { 200, 150, 400, 300 };
		//SDL_RenderFillRect(renderer, &rect);

	}

	// Cleanup
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
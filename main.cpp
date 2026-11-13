#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include "Material.h"
#include "World.h"

// Grid size lives in World.h so main.cpp and World.cpp always agree
constexpr int GRID_WIDTH = World::GRID_WIDTH;
constexpr int GRID_HEIGHT = World::GRID_HEIGHT;
constexpr int CELL_SIZE = 4;
constexpr int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;
constexpr double FIXED_TIMESTAMP = 1.0 / 60.0;

void renderGrid(SDL_Renderer* renderer, const std::vector<Material>& grid)
{
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			int index = y * GRID_WIDTH + x;

			if (grid[index] == Material::Sand)
			{
				float screenX = static_cast<float>(x * CELL_SIZE);
				float screenY = static_cast<float>(y * CELL_SIZE);

				SDL_FRect rect{
					screenX,
					screenY,
					static_cast<float>(CELL_SIZE),
					static_cast<float>(CELL_SIZE)
				};

				SDL_RenderFillRect(renderer, &rect);
			}
		}
	}
}

// Convert a mouse position (pixels) to a grid cell and place sand there
void placeSand(World& world, float mouseX, float mouseY)
{
	int x = static_cast<int>(mouseX) / CELL_SIZE;
	int y = static_cast<int>(mouseY) / CELL_SIZE;

	world.placeMaterial(x, y, Material::Sand); // World does the bounds check
}

int main(int argc, char *argv[])
{
	World world;

	world.placeMaterial(50, 30, Material::Sand);
	world.placeMaterial(60, 50, Material::Sand);
	world.placeMaterial(60, 55, Material::Sand);

	// Initialize SDL video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
		return -1;
	}

	// Create a window
	SDL_Window* window = SDL_CreateWindow("C++ Physics Simulator", WINDOW_WIDTH, WINDOW_HEIGHT, NULL);

	if (!window)
	{
		std::cerr << "Window creation error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	
	if (!renderer)
	{
		std::cerr << "Renderer creation error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -1;
	}
	
	// Main event loop
	bool isDrawing = false;
	bool running = true;
	SDL_Event event;
	
	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					isDrawing = true;
					placeSand(world, event.button.x, event.button.y);
				}
				break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					isDrawing = false;
				}
				break;

			case SDL_EVENT_MOUSE_MOTION:
				// If the left mouse button is held down, record the path
				if (isDrawing)
				{
					placeSand(world, event.motion.x, event.motion.y);
				}
				break;
			}
		}

		SDL_Delay(static_cast<Uint32>(FIXED_TIMESTAMP * 1000)); // ~60 FPS cap (SDL_Delay takes milliseconds)

		// Update physics
		world.update();

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		renderGrid(renderer, world.getCells());
		SDL_RenderPresent(renderer);
	}

	// Cleanup
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
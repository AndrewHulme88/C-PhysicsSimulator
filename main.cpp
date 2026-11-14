#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Material.h"
#include "World.h"

// Grid size lives in World.h so main.cpp and World.cpp always agree
constexpr int GRID_WIDTH = World::GRID_WIDTH;
constexpr int GRID_HEIGHT = World::GRID_HEIGHT;
constexpr int CELL_SIZE = 4;
constexpr int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;
constexpr double FIXED_TIMESTAMP = 1.0 / 60.0;

void renderGrid(SDL_Renderer* renderer, 
	const std::vector<Material>& grid, 
	const std::vector<float>& waterMass)
{
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			int index = y * GRID_WIDTH + x;

			switch (grid[index])
			{
			case Material::Empty:
				continue;

			case Material::Sand:
				SDL_SetRenderDrawColor(renderer, 230, 190, 80, 255);
				break;

			case Material::Stone:
				SDL_SetRenderDrawColor(renderer, 120, 120, 120, 225);
				break;

			case Material::Water:
				SDL_SetRenderDrawColor(renderer, 50, 130, 230, 255);
				break;
			}

			SDL_FRect rect{
				static_cast<float>(x * CELL_SIZE),
				static_cast<float>(y * CELL_SIZE),
				static_cast<float>(CELL_SIZE),
				static_cast<float>(CELL_SIZE)
			};

			if (grid[index] == Material::Water)
			{
				float fill = std::clamp(waterMass[index], 0.0f, 1.0f);

				rect.h = CELL_SIZE * fill;
				rect.y += CELL_SIZE - rect.h; // Fill upward from the cell bottom
			}

			SDL_RenderFillRect(renderer, &rect);
		}
	}
}

// Convert a mouse position (pixels) to a grid cell and place sand there
void placeSand(World& world, float mouseX, float mouseY)
{
	int x = static_cast<int>(mouseX) / CELL_SIZE;
	int y = static_cast<int>(mouseY) / CELL_SIZE;

	world.placeMaterial(x, y, Material::Water); // World does the bounds check
}

int main(int argc, char *argv[])
{
	World world;

	for (int x = 40; x <= 70; ++x)
	{
		world.placeMaterial(x, 80, Material::Stone);
	}

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
		renderGrid(renderer, world.getCells(), world.getWaterMass());
		SDL_RenderPresent(renderer);
	}

	// Cleanup
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
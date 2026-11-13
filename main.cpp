#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <random>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr int CELL_SIZE = 4;
constexpr int GRID_WIDTH = WINDOW_WIDTH / CELL_SIZE;
constexpr int GRID_HEIGHT = WINDOW_HEIGHT / CELL_SIZE;

enum class Material
{
	Empty,
	Sand
};

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

void placeSand(std::vector<Material>& grid, float mouseX, float mouseY)
{
	int x = static_cast<int>(mouseX) / CELL_SIZE;
	int y = static_cast<int>(mouseY) / CELL_SIZE;

	if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
	{
		return;
	}

	int index = y * GRID_WIDTH + x;
	grid[index] = Material::Sand;
}

void updateSand(std::vector<Material>& grid)
{
	// Static random generator so it isn't re-seaded every frame
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> coinFlip(0, 1);

	// Loop from the second to last row up to the top row
	for (int y = GRID_HEIGHT - 2; y >= 0; --y)
	{
		// Randomize horizontal scan order to prevent sand moving right to be updated twice in one frame
		bool scanLeftToRight = coinFlip(rng) == 0;

		for (int i = 0; i < GRID_WIDTH; ++i)
		{
			int x = scanLeftToRight ? i : (GRID_WIDTH - 1 - i);
			int index = y * GRID_WIDTH + x;

			// Only apply gravity to sand
			if (grid[index] == Material::Sand)
			{
				int below = (y + 1) * GRID_WIDTH + x;

				// Fall straight down if empty
				if (grid[below] == Material::Empty)
				{
					grid[below] = Material::Sand;
					grid[index] = Material::Empty;
				}
				// Otherwise check diagonals in random order 
				else
				{
					bool checkLeftFirst = coinFlip(rng) == 0;

					int firstX = checkLeftFirst ? (x - 1) : (x + 1);
					int secondX = checkLeftFirst ? (x + 1) : (x - 1);

					// Helper check for the first chosen diagonal
					if (firstX >= 0 && firstX < GRID_WIDTH && grid[(y + 1) * GRID_WIDTH + firstX] == Material::Empty)
					{
						grid[(y + 1) * GRID_WIDTH + firstX] = Material::Sand;
						grid[index] = Material::Empty;
					}
					// Fallback check for the opposite diagonal
					else if (secondX >= 0 && secondX < GRID_WIDTH && grid[(y + 1) * GRID_WIDTH + secondX] == Material::Empty)
					{
						grid[(y + 1) * GRID_WIDTH + secondX] = Material::Sand;
						grid[index] = Material::Empty;
					}
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	std::vector<Material> grid(GRID_WIDTH * GRID_HEIGHT, Material::Empty);

	grid[30 * GRID_WIDTH + 50] = Material::Sand;
	grid[50 * GRID_WIDTH + 60] = Material::Sand;
	grid[55 * GRID_WIDTH + 60] = Material::Sand;

	// Initialize SDL video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
		return -1;
	}

	// Create a window
	SDL_Window* window = SDL_CreateWindow("C++ Physics Simulator", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

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
					placeSand(grid, event.button.x, event.button.y);
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
					placeSand(grid, event.motion.x, event.motion.y);
				}
				break;
			}
		}

		SDL_Delay(16); // 60 FPS cap

		// Update physics
		updateSand(grid);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		renderGrid(renderer, grid);
		SDL_RenderPresent(renderer);
	}

	// Cleanup
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
#include "World.h"
#include <random>

void World::update()
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
			if (cells[index] == Material::Sand)
			{
				int below = (y + 1) * GRID_WIDTH + x;

				// Fall straight down if empty
				if (cells[below] == Material::Empty)
				{
					cells[below] = Material::Sand;
					cells[index] = Material::Empty;
				}
				// Otherwise check diagonals in random order 
				else
				{
					bool checkLeftFirst = coinFlip(rng) == 0;

					int firstX = checkLeftFirst ? (x - 1) : (x + 1);
					int secondX = checkLeftFirst ? (x + 1) : (x - 1);

					// Helper check for the first chosen diagonal
					if (firstX >= 0 && firstX < GRID_WIDTH && cells[(y + 1) * GRID_WIDTH + firstX] == Material::Empty)
					{
						cells[(y + 1) * GRID_WIDTH + firstX] = Material::Sand;
						cells[index] = Material::Empty;
					}
					// Fallback check for the opposite diagonal
					else if (secondX >= 0 && secondX < GRID_WIDTH && cells[(y + 1) * GRID_WIDTH + secondX] == Material::Empty)
					{
						cells[(y + 1) * GRID_WIDTH + secondX] = Material::Sand;
						cells[index] = Material::Empty;
					}
				}
			}
		}
	}
}

void World::placeMaterial(int x, int y, Material material)
{
	if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
	{
		return;
	}

	cells[y * GRID_WIDTH + x] = material;
}

const std::vector<Material>& World::getCells() const
{
	return cells;
}

#include "World.h"
#include <random>
#include <algorithm>
#include <cmath>

void World::update()
{
	// Static random generator so it isn't re-seaded every frame
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> coinFlip(0, 1);

	for (int y = GRID_HEIGHT - 1; y >= 0; --y)
	{
		// Randomize horizontal scan order to prevent sand moving right to be updated twice in one frame
		bool scanLeftToRight = coinFlip(rng) == 0;

		for (int i = 0; i < GRID_WIDTH; ++i)
		{
			int x = scanLeftToRight ? i : (GRID_WIDTH - 1 - i);
			int index = y * GRID_WIDTH + x;

			// Sand
			// Only apply gravity to sand, skip the bottom row
			if (cells[index] == Material::Sand && y + 1 < GRID_HEIGHT)
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

	updateWater();
}

void World::updateWater()
{
	constexpr float compression = 0.02f;
	constexpr float minTransfer = 0.00001f;

	const auto& current = waterMass;
	auto next = current;

	auto isOpen = [&](int index)
	{
		return cells[index] == Material::Empty || cells[index] == Material::Water;
	};

	// Given the combined water in two vertically adjacent cells
	// calculate how much should settle in the lower cell
	auto stableBottom = [=](float total) -> float
	{
		if (total <= 1.0f)
		{
			return total;
		}

		if (total < 2.0f + compression)
		{
			return (1.0f + compression * total) / (1.0f + compression);
		}

		return (total + compression) / 2.0f;
	};

	// Positive flow: a -> b. Negative flow: b -> a
	auto exchange = [&](int a, int b, float flow)
	{
		int from = flow > 0.0f ? a : b;
		int to = flow > 0.0f ? b : a;

		// A cell has four neighbours. Limiting each outgoing transfer
		// to 1/4 of its starting amount prevents it sending too much
		float amount = std::min(std::abs(flow), current[from] * 0.25f);

		if (amount < minTransfer)
		{
			return;
		}

		next[from] -= amount;
		next[to] += amount;
	};

	for (int y = 0; y < GRID_HEIGHT; ++y)
	{
		for (int x = 0; x < GRID_WIDTH; ++x)
		{
			int index = y * GRID_WIDTH + x;

			if (!isOpen(index))
			{
				continue;
			}

			// Process each horizontal pair once
			if (x + 1 < GRID_WIDTH)
			{
				int right = index + 1;

				if (isOpen(right))
				{
					float flow = (current[index] - current[right]) * 0.25f;

					exchange(index, right, flow);
				}
			}

			// Process each vertical pair once
			if (y + 1 < GRID_HEIGHT)
			{
				int below = index + GRID_WIDTH;

				if (isOpen(below))
				{
					float total = current[index] + current[below];
					float desiredBelow = stableBottom(total);

					float flow = (desiredBelow - current[below]) * 0.5f;

					exchange(index, below, flow);
				}
			}
		}
	}

	waterMass.swap(next);

	// Keep material labels consistent with the new amounts
	for (int index = 0; index < GRID_WIDTH * GRID_HEIGHT; ++index)
	{
		if (isOpen(index))
		{
			cells[index] = waterMass[index] > 0.0f ? Material::Water : Material::Empty;
		}
	}
}

void World::placeMaterial(int x, int y, Material material)
{
	if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
	{
		return;
	}

	int index = y * GRID_WIDTH + x;

	if (material == Material::Water)
	{
		// Water can't be pointed inside a solid
		// ------------ Will need to be changed if more solid materials are added ------------
		if (cells[index] == Material::Sand || cells[index] == Material::Stone)
		{
			return;
		}

		// Fill the cell without removing existing compressed water
		waterMass[index] = std::max(waterMass[index], 1.0f);
	}
	else
	{
		// Replacing water with another material removes its water
		waterMass[index] = 0.0f;
	}

	cells[index] = material;
}

const std::vector<Material>& World::getCells() const
{
	return cells;
}

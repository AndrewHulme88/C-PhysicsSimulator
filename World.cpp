#include "World.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <utility>

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
				auto tryMoveSand = [&](int targetX) -> bool
				{
					if (targetX < 0 || targetX >= GRID_WIDTH)
					{
						return false;
					}

					int target = (y + 1) * GRID_WIDTH + targetX;

					if (cells[target] != Material::Empty &&
						cells[target] != Material::Water)
					{
						return false;
					}

					// Move sand into the destination and bring whatever was
					// there including its water amount into the old sand cell
					std::swap(cells[index], cells[target]);
					std::swap(waterMass[index], waterMass[target]);

					return true;
				};

				// Prefer falling straight down, including through water
				if (tryMoveSand(x))
				{
					continue;
				}

				// Otherwise try diagonals in random order
				int direction = coinFlip(rng) == 0 ? -1 : 1;

				if (!tryMoveSand(x + direction))
				{
					tryMoveSand(x - direction);
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
	constexpr int LEVELING_PASSES = 12;

	auto next = waterMass;
	auto remaining = waterMass;

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
		float amount = std::min(std::abs(flow), remaining[from]);

		if (amount < minTransfer)
		{
			return;
		}

		remaining[from] -= amount;
		next[from] -= amount;
		next[to] += amount;
	};

	// First: vertical flow, working up from bottom
	// Lower cells make room before above try to fall
	for (int y = GRID_HEIGHT - 2; y >=0; --y)
	{
		for (int x = 0; x < GRID_WIDTH; ++x)
		{
			int above = y * GRID_WIDTH + x;
			int below = above + GRID_WIDTH;

			if (!isOpen(above) || !isOpen(below))
			{
				continue;
			}

			float total = next[above] + next[below];
			float desiredBelow = stableBottom(total);
			float flow = desiredBelow - next[below];

			exchange(above, below, flow);
		}
	}

	// Second: sideways flow, using amounts after vertical flow
	for (int pass = 0; pass < LEVELING_PASSES; ++pass)
	{
		// Each pass works from the previous pass's result
		const auto sideways = next;
		remaining = sideways;

		for (int y = 0; y < GRID_HEIGHT; ++y)
		{
			for (int x = 0; x < GRID_WIDTH - 1; ++x)
			{
				int left = y * GRID_WIDTH + x;
				int right = left + 1;

				if (!isOpen(left) || !isOpen(right))
				{
					continue;
				}

				float flow = (sideways[left] - sideways[right]) * 0.25f;
				int from = flow > 0.0f ? left : right;
				int below = from + GRID_WIDTH;

				// Only supported water spreads sideways
				// This keeps falling drops from dispersing in midair
				bool supported = from / GRID_WIDTH == GRID_HEIGHT - 1 ||
					!isOpen(below) ||
					sideways[below] >= 1.0f - minTransfer;

				if (!supported)
				{
					continue;
				}

				exchange(left, right, flow);
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

#include "World.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <utility>
#include <stdexcept>

World::World(int width, int height, std::uint32_t seed)
	: width(width), height(height), rng(seed)
{
	if (width <= 0 || height <= 0)
	{
		throw std::invalid_argument("World dimensions must be positive");
	}

	const auto count = static_cast<std::size_t>(width) * height;

	cells.resize(count);
}

bool World::inBounds(int x, int y) const
{
	return x >= 0 && x < width && y >= 0 && y < height;
}

std::size_t World::indexOf(int x, int y) const
{
	return static_cast<std::size_t>(y) * width + x;
}

void World::replaceCell(
	std::size_t index,
	Material material,
	float amount
)
{
	if (material == Material::Empty)
	{
		cells[index] = Cell{};
		return;
	}

	const bool isLiquid = definition(material).movement == MovementType::Liquid;

	cells[index] = Cell{ material, isLiquid ? amount : 1.0f };
}

bool World::tryDisplace(std::size_t from, std::size_t to)
{
	if (from == to)
	{
		return false;
	}

	const auto& moving = definition(cells[from].material);
	const auto& target = definition(cells[to].material);

	// Only powders and liquids use this displacement rule
	if (moving.movement != MovementType::Powder &&
		moving.movement != MovementType::Liquid)
	{
		return false;
	}

	const bool targetIsEmpty = cells[to].material == Material::Empty;
	
	const bool targetIsLighterLiquid =
		target.movement == MovementType::Liquid &&
		moving.density > target.density;

	if (!targetIsEmpty && !targetIsLighterLiquid)
	{
		return false;
	}

	std::swap(cells[from], cells[to]);

	return true;
 }

void World::update()
{
	updatePowders();
	updateLiquid(Material::Water);
	updateLiquid(Material::Oil);
}

void World::updatePowders()
{
	// Random generator so it isn't re-seaded every frame
	std::uniform_int_distribution<int> coinFlip(0, 1);

	for (int y = height - 1; y >= 0; --y)
	{
		// Randomize horizontal scan order to prevent sand moving right to be updated twice in one frame
		bool scanLeftToRight = coinFlip(rng) == 0;

		for (int i = 0; i < width; ++i)
		{
			int x = scanLeftToRight ? i : (width - 1 - i);
			int index = y * width + x;

			// Powder type material physics
			if (definition(cells[index].material).movement == MovementType::Powder && y + 1 < height)
			{
				auto tryMoveSand = [&](int targetX) -> bool
				{
						if (!inBounds(targetX, y + 1))
						{
							return false;
						}

						return tryDisplace(index, indexOf(targetX, y + 1));
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
}

void World::updateLiquid(Material material)
{
	constexpr float compression = 0.02f;
	constexpr float minTransfer = 0.00001f;
	constexpr int LEVELING_PASSES = 12;

	std::vector<float> next(cells.size(), 0.0f);

	for (std::size_t index = 0; index < cells.size(); ++index)
	{
		if (cells[index].material == material)
		{
			next[index] = cells[index].amount;
		}
	}

	auto remaining = next;

	auto isOpen = [&](int index)
	{
		return cells[index].material == Material::Empty || cells[index].material == material;
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
	for (int y = height - 2; y >=0; --y)
	{
		for (int x = 0; x < width; ++x)
		{
			int above = y * width + x;
			int below = above + width;

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

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width - 1; ++x)
			{
				int left = y * width + x;
				int right = left + 1;

				if (!isOpen(left) || !isOpen(right))
				{
					continue;
				}

				float flow = (sideways[left] - sideways[right]) * definition(material).sidewaysFlow;
				int from = flow > 0.0f ? left : right;
				int below = from + width;

				// Only supported water spreads sideways
				// This keeps falling drops from dispersing in midair
				bool supported = from / width == height - 1 ||
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

	// Store the calculated water amounts back in the cells
	for (int index = 0; index < width * height; ++index)
	{
		if (!isOpen(index))
		{
			continue;
		}

		if (next[index] > 0.0f)
		{
			replaceCell(index, material, next[index]);
		}
		else
		{
			replaceCell(index, Material::Empty);
		}
	}
}

void World::placeMaterial(int x, int y, Material material)
{
	if (!inBounds(x, y))
	{
		return;
	}

	const auto index = indexOf(x, y);

	if (definition(material).movement == MovementType::Liquid)
	{
		const auto existing = cells[index].material;

		// Liquid placement fills empty cells or the same liquid
		if (existing != Material::Empty && existing != material)
		{
			return;
		}

		replaceCell(
			index,
			material,
			std::max(cells[index].amount, 1.0f)
		);
	}
	else
	{
		replaceCell(index, material);
	}
}

const std::vector<Cell>& World::getCells() const
{
	return cells;
}

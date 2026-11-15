#pragma once

#include <vector>
#include <random>
#include <cstdint>
#include <cstddef>

#include "Material.h"

class World
{
public:
	explicit World(
		int width = 200,
		int height = 150,
		std::uint32_t seed = 12345u
	);

	int getWidth() const { return width; }
	int getHeight() const { return height; }

	// Advance the simulation by one step
	void update();

	// Set the cell at grid position (x, y). Out-of-bounds positions are ignored.
	void placeMaterial(int x, int y, Material material);

	const std::vector<Material>& getCells() const;

	const std::vector<float>& getWaterMass() const
	{
		return waterMass;
	}

private:
	int width;
	int height;

	std::mt19937 rng;

	std::vector<Material> cells;
	std::vector<float> waterMass;

	bool inBounds(int x, int y) const;
	std::size_t indexOf(int x, int y) const;

	void updateWater();
};
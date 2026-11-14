#pragma once

#include <vector>
#include "Material.h"

class World
{
public:
	static constexpr int GRID_WIDTH = 200;
	static constexpr int GRID_HEIGHT = 150;

	// Advance the simulation by one step
	void update();

	// Set the cell at grid position (x, y). Out-of-bounds positions are ignored.
	void placeMaterial(int x, int y, Material material);

	// Read-only access to the grid (e.g. for rendering)
	const std::vector<Material>& getCells() const;

	const std::vector<float>& getWaterMass() const
	{
		return waterMass;
	}

private:
	std::vector<Material> cells = std::vector<Material>(GRID_WIDTH * GRID_HEIGHT, Material::Empty);

	std::vector<float> waterMass = std::vector<float>(GRID_WIDTH * GRID_HEIGHT, 0.0f);

	void updateWater();
};
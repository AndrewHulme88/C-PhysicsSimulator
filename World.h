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

private:
	std::vector<Material> cells = std::vector<Material>(GRID_WIDTH * GRID_HEIGHT, Material::Empty);
};
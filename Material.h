#pragma once

#include <array>
#include <cstddef>

enum class Material
{
	Empty,
	Sand,
	Stone,
	Water,
	Oil
};

enum class MovementType
{
	None,
	Solid,
	Powder,
	Liquid
};

struct MaterialDefinition
{
	MovementType movement;
	float density;
	float sidewaysFlow;
};

inline constexpr std::array<MaterialDefinition, 5> materialDefinition{ {
		// Movement			   Density  Sideways flow
		{ MovementType::None,	0.0f,	0.0f }, // Empty
		{ MovementType::Powder,	2.0f,	0.0f }, // Sand
		{ MovementType::Solid,	3.0f,	0.0f }, // Stone
		{ MovementType::Liquid, 1.0f,	0.25f }, // Water
		{ MovementType::Liquid,	0.8f,	0.10f }, // Oil
	} };

inline constexpr const MaterialDefinition& definition(Material material)
{
	return materialDefinition[static_cast<std::size_t>(material)];
}

struct Cell
{
	Material material = Material::Empty;

	// Liquids use fractional amounts. Other occupied cell use 1
	float amount = 0.0f;
};
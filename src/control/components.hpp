#pragma once

#include "../simulation/pointbin.hpp"
#include <engine/game/core/entity.hpp>
#include <glm/glm.hpp>
#include <array>

namespace game {

	using NeighbourStructure = sim::PointBin<game::Entity, 2, float>;

namespace components{

	struct Growth
	{
		static constexpr float AGE_LEN_SCALE = 10.f;

		Growth(const glm::vec2& _dir, float _age = 0.f) : direction(_dir), age(_age), grad{} {}

		glm::vec2 direction;
		glm::vec2 grad;
		float age;
	};

	struct BaseColor
	{
		BaseColor(const glm::vec4 _color) : value(_color) {}
		glm::vec4 value;
	};

	struct Branch
	{
		std::array<Entity, 2> childs;
	};

	// indicates an active (still growing) tip
	struct Hyphal {};

}}
#pragma once

#include "components.hpp"
#include "simulation/pointbin.hpp"
#include <engine/game/components/simpleComponents.hpp>
#include <engine/game/components/components2D.hpp>
#include <engine/game/core/componentaccess.hpp>
#include <engine/game/core/lifetimeManager2.hpp>

namespace game {
namespace systems {

	class SimSystem 
	{
	public:
		SimSystem();

		using Components = ComponentTuple<
			WriteAccess<components::Position2D>
			, WriteAccess<components::Growth>
			, WriteAccess<components::Hyphal>
			, WriteAccess<components::BaseColor>>;
		void update(Components _comps, float _dt, EntityCreator& _creator, game::NeighbourStructure& _neighbourStructure);
	private:
		math::random::xoshiro128ss m_rng;
	};
}}
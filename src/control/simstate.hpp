#pragma once

#include <engine/game/core/gamestate.hpp>
#include <engine/game/core/world.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/graphics/camera.hpp>

namespace control {

	class SimState : public game::GameState
	{
	public:
		SimState();

		void process(float _deltaTime) override;
		void draw(float _deltaTime) override;

	private:
		game::World<graphics::Camera> m_world;
	};
}
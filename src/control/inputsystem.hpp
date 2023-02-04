#pragma once

#include "rendersystem.hpp"
#include "simsystem.hpp"
#include <engine/game/core/componentaccess.hpp>
#include <engine/game/core/lifetimeManager2.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/graphics/camera.hpp>
#include <engine/graphics/renderer/mesh.hpp>

namespace game{
namespace systems {

	class InputSystem
	{
	public:
		InputSystem();
		using Components = ComponentTuple<
			WriteAccess<components::Position2D>
		, WriteAccess<components::Growth>
		, WriteAccess<components::Hyphal>>;
		void update(Components _comps, EntityCreator& _creator
			, graphics::Camera& _camera);
	private:
		std::unique_ptr<input::InputInterface> m_inputs;
	};
}}
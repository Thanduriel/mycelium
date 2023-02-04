#include "simstate.hpp"
#include "simsystem.hpp"
#include "inputsystem.hpp"
#include "rendersystem.hpp"
#include <engine/game/systems/transforms.hpp>
#include <engine/game/systems/meshdrawnig.hpp>
#include <engine/graphics/core/device.hpp>

namespace control {

	constexpr glm::vec2 MAP_SIZE = glm::vec2(1920.f/2.f, 1080.f/2.f);
	constexpr float PARTITION_SIZE = 24.f;

	SimState::SimState()
		: m_world(graphics::Camera(MAP_SIZE, glm::vec2(0.5f), -1.f, 1.f))
	{
		using FloatT = double;

		graphics::Device::setClearColor(glm::vec4(0.2f, 0.1f, 0.2f, 1.f));

		m_world.addResource<game::NeighbourStructure>(MAP_SIZE, MAP_SIZE / PARTITION_SIZE);
		m_world.addSystem(std::make_unique<game::systems::SimSystem>(), game::SystemGroup::Process);
		
		m_world.addSystem(std::make_unique<game::systems::InputSystem>(), game::SystemGroup::Process);
	//	m_world.addSystem(std::make_unique<game::systems::Transforms>(), game::SystemGroup::Process);

		auto renderSystem = m_world.addSystem(std::make_unique<game::systems::RenderSystem>(), game::SystemGroup::Draw);
	}

	void SimState::process(float _deltaTime)
	{
		m_world.process(game::SystemGroup::Process, _deltaTime);

		if (input::InputManager::isKeyPressed(input::Key::ESCAPE))
			finish();
	}

	void SimState::draw(float _deltaTime)
	{
		m_world.process(game::SystemGroup::Draw, _deltaTime);
	}
}
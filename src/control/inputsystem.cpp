#include "inputsystem.hpp"
#include <engine/input/keyboardInterface.hpp>
#include <engine/utils/config.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/utils/meshLoader.hpp>
#include <engine/graphics/resources.hpp>
#include <engine/math/random.hpp>
#include <GLFW/glfw3.h>
#include <numbers>


namespace game {
namespace systems {

	using namespace input;

	enum struct Actions {
		SHOW_E,
		SHOW_B,
		LAYER_UP,
		LAYER_DOWN,
		CAMERA_FORWARD,
		CAMERA_BACKWARD,
		CAMERA_ROTATE,
		SPAWN_PARTICLE,
		SPAWN_MOD
	};

	InputSystem::InputSystem()
		: m_inputs(new KeyboardInterface(utils::Config::get()["inputs"]["keyboard"],
			{ {"showE", Key::E},
			{"showB", Key::B},
			{"layerUp", Key::UP},
			{"layerDown", Key::DOWN},
			{"cameraForward", MouseButton::LEFT}, // not in use
			{"cameraBackward", Key::DOWN}, // not in use
			{"rotateCamera", MouseButton::LEFT},
			{"spawnParticle", MouseButton::RIGHT},
			{"spawnMod", Key::LEFT_SHIFT} },
			{}))
	{
	}


	void InputSystem::update(Components _comps, game::EntityCreator& _creator
		, graphics::Camera& _camera
		, game::NeighbourStructure& _neighbourStructure)
	{
		constexpr std::array<Color, 3> COLORS = { {
			{0.f, 1.f, 0.f, 0.5f},
			{1.f, 0.f, 0.f, 0.5f},
			{0.f, 0.f, 1.f, 0.5f},
		} };

		if (m_inputs->getKeyState(Actions::SPAWN_PARTICLE) == ActionState::PRESSED)
		{
			const glm::vec3 pos = _camera.toWorldSpace(m_inputs->getCursorPos());
			const Entity ent = _creator.create();
			_neighbourStructure.add(pos, ent);
			CreateComponents(_comps, ent)
				.add<components::Position2D>(pos)
				.add<components::Growth>(math::random::direction2D())
				.add<components::Hyphal>()
				.add<components::BaseColor>(COLORS[m_currentColor++]);
		}

		if (m_inputs->getKeyState(Actions::CAMERA_ROTATE) == ActionState::PRESSED)
		{
			const glm::vec3 pos = _camera.toWorldSpace(m_inputs->getCursorPos());
			const Entity ent = _creator.create();

			CreateComponents(_comps, ent)
				.add<components::Position2D>(pos)
				.add<components::Resource>();
		}
	}
}}
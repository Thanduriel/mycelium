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
		, graphics::Camera& _camera)
	{
		if (m_inputs->getKeyState(Actions::SPAWN_PARTICLE) == ActionState::PRESSED)
		{
			CreateComponents(_comps, _creator.create())
				.add<components::Position2D>(glm::vec2(0.f))
				.add<components::Growth>(glm::vec2(1.0f, 0.f))
				.add<components::Hyphal>();
		}
	}
}}
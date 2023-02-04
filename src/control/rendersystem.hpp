#pragma once

#include "components.hpp"
#include <engine/game/components/components2D.hpp>
#include <engine/game/core/componentaccess.hpp>
#include <engine/game/components/simpleComponents.hpp>
#include <engine/graphics/renderer/linerenderer.hpp>

namespace game {
namespace systems {

	class RenderSystem
	{
	public:
		RenderSystem();

		using Components = ComponentTuple<
			ReadAccess<components::Position2D>
			, ReadAccess<components::Growth>
			, ReadAccess<components::BaseColor>>;
		void update(Components _components, const graphics::Camera& _camera);

	private:
		graphics::LineRenderer m_lineRenderer;
		
	};
}}
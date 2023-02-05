#include "rendersystem.hpp"
#include <engine/graphics/core/texture.hpp>

namespace game{
namespace systems{

	RenderSystem::RenderSystem()
	{
	}

	void RenderSystem::update(Components _comps, const graphics::Camera& _camera)
	{
		using namespace components;

		m_lineRenderer.clear();
		_comps.execute([this](const Growth& growth, Position2D& pos, const BaseColor& _color)
			{
				constexpr glm::vec4 color(0.f, 1.f, 0.f, 0.5f);
				constexpr float thickness = 0.005f;

				const glm::vec2 end = pos.value + growth.direction * growth.length;
				m_lineRenderer.draw(glm::vec3(pos.value.x, pos.value.y, 0.f),
					glm::vec3(end.x, end.y, 0.f), _color.value, thickness * std::sqrt(std::log(1.f + growth.age)));

			//	const glm::vec2 endG = end + growth.grad * 50.f;
			//	constexpr glm::vec4 color2(1.f, 1.f, 1.f, 0.25f);
			//	m_lineRenderer.draw(glm::vec3(end.x, end.y, 0.f),
			//		glm::vec3(endG.x, endG.y, 0.f), color2, 0.005f);
			});
		m_lineRenderer.present(_camera);
	}
}}
#include "mycelium.hpp"

namespace sim {

	using namespace glm;

	Mycelium::Cell& Mycelium::getCell(const glm::vec2& _position)
	{
		const glm::vec2 relPos = clamp(_position / m_size, vec2(0.f), vec2(0.999f));
		const glm::ivec2 idx = relPos * vec2(m_partition);
		return m_cells[idx.x + idx.y * m_partition.x];
	}

	void Mycelium::update(float _dt) 
	{
		static float val = 0.f;
		val += _dt;
		if (val >= 1.f) {
			val = 0.f;
			for (Node* node : m_hyphals)
			{
				const glm::vec2 dir = math::random::direction2d(m_rng);
				m_alloc.create(node->position + dir);

			}
		}
	}
}
#include "simsystem.hpp"
#include<engine/math/geometrictypes.hpp>
#include<spdlog/spdlog.h>
#include <chrono>

namespace game{
namespace systems {
	SimSystem::SimSystem()
	{}

	template<int Exp>
	glm::vec2 normGrad(const glm::vec2& v, const glm::vec2& p) {
		const glm::vec2 dif = v - p;
		const float lSqr = glm::dot(dif,dif);
		if (lSqr < 1e-8)
			return glm::vec2{};
		const float s = Exp * std::powf(lSqr, Exp-1);
		return 2.f * s * glm::vec2(p[0] - v[0], p[1] - v[1]);
	}

	using namespace components;
	void SimSystem::update(Components _comps, float _dt, EntityCreator& _creator, game::NeighbourStructure& _neighbourStructure)
	{
		struct Spawn
		{
			glm::vec2 position;
			glm::vec2 direction;
			float age;
			Color color;
		};

		std::vector<Spawn> spawns;
		std::vector<Entity> deadEnds;

		_comps.execute([&, _dt](Entity ent
			, const Hyphal& hyphal
			, const Position2D& position
			, const BaseColor& color
			, Growth& growth)
			{
				growth.age += _dt;
				const glm::vec2 pos = position.value + growth.direction * growth.age * Growth::AGE_LEN_SCALE;

				glm::vec2 grad{};
				// gradient of density field
				_neighbourStructure.iterateNeighbours(pos, 100.f, [ent, pos, &grad](const glm::vec2& v, Entity oth) {
					if (ent != oth) {
						grad += normGrad<-1>(v, pos);
					}
					});
				const float lenGrad = glm::length(grad);
				float s = 1.f;
				if (lenGrad > 1e-8)
				{
					grad = -glm::normalize(grad);
					s = glm::dot(grad, growth.direction);
				}
				else
				{
					grad = math::random::direction2d(m_rng);
				}
				growth.grad = grad;

				constexpr float branchThreshold = 40.f;
				std::uniform_real_distribution<float> branchDist(1.f, branchThreshold);
				if (s < 0.0)
				{
					deadEnds.push_back(ent);
				}
				else if (branchDist(m_rng) < growth.age)
				{
					spawns.push_back({ pos, growth.direction, 0.f, color.value });
					spawns.push_back({ pos, glm::normalize(2.f * grad + growth.direction), 0.f, color.value });
					deadEnds.push_back(ent);
				}
				else if(s < 0.8f && growth.age > 0.5f)
				{
					// change of direction
					spawns.push_back({ pos, grad, 0.f, color.value });
					deadEnds.push_back(ent);
				}
			});

		for (const Spawn& spawn : spawns)
		{
			const Entity ent = _creator.create();
			_neighbourStructure.add(spawn.position, _creator.create());
			CreateComponents(_comps, ent)
				.add<components::Position2D>(spawn.position)
				.add<components::Growth>(spawn.direction, spawn.age)
				.add<components::Hyphal>()
				.add<components::BaseColor>(spawn.color);
		}

		for (Entity ent : deadEnds)
		{
			getComp<components::Hyphal>(_comps).erase(ent);
		}


	}
}}
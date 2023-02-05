#include "simsystem.hpp"
#include<engine/math/geometrictypes.hpp>
#include<spdlog/spdlog.h>
#include <chrono>

namespace game{
namespace systems {
	SimSystem::SimSystem()
		: m_rng(math::random::g_random())
	{}

	constexpr float growthMomentum = 0.6f;
	constexpr float minAge = 0.5f;
	constexpr float branchThreshold = 2.5f;
	constexpr float branchProbability = 0.75f;
	constexpr float randomGrowth = 0.1f;

	constexpr float fieldScale1 = 1.f;
	constexpr float fieldScale2 = 4.f;
	constexpr float resourcePullScale = 1024.f;

	constexpr float boundarySize = 16.f;

	float sqrDist(const glm::vec2& v, const glm::vec2& p)
	{
		const glm::vec2 dif = v - p;
		return glm::dot(dif, dif);
	}

	template<float Exp>
	glm::vec2 normGrad(const glm::vec2& v, const glm::vec2& p, float dSqr)
	{
		const float s = Exp * std::powf(dSqr, Exp-1);
		return 2.f * s * glm::vec2(p[0] - v[0], p[1] - v[1]);
	}

	std::pair<glm::vec2, float> evalFields(const glm::vec2& v, const glm::vec2& p) 
	{
		glm::vec2 grad{};
		float density = 0.f;
		const float dSqr = sqrDist(v, p);
		if (dSqr < 1e-10)
			return { grad, density };

		grad += fieldScale1 * normGrad<-1.f>(v, p, dSqr);
		grad += fieldScale2 * normGrad<-2.f>(v, p, dSqr);
		density += 1.f / std::sqrt(dSqr);

		return { grad, density };
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

		_dt *= 3.f;

		_comps.execute([&, _dt](Growth& growth)
			{
				growth.age += _dt;
			});

		std::vector<glm::vec2> resources;

		_comps.execute([&resources](const Resource&, const Position2D& position)
			{
				resources.push_back(position.value);
			});

		_comps.execute([&, _dt](Entity ent
			, const Hyphal& hyphal
			, const Position2D& position
			, const BaseColor& color
			, Growth& growth)
			{
				growth.length += _dt * Growth::AGE_LEN_SCALE;
				growth.branchTime += _dt;

				std::uniform_real_distribution<float> branchDist;
				if (growth.branchTime > Growth::REROLL_BRANCH_TIME)
				{
					if (branchDist(m_rng) < branchProbability)
						growth.branchTime = -branchDist(m_rng);
					else
						growth.branchTime = 1.f;
				}

				const glm::vec2 pos = position.value + growth.direction * growth.length;

				glm::vec2 grad{};
				float density = 0.f;
				// gradient of density field
				_neighbourStructure.iterateNeighbours(pos, 100.f, [&,ent, pos](const glm::vec2& v, Entity oth) {
					if (ent != oth) {
						auto [g,d] = evalFields(v, pos);
						grad += g;
						density += d;
					}
					});

				for (const glm::vec2& r : resources)
				{
					const float dSqr = sqrDist(r, pos);
					const auto te = resourcePullScale * normGrad<-2.f>(r, pos, dSqr);
					grad -= resourcePullScale * normGrad<-1.f>(r, pos, dSqr);
				}

				// boundary condition
				if (pos.x > MAP_SIZE.x - boundarySize)
				{
					deadEnds.push_back(ent);
					return;
			//		grad.x += -1.f;
			//		density = std::numeric_limits<float>::max();
				}
				else if (pos.x < boundarySize)
				{
					deadEnds.push_back(ent);
					return;
			//		grad.x += 1.f;
			//		density = std::numeric_limits<float>::max();
				}
				else if (pos.y > MAP_SIZE.y - boundarySize)
				{
					deadEnds.push_back(ent);
					return;
			//		grad.x += -1.f;
			//		density = std::numeric_limits<float>::max();
				}
				else if (pos.y < boundarySize)
				{
					deadEnds.push_back(ent);
					return;
			//		grad.y += 1.f;
			//		density = std::numeric_limits<float>::max();
				}

				const float lenGrad = glm::length(grad);
				float s = 1.f;
				if (lenGrad > 1e-8)
				{
					grad = -glm::normalize(grad);
					s = glm::dot(grad, growth.direction);
				}
				else
				{
					grad = math::random::direction2D(m_rng);
				}
				growth.grad = grad;

				// sharp turns indicate that the way is blocked
				if (s < 0.0)
				{
					deadEnds.push_back(ent);
				}
				else if (growth.branchTime > 0.f && growth.branchTime < 1.f && density < branchThreshold)
				{
					spawns.push_back({ pos, growth.direction, 0.f, color.value });
					const glm::vec2 orthDir = branchDist(m_rng) > 0.5f ? glm::vec2(-growth.direction.y, growth.direction.x)
						: glm::vec2(growth.direction.y, -growth.direction.x);
					const float dirMomentum = branchDist(m_rng) * 0.5f;
					spawns.push_back({ pos, dirMomentum * growth.direction + (1.f-dirMomentum) * orthDir, 0.f, color.value });
					deadEnds.push_back(ent);
				}
				else if(growth.age > minAge)
				{
					// change of direction
					glm::vec2 newDir = growthMomentum * growth.direction + (1.f - growthMomentum) * grad;
					// random contribution
					newDir = randomGrowth * math::random::direction2D(m_rng) + (1.f - randomGrowth) * newDir;
					spawns.push_back({ pos, newDir, 0.f, color.value });
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
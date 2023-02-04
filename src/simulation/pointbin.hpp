#pragma once

#include <engine/utils/blockalloc.hpp>
#include <engine/math/random.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace sim {

	template<typename T, int Dim, typename FloatT = float>
	class PointBin
	{
	public:
		using VecT = glm::vec<Dim, FloatT, glm::defaultp>;
		using IVecT = glm::vec<Dim, int32_t, glm::defaultp>;
		using Bin = std::vector<std::pair<VecT, T>>;

		PointBin(const VecT& _size, const IVecT& _partition)
			: m_size(_size), m_partition(_partition), m_bins(_partition.x* _partition.y)
		{}

		void add(const VecT& _position, const T& _el)
		{
			Bin& bin = (*this)[getBinIdx(_position)];
			bin.emplace_back(_position, _el);
		}

		template<typename Pred>
		void iterateNeighbours(const VecT& _position, FloatT r, Pred _pred)
		{
			const IVecT start = getBinIdx(_position - r);
			const IVecT end = getBinIdx(_position + r);

			IVecT cur{};
			iterateNeighboursImpl<Dim-1>(start, end, cur, _pred);
		}

		const Bin& operator[](IVecT idx) const
		{
			return m_bins[idx.x + idx.y * m_partition.x];
		}

		Bin& operator[](IVecT idx)
		{
			return m_bins[idx.x + idx.y * m_partition.x];
		}
	private:
		template<int D, typename Pred>
		void iterateNeighboursImpl(const IVecT& _start, const IVecT& _end, IVecT& cur, Pred _pred)
		{
			for (int i = _start[D]; i < _end[D]; ++i)
			{
				cur[D] = i;
				if constexpr (D == 0)
				{
					for (auto& [pos, el] : (*this)[cur])
					{
						_pred(pos, el);
					}
				}
				else
					iterateNeighboursImpl<D - 1>(_start, _end, cur, _pred);
			}
		}

		IVecT getBinIdx(const VecT& _position)
		{
			const VecT relPos = clamp(_position / m_size, VecT(0.f), VecT(0.999f));
			return relPos * VecT(m_partition);
		}

		VecT m_size;
		IVecT m_partition;
		std::vector<Bin> m_bins;
	};

}
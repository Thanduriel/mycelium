#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <random>
#include <cassert>
#include <iostream>
#include <execution>
#include <ranges>
#include <algorithm>

namespace sim {

	template<typename It, typename Fn>
	void runMultiThreaded(It _begin, It _end, Fn _fn, int _numThreads = 1)
	{
		if (_numThreads == 1)
			_fn(_begin, _end);
		else
		{
			std::vector<std::thread> threads;
			threads.reserve(_numThreads - 1);

			using DistanceType = decltype(_end - _begin);
			const DistanceType n = static_cast<DistanceType>(_numThreads);
			const DistanceType rows = (_end - _begin) / n;
			for (DistanceType i = 0; i < n - 1; ++i)
				threads.emplace_back(_fn, i * rows, (i + 1) * rows);
			_fn((n - 1) * rows, _end);

			for (auto& thread : threads)
				thread.join();
		}
	}


	// Cubic mesh with periodic boundary
	template<typename T, bool WithTimeInterpolation = false>
	class CubicMesh
	{
	public:
		using FloatT = T;
		using Vec = glm::vec<3,T>;
		using Index = int64_t;
		using FlatIndex = std::size_t;
		using SizeVec = glm::vec<3, Index>;
		using IndVec = glm::vec<3, Index>;

		CubicMesh(const SizeVec& _gridSize, const Vec& _cellSize)
			: m_size{ _gridSize }
			, m_cellSize{_cellSize}
			, m_numElem(static_cast<FlatIndex>(_gridSize.x* _gridSize.y* _gridSize.z))
			, m_E(new Vec[m_numElem]{})
			, m_B(new Vec[m_numElem]{})
			, m_J(new Vec[m_numElem]{})
			, m_prevE(WithTimeInterpolation ? new Vec[m_numElem]{} : nullptr)
		{
			std::default_random_engine rng(0xA4FE1099);
			std::uniform_real_distribution<T> dist(0.0, 1.0);
			auto gen = [&]() { return Vec(dist(rng), dist(rng), dist(rng)); };

		//	std::generate_n(m_E.get(), m_numElem, gen);
		//	std::generate_n(m_B.get(), m_numElem, gen);
		//	std::generate_n(m_J.get(), m_numElem, gen);
		//	J(_gridSize.x / 2, _gridSize.y / 2, _gridSize.z / 2) = Vec{0.0,0.0,-10.0};
			E(_gridSize.x / 2, _gridSize.y / 2, _gridSize.z / 2) = Vec{ 8.0,0.0,0.0 };
			E(_gridSize.x / 4, _gridSize.y / 4, _gridSize.z / 2) = Vec{ -8.0,0.0,0.0 };
		//	E(_gridSize.x / 2+1, _gridSize.y / 2, _gridSize.z / 2) = Vec{ 0.0,2.0,1.0 };
		}

		const Vec& E(Index _x, Index _y, Index _z) const { return m_E[validFlatIndex({ _x,_y,_z })]; }
		Vec& E(Index _x, Index _y, Index _z) { return m_E[validFlatIndex({ _x,_y,_z })]; }
		const Vec& B(Index _x, Index _y, Index _z) const { return m_B[validFlatIndex({ _x,_y,_z })]; }
		Vec& B(Index _x, Index _y, Index _z) { return m_B[validFlatIndex({ _x,_y,_z })]; }
		// cells and dual cells have the same size and all edges are spacelike
		// *B == H
		const Vec& H(Index _x, Index _y, Index _z) { return m_B[validFlatIndex({ _x,_y,_z })]; }
		const Vec& J(Index _x, Index _y, Index _z) const { return m_J[validFlatIndex({ _x,_y,_z })]; }
		Vec& J(Index _x, Index _y, Index _z) { return m_J[validFlatIndex({ _x,_y,_z })]; }
		
		// Interpolated field values.
		Vec smoothE(T _x, T _y, T _z) const 
		{
			IndVec bot = validIndex(IndVec(static_cast<Index>(_x)
				, static_cast<Index>(_y)
				, static_cast<Index>(_z)));
			const IndVec top = validIndex(IndVec(static_cast<Index>(std::ceil(_x))
				, static_cast<Index>(std::ceil(_y))
				, static_cast<Index>(std::ceil(_z))));
			const Vec l1(_x - bot.x, _y - bot.y, _z - bot.z);

			if constexpr (WithTimeInterpolation)
			{
				// E lives at half time-steps
				return T(0.5) * triLerp(bot, top, l1, m_E.get())
					+ T(0.5) * triLerp(bot, top, l1, m_prevE.get());
			}
			return triLerp(bot, top, l1, m_E.get());
		}
		Vec smoothB(T _x, T _y, T _z) const 
		{
			IndVec bot = validIndex(IndVec(static_cast<Index>(_x)
				, static_cast<Index>(_y)
				, static_cast<Index>(_z)));
			const IndVec top = validIndex(IndVec(static_cast<Index>(std::ceil(_x))
				, static_cast<Index>(std::ceil(_y))
				, static_cast<Index>(std::ceil(_z))));
			const Vec l1(_x - bot.x, _y - bot.y, _z - bot.z);

			return triLerp(bot, top, l1, m_B.get());
		}

		// raw access to the fields
		const Vec* E() const { return m_E.get(); }
		Vec* E() { return m_E.get(); }
		const Vec* B() const { return m_B.get(); }
		const Vec* J() const { return m_J.get(); }
		const Vec* prevE() const 
		{
			if constexpr (WithTimeInterpolation)
				return m_prevE.get();
			return m_E.get();
		}

		// Evaluate the charge density on the grid cell.
		// This quantity is conserved.
		T p(Index _x, Index _y, Index _z) const 
		{ 
			const Vec& E_ = E(_x, _y, _z);
			return (E_.x - E(_x - 1, _y, _z).x) / m_cellSize.x
				+ (E_.y - E(_x, _y - 1, _z).y) / m_cellSize.y
				+ (E_.z - E(_x, _y, _z - 1).z) / m_cellSize.z;
		}

		// Evaluate the divergence of B on a grid cell.
		// B should be divergence free.
		T divB(Index _x, Index _y, Index _z) const
		{
			const Vec& B_ = B(_x, _y, _z);
			return (B(_x + 1, _y, _z).x - B_.x) / m_cellSize.x
				+ (B(_x, _y+1, _z).y - B_.y) / m_cellSize.y
				+ (B(_x, _y, _z+1).z - B_.z) / m_cellSize.z;
		}

		T energy() const 
		{
			T e = 0;
			for (FlatIndex i = 0; i < m_numElem; ++i) 
			{
				const Vec& E = m_E[i];
				const Vec& B = m_B[i];
				e += E.x * E.x + E.y * E.y + E.z * E.z;
				e += B.x * B.x + B.y * B.y + B.z * B.z;
			}
			return e;
		}

		// Returns a valid index by handling periodic boundary conditions.
		IndVec validIndex(const IndVec& _ind) const
		{
			IndVec v{ _ind.x % m_size.x, _ind.y % m_size.y, _ind.z % m_size.z };
			if (v.x < 0) v.x += m_size.x;
			if (v.y < 0) v.y += m_size.y;
			if (v.z < 0) v.z += m_size.z;
			return v;
		}

		template<std::floating_point T>
		Vec validPosition(T _x, T _y, T _z) const
		{
			Vec v{ std::fmod(_x, static_cast<T>(m_size.x))
				, std::fmod(_y, static_cast<T>(m_size.y))
				, std::fmod(_z, static_cast<T>(m_size.z)) };
			if (v.x < 0.f) v.x += m_size.x;
			if (v.y < 0.f) v.y += m_size.y;
			if (v.z < 0.f) v.z += m_size.z;
			return v;
		}

		FlatIndex flatIndex(const IndVec& _ind) const
		{
			return static_cast<FlatIndex>(_ind.x + _ind.y * m_size.x + _ind.z * m_size.x * m_size.y);
		}

		FlatIndex validFlatIndex(const IndVec& _ind) const
		{
			const FlatIndex flatIdx = flatIndex(validIndex(_ind));
			assert(flatIdx < m_numElem);
			return flatIdx;
		}

		const SizeVec size() const { return m_size; }
		const Vec& cellSize() const { return m_cellSize; }
		FlatIndex numElem() const { return m_numElem; }
		void swapE() 
		{
			if constexpr(WithTimeInterpolation)
				std::swap(m_E, m_prevE); 
		}
	private:
		Vec triLerp(const IndVec bot, const IndVec top, Vec l1, const Vec* data) const
		{
			const Vec l0 = Vec(1.0) - l1;

			// no bound checks necessary if we assume that bot and top are valid
			auto v = [this,data](Index x, Index y, Index z) 
				{ return data[flatIndex(IndVec(x, y, z))]; };

			const Vec c00 = l0.x * v(bot.x, bot.y, bot.z) + l1.x * v(top.x, bot.y, bot.z);
			const Vec c01 = l0.x * v(bot.x, bot.y, top.z) + l1.x * v(top.x, bot.y, top.z);
			const Vec c10 = l0.x * v(bot.x, top.y, bot.z) + l1.x * v(top.x, top.y, bot.z);
			const Vec c11 = l0.x * v(bot.x, top.y, top.z) + l1.x * v(top.x, top.y, top.z);

			const Vec c0 = l0.y * c00 + l1.y * c10;
			const Vec c1 = l0.y * c01 + l1.y * c11;

			return l0.z * c0 + l1.z * c1;
		}

		SizeVec m_size;
		Vec m_cellSize;
		FlatIndex m_numElem;
		std::unique_ptr<Vec[]> m_E;
		std::unique_ptr<Vec[]> m_prevE;
		std::unique_ptr<Vec[]> m_B;
		std::unique_ptr<Vec[]> m_J;
	};

	template<typename T>
	class CubicIntegrator
	{
	public:
		CubicIntegrator(T _maxCellSize, T _dt = 0, int _numThreads = 1)
			: m_dt(_dt), m_numThreads(_numThreads)
		{

			//const Vec3<T>& cell = _mesh.cellSize();
			//const T maxCellSize = std::max(std::max(cell.x, cell.y), cell.z);
			const T cfl = stableTimeThreshold(_maxCellSize);
			m_maxDt = cfl * static_cast<T>(0.99);
			if (m_dt == 0)
				m_dt = m_maxDt;
			else if (m_dt >= cfl)
				std::cout << "[Warning] The step-size is to large, the simulation will be unstable.\n";
		}

		template<bool TimeInterpol>
		void step(CubicMesh<T, TimeInterpol>& mesh) const
		{
			using Mesh = CubicMesh<T, TimeInterpol>;
			using Index = Mesh::Index;
			using Vec = Mesh::Vec;
			using IndVec = Mesh::IndVec;
			const auto& size = mesh.size();
			const Vec& cellSize = mesh.cellSize();

			// dF = 0
			runMultiThreaded(int64_t(0), size.z, [&] (Index zStart, Index zEnd)
				{
					for (Index iz = zStart; iz < zEnd; ++iz)
						for (Index iy = 0; iy < size.y; ++iy)
							for (Index ix = 0; ix < size.x; ++ix)
							{
								Vec& B = mesh.B(ix, iy, iz);
								const Vec& E = mesh.E(ix, iy, iz);
								B.x += m_dt * ((mesh.E(ix, iy, iz + 1).y - E.y) / cellSize.z
									- (mesh.E(ix, iy + 1, iz).z - E.z) / cellSize.y);
								B.y += m_dt * ((mesh.E(ix + 1, iy, iz).z - E.z) / cellSize.x
									- (mesh.E(ix, iy, iz + 1).x - E.x) / cellSize.z);
								B.z += m_dt * ((mesh.E(ix, iy + 1, iz).x - E.x) / cellSize.y
									- (mesh.E(ix + 1, iy, iz).y - E.y) / cellSize.x);
							}
				}, m_numThreads);

			mesh.swapE();
			
			// d*F = J'
			runMultiThreaded(int64_t(0), size.z, [&](Index zStart, Index zEnd)
				{
					for (Index iz = zStart; iz < zEnd; ++iz)
						for (Index iy = 0; iy < size.y; ++iy)
							for (Index ix = 0; ix < size.x; ++ix)
							{
								constexpr T κE = 1.0;
								const auto idx = mesh.flatIndex({ ix, iy, iz });
								const Vec& H = mesh.B()[idx];
								const Vec& J = mesh.J()[idx];
								Vec& E = mesh.E()[idx];
								const Vec& prevE = mesh.prevE()[idx];
								E.x = κE * prevE.x + m_dt * ((H.z - mesh.H(ix, iy - 1, iz).z) / cellSize.y
									- (H.y - mesh.H(ix, iy, iz - 1).y) / cellSize.z
									- J.x);
								E.y = κE * prevE.y + m_dt * ((H.x - mesh.H(ix, iy, iz - 1).x) / cellSize.z
									- (H.z - mesh.H(ix - 1, iy, iz).z) / cellSize.x
									- J.y);
								E.z = κE * prevE.z + m_dt * ((H.y - mesh.H(ix - 1, iy, iz).y) / cellSize.x
									- (H.x - mesh.H(ix, iy - 1, iz).x) / cellSize.y
									- J.z);
							}
				}, m_numThreads);

		}

		template<bool TimeInterpol>
		void step(CubicMesh<T, TimeInterpol>& mesh, T _dt)
		{
			using Mesh = CubicMesh<T, TimeInterpol>;
			using Index = Mesh::Index;
			using Vec = Mesh::Vec;
			using IndVec = Mesh::IndVec;
			const auto& size = mesh.size();
			const Vec& cellSize = mesh.cellSize();

			if (_dt > m_maxDt)
				std::cout << "[Warning] The step-size is to large, the simulation will be unstable.\n";

			// dF = 0
			runMultiThreaded(int64_t(0), size.z, [&](Index zStart, Index zEnd)
				{
					for (Index iz = zStart; iz < zEnd; ++iz)
						for (Index iy = 0; iy < size.y; ++iy)
							for (Index ix = 0; ix < size.x; ++ix)
							{
								Vec& B = mesh.B(ix, iy, iz);
								const Vec& E = mesh.E(ix, iy, iz);
								B.x += _dt * ((mesh.E(ix, iy, iz + 1).y - E.y) / cellSize.z
									- (mesh.E(ix, iy + 1, iz).z - E.z) / cellSize.y);
								B.y += _dt * ((mesh.E(ix + 1, iy, iz).z - E.z) / cellSize.x
									- (mesh.E(ix, iy, iz + 1).x - E.x) / cellSize.z);
								B.z += _dt * ((mesh.E(ix, iy + 1, iz).x - E.x) / cellSize.y
									- (mesh.E(ix + 1, iy, iz).y - E.y) / cellSize.x);
							}
				}, m_numThreads);

			mesh.swapE();

			// d*F = J'
			runMultiThreaded(int64_t(0), size.z, [&](Index zStart, Index zEnd)
				{
					for (Index iz = zStart; iz < zEnd; ++iz)
						for (Index iy = 0; iy < size.y; ++iy)
							for (Index ix = 0; ix < size.x; ++ix)
							{
								constexpr T κE = 1.0;
								const auto idx = mesh.flatIndex({ ix, iy, iz });
								const Vec& H = mesh.B()[idx];
								const Vec& J = mesh.J()[idx];
								Vec& E = mesh.E()[idx];
								const Vec& prevE = mesh.prevE()[idx];
								E.x = κE * prevE.x + _dt * ((H.z - mesh.H(ix, iy - 1, iz).z) / cellSize.y
									- (H.y - mesh.H(ix, iy, iz - 1).y) / cellSize.z
									- J.x);
								E.y = κE * prevE.y + _dt * ((H.x - mesh.H(ix, iy, iz - 1).x) / cellSize.z
									- (H.z - mesh.H(ix - 1, iy, iz).z) / cellSize.x
									- J.y);
								E.z = κE * prevE.z + _dt * ((H.y - mesh.H(ix - 1, iy, iz).y) / cellSize.x
									- (H.x - mesh.H(ix, iy - 1, iz).x) / cellSize.y
									- J.z);
							}
				}, m_numThreads);

		}

		T dt() const { return m_dt; }
		T stableTimeThreshold(T _maxCellSize) const 
		{
			constexpr T sqrt3 = 1.7320508075688772;
			return _maxCellSize / sqrt3;
		}
	private:
		T m_dt;
		T m_maxDt;
		int m_numThreads;
	};

	using SimpleCubicMesh = CubicMesh<double, true>;
	using SimpleCubicIntegrator = CubicIntegrator<double>;
}
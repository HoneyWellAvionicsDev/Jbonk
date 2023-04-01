#pragma once

#include "Jbonk/Math/Matrix.h"

#include "RigidBody.h"
#include "SystemState.h"

#include <queue>

namespace Enyoo
{
	using Matrix = Jbonk::Math::Matrix<double>;
	using Vector = Jbonk::Math::Matrix<double>;

	struct MatrixRow
	{
		MatrixRow() = default;
		MatrixRow(Matrix::iterator start, Matrix::iterator end, size_t index)
			: row_start(start), row_end(end), column_index(index) {}

		Matrix::iterator row_start;
		Matrix::iterator row_end;
		size_t column_index;
	};

	struct ConstraintSlice
	{
		ConstraintSlice() = default;

		std::vector<MatrixRow> rows;
	};

	struct ConstraintOutput
	{
		Vector C;
		std::vector<Matrix> J;
		std::vector<Matrix> Jdot;
		ConstraintSlice jacobi;
		ConstraintSlice jacobi_dot;
		//vbias
		//limits
		Vector ks;
		Vector kd;
	};

	class Constraint
	{
	public:
		Constraint(uint32_t ConstraintCount, uint32_t BodyCount);
		virtual ~Constraint() = default;

		virtual void Calculate(ConstraintOutput& output, SystemState& state) = 0;

		void SetIndex(size_t index) { m_Index = index; }
		auto GetBodies() const -> std::deque<RigidBody*> { return m_Bodies; }
		auto GetMatrices() -> std::vector<Matrix>& { return m_Matrices; } 
		constexpr size_t GetConstraintCount() const { return m_ConstraintCount; }
		constexpr size_t GetBodyCount() const { return m_BodyCount; }
		constexpr size_t GetIndex() const { return m_Index; }
		RigidBody* GetBody(size_t index) { return m_Bodies[index]; }

	protected:
		std::deque<RigidBody*> m_Bodies;
		std::vector<Matrix> m_Matrices; //TODO: do we really want to store each constraints respective matrices inside the constraint itself?
		size_t m_Index;                 //no lets instead just store a pointer to an object that store
		uint32_t m_BodyCount;
		uint32_t m_ConstraintCount;
	};
}
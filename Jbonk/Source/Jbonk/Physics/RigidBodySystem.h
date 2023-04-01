#pragma once

#include "Jbonk/Math/Matrix.h"

#include "SystemState.h"
#include "KDTree.h"
#include "RigidBody.h"
#include "GravitationalAccelerator.h"
#include "SpringForce.h"
#include "Motor.h"
#include "EulersMethodIntegrator.h"
#include "RungeKutta4thIntegrator.h"
#include "ConjugateGradientMethod.h"
#include "FixedPositionConstraint.h"
#include "LinkConstraint.h"
#include "CircleConstraint.h"

#include <vector>
#include <map>

namespace Enyoo
{
	static Jbonk::Ref<std::vector<BodyPoint>> s_RigidBodyPoints;
	static Jbonk::Ref<KDTree<BodyPoint>> s_RigidBodyKDTree;
	
	struct Matrices
	{
		Matrix SparseJacobian;
		Matrix SparseJacobianDot;
		Matrix Mass, W; // inverse of mass matrix 
		Vector C;
		Vector ks, kd;
		Vector Q;  // applied force global vector
		Vector qdot;
		Vector Qhat;

		Vector lambda;

		Matrix JdotQdot;
		Matrix WQ;
		Matrix JWQ;
	};

	class RigidBodySystem
	{
	public:
		using RigidBodyPtr = Jbonk::Ref<RigidBody>;
		using ConstraintPtr = Jbonk::Ref<Constraint>;
		using ForceGeneratorPtr = Jbonk::Ref<ForceGenerator>;
		using PointerPair = std::pair<Constraint*, RigidBody*>;
		using IndexMap = std::unordered_map<PointerPair, size_t, Utilities::HashPointerFn>;

	public:

		//TODO: need a proper constructor 
		RigidBodySystem() = default;

		void Initialize();

		void Step(double dt, uint32_t steps = 1);

		void AddRigidBody(const RigidBodyPtr& body);
		void RemoveRigidBody(const RigidBodyPtr& body);
		void AddForceGen(const ForceGeneratorPtr& forceGen);
		void RemoveForceGen(const ForceGeneratorPtr& forceGen);
		void AddConstraint(const ConstraintPtr& constraint);
		void RemoveConstraint(const ConstraintPtr& constraint);

		constexpr size_t GetRigidBodyCount() const { return m_RigidBodies.size(); }
		size_t GetForceGenCount() const { return m_ForceGenerators.size(); }
		size_t GetConstraintCount() const { return m_Constraints.size(); }
		size_t GetTotalConstraintCount() const;
		auto GetAllRigidBodies() const -> std::vector<RigidBodyPtr> { return m_RigidBodies; }

		double GetTotalSystemEnergy() const;
		
		static auto NnRadiusIndexSearch(RigidBody* body, double radius) -> std::vector<size_t>;
		static size_t TreeIndexToBodyIndex(size_t index);

	private:
		void PopulateSystemState();
		void PopulateMassMatrices();
		void UpdateForces();
		void ResolveConstraints();
		void ResolveConstraintsO();
		void FlushConstraints();
	private:
		SystemState m_State;
		
		std::vector<RigidBodyPtr> m_RigidBodies;
		std::vector<ForceGeneratorPtr> m_ForceGenerators;
		std::vector<ConstraintPtr> m_Constraints;

		RungeKutta4thIntegrator m_TimeIntegrator;
		ConjugateGradientMethod m_LinearEquationSolver;

		Matrices m_Matrices;
		IndexMap m_ConstaintBodyIndex;
		std::map<size_t, size_t> m_ConstrainedBodies; //could also just been a vector of pairs
	};

	class ConstraintSliceBuilder
	{
	public:
		using PointerPair = RigidBodySystem::PointerPair;
		using IndexMap = RigidBodySystem::IndexMap;

		static auto GenerateConstraintSlice(Constraint* constraint, const IndexMap& map) -> ConstraintSlice
		{
			ConstraintSlice slice;
			auto bodies = constraint->GetBodies();
			auto matrices = constraint->GetMatrices();
			size_t index = 0;

			for (RigidBody* body : bodies)
			{
				PointerPair key( constraint, body );
				auto& matrix = matrices[index];
				auto matrix_mid = matrix.begin() + ( matrix.Size() / 2 );
				//slice.rows.emplace_back(matrix.begin(), matrix_mid, map.at(key));
				//slice.rows.emplace_back(matrix_mid, matrix.end(), map.at(key));
				index++;
			}

			return slice;
		}

		static void ConditionSlice(ConstraintSlice& slice)
		{

		}
	};
}


/*
* TODO:
* 
* 1. Fix fixed bodies not actually being fixed in place (done)
* 2. Correct moment of intertia for each body shape (done)
* 3. Rolling Constraint (done)
* 4. Speed up matrix multiplication
* 5. Add UI for step count (done)
* 6. Support Removing and adding bodies during runtime
* 7. Rigidbody system profiler
* 8. Stop using size_t as indexes (its unsigned)
*/

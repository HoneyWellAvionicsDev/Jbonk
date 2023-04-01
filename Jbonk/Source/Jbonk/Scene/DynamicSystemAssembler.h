#pragma once

#include "Entity.h"

#include "Jbonk/Math/Matrix.h"

#include <map>
#include <queue>

namespace Jbonk
{
	static const float s_LinkPointRadius = 0.1;

	class Scene;

	using Enyoo::RigidBodySystem;
	using Enyoo::LinkConstraint;
	using Enyoo::FixedPositionConstraint;
	using Enyoo::CircleConstraint;
	using Enyoo::Spring;
	using Enyoo::RigidBody;
	using Enyoo::LocalGravity;
	using Enyoo::GravitationalAccelerator;
	using Enyoo::ConstantSpeedMotor;

	using namespace Enyoo::Utilities;

	using vec2 = glm::dvec2;

	struct Joint
	{
		Joint() = default;
		Joint(Entity focus, Entity target, const vec2& worldPoint)
			: Ent1(focus), Ent2(target), WorldPoint(worldPoint) {}

		Entity Ent1;
		Entity Ent2;
		vec2 WorldPoint;
	};

	class DynamicSystemAssembler
	{
	public:
		using LocalPoints = std::pair<vec2, vec2>;
		using EntityView = entt::basic_view<entt::entity, entt::get_t<RigidBodyComponent, LinkPointsComponent>, entt::exclude_t<>, void>;
		using AdjacencyList = std::unordered_multimap<Entity, Entity>;
	public:
		DynamicSystemAssembler(Scene* scene, EntityView entityView);

		auto CreateLinkConstraint(Entity focus, Entity target) -> Ref<LinkConstraint>;
		auto CreateFixedConstraint(Entity focus, Entity target, const vec2& focusLocal) -> Ref<FixedPositionConstraint>;
		[[NODISCARD]] auto CreateSpring(Entity endBody1, Entity endBody2, const vec2& body1Local, const vec2& body2Local) -> Ref<Spring>;

		void GenerateRigidBodies();
		void GenerateConstraints();
		void GenerateForceObjects();

		auto GetMatchingLocals(Entity focusEntity, Entity targetEntity) const -> LocalPoints;
		bool Adjacent(Entity focus, Entity target) const;

	private:
		bool FixedBody(Entity entity) const;
		bool SpringBody(Entity entity) const;
		bool Proximity(const vec2& focusWorld, const vec2& targetWorld) const;

		auto HashJoint(const Joint& joint) const -> size_t;
		auto FindAdjacentFixedBody(Entity entity) const -> Entity;
		auto FindSpringEndBody(const vec2& focusWorld, UUID uuid) const -> Entity;

		void CreateSpringForce(Entity entity);
		void GenerateAdjacencyList(const EntityView& view);
		void HandleFixedBodies(const EntityView& view);
		void HandleLinkedBodies(const EntityView& view);
		void HandleConstraints();

	private:
		Ref<RigidBodySystem> m_RigidBodySystem;
		Scene* m_Scene = nullptr;
		AdjacencyList m_AdjacencyList;
		EntityView m_EntityView;
		std::unordered_set<size_t> m_Joints;
		std::unordered_set<Entity> m_Fixed;
	};
}
/* option 1
*
* option 2
* def joint: the world coord of the two link points from two adjacent bodies
* goal: no duplicate joints between the same two bodies
* everytime we link a focus to a target we will create a hash from focus * target + the joint (be sure to test this hash func so that target * focus + joint creates the same hash
* check that this hash is not in the set and that target body is not a spring
* if not then link both bodies and push the hash into the set
* otherwise move on to the next adjacent body from the focus body
* 
*
* 
* 
* Weird observations / limitations:
* Really massive bodies cause system instability with link and fixed constraints (they will start spinning and system will not converge)
* This also happens upon applying really large forces
* Two bodies between two fixed points that should be linked to each other will not
* If you place a third body between the two linked bodies then the link will create but IF the original two bodies are not at an angle, then the system does not converge
* 
* Things I wanna do with this class:
* 1. Handle springs
* 2. Handle other constraints
*/

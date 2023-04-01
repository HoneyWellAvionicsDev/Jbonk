#include "jbpch.h"

#include "DynamicSystemAssembler.h"
#include "Components.h"
#include "Entity.h"

#include "Jbonk/Physics/RigidBodySystem.h"


namespace Jbonk
{
	DynamicSystemAssembler::DynamicSystemAssembler(Scene* scene, EntityView entityView)
		: m_Scene(scene), m_EntityView(entityView)
	{
		m_RigidBodySystem = m_Scene->GetRigidBodySystem();
		GenerateAdjacencyList(m_EntityView);
	}

	auto DynamicSystemAssembler::CreateLinkConstraint(Entity focus, Entity target) -> Ref<LinkConstraint>
	{
		JB_CORE_ASSERT(focus != target);

		if (SpringBody(focus) || SpringBody(target))
			return nullptr;

		auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto [focusLocal, targetLocal] = GetMatchingLocals(focus, target);
		vec2 world = targetBody->LocalToWorld(targetLocal);

		Joint joint = Joint(focus, target, world);
		size_t hash = HashJoint(joint);

		if (m_Joints.count(hash)) 
			return nullptr;

		Ref<LinkConstraint> linkConstraint = CreateRef<LinkConstraint>();
		linkConstraint->SetFirstBody(focusBody.get());
		linkConstraint->SetSecondBody(targetBody.get());
		linkConstraint->SetFirstBodyLocal(focusLocal);
		linkConstraint->SetSecondBodyLocal(targetLocal);

		m_Scene->GetRigidBodySystem()->AddConstraint(linkConstraint);
		m_Joints.insert(hash);

		return linkConstraint;
	}

	auto DynamicSystemAssembler::CreateFixedConstraint(Entity focus, Entity target, const vec2& focusLocal) -> Ref<FixedPositionConstraint>
	{
		auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto locals = GetMatchingLocals(focus, target);
		vec2 worldJ = targetBody->LocalToWorld(locals.second);

		Joint joint = Joint(focus, target, worldJ);
		size_t hash = HashJoint(joint);

		if (m_Joints.count(hash))
			return nullptr;

		Ref<FixedPositionConstraint> fixedPositionConstraint = CreateRef<FixedPositionConstraint>();
		vec2 world = focusBody->LocalToWorld(focusLocal);
		fixedPositionConstraint->SetBody(focusBody.get());
		fixedPositionConstraint->SetLocalPosition(focusLocal);
		fixedPositionConstraint->SetWorldPosition(world);
		m_RigidBodySystem->AddConstraint(fixedPositionConstraint);
		
		return fixedPositionConstraint;
	}

	auto DynamicSystemAssembler::CreateSpring(Entity endBody1, Entity endBody2, const vec2& body1Local, const vec2& body2Local) -> Ref<Spring>
	{
		auto& focusBody = endBody1.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = endBody2.GetComponent<RigidBodyComponent>().RuntimeBody;

		Ref<Spring> springForceGenerator = CreateRef<Spring>();
		springForceGenerator->SetFirstBody(focusBody.get());
		springForceGenerator->SetSecondBody(targetBody.get());
		springForceGenerator->SetFirstPosition(body1Local);
		springForceGenerator->SetSecondPosition(body2Local);

		return springForceGenerator;
	}

	void DynamicSystemAssembler::GenerateRigidBodies()
	{
		auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();

		for (auto e : view)
		{
			Entity entity = { e, m_Scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rbc = entity.GetComponent<RigidBodyComponent>();

			double Mass = 1.0;
			double MomentInertia = 1.0;

			switch (rbc.Shape)
			{
			case RigidBodyComponent::BodyShape::Rect:
				Mass = transform.Scale.x * transform.Scale.y * rbc.Density;
				MomentInertia = (1.0 / 12.0) * Mass * transform.Scale.x * transform.Scale.x * transform.Scale.y * transform.Scale.y;
				break;
			case RigidBodyComponent::BodyShape::Circle:
				Mass = rbc.Density * glm::pi<double>() * transform.Scale.x * transform.Scale.y * (1.0 / 4.0);
				MomentInertia = (1.0 / 8.0) * Mass * transform.Scale.x * transform.Scale.y;
				break;
			}

			Ref<RigidBody> body = CreateRef<RigidBody>();
			body->Position = { transform.Translation.x, transform.Translation.y };
			body->Theta = transform.Rotation.z;
			body->Velocity = vec2( 0.0 );
			body->AngularVelocity = 0.0;
			body->Mass = Mass;
			body->MomentInertia = MomentInertia; 
			body->Fixed = rbc.Fixed;

			rbc.RuntimeBody = body;
			m_RigidBodySystem->AddRigidBody(body);
		}
	}

	void DynamicSystemAssembler::GenerateConstraints()
	{
		GenerateAdjacencyList(m_EntityView);
		HandleFixedBodies(m_EntityView);
		HandleLinkedBodies(m_EntityView);
		HandleConstraints();
	}

	void DynamicSystemAssembler::GenerateForceObjects()
	{
		auto view = m_Scene->GetAllEntitiesWith<ForceGeneratorComponent>();
		for (auto e : view)
		{
			Entity entity = { e, m_Scene };
			auto& fgc = entity.GetComponent<ForceGeneratorComponent>();
			auto& tc = entity.GetComponent<TransformComponent>();

			switch (fgc.Type)
			{
				case ForceGeneratorComponent::GeneratorType::Gravity:
				{
					Ref<LocalGravity> gravGen = CreateRef<LocalGravity>();
					gravGen->SetGravity(fgc.LocalGravity);
					m_RigidBodySystem->AddForceGen(gravGen);
					fgc.RuntimeGenerator = gravGen;
					break;
				}
				case ForceGeneratorComponent::GeneratorType::GravitationalField:
				{
					Ref<GravitationalAccelerator> gravSphere = CreateRef<GravitationalAccelerator>();
					auto& body = entity.GetComponent<RigidBodyComponent>().RuntimeBody;
					gravSphere->SetSourceBody(body.get());
					gravSphere->SetInfluenceRadius(200.0);
					gravSphere->SetRepulisveForce(fgc.RepulsiveForce);
					m_RigidBodySystem->AddForceGen(gravSphere);
					fgc.RuntimeGenerator = gravSphere;
					break;
				}
				case ForceGeneratorComponent::GeneratorType::Spring:
				{
					CreateSpringForce(entity);
					break;
				}
				case ForceGeneratorComponent::GeneratorType::Motor:
				{
					Ref<ConstantSpeedMotor> motor = CreateRef<ConstantSpeedMotor>();
					Entity target = m_Scene->GetEntity(fgc.targetID);
					if (!target.HasComponent<RigidBodyComponent>())
						break;
					auto& baseBody = entity.GetComponent<RigidBodyComponent>().RuntimeBody;
					auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;
					motor->SetBaseBody(baseBody.get());
					motor->SetTargetBody(targetBody.get());
					motor->SetMaxTorque(static_cast<double>(fgc.MaxTorque));
					motor->SetSpeed(static_cast<double>(fgc.AngularVelocity));
					m_RigidBodySystem->AddForceGen(motor);
					fgc.RuntimeGenerator = motor;
					break;
				}
			}
		}
	}

	bool DynamicSystemAssembler::Adjacent(Entity focus, Entity target) const
	{
		auto range = m_AdjacencyList.equal_range(target);

		for (auto it = range.first; it != range.second; it++)
		{
			if (it->second == focus)
				return true;
		}

		return false;
	}

	auto DynamicSystemAssembler::GetMatchingLocals(Entity focusEntity, Entity targetEntity) const -> LocalPoints
	{
		auto focusRange = m_Scene->GetLinkPoints(focusEntity.GetUUID());
		auto targetRange = m_Scene->GetLinkPoints(targetEntity.GetUUID());
		auto& focusTransform = focusEntity.GetComponent<TransformComponent>();
		auto& targetTransform = targetEntity.GetComponent<TransformComponent>();

		for (auto it = focusRange.first; it != focusRange.second; it++)
		{
			for (auto iter = targetRange.first; iter != targetRange.second; iter++)
			{
				auto focusWorld = LocalToWorld(it->second, focusTransform.Rotation, focusTransform.Translation);
				auto targetWorld = LocalToWorld(iter->second, targetTransform.Rotation, targetTransform.Translation);

				if (Proximity(focusWorld, targetWorld))
					return { it->second, iter->second };
			}
		}

		JB_CORE_ASSERT(false, "How did this happen?");
		return {};
	}

	void DynamicSystemAssembler::CreateSpringForce(Entity entity)
	{
		LinkPointMapIterator firstLP, secondLP;
		auto& fgc = entity.GetComponent<ForceGeneratorComponent>();
		auto& tc = entity.GetComponent<TransformComponent>();
		auto [rangeAdjB, rangeAdjE] = m_AdjacencyList.equal_range(entity);
		auto [rangeB, rangeE] = m_Scene->GetLinkPoints(entity.GetUUID());
		bool foundTop = false, foundBottom = false;

		for (auto it = rangeB; it != rangeE && !(foundTop && foundBottom); it++)
		{
			if (!foundTop)
			{
				firstLP = it;
				foundTop = true;
			}
			else if (!foundBottom)
			{
				secondLP = it;
				foundBottom = true;
			}
		}

		if (!foundTop || !foundBottom)
			return;

		vec2 topWorld = LocalToWorld(firstLP->second, tc.Rotation, tc.Translation);
		vec2 bottomWorld = LocalToWorld(secondLP->second, tc.Rotation, tc.Translation);

		Entity top = FindSpringEndBody(topWorld, firstLP->first);
		Entity bottom = FindSpringEndBody(bottomWorld, secondLP->first);

		if (!top || !bottom)
			return;

		auto [localTop, entLocal] = GetMatchingLocals(top, entity);
		auto [localBottom, entLocal_] = GetMatchingLocals(bottom, entity);

		Ref<Spring> spring = CreateSpring(top, bottom, localTop, localBottom);
		spring->SetSpringConstant(static_cast<double>(fgc.SpringConstant));
		spring->SetDampingValue(static_cast<double>(fgc.SpringDamp));
		spring->SetRestLength(static_cast<double>(fgc.SpringRestLen));

		m_RigidBodySystem->AddForceGen(spring);
		fgc.RuntimeGenerator = spring;
	}

	bool DynamicSystemAssembler::FixedBody(Entity entity) const
	{
		return entity.GetComponent<RigidBodyComponent>().Fixed;
	}

	bool DynamicSystemAssembler::SpringBody(Entity entity) const
	{
		if (!entity.HasComponent<ForceGeneratorComponent>())
			return false;

		return entity.GetComponent<ForceGeneratorComponent>().Type == ForceGeneratorComponent::GeneratorType::Spring;
	}

	bool DynamicSystemAssembler::Proximity(const vec2& focusWorld, const vec2& targetWorld) const
	{
		return glm::abs(targetWorld.x - focusWorld.x) < s_LinkPointRadius && glm::abs(targetWorld.y - focusWorld.y) < s_LinkPointRadius;
	}

	auto DynamicSystemAssembler::HashJoint(const Joint& joint) const -> size_t
	{
		size_t seed = 0;
		Entity ent1{ joint.Ent1.GetHandle(), joint.Ent1.GetScene() };
		Entity ent2{ joint.Ent2.GetHandle(),  joint.Ent2.GetScene() };

		size_t hash1 = static_cast<size_t>(ent1.GetUUID() * ent2.GetUUID());
		size_t hash2 = static_cast<size_t>(joint.WorldPoint.x * joint.WorldPoint.y);
		seed ^= hash1 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2);
		seed ^= hash2 + 0x9e3779b9 + (hash2 << 6) + (hash2 >> 2);
		return seed;
	}

	auto DynamicSystemAssembler::FindAdjacentFixedBody(Entity entity) const -> Entity
	{
		auto [first, last] = m_AdjacencyList.equal_range(entity);
		for (auto it = first; it != last; it++)
		{
			if (FixedBody(it->second))
				return it->second;
		}

		return Entity();
	}

	void DynamicSystemAssembler::GenerateAdjacencyList(const EntityView& view)
	{
		for (auto e : view)
		{
			Entity focus = { e, m_Scene };
			auto& focusTransform = focus.GetComponent<TransformComponent>();
			auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
			auto range = m_Scene->GetLinkPoints(focus.GetUUID());

			for (auto it = range.first; it != range.second; it++)
			{
				vec2 focusWorld =LocalToWorld(it->second, focusTransform.Rotation, focusTransform.Translation);

				auto LinkMap = m_Scene->GetLinkPointMap();
				for (auto iter = LinkMap.begin(); iter != LinkMap.end(); iter++)
				{
					if (iter->first == it->first)
						continue;
				
					Entity target = m_Scene->GetEntity(iter->first);
					auto& targetTransform = target.GetComponent<TransformComponent>();
					vec2 targetWorld = LocalToWorld(iter->second, targetTransform.Rotation, targetTransform.Translation);
				
					if (Proximity(focusWorld, targetWorld))
						m_AdjacencyList.emplace(focus, target);
				
				}
			}
		}
	}
	
	void DynamicSystemAssembler::HandleFixedBodies(const EntityView& view)
	{
		for (auto e : view)
		{
			Entity entity{ e, m_Scene };

			auto& rbc = entity.GetComponent<RigidBodyComponent>();

			if (!rbc.Fixed)
				continue;

			auto range = m_AdjacencyList.equal_range(entity);
			
			for (auto it = range.first; it != range.second; it++)
			{
				JB_CORE_ASSERT(entity != it->second, "this should never happen");
			
				auto [focusLocal, targetLocal] = GetMatchingLocals(it->second, entity);
				auto& body = it->second.GetComponent<RigidBodyComponent>().RuntimeBody;
				vec2 world = body->LocalToWorld(focusLocal);

				Joint joint = Joint( it->second, entity, world);
				size_t hash = HashJoint(joint);
				CreateFixedConstraint(it->second, entity, focusLocal);
				m_Fixed.insert(it->second);
				m_Joints.insert(hash);
			}
		}
	}

	void DynamicSystemAssembler::HandleLinkedBodies(const EntityView& view)
	{
		for (auto e : view)
		{
			Entity entity{ e, m_Scene };

			if (FixedBody(entity)) 
				continue;

			auto [first, second] = m_AdjacencyList.equal_range(entity);
			for (auto it = first; it != second; it++)
			{
				if (FixedBody(it->second))
					continue;

				if (m_Fixed.count(it->second))
					continue;

				CreateLinkConstraint(entity, it->second);
			}
		}
	}

	void DynamicSystemAssembler::HandleConstraints()
	{
		auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent, ConstraintComponent>();
		for (auto e : view)
		{
			Entity focus{ e, m_Scene };
			auto& cc = focus.GetComponent<ConstraintComponent>();
			auto targetID = cc.TargetID;
			Entity target = m_Scene->GetEntity(targetID);
			auto& focusTc = focus.GetComponent<TransformComponent>();
			auto& targetTc = target.GetComponent<TransformComponent>();
			auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
			auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;

			switch (cc.Type)
			{
				case ConstraintComponent::ConstraintType::Static:
				{
					break;
				}
				case ConstraintComponent::ConstraintType::Rolling:
				{
					Ref<CircleConstraint> circle = CreateRef<CircleConstraint>();
					circle->SetBaseBody(targetBody.get());
					circle->SetRollingBody(focusBody.get());
					circle->SetRadius(focusTc.Scale.x / 2.0f);
					circle->SetLocal({ 0.0, targetTc.Scale.y / 2.0f });
					m_RigidBodySystem->AddConstraint(circle);
					cc.RuntimeConstraint = circle;

					break;
				}
				case ConstraintComponent::ConstraintType::RollingFriction:
				{
					break;
				}
				case ConstraintComponent::ConstraintType::FlatSurface:
				{
					break;
				}
			}
		}
	}


	auto DynamicSystemAssembler::FindSpringEndBody(const vec2& focusWorld, UUID uuid) const -> Entity
	{
		auto LinkMap = m_Scene->GetLinkPointMap();
		for (auto it = LinkMap.begin(); it != LinkMap.end(); it++)
		{
			if (it->first == uuid)
				continue;

			Entity target = m_Scene->GetEntity(it->first);
			if (SpringBody(target))
				continue;

			auto& targetTransform = target.GetComponent<TransformComponent>();
			vec2 targetWorld = LocalToWorld(it->second, targetTransform.Rotation, targetTransform.Translation);
			if (Proximity(focusWorld, targetWorld))
				return target;
		}

		return Entity();
	}
}
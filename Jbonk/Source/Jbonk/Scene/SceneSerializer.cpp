#include "jbpch.h"

#include "SceneSerializer.h"
#include "Entity.h"
#include "Components.h"
#include "ScriptableEntity.h"


#include <yaml-cpp/yaml.h>

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

namespace Jbonk
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow; //[0 1 2 3] instead of x:1 y:1
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow; 
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow; 
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static std::string RigidBody2DBodyTypeToString(RigidBody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case RigidBody2DComponent::BodyType::Static:    return "Static";
			case RigidBody2DComponent::BodyType::Dynamic:   return "Dynamic";
			case RigidBody2DComponent::BodyType::Kinematic: return "Kinematic";
		}

		JB_CORE_ASSERT(false, "Unknown body type");
		return "Static";
	}

	static RigidBody2DComponent::BodyType RigidBody2DTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")		return RigidBody2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic")	return RigidBody2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic")	return RigidBody2DComponent::BodyType::Kinematic;

		JB_CORE_ASSERT(false, "Unknown body type");
		return RigidBody2DComponent::BodyType::Static;
	}

	static RigidBodyComponent::BodyShape RigidBodyShapeFromString(const std::string& bodyShapeString)
	{
		if (bodyShapeString == "Rect")		return RigidBodyComponent::BodyShape::Rect;
		if (bodyShapeString == "Circle")	return RigidBodyComponent::BodyShape::Circle;
		//if (bodyTypeString == "")	return RigidBody2DComponent::BodyType::;

		JB_CORE_ASSERT(false, "Unknown body type");
		return RigidBodyComponent::BodyShape::Rect;
	}

	static std::string RigidBodyBodyShapeToString(RigidBodyComponent::BodyShape bodyShape)
	{
		switch (bodyShape)
		{
			case RigidBodyComponent::BodyShape::Rect:     return "Rect";
			case RigidBodyComponent::BodyShape::Circle:   return "Circle";
			//case RigidBodyComponent::BodyShape::: return "";
		}

		JB_CORE_ASSERT(false, "Unknown body type");
		return "Rect";
	}

	static std::string ForceGenTypeToString(ForceGeneratorComponent::GeneratorType genType)
	{
		switch (genType)
		{
			case ForceGeneratorComponent::GeneratorType::Gravity:				return "Gravity";
			case ForceGeneratorComponent::GeneratorType::Spring:				return "Test1";
			case ForceGeneratorComponent::GeneratorType::GravitationalField:	return "Test2";
			case ForceGeneratorComponent::GeneratorType::Motor:					return "Motor";
		}

		JB_CORE_ASSERT(false, "Unknown generator type");
		return "Gravity";
	}

	static ForceGeneratorComponent::GeneratorType ForceGenTypeFromString(const std::string& genTypeStr)
	{
		if (genTypeStr == "Gravity")		return ForceGeneratorComponent::GeneratorType::Gravity;
		if (genTypeStr == "Test1")			return ForceGeneratorComponent::GeneratorType::Spring;
		if (genTypeStr == "Test2")			return ForceGeneratorComponent::GeneratorType::GravitationalField;
		if (genTypeStr == "Motor")			return ForceGeneratorComponent::GeneratorType::Motor;

		JB_CORE_ASSERT(false, "Unknown generator type");
		return ForceGeneratorComponent::GeneratorType::Gravity;
	}

	static std::string ConstraintTypeToString(ConstraintComponent::ConstraintType constType)
	{
		switch (constType)
		{
			case ConstraintComponent::ConstraintType::Static:				return "Static";
			case ConstraintComponent::ConstraintType::FlatSurface:			return "FlatSurface";
			case ConstraintComponent::ConstraintType::Rolling:				return "Rolling";
			case ConstraintComponent::ConstraintType::RollingFriction:		return "RollingFriction";
		}

		JB_CORE_ASSERT(false, "Unknown constraint type");
		return "Static";
	}

	static ConstraintComponent::ConstraintType ConstraintTypeFromString(const std::string& constTypeStr)
	{
		if (constTypeStr == "Static")					return ConstraintComponent::ConstraintType::Static;
		if (constTypeStr == "FlatSurface")				return ConstraintComponent::ConstraintType::FlatSurface;
		if (constTypeStr == "Rolling")					return ConstraintComponent::ConstraintType::Rolling;
		if (constTypeStr == "RollingFriction")			return ConstraintComponent::ConstraintType::RollingFriction;

		JB_CORE_ASSERT(false, "Unknown constraint type");
		return ConstraintComponent::ConstraintType::Static;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		:m_Scene(scene)
	{
	}

	void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		JB_CORE_ASSERT(entity.HasComponent<IDComponent>(), "Entity must have a UUID");

		out << YAML::BeginMap; 
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID(); 

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; 

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; 
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; 

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; 
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; 

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; 
			out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int>(camera.GetProjectionType());
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; 

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; 
		}

		if (entity.HasComponent<LinkPointsComponent>())
		{
			out << YAML::Key << "LinkPointsComponent";
			out << YAML::BeginMap; 
			
			auto& linkPointsComponent = entity.GetComponent<LinkPointsComponent>();

			out << YAML::Key << "Count" << YAML::Value << linkPointsComponent.Count; //this count is so stupid, just use the count method

			auto range = m_Scene->GetLinkPoints(entity.GetUUID());
			int i = 0;

			for (auto it = range.first; it != range.second; it++)
			{
				out << YAML::Key << "LinkPoint" + std::to_string(i) << YAML::Value << it->second;
				i++;
			}

			out << YAML::EndMap; 
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; 

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			out << YAML::Key << "TileFactor" << YAML::Value << spriteRendererComponent.TileFactor;

			if(spriteRendererComponent.Texture)
				out << YAML::Key << "TexturePath" << YAML::Value << spriteRendererComponent.Texture->GetPath();

			out << YAML::EndMap; 
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; 

			auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
			out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

			out << YAML::EndMap; 
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{
			out << YAML::Key << "NativeScriptComponent";
			out << YAML::BeginMap; 

			auto& nativeScriptComponent = entity.GetComponent<NativeScriptComponent>();
			out << YAML::Key << "NativeScriptClass" << YAML::Value << Utils::ScriptTypeToString(nativeScriptComponent.Type);

			out << YAML::EndMap;
		}

		if (entity.HasComponent<RigidBody2DComponent>())
		{
			out << YAML::Key << "RigidBody2DComponent";
			out << YAML::BeginMap;

			auto& rigidBody2DComponent = entity.GetComponent<RigidBody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rigidBody2DComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rigidBody2DComponent.FixedRotation;

			out << YAML::EndMap; 
		}

		if (entity.HasComponent<RigidBodyComponent>())
		{
			out << YAML::Key << "RigidBodyComponent";
			out << YAML::BeginMap;

			auto& rigidBodyComponent = entity.GetComponent<RigidBodyComponent>();
			out << YAML::Key << "Density" << YAML::Value << rigidBodyComponent.Density;
			out << YAML::Key << "Fixed" << YAML::Value << rigidBodyComponent.Fixed;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ForceGeneratorComponent>())
		{
			out << YAML::Key << "ForceGeneratorComponent";
			out << YAML::BeginMap;

			auto& forceGenComponent = entity.GetComponent<ForceGeneratorComponent>();
			out << YAML::Key << "GeneratorType" << YAML::Value << ForceGenTypeToString(forceGenComponent.Type);
			if (forceGenComponent.Type == ForceGeneratorComponent::GeneratorType::Gravity)
				out << YAML::Key << "LocalGravity" << YAML::Value << forceGenComponent.LocalGravity;
			
			out << YAML::Key << "ReplusiveForce" << YAML::Value << forceGenComponent.RepulsiveForce;

			if (forceGenComponent.Type == ForceGeneratorComponent::GeneratorType::Spring)
			{
				out << YAML::Key << "SpringConstant" << YAML::Value << forceGenComponent.SpringConstant;
				out << YAML::Key << "SpringDamp" << YAML::Value << forceGenComponent.SpringDamp;
				out << YAML::Key << "SpringRestLength" << YAML::Value << forceGenComponent.SpringRestLen;
			}

			if (forceGenComponent.Type == ForceGeneratorComponent::GeneratorType::Motor)
			{
				out << YAML::Key << "TargetID" << YAML::Value << forceGenComponent.targetID;
				out << YAML::Key << "MaxTorque" << YAML::Value << forceGenComponent.MaxTorque;
				out << YAML::Key << "AngularVelocity" << YAML::Value << forceGenComponent.AngularVelocity;
			}

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ConstraintComponent>())
		{
			out << YAML::Key << "ConstraintComponent";
			out << YAML::BeginMap;

			auto& cc = entity.GetComponent<ConstraintComponent>();
			out << YAML::Key << "ConstraintType" << YAML::Value << ConstraintTypeToString(cc.Type);
			out << YAML::Key << "TargetID" << YAML::Value << cc.TargetID;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap;

			auto& boxCollider2DComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << boxCollider2DComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << boxCollider2DComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << boxCollider2DComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << boxCollider2DComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << boxCollider2DComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << boxCollider2DComponent.RestitutionThreshold;

			out << YAML::EndMap; 
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; 

			auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; 
		}

		out << YAML::EndMap; //Entity
	}

	void SceneSerializer::Serialize(const std::filesystem::path& filepath)
	{
		const std::filesystem::path fileName = filepath.filename().replace_extension();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << fileName.string();
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { (entt::entity)((m_Scene->m_Registry.size() - 1) - (int)entityID), m_Scene.get() };
			if (!entity)
				return;

			SerializeEntity(out, entity); 
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fileout(filepath); //add flags to create dir if DNE
		fileout << out.c_str();
		JB_CORE_TRACE("Scene saved");
	}

	void SceneSerializer::SerializeRuntime(const std::filesystem::path& filepath)
	{
		JB_CORE_ASSERT(false, "implement");
	}

	bool SceneSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			JB_CORE_ERROR("Failed to load scene file '{0}'\n	{1}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;
		
		std::string sceneName = data["Scene"].as<std::string>();
		JB_CORE_TRACE("Deserializing scene '{0}'", sceneName);
		m_Scene->SetSceneName(sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>(); 

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				JB_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}
				
				auto linkPointsComponent = entity["LinkPointsComponent"];
				if (linkPointsComponent)
				{
					auto& lpc = deserializedEntity.AddComponent<LinkPointsComponent>();
					lpc.Count = linkPointsComponent["Count"].as<int>();

					for (int i = 0; i < lpc.Count; i++)
					{
						const auto& linkPoint = linkPointsComponent["LinkPoint" + std::to_string(i)];
						m_Scene->AddLinkPoint(uuid, linkPoint.as<glm::vec2>());
					}
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
					src.TileFactor = spriteRendererComponent["TileFactor"].as<float>();
					if (spriteRendererComponent["TexturePath"])
						src.Texture = Texture2D::Upload(spriteRendererComponent["TexturePath"].as<std::string>());
				}

				auto circleRendererComponent = entity["CircleRendererComponent"];
				if (circleRendererComponent)
				{
					auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();
					crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
					crc.Thickness = circleRendererComponent["Thickness"].as<float>();
					crc.Fade = circleRendererComponent["Fade"].as<float>();
				}

				auto nativeScriptComponent = entity["NativeScriptComponent"];
				if (nativeScriptComponent)
				{
					auto& nsc = deserializedEntity.AddComponent<NativeScriptComponent>();
					nsc.Type = Utils::ScriptTypeFromString(nativeScriptComponent["NativeScriptClass"].as<std::string>());
					
					switch (nsc.Type)
					{
						case ScriptType::CameraController:		nsc.Bind<CameraController>(); break;
						case ScriptType::Test:					nsc.Bind<Test>();			  break;
					}
				}

				auto rigidBody2DComponent = entity["RigidBody2DComponent"];
				if (rigidBody2DComponent)
				{
					auto& rbc = deserializedEntity.AddComponent<RigidBody2DComponent>();
					rbc.Type = RigidBody2DTypeFromString(rigidBody2DComponent["BodyType"].as<std::string>());
					rbc.FixedRotation = rigidBody2DComponent["FixedRotation"].as<bool>();
				}
				
				auto rigidBodyComponent = entity["RigidBodyComponent"];
				if (rigidBodyComponent)
				{
					auto& rbc = deserializedEntity.AddComponent<RigidBodyComponent>();
					rbc.Density = rigidBodyComponent["Density"].as<float>();
					rbc.Fixed = rigidBodyComponent["Fixed"].as<bool>();
				}

				auto forceGenComponent = entity["ForceGeneratorComponent"];
				if (forceGenComponent)
				{
					auto& fgc = deserializedEntity.AddComponent<ForceGeneratorComponent>();
					fgc.Type = ForceGenTypeFromString(forceGenComponent["GeneratorType"].as<std::string>());
					if (fgc.Type == ForceGeneratorComponent::GeneratorType::Gravity)
						fgc.LocalGravity = forceGenComponent["LocalGravity"].as<glm::vec2>();
					fgc.RepulsiveForce = forceGenComponent["ReplusiveForce"].as<bool>();
					if (fgc.Type == ForceGeneratorComponent::GeneratorType::Spring)
					{
						fgc.SpringConstant = forceGenComponent["SpringConstant"].as<float>();
						fgc.SpringDamp = forceGenComponent["SpringDamp"].as<float>();
						fgc.SpringRestLen = forceGenComponent["SpringRestLength"].as<float>();
					}
					if (fgc.Type == ForceGeneratorComponent::GeneratorType::Motor)
					{
						fgc.targetID = forceGenComponent["TargetID"].as<uint64_t>();
						fgc.MaxTorque = forceGenComponent["MaxTorque"].as<float>();
						fgc.AngularVelocity = forceGenComponent["AngularVelocity"].as<float>();
					}
				}

				auto constraintComponent = entity["ConstraintComponent"];
				if (constraintComponent)
				{
					auto& cc = deserializedEntity.AddComponent<ConstraintComponent>();
					cc.Type = ConstraintTypeFromString(constraintComponent["ConstraintType"].as<std::string>());
					cc.TargetID = constraintComponent["TargetID"].as<uint64_t>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bcc = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					bcc.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
					bcc.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
					bcc.Density = boxCollider2DComponent["Density"].as<float>();
					bcc.Friction = boxCollider2DComponent["Friction"].as<float>();
					bcc.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bcc.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
					cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
					cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
					cc2d.Density = circleCollider2DComponent["Density"].as<float>();
					cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
					cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				if (rigidBodyComponent && circleRendererComponent)
				{
					auto& rbc = deserializedEntity.GetComponent<RigidBodyComponent>();
					rbc.Shape = RigidBodyComponent::BodyShape::Circle;
				}

				if (rigidBodyComponent && spriteRendererComponent)
				{
					auto& rbc = deserializedEntity.GetComponent<RigidBodyComponent>();
					rbc.Shape = RigidBodyComponent::BodyShape::Rect;
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::filesystem::path& filepath)
	{
		JB_CORE_ASSERT(false, "implement");
		return false;
	}
}
